/* -*- mode: c++ -*- */
#ifndef IPACKET
#define IPACKET

#include <iostream>
#include <string>
#include <vector>
#include <math.h>
#include "Word.h"
#include "Event.h"

using namespace std;

  static int NUM_EVENTS = 42; // n of events in a packet
  static int NUM_BYTES = 512; // n of bytes in a packet
  static int NUM_BITS = 8*NUM_BYTES; // n of bits in a packe
  static int NUM_PN_BITS = 48; // n of packet number bits
  static int NUM_WORDS_PER_PACKET = NUM_EVENTS*NUM_WORDS_PER_EVENT; // n of words in a packet
  static int NUM_ZEROS = 10; // n of zeros coded in a packet

class Packet {

 private:
  std::string packet_;
  double packet_number_;
  string board_address_;

public:

  std::vector<Event> events_;

  // default constructor
  Packet()
    {
      packet_="";
      events_.clear();
      packet_number_=0.;
      board_address_="";
    }


  // constructor from binary string
  Packet(string packet)
    {
      if( packet.size() != NUM_BITS ){
	std::clog << " warning: a packet has length " << packet.size() << " instead of " << NUM_BITS << " packet: " << packet << " stored as: " << packet_ << endl;
	exit(0);
      }
      events_.clear();
      packet_number_=0.;
      board_address_="";
      //packet_ = std::string(packet,0,NUM_BITS);
      //packet_ = packet.substr(0,NUM_BITS);
      packet_=packet;
    }

  // constructor from HEX string
  Packet(string packet, bool useless)
    {
      packet_ = packet;
      events_.clear();
      packet_number_=0.;
      board_address_="";
    }

  //! destructor
  virtual ~Packet(){};

  virtual void dump (ostream & a_out         = cout,
		     const string & a_title  = "",
		     const string & a_indent = ""){
      string indent;
      if (! a_indent.empty ()) indent = a_indent;
      if (! a_title.empty ())
	{
	  a_out << indent << a_title << endl;
	}

      clog << " packet number " << packet_number() << " board address " << board_address() << " events: " << endl;
      for(vector<Event>::iterator iw=events_.begin(); iw!= events_.end(); ++iw){
	clog << " event " << iw - events_.begin() << " :"; iw->dump();
      }
      clog << " " << endl;


      return;
  }


  
  void set(string packet)
    {
      packet_ = string(packet,0,NUM_BITS);
      if( packet.size() != NUM_BITS ){
        clog << " warning: a packet has length " << packet.size() << " instead of " << NUM_BITS << " packet: " << packet << " stored as: " << packet_ << endl;
	exit(0);
      }

    }

  std::vector<Event> events(void)
  {
    return events_;
  }


  string packet(void)
  {
    return packet_;
  }

  double packet_number(void)
  {
    return packet_number_;
  }

  string board_address(void)
  {
    return board_address_;
  }

  void packet_to_physical_parameters(void){
    //std::clog << "In packet_to_physical_parameters" << std::endl;
    double dpn = 0; // packet number
    double olddpn = 0;
    
    // read packet number    
    //std::clog << " packet number: " << NUM_PN_BITS;
    char pn[NUM_PN_BITS];
    //char *pn = (char*)malloc(sizeof(char)*(NUM_PN_BITS+1));
    //    string spn="";
    //std::vector<char> pn;
    for(size_t i=0; i<NUM_PN_BITS; i++){
      //std::clog << " i " << i << "; N words/packet " << NUM_WORDS_PER_PACKET << "; N bits/word " << NUM_BITS_PER_WORD << "; entry " << NUM_WORDS_PER_PACKET*NUM_BITS_PER_WORD+i << "; P size " << packet_.size() << std::endl;
      //pn[i]=packet_.at(NUM_WORDS_PER_PACKET*NUM_BITS_PER_WORD+i);
      //std::clog << "packet_ : " << packet_ << std::endl;
      size_t uu=0;
      uu=NUM_WORDS_PER_PACKET*NUM_BITS_PER_WORD+i;
      string same_packet=packet_;
      //char sww = packet_[NUM_WORDS_PER_PACKET*NUM_BITS_PER_WORD+i];
      //      std::clog << "about to be assigned to pn[i]" << endl;
      //std::clog << "pn[ " << i << " ] " << pn[i] << std::endl;
      pn[i]=same_packet[uu];
      //std::clog << "pn[ " << i << " ] " << pn[i] << std::endl;
      //char sww=same_packet[uu];
      //std::clog << "assigned" << sww << endl; fflush(stdout);
      //std::clog << "pn[ " << i << " ] " << pn[i] << std::endl;
      //std::clog << packet_.at(NUM_WORDS_PER_PACKET*NUM_BITS_PER_WORD+i); fflush(stdout);
      //      spn.append(&sww);
      //pn.push_back(sww);
    }
    //std::clog << "Bits are " << pn << std::endl;
    string spn(pn);
    //std::string spn(pn.begin(),pn.end());
    olddpn = dpn;
    dpn = (double)bitset<48>(spn).to_ulong();
    //std::clog << "dpn";
    //std::clog << "    = "; fflush(stdout); clog << dpn << std::endl;
    
    //    double interval = dpn - olddpn;
    //    clog << " interval " << interval << endl;

    packet_number_=dpn;
    //    free(pn);

    // loop on events in the packet
    for(size_t iev=0; iev<NUM_EVENTS; iev++){
      string event=string(packet_,NUM_BITS_PER_EVENT*iev,NUM_BITS_PER_EVENT);
      Event E(event);
      E.event_to_physical_parameters();
      //E.dump();
      events_.push_back(E);
    }

    //std::clog << " Here should come " << NUM_ZEROS << " zeros: " << endl;
    char ze[NUM_ZEROS];
    for(size_t i=0; i<NUM_ZEROS; i++){
      ze[i]=packet_.at(NUM_WORDS_PER_PACKET*NUM_BITS_PER_WORD+NUM_PN_BITS+i);
      //std::clog << packet_.at(NUM_WORDS_PER_PACKET*NUM_BITS_PER_WORD+NUM_PN_BITS+i);
    }
    string sze(ze);
    if( bitset<10>(sze).to_ulong() != 0 ){
      std::clog << " Problem: ten zeros are:" << sze << endl;
      exit(0);
    }
    //std::clog << endl;
    
    //    clog << " board address: ";
    char bad[NUM_BITS_BOARD_ADDRESS];
    for(size_t i=0; i<NUM_BITS_BOARD_ADDRESS; i++){
      bad[i]=packet_.at(NUM_WORDS_PER_PACKET*NUM_BITS_PER_WORD+58+i);
      //      clog << packet_.at(NUM_WORDS_PER_PACKET*NUM_BITS_PER_WORD+58+i);
    }
    board_address_ = string(bad,0,NUM_BITS_BOARD_ADDRESS);

    for(vector<Event>::iterator iev=events_.begin(); iev!=events_.end(); iev++){
      iev->set_packet_number(packet_number());
      iev->set_board_address(board_address());
    }
    
    return;
  }
  
  void Write(ofstream &data_file){

    for(vector<Event>::iterator iev=events_.begin(); iev!= events_.end(); ++iev){

      iev->Write(data_file);
      
    }

    return;
  }

  void Write_raw(ofstream &data_file){

    data_file << packet_;
    data_file << endl;
    
    return;
  }

  double first_time(void){
    return  events().begin()->time("clock");
  }

  double last_time(void){
    return  events().back().time("clock");
  }

  bool overlap(Packet p){

    clog << " checking packets: " << endl;
    dump();
    clog << " and " << endl;
    p.dump();

    clog << " ref times: " << first_time() << " and " << last_time() << endl;
    clog << " proposed times: " << p.first_time() << " and " << p.last_time() << endl;

    if( first_time() < p.last_time() &&
	last_time() > p.first_time() ) {
      clog << " overlap! " << endl;
      return  true;
    }

    clog << " no overlap " << endl;

    return false;
  }

  void get_coincident_events(Packet p, double window, vector<Event> *evs){

    for(vector<Event>::iterator iev=events_.begin(); iev!=events_.end(); iev++){
      for(vector<Event>::iterator ievp=p.events_.begin(); ievp!=p.events_.end(); ievp++){

	if( iev->coincidence(*ievp, window) ){
	  if( !iev->written() ){
	    evs->push_back(*iev);
	    iev->set_written(true);
	  }
	  if( ! ievp->written() ){
	    evs->push_back(*ievp);
	    ievp->set_written(true);
	  }
	}
      }
    }

    return;
  }

  void add_to_list(vector<Event> * all_events){

    for(vector<Event>::iterator iev=events_.begin(); iev!= events_.end(); ++iev){

      all_events->push_back(*iev);
      
    }
    
    return;
  }

 bool goto_next_event(Event *e){
    for(vector<Event>::iterator iev=events_.begin(); iev!=events_.end(); ++iev){
      if( iev->analyzed() ) continue;
      iev->set_analyzed(true);
      *e = *iev;
      return true;
    }

    return false;

  }


};

#endif

