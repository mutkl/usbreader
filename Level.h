/* -*- mode: c++ -*- */
#ifndef ILEVEL
#define ILEVEL

#include <iostream>
#include <string>
#include <vector>
#include <deque>
#include "Event.h"
#include "Packet.h"
#include "Signal.h"
#include <time.h>
#include <unistd.h>

using namespace std;

class Level {

  double t_;
  size_t N_;
  double minimum_t_;
  size_t minimum_index_;
  double next_to_minimum_t_;
  size_t next_to_minimum_index_;
  double window_;
  ifstream in_[LARGE_NUMBER];
  size_t in_number_[LARGE_NUMBER];
  vector<Event>::iterator actual_event_[LARGE_NUMBER];
  vector<Event>::iterator next_event_[LARGE_NUMBER];

public:

  Packet packets_[100];

  // default constructor
  Level()
    {
      t_=0.;
      N_=0;
      window_=0.;
    }


  // constructor
  Level(size_t N, double window)
    {
      if( N < 1 ){
	clog << " error: building level with " << N << " events " << endl;
	exit(0);
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
  virtual ~Level(){

    
  };


  Event get_actual_event(size_t iboard){
    return *actual_event_[iboard];
  }

  Event get_next_event(size_t iboard){
    return *next_event_[iboard];
  }


  virtual void dump (ostream & a_out         = clog,
		     const string & a_title  = "",
		     const string & a_indent = ""){
    {
      string indent;
      if (! a_indent.empty ()) indent = a_indent;
      if (! a_title.empty ())
	{
	  a_out << indent << a_title << endl;
	}

      for(size_t i=0; i<N_; i++){
	clog << "[";
	if( get_actual_event(i).written() ) clog << "*";
	clog << get_actual_event(i).board_address() << " "  << setprecision(12) << get_actual_event(i).time("s");
	for(size_t j=0; j<get_actual_event(i).channels().size(); j++)
	  clog << " " << get_actual_event(i).channels()[j];
	clog << "]";
      }
      clog << endl;
      //      a_out << " t = " << setprecision(ndigits/2.) << t_ << " +- " << setprecision(ndigits/2) << window_ << endl;
      return;
    }
  }

  /*
  void set_from_packets( deque< deque<Packet> > packets, size_t N ) {
    if( N != N_ ){
      clog << " error: setting level from packets, but N " << N << " != N_ " << N_ << endl;
	exit(0);
	
    }

    //    clog << " setting " << N << " packets " << endl;
    for(size_t i=0; i<N_; i++){
      //      clog << " packet " << i ; fflush(stdout);
      //      packets[i].dump();
      if( packets[i].size() == 0 ){
	clog << " error: setting level from packets, but board " << i << " of " << N_ << " has no packets " << endl;
	exit(0);
      }
      if( packets[i][0].events().size() == 0 ){
	clog << " error: setting level from packets, but packet 0 of board " << i << " of " << N_ << " has no events " << endl;
	exit(0);
      }
      events_[i] = packets[i][0].events()[0];
    }

    recalculate_minima();

    return;
  }
  
  void set_event( Event e, size_t index){
    if( index >= N_ ){
      clog << " error: setting event " << index << " maximum: " << N_ << endl;
      exit(0);
    }
    
    events_[index] = e;

    recalculate_minima();
  }
  
  */

  double time(void){
    return t_;
  }

  double window(void){
    return window_;
  }

  Event get_event(size_t index){
    if( index >= N_ ){
      clog << " error: getting event " << index << " maximum: " << N_ << endl;
      exit(0);
    }

    return get_actual_event(index);
     
  }

  double minimum_time(void){
    return minimum_t_;
  }

  size_t minimum_index(void){
    return minimum_index_;
  }

  double next_to_minimum_time(void){
    return next_to_minimum_t_;
  }

  double next_to_minimum_index(void){
    return next_to_minimum_index_;
  }

  void recalculate_minima(void){
    double min=9999999999999999.;
    size_t ind=0;
    double next_to_min=9999999999999999.;
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


  void set_time_to_minimum(void){
    t_ = minimum_t_;
    return ;
  }


  bool select(vector<size_t> * indexes, vector< Signal > signals, bool print=false){

    if( signals.size() == 0 ){
      clog << " problem: no pattern to look for provided " << endl;
      exit(0);
    }

    for(vector<Signal>::iterator is=signals.begin(); is!=signals.end(); ++is){
      if( print )
	clog << " look for signal: "; is->dump();

      size_t iboard = get_board_with_address(is->board_address());
      
      if( board_has_channel(iboard, is->channel(), print) != is->active() ){
	if( print )
	  clog << " ... board " << is->board_address() << " has channel " << is->channel() << " on status different from requirement: " << is->active() << endl;
	return false;
      }
      
      if( !board_has_coincidence(iboard, minimum_time(), print)){
	if( print )
	  clog << " ... board " << is->board_address() << " has no coincidence " << endl;
	return false;
      } 
      if( print )
	clog << "  ... this signal was found " << endl;
      indexes->push_back(iboard);
    }

    if( print )
      clog << "  select !!! " << endl;

    return true;
    
  }


  size_t n_coincidences(vector<size_t> * indexes, bool print=false){
    // n of boards with actual event in coincidence with minimum time
    
    for(size_t iboard=0; iboard<N_; iboard++)
      if( board_has_coincidence(iboard, minimum_time(), print))
	indexes->push_back(iboard);

    
    return indexes->size();
    
  }
  


  string int_to_string(int i){
    char c[20];
    sprintf(c,"%d",i);
    string sval(c);
    return sval;
  }

  string get_file_number(size_t i){
    
    size_t limit = 6;
    string numb=int_to_string((int)i);
    if( numb.size() > limit ){
      clog << " problem: file number " << i << " exceeds limit " << endl;
      exit(0);
    }
    size_t n_extra_zeros=6-numb.size();
    string output="";
    for(size_t i=0; i<n_extra_zeros; i++)
      output += '0';
    return output+numb;
    
  }


  void open_data_files(vector<string> names){
    for(size_t i=0; i<N_; i++){
      string snumb = get_file_number(in_number_[i]);
      string fname = "raw_data_"+names[i]+"_"+snumb+".txt";
      string snumbnext = get_file_number(in_number_[i]+1);
      string fnamenext = "raw_data_"+names[i]+"_"+snumbnext+".txt";
      in_[i].open(fname.c_str());
      ifstream nextfile;
      nextfile.open(fnamenext.c_str());
      if (!in_[i].good() || !nextfile.good() ){
	clog << " selector needs file " << fname << " but this file or the next are not there yet " << endl;
	string asker="./select";
	string commandname="./receive_one";
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
	  clog << asker << " will stop " << endl;
	  exit(0);
	}
      }
      clog << " selector opening file " << fname << endl;

      std::string lineData;
      getline(in_[i], lineData);

      //      clog << " board " << i << " first event: " << lineData << endl;
      Packet p(lineData);
      p.packet_to_physical_parameters();

      packets_[i] = p;
      next_event_[i]=packets_[i].events_.begin();
      next_event_[i]++;
      p.events_[0].set_analyzed(true);
      actual_event_[i]=packets_[i].events_.begin();

    }

    dump();

    return;

  }

  void close(void){
    for(size_t i=0; i<N_; i++){
      in_[i].close();
    }

  }


  size_t get_board_with_address(string address){
    for(size_t i=0; i<N_; i++)
      if( get_actual_event(i).board_address() == address )
	return i;

    clog << " problem: level is requested board address " << address << " which is not found, addresses are: ";
    for(size_t i=0; i<N_; i++)
      clog << " " <<  get_actual_event(i).board_address();
    clog << " quitting " << endl;
    exit(0);


  }


  bool increase_the_time(void){

    return increase_the_time_for_board(minimum_index());
    
  }

  

  void delete_file(string filename){
    string strpath=get_current_dir()+"/";
    char *spath=(char*)strpath.c_str();
    strcat(spath, filename.c_str());
    unlink(spath);
    return;
  }
  
  string get_current_dir(){
    
    char cCurrentPath[100];
    
    if (!getcwd(cCurrentPath, sizeof(cCurrentPath) / sizeof(char)))
      {
	return 0;
      }
    
    cCurrentPath[sizeof(cCurrentPath) - 1] = '\0'; /* not really required */
    
    string answer=cCurrentPath;
    
    return answer;
    
  }


  bool fexists(const char *filename)
  {
    ifstream ifile(filename);
    return ifile;
  }

  bool increase_the_time_for_board(size_t index){
    //std::clog << " index: " << index << std::endl;
    //std::clog << " Increasing time for board " << index << " N_" << N_ << std::endl;

    if( index >= N_ ){
      clog << " Index: " << index << " Input files size: " << N_ << endl;
      exit(0);
    }

    // read next event from new packet, if:
    // 1) actual event is the last of the packet, or
    // 2) last event of the packet is still below reference
    if( (next_event_[index] == packets_[index].events_.end()) ||
	(packets_[index].events_.back().time("s") <= minimum_time()) ){

      std::string lineData;
      getline(in_[index], lineData);

      // if file is over, read new file
      if (!in_[index].good()){
	in_[index].close();

	// delete old files
	if( in_number_[index] > 2 ){
	  string snumbprev = get_file_number(in_number_[index]-2);
	  string fnameprev = "raw_data_"+get_actual_event(index).board_address()+"_"+snumbprev+".txt";
	  if (fexists( fnameprev.c_str() )) {
	    delete_file(fnameprev);
	  }
	}
	in_number_[index] ++;
	string snumb = get_file_number(in_number_[index]);
	string fname = "raw_data_"+get_actual_event(index).board_address()+"_"+snumb+".txt";
	string snumbnext = get_file_number(in_number_[index]+1);
	string fnamenext = "raw_data_"+get_actual_event(index).board_address()+"_"+snumbnext+".txt";

	in_[index].open(fname.c_str());
	ifstream nextfile;
	nextfile.open(fnamenext.c_str());
	if (!in_[index].good() || !nextfile.good() ){
	  std::clog << " Selector needs file " << fname << " but this file or the next are not there yet " << endl;
	  string asker="./select";
	  string commandname="./receive_one";
	  bool running=true;
	  bool good;
	  while(running){
	    running=is_program_running(asker.c_str(), commandname.c_str());
	    in_[index].open(fname.c_str());
	    nextfile.open(fnamenext.c_str());
	    good = in_[index].good() && nextfile.good();
	    if( good ) break;
	  }
	  
	  if( !running ){
	    std::clog << asker << " will stop " << endl; fflush(stdout);
	    exit(0);
	  }

	  std::clog << asker << " will continue " << endl;
	}
	std::clog << " selector opening file " << fname << endl;
    	getline(in_[index], lineData);
	std::clog << " selector opened file " << fname << endl;
      }
      
      Packet p(lineData);
      //Packet* p = new Packet(lineData);
      p.packet_to_physical_parameters();

      packets_[index] = p;
      next_event_[index]=packets_[index].events_.begin();
      next_event_[index]++;
      p.events_[0].set_analyzed(true);
      actual_event_[index]=packets_[index].events_.begin();
    }
    // read next event from same packet (hence the same file)
    else{
      //std::clog << " reading next " << endl;fflush(stdout);
      next_event_[index]++;
      actual_event_[index]++;
      actual_event_[index]->set_analyzed(true);
    }
  
    //    std::clog << " Recalculating " << endl;fflush(stdout);

    recalculate_minima();
    set_time_to_minimum();
    
    return true;
    
  }


  void set_event_number(size_t ievent, size_t eventnumber){
    if( ievent >= N_ ){
      clog << " problem: cannot set event number for event " << ievent << " only " << N_ << " events are expected " << endl;
      exit(0);
    }

    actual_event_[ievent]->set_event_number(eventnumber);

    return;
  }

  void set_event_written(size_t ievent, bool written){
    if( ievent >= N_ ){
      clog << " problem: cannot set event written event " << ievent << " only " << N_ << " events are expected " << endl;
      exit(0);
    }

    actual_event_[ievent]->set_written(written);

    return;
  }


  void set_event_type(size_t ievent, string type){
    if( ievent >= N_ ){
      clog << " problem: cannot set type for event " << ievent << " only " << N_ << " events are expected " << endl;
      exit(0);
    }

    actual_event_[ievent]->set_type(type);

    return;
  }


  bool board_has_channel(size_t iboard, size_t ichannel, bool print=false){

    if( !get_actual_event(iboard).is_channel_on(ichannel ) ){
      if( print ){
	//	clog << " channel " << ichannel << " of board " << iboard << " is not on " << endl;
      }
      
      return false;
    }

    if( print ){
      //      clog << " channel " << ichannel << " of board " << iboard << " is on ";
    }
    return true;

  }


  void wait ( double seconds )
  {
    clock_t endwait;
    endwait = clock () + (clock_t)(seconds * 1e6) ;
    while (clock() < endwait) {}
  }

  bool board_has_coincidence(size_t iboard, double tzero, bool print=false){

    if( fabs(get_actual_event(iboard).time("s") - tzero) >= window_ ){
      if( print ){
	clog << " time " << get_actual_event(iboard).time("s") << " incompatible with reference time " << tzero << endl;
      }
      return false;
    }
    
    if( print ){
      clog << "; time " << get_actual_event(iboard).time("s") << " compatible with reference time " << tzero << endl;
    }
    return true;
  }



  bool is_program_running(string asker, string commandname){
    // Check if process is running via command-line
    
    //    clog << asker << " is checking if " << commandname << " is running " << endl;
    
    char child[100]; 
    char die[100];	
    
    string name1="pidof -x ";
    string name2=" > /dev/null ";
    // Check if process is running via command-line
    
    strcpy (child, name1.c_str());
    strcat (child, commandname.c_str());
    strcat (child, name2.c_str());
    
    // loop to execute kill
    if(0 == system(child)) {
      //      clog << commandname << " is running, so " << asker << " will hold on " << endl; fflush(stdout);
      return true;
    }
    
    return false;
  }
  
  
};

#endif

