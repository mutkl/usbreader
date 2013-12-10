/* -*- mode: c++ -*- */
#ifndef PACKET_H
#define PACKET_H

#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <math.h>
#include "Word.h"
#include "Event.h"
#include <bitset>
#include <stdio.h>

const static int NUM_EVENTS = 42; // n of events in a packet
const static int NUM_BYTES = 512; // n of bytes in a packet
const static int NUM_BITS = 8*NUM_BYTES; // n of bits in a packe
const static int NUM_PN_BITS = 48; // n of packet number bits
//const static int NUM_WORDS_PER_EVENT = 6; // n of words per event         
const static int NUM_WORDS_PER_PACKET = NUM_EVENTS*NUM_WORDS_PER_EVENT;
const static int NUM_ZEROS = 10; // n of zeros coded in a packet

class Packet {
  
private:
  
  std::string packet_;
  unsigned char* packet_hex_;
  double packet_number_;
  std::string board_address_;  
  std::vector<Event> events_;
  
public:
  
  Packet();
  Packet(std::string packet);
  Packet(unsigned char* packet);
  Packet(std::string packet, bool useless);
  ~Packet();
  
  void dump();
  
  void set(std::string packet);
  std::vector<Event> events(){ return events_; }
  std::string packet(){ return packet_; }
  unsigned char* packet_hex(){ return packet_hex_; }
  
  double packet_number(){ return packet_number_; }
  std::string board_address(){ return board_address_; }
  
  void packet_to_physical_parameters();
  void Write(std::ostream &data_file);
  
  void Write_raw(std::ostream &data_file);
  bool overlap(Packet p);
  double first_time(){ return events_.begin()->time("clock"); }

  double last_time(){ return  events_.back().time("clock"); }
  
  void get_coincident_events(Packet p, double window, std::vector<Event> *evs);
  void add_to_list(std::vector<Event> * all_events);
  bool goto_next_event(Event *e);

};
#endif

