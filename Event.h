/* -*- mode: c++ -*- */
#ifndef IEVENT
#define IEVENT

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <math.h>
#include "Word.h"

using namespace std;

static int NUM_WORDS_PER_EVENT = 6; // n of words per event                                                                                       
static int NUM_CHANNELS = 32; // n of channels on board                                                                                           
static int NUM_BITS_PER_EVENT = NUM_WORDS_PER_EVENT*NUM_BITS_PER_WORD; // n of bits per event                                                     
static int NUM_TIMESTAMP_BITS = 57; // number of bits of timestamp                                                                                
static int NUM_TIMESTAMP_BITS_A = 27; // split time stamp in 2 nunmbers (too large to fit in one number)                                          
static int NUM_TIMESTAMP_BITS_B = NUM_TIMESTAMP_BITS - NUM_TIMESTAMP_BITS_A; 
static int NUM_BITS_BOARD_ADDRESS = 6; // n of bits of board address
                    
  static double CLOCK_CYCLE_LENGTH=25e-9;  // 25 ns
  static int ndigits = 20; // precision

class Event {

  string event_;
  std::vector<Word> words_;
  std::vector<size_t> channels_;
  double time_;
  bool written_;
  bool analyzed_;
  double packet_number_;
  string board_address_;
  size_t event_number_;
  string type_;

public:

  // default constructor
  Event()
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
  Event(string event)
    {
      event_ = string(event,0,NUM_BITS_PER_EVENT);
      if( event.size() != NUM_BITS_PER_EVENT ){
        clog << " warning: an event has length " << event.size() << " instead of " << NUM_BITS_PER_EVENT << " event: " << event << " stored as: " << event_ << endl;
	exit(0);
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
  Event(std::vector<size_t> channels, double time, double packet_number, string board_address)
    {
      channels_ = channels;
      time_ = time;
      written_ = false;
      analyzed_ = false;
      packet_number_=packet_number;
      board_address_=board_address;
      type_="FAST";

    }


  // constructor from binary string
  Event(string lineData, bool useless){
    type_="FAST";
    double local_d;
    std::vector<double> row;
    std::stringstream lineStream(lineData);
    
    while(lineStream >> local_d ){
      row.push_back(local_d);
    }
    
    if( !lineStream.eof() ){
      cout << "problem: no eof" << endl;
      exit(0);
    }
    
    if( row.size() < 3 ){
      cout << " problem: row size is " << row.size() << endl;
      exit(0);
    }
    
    double board_address=row[0];
    char c[NUM_BITS_BOARD_ADDRESS];
    sprintf(c,"%g",board_address);
    string ba(c);

    double PN = row[1];
    double time = row[2];
    
    vector<size_t> C;
    
    for(size_t i=3; i<row.size(); i++){
      size_t index = (size_t)(row[i]+0.5);
      C.push_back(index);
    }

    channels_=C;
    time_=time/CLOCK_CYCLE_LENGTH;
    packet_number_=PN;
    board_address_=ba;

  }



  //! destructor
  virtual ~Event(){};

  virtual void dump (ostream & a_out         = cout,
		     const string & a_title  = "",
		     const string & a_indent = ""){
      string indent;
      if (! a_indent.empty ()) indent = a_indent;
      if (! a_title.empty ())
	{
	  a_out << indent << a_title << endl;
	}

      //      if( written() ) clog << "*";
      clog << " evn " << event_number_ << " board " << board_address() ;
      //      for(vector<Word>::iterator iw=words_.begin(); iw!= words_.end(); ++iw){
      //	clog << " word " << iw - words_.begin() << " :"; iw->dump();
      //      }
      clog << " time " << setprecision(ndigits) << time("s") << " channels: ";
      for(vector<size_t>::iterator ic=channels_.begin(); ic!= channels_.end(); ++ic){
	clog << *ic << " ";
      }
      if( type_.compare("SLOW") == 0 )
	clog << " slow ";
      clog <<  " " << endl;

      return;
  }


  
  void set(string event)
    {
      event_ = string(event,0,NUM_BITS_PER_EVENT);
      if( event.size() != NUM_BITS_PER_EVENT ){
        clog << " warning: an event has length " << event.size() << " instead of " << NUM_BITS_PER_EVENT << " event: " << event << " stored as: " << event_ << endl;
	exit(0);
      }

    }

  void set_words(std::vector<Word> words){
    for(vector<Word>::iterator iw=words.begin(); iw!= words.end(); ++iw){
      if( words_.size() < NUM_WORDS_PER_EVENT )
	words_.push_back(*iw);
    } 
    if( words.size() != NUM_WORDS_PER_EVENT ){
      clog << " warning: building an event with " << words.size() << " words instead of " << NUM_WORDS_PER_EVENT << " words, keeping only first w\
ords " << endl;
      exit(0);
    }
  }

  void set_written(bool flag){
    written_=flag;
  }

  void set_analyzed(bool flag){
    analyzed_=flag;
  }

  void set_type(string flag){
    type_=flag;
  }


  void set_packet_number(double pn){
    packet_number_=pn;
  }


  void set_board_address(string b){
    board_address_=b;
  }


  std::vector<Word> words(void)
  {
    return words_;
  }


  string event(void)
  {
    return event_;
  }


  bool written(void){
    return written_;
  }

  bool analyzed(void){
    return analyzed_;
  }

  string type(void){
    return type_;
  }

  double packet_number(void){
    return packet_number_;
  }

  string board_address(void){
    return board_address_;
  }

  bool time_stamp(size_t iword, size_t ichar){
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

  bool get_channel_number(size_t iword, size_t ichar, size_t * ichannel, char a){
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




  void event_to_physical_parameters(void){

    channels_.clear();

    char ts[NUM_TIMESTAMP_BITS], tsa[NUM_TIMESTAMP_BITS_A], tsb[NUM_TIMESTAMP_BITS_B]; 
    size_t ts_index=0;
    size_t ichannel=0;
    // loop on words in the event
    for(size_t iw=0; iw<NUM_WORDS_PER_EVENT; iw++){

      Word w(string(event_,NUM_BITS_PER_WORD*iw,NUM_BITS_PER_WORD));
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
	    clog << " problem: ts_index: " << ts_index << endl;
	    exit(0);
	  }
	}
	
	
	//	clog << out;
      } // read all word                                                            

                                                                
      //      clog << endl;
    } // read all words in the event                
    
    //    clog << endl;
    string sts(ts);
    string stsa(tsa);
    string stsb(tsb);
    double ta = (double)bitset<30>(stsa).to_ulong()*pow(2.,NUM_TIMESTAMP_BITS_A);
    time_ = ta+(double)bitset<27>(stsb).to_ulong();
    
    return;
  }    


  vector<size_t> channels(void){
    return channels_;
  }

  double time(string flag="s"){
    if( flag=="s" )
      return time_*CLOCK_CYCLE_LENGTH;  // seconds
    return time_; // clock cycles
  }


  size_t event_number(){
    return event_number_;
  }

  void set_event_number(size_t i){
    event_number_=i;
    return;
  }

  
  void Write(ofstream &data_file){

    //    clog << " time stamp: " << setprecision(ndigits) << time("clock") << " cycles = " << setprecision(ndigits) << time("s") << " s on channels: ";
      
    data_file << event_number_ << " ";
    data_file << board_address_ << " ";
    data_file << (int)(packet_number() + 0.5) << " ";
    if( type_.compare("SLOW") == 0 )
      data_file << "1" << " ";
    else
      data_file << "0" << " ";
    data_file << setprecision(ndigits) << time("s")  << " ";
    vector<size_t> cs = channels();
      
    for(vector<size_t>::iterator ic=cs.begin(); ic!=cs.end(); ++ic){
      data_file << *ic << " ";
      //      clog << *ic << " ";
    }
    data_file <<  "\n";
      
    //    clog << endl;
 
    
    return;
  }

  bool coincidence(Event e, double time_window){
    if( fabs(time("s") - e.time("s")) < time_window ){
      //      clog << " coincident events: "; dump(); clog << " and " ; e.dump();
      //      clog << " times: " << time("s") << " and " << e.time("s") ;
      return true;
    }
    return false;
  }


  bool is_channel_on(size_t i){
    return (std::find(channels_.begin(), channels_.end(), i) != channels_.end());    
  }


  bool operator<(const Event& e) const{
    return (time_ < e.time_ );
  }

};

#endif

