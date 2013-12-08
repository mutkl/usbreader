#include "Event.h"

Event::Event()
{
  event_="";
  words_.clear();
  channels_.clear();
  time_ = 0.;
  written_ = false;
  analyzed_ = false;
  packet_number_=0.;
  board_address_="";
  event_number_=0;
  type_="FAST";
}


// constructor
Event::Event(std::string event)
{
  event_ = std::string(event,0,NUM_BITS_PER_EVENT);
  if( event.size() != NUM_BITS_PER_EVENT ){
    std::cout << " warning: an event has length " << event.size() << " instead of " << NUM_BITS_PER_EVENT << " event: " << event << " stored as: " << event_ << std::endl;
    std::exit(0);
  }

  words_.clear();
  channels_.clear();
  time_ = 0.;
  written_ = false;
  analyzed_ = false;
  packet_number_=0.;
  board_address_="";
  event_number_=0;
  type_="FAST";

}

// constructor
Event::Event(std::vector<size_t> channels, double time, double packet_number, std::string board_address)
{
  channels_ = channels;
  time_ = time;
  written_ = false;
  analyzed_ = false;
  packet_number_=packet_number;
  board_address_=board_address;
  type_="FAST";

}


// constructor from binary std::string
Event::Event(std::string lineData, bool useless){
  type_="FAST";
  double local_d;
  std::vector<double> row;
  std::stringstream lineStream(lineData); 
  //std::stringstream lineStream; 
  //lineStream << lineData;
  
  while(lineStream >> local_d ){
    row.push_back(local_d);
  }
    
  if( !lineStream.eof() ){
    std::cout << "problem: no eof" << std::endl;
    std::exit(0);
  }
    
  if( row.size() < 3 ){
    std::cout << " problem: row size is " << row.size() << std::endl;
    std::exit(0);
  }
    
  double board_address=row[0];
  char c[NUM_BITS_BOARD_ADDRESS];
  sprintf(c,"%g",board_address);
  std::string ba(c);

  double PN = row[1];
  double time = row[2];
    
  std::vector<size_t> C;
    
  for(size_t i=3; i<row.size(); i++){
    size_t index = (size_t)(row[i]+0.5);
    C.push_back(index);
  }

  channels_=C;
  time_=time/CLOCK_CYCLE_LENGTH;
  packet_number_=PN;
  board_address_=ba;

}


Event::~Event(){}


void Event::dump (){
  
  if( type_.compare("SLOW") == 0 )
    std::cout << " slow ";
  
}



void Event::set(std::string event)
{
  event_ = std::string(event,0,NUM_BITS_PER_EVENT);
  if( event.size() != NUM_BITS_PER_EVENT ){
    std::cout << " warning: an event has length " << event.size() << " instead of " << NUM_BITS_PER_EVENT << " event: " << event << " stored as: " << event_ << std::endl;
    std::exit(0);
  }
      
}

void Event::set_words(std::vector<Word> words){
  for(std::vector<Word>::iterator iw=words.begin(); iw!= words.end(); ++iw){
    if( words_.size() < NUM_WORDS_PER_EVENT )
      words_.push_back(*iw);
  } 
  if( words.size() != NUM_WORDS_PER_EVENT ){
    std::cout << " warning: building an event with " << words.size() << " words instead of " << NUM_WORDS_PER_EVENT << " words, keeping only first w\
ords " << std::endl;
    std::exit(0);
  }
}


bool Event::time_stamp(size_t iword, size_t ichar){
  // is character ichar of word iword a binary digit of the time stamp?

  switch(iword){
  case 0:{
    return false;
  }
  case 1:{
    return false;
  }
  case 2:{
    if( ichar <= 3 ) return false ;
    return true;
  }
  case 3:{
    if( ichar == 0 ) return false ;
    return true;
  }
  case 4:{
    if( ichar == 0 ) return false ;
    return true;
  }
  case 5:{
    if( ichar == 0 ) return false ;
    return true;
  }
  default:
    return false ;
  }

  return false ;
  
}

bool Event::get_channel_number(size_t iword, size_t ichar, size_t * ichannel, char a){
  // is character ichar of word iword a binary digit of an active channel?
  // if yes, store in *ichannel the channel number
  
  if( a=='0' ) // even if it's a channel, it's off
    return false;
  
  // if it's a channel, it's on
  switch(iword){
  case 0:{
    if( ichar==0 ) return false;
    if( ichar==1) return false;
    *ichannel = 33-ichar;  // 31 30 ... 18
    return true;
  }
  case 1:{
    if( ichar==0 ) return false ;
    *ichannel = 18-ichar; // 17 16 ... 3
    return true;
  }
  case 2:{
    if( ichar==0 ) return false ;
    if( ichar>3 ) return false ;
    *ichannel = 3-ichar; // 2 1 0
    return true;
  }
  default:
    return false ;
  }
  
  return false ;
  
}




void Event::event_to_physical_parameters(){

  channels_.clear();

  char ts[NUM_TIMESTAMP_BITS], tsa[NUM_TIMESTAMP_BITS_A], tsb[NUM_TIMESTAMP_BITS_B]; 
  size_t ts_index=0;
  size_t ichannel=0;
  // loop on words in the event
  for(size_t iw=0; iw<NUM_WORDS_PER_EVENT; iw++){

    Word w(std::string(event_,NUM_BITS_PER_WORD*iw,NUM_BITS_PER_WORD));
    words_.push_back(w);
	      
    for(size_t ibit=0; ibit<NUM_BITS_PER_WORD; ibit++){
      char out=event_.at(NUM_BITS_PER_WORD*iw+ibit);
	

      if( get_channel_number(iw, ibit, &ichannel, out) ){
	channels_.push_back(ichannel);
      }

      if( time_stamp(iw, ibit) ){
	ts[ts_index] = out;
	  
	if( ts_index < NUM_TIMESTAMP_BITS_B )
	  tsa[ts_index] = out;
	else
	  tsb[ts_index - NUM_TIMESTAMP_BITS_B] = out;
	ts_index++;
	  
	if( ts_index > NUM_TIMESTAMP_BITS ){
	  std::cout << " problem: ts_index: " << ts_index << std::endl;
	  std::exit(0);
	}
      }
	
	
      //	std::cout << out;
    } // read all word                                                            

                                                                
      //      std::cout << std::endl;
  } // read all words in the event                
    
    //    std::cout << std::endl;
  std::string sts(ts);
  std::string stsa(tsa);
  std::string stsb(tsb);
  double ta = (double)std::bitset<30>(stsa).to_ulong()*pow(2.,NUM_TIMESTAMP_BITS_A);
  time_ = ta+(double)std::bitset<27>(stsb).to_ulong();
    
  return;
}    



double Event::time(std::string flag){
  if( flag=="s" )
    return time_*CLOCK_CYCLE_LENGTH;  // seconds
  return time_; // clock cycles
}

void Event::set_event_number(size_t i){
  event_number_=i;
  return;
}

void Event::Write(std::ostream & data_file){
    
  //    std::cout << " time stamp: " << setprecision(ndigits) << time("clock") << " cycles = " << setprecision(ndigits) << time("s") << " s on channels: ";
      
  data_file << event_number_ << " ";
  data_file << board_address_ << " ";
  data_file << (int)(packet_number() + 0.5) << " ";
  if( type_.compare("SLOW") == 0 )
    data_file << "1" << " ";
  else
    data_file << "0" << " ";
  data_file << std::setprecision(ndigits) << time("s")  << " ";
  std::vector<size_t> cs = channels();
      
  for(std::vector<size_t>::iterator ic=cs.begin(); ic!=cs.end(); ++ic){
    data_file << *ic << " ";
    //      std::cout << *ic << " ";
  }
  data_file <<  "\n";
      
  //    std::cout << std::endl;
 
    
  return;
}

bool Event::coincidence(Event e, double time_window){
  if( fabs(time("s") - e.time("s")) < time_window ){
    //      std::cout << " coincident events: "; dump(); std::cout << " and " ; e.dump();
    //      std::cout << " times: " << time("s") << " and " << e.time("s") ;
    return true;
  }
  return false;
}


