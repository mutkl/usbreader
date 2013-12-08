#include "Level.h"

Level::Level()
{
  t_=0.;
  N_=0;
  window_=0.;
}
  
  
// constructor
Level::Level(size_t N, double window)
{
  if( N < 1 ){
    std::cout << " error: building level with " << N << " events " << std::endl;
    std::exit(0);
  }
  N_ = N;
  window_=window;
  for(size_t i=0; i<N_; i++){
    // skip first file for each board
    // containing leftovers from previous run
    in_number_[i]=1;
  }
}


//! destructor

Level::~Level(){}


void Level::dump(){
 
  for(size_t i=0; i<N_; i++){
    std::cout << "[";
    if( get_actual_event(i).written() ) std::cout << "*";
    std::cout << get_actual_event(i).board_address() << " "  << std::setprecision(12) << get_actual_event(i).time("s");
    for(size_t j=0; j<get_actual_event(i).channels().size(); j++)
      std::cout << " " << get_actual_event(i).channels()[j];
    std::cout << "]";
  }
  std::cout << std::endl;
  //      a_out << " t = " << std::setprecision(ndigits/2.) << t_ << " +- " << std::setprecision(ndigits/2) << window_ << std::endl;
  return;
}

Event Level::get_event(size_t index){
  
  if( index >= N_ ){
    std::cout << " error: getting event " << index << " maximum: " << N_ << std::endl;
    std::exit(0);
  }
  
  return get_actual_event(index);
  
}


void Level::recalculate_minima(){
  double min=FLT_MAX;
  size_t ind=0;
  double next_to_min=FLT_MAX;
  size_t next_to_ind=0;
    
  for(size_t i=0; i<N_; i++){
    if( get_actual_event(i).time("s") < min ){
      next_to_min = min;
      next_to_ind=ind;
      min = get_actual_event(i).time("s");
      ind=i;
    }
    else if( get_actual_event(i).time("s") < next_to_min ){
      next_to_min = get_actual_event(i).time("s");
      next_to_ind=i;
    }
  }

  minimum_t_=min;
  minimum_index_=ind;
  next_to_minimum_t_=next_to_min;
  next_to_minimum_index_=next_to_ind;

  return;

}


bool Level::select(std::vector<size_t> * indexes, std::vector< Signal > signals, bool print){

  if( signals.size() == 0 ){
    std::cout << " problem: no pattern to look for provided " << std::endl;
    std::exit(0);
  }

  for(std::vector<Signal>::iterator is=signals.begin(); is!=signals.end(); ++is){
    if( print )
      std::cout << " look for signal: "; is->dump();

    size_t iboard = get_board_with_address(is->board_address());
      
    if( board_has_channel(iboard, is->channel(), print) != is->active() ){
      if( print )
	std::cout << " ... board " << is->board_address() << " has channel " << is->channel() << " on status different from requirement: " << is->active() << std::endl;
      return false;
    }
      
    if( !board_has_coincidence(iboard, minimum_time(), print)){
      if( print )
	std::cout << " ... board " << is->board_address() << " has no coincidence " << std::endl;
      return false;
    } 
    if( print )
      std::cout << "  ... this signal was found " << std::endl;
    indexes->push_back(iboard);
  }

  if( print )
    std::cout << "  select !!! " << std::endl;

  return true;
    
}

size_t Level::n_coincidences(std::vector<size_t> * indexes, bool print){
  // n of boards with actual event in coincidence with minimum time
    
  indexes->clear();

  for(size_t iboard=0; iboard<N_; ++iboard){
    if( board_has_coincidence(iboard, minimum_time(), print)){
      indexes->push_back(iboard);
    }
  }
    
  return indexes->size();
    
}
  

std::string Level::int_to_string(int i){
  char c[20];
  sprintf(c,"%d",i);
  std::string sval(c);
  return sval;
}


std::string Level::get_file_number(size_t i){
    
  size_t limit = 6;
  std::string numb=int_to_string((int)i);
  if( numb.size() > limit ){
    std::cout << " problem: file number " << i << " exceeds limit " << std::endl;
    std::exit(0);
  }
  size_t n_extra_zeros=6-numb.size();
  std::string output="";
  for(size_t i=0; i<n_extra_zeros; i++)
    output += '0';
  return output+numb;
    
}


void Level::assign_packet(size_t i, Packet p){
  stored_packets_[i] = p;
  stored_packets_[i].packet_to_physical_parameters();
}

  
void Level::init(Packet* stored_packets,size_t n){

  for(size_t i =0 ;i<n;++i){
    assign_packet(i,stored_packets[i]);
  }
  
  for(size_t i = 0 ; i<N_; ++i){
    actual_event_[i] = stored_packets_[i].events().begin();
  }
  
  recalculate_minima();
  set_time_to_minimum();
}

void Level::get_earliest_board(){
  //find board with the lowest time stamp
    
  double time_min = FLT_MAX;
  double time_local;
  size_t imin;
    
  for(size_t i = 0 ; i < N_; ++i){

    time_local = actual_event_[i]->time("s");
      
    if(time_local < time_min){
      time_min = time_local;
      imin = i;
    }
      
  }
  
  minimum_index_ = imin;
  minimum_t_     = time_min;
  
  
}


void Level::open_data_files(std::vector<std::string> names){
  for(size_t i=0; i<N_; i++){
    std::string snumb = get_file_number(in_number_[i]);
    std::string fname = "raw_data_"+names[i]+"_"+snumb+".txt";
    std::string snumbnext = get_file_number(in_number_[i]+1);
    std::string fnamenext = "raw_data_"+names[i]+"_"+snumbnext+".txt";
    in_[i].open(fname.c_str());
    std::ifstream nextfile;
    nextfile.open(fnamenext.c_str());
    if (!in_[i].good() || !nextfile.good() ){
      std::cout << " selector needs file " << fname << " but this file or the next are not there yet " << std::endl;
      std::string asker="./select";
      std::string commandname="./receive_one";
      bool running=true;
      bool good;
      while(running){
	running=is_program_running(asker.c_str(), commandname.c_str());
	in_[i].open(fname.c_str());
	nextfile.open(fnamenext.c_str());
	good = in_[i].good() && nextfile.good();
	if( good ) break;
      }

      if( !running ){
	std::cout << asker << " will stop " << std::endl;
	std::exit(0);
      }
    }
    //std::cout << " selector opening file " << fname << std::endl;

    std::string lineData;
    getline(in_[i], lineData);

    //std::cout << " board " << i << " first event: " << lineData << std::endl;
    Packet p(lineData);
    p.packet_to_physical_parameters();

    packets_[i] = p;
    next_event_[i]=packets_[i].events().begin();
    next_event_[i]++;
    p.events()[0].set_analyzed(true);
    actual_event_[i]=packets_[i].events().begin();

  }

  dump();

  return;

}


void Level::close(){
  for(size_t i=0; i<N_; i++){
    in_[i].close();
  }
    
}


size_t Level::get_board_with_address(std::string address){
  for(size_t i=0; i<N_; i++)
    if( get_actual_event(i).board_address() == address )
      return i;

  std::cout << " problem: level is requested board address " << address << " which is not found, addresses are: ";
  for(size_t i=0; i<N_; i++)
    std::cout << " " <<  get_actual_event(i).board_address();
  std::cout << " quitting " << std::endl;
  std::exit(0);


}

void Level::delete_file(std::string filename){
  std::string strpath=get_current_dir()+"/";
  char *spath=(char*)strpath.c_str();
  std::strcat(spath, filename.c_str());
  unlink(spath);
  return;
}

std::string Level::get_current_dir(){
    
  char cCurrentPath[100];
    
  if (!getcwd(cCurrentPath, sizeof(cCurrentPath) / sizeof(char)))
    {
      return 0;
    }
    
  cCurrentPath[sizeof(cCurrentPath) - 1] = '\0'; /* not really required */
    
  std::string answer=cCurrentPath;
    
  return answer;
    
}

bool Level::fexists(const char *filename)
{
  std::ifstream ifile(filename);
  return ifile;
}


bool Level::increase_the_time_for_board(size_t index){

  if( index >= N_ ){
    std::cout << " Index: " << index << " Input files size: " << N_ << std::endl;
    std::exit(0);
  }

  std::vector<Event>::iterator next_event = actual_event_[index];
  next_event++;

  if(next_event == stored_packets_[index].events().end()){
      
    return false;

  }
  actual_event_[index] = next_event;
    
  recalculate_minima();
  set_time_to_minimum();
    
  return true;
    
}


void Level::set_event_number(size_t ievent, size_t eventnumber){
  if( ievent >= N_ ){
    std::cout << " problem: cannot set event number for event " << ievent << " only " << N_ << " events are expected " << std::endl;
    std::exit(0);
  }

  actual_event_[ievent]->set_event_number(eventnumber);

  return;
}



void Level::set_event_written(size_t ievent, bool written){
  if( ievent >= N_ ){
    std::cout << " problem: cannot set event written event " << ievent << " only " << N_ << " events are expected " << std::endl;
    std::exit(0);
  }

  actual_event_[ievent]->set_written(written);

  return;
}


void Level::set_event_type(size_t ievent, std::string type){
  if( ievent >= N_ ){
    std::cout << " problem: cannot set type for event " << ievent << " only " << N_ << " events are expected " << std::endl;
    std::exit(0);
  }

  actual_event_[ievent]->set_type(type);

  return;
}


bool Level::board_has_channel(size_t iboard, size_t ichannel, bool print){

  if( !get_actual_event(iboard).is_channel_on(ichannel ) ){
    if( print ){
      //	std::cout << " channel " << ichannel << " of board " << iboard << " is not on " << std::endl;
    }
      
    return false;
  }

  if( print ){
    //      std::cout << " channel " << ichannel << " of board " << iboard << " is on ";
  }
  return true;

}

void Level::wait ( double seconds )
{
  clock_t endwait;
  endwait = clock () + (clock_t)(seconds * 1e6) ;
  while (clock() < endwait) {}
}

bool Level::board_has_coincidence(size_t iboard, double tzero, bool print){
    
  if( fabs(get_actual_event(iboard).time("s") - tzero) >= window_ ){
    if( print ){
   
	
      std::cout << std::setprecision(20) << " time " << get_actual_event(iboard).time("s") << " incompatible with reference time " << std::setprecision(20) <<  tzero << std::endl;
    }
    return false;
  }
    
  if( print ){
    std::cout<< std::setprecision(20)  << "; time " << get_actual_event(iboard).time("s") << " compatible with reference time " <<std::setprecision(20) <<  tzero << std::endl;
  }
  return true;
}

bool Level::is_program_running(std::string asker, std::string commandname){
  // Check if process is running via command-line
    
  //    std::cout << asker << " is checking if " << commandname << " is running " << std::endl;
    
  char child[100]; 
  char die[100];	
    
  std::string name1="pidof -x ";
  std::string name2=" > /dev/null ";
  // Check if process is running via command-line
    
  strcpy (child, name1.c_str());
  std::strcat (child, commandname.c_str());
  std::strcat (child, name2.c_str());
    
  // loop to execute kill
  if(0 == system(child)) {
    //      std::cout << commandname << " is running, so " << asker << " will hold on " << std::endl; fflush(stdout);
    return true;
  }
    
  return false;
}

