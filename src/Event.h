/* -*- mode: c++ -*- */
#ifndef EVENT_H
#define EVENT_H
#include <fstream>
#include <bitset>
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <math.h>
#include "Word.h"
#include <stdio.h>
#include <sstream>
#include <iomanip>

const static int NUM_WORDS_PER_EVENT = 6; // n of words per event
const static int NUM_CHANNELS = 32; // n of channels on board 
const static int NUM_BITS_PER_EVENT = NUM_WORDS_PER_EVENT*NUM_BITS_PER_WORD; // n of bits per event   
//const static int NUM_BITS_PER_WORD  = 16;
const static int NUM_TIMESTAMP_BITS = 57; // number of bits of timestamp 
const static int NUM_TIMESTAMP_BITS_A = 27; // split time stamp in 2 nunmbers (too large to fit in one number)
const static int NUM_TIMESTAMP_BITS_B = NUM_TIMESTAMP_BITS - NUM_TIMESTAMP_BITS_A; 
const static int NUM_BITS_BOARD_ADDRESS = 6; // n of bits of board address
const static double CLOCK_CYCLE_LENGTH=25e-9;  // 25 ns
const static int ndigits = 20; // precision

class Event {

private:
  
  
  std::string event_;
  std::vector<Word> words_;
  std::vector<size_t> channels_;
  double time_;
  bool written_;
  bool analyzed_;
  double packet_number_;
  std::string board_address_;
  size_t event_number_;
  std::string type_;

public:
  
  Event();
  Event(std::string event);
  Event(std::vector<size_t> channels, double time, double packet_number, std::string board_address);
  Event(std::string lineData, bool useless);
  ~Event();
  
  void dump();
  
  void set(std::string event);
  void set_words(std::vector<Word> words);
  void set_written(bool flag){  written_=flag; }
  
  void set_analyzed(bool flag){ analyzed_=flag; }
  
  void set_type(std::string flag){ type_=flag;}
  void set_packet_number(double pn){ packet_number_=pn; }
  void set_board_address(std::string b){ board_address_=b; }

  std::vector<Word> words(){ return words_; }

  std::string event() { return event_; }

  bool written(){ return written_; }

  bool analyzed(){ return analyzed_; }

  std::string type(){ return type_; }

  double packet_number(){ return packet_number_; }

  std::string board_address(){ return board_address_; }
  
  bool time_stamp(size_t iword, size_t ichar);

  bool get_channel_number(size_t iword, size_t ichar, size_t * ichannel, char a);

  void event_to_physical_parameters();

  std::vector<size_t> channels(){ return channels_; }
  
  size_t event_number(){ return event_number_; }
  
  double time(std::string flag="s");
  
  void set_event_number(size_t i);
  
  void Write(std::ostream & data_file);  
  
  bool coincidence(Event e, double time_window);
  
  bool is_channel_on(size_t i){   return (std::find(channels_.begin(), channels_.end(), i) != channels_.end());   }
  
  
  bool operator<(const Event& e) const{    return (time_ < e.time_ ); }
  
};

#endif

