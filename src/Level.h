/* -*- mode: c++ -*- */
#ifndef LEVEL_H
#define LEVEL_H

#include <iostream>
#include <stdio.h>
#include <cstring>
#include <cstdio>
#include <string>
#include <vector>
#include <deque>
#include "Event.h"
#include "Packet.h"
#include "Signal.h"
#include <time.h>
#include <unistd.h>
#include <float.h>
#include <algorithm>


class Level {

private:
  
  
  double t_;
  size_t N_;
  double minimum_t_;
  size_t minimum_index_;
  double next_to_minimum_t_;
  size_t next_to_minimum_index_;
  double window_;
  Packet stored_packets_[LARGE_NUMBER];
  
  std::ifstream in_[LARGE_NUMBER];
  size_t in_number_[LARGE_NUMBER];
  std::vector<Event>::iterator actual_event_[LARGE_NUMBER];
  std::vector<Event>::iterator next_event_[LARGE_NUMBER];
  
  
public:
 
  Level();
  Level(size_t N, double window);
  ~Level();
  
  Packet packets_[100];
  
  
  
  Event get_event(size_t index);
  Event get_actual_event(size_t iboard){ return *actual_event_[iboard]; }
  Event get_next_event(size_t iboard){ return *next_event_[iboard]; }
  

  void dump();

  double time(){ return t_; }

  double window(){ return window_; }


  double minimum_time(){ return minimum_t_; }

  size_t minimum_index(){ return minimum_index_; }

  double next_to_minimum_time(){ return next_to_minimum_t_; }

  double next_to_minimum_index(){ return next_to_minimum_index_; }

  void set_time_to_minimum(){ t_ = minimum_t_; }

  bool select(std::vector<size_t> * indexes, std::vector< Signal > signals, bool print=false);
  size_t n_coincidences(std::vector<size_t> * indexes, bool print=false);  

  std::string int_to_string(int i);
  std::string get_file_number(size_t i);
  
  
  void assign_packet(size_t i, Packet p);
  void init(Packet* stored_packets,size_t n);
  void get_earliest_board();
  void open_data_files(std::vector<std::string> names);
  void close();
  size_t get_board_with_address(std::string address);
  void recalculate_minima();
  bool increase_the_time(){ return increase_the_time_for_board(minimum_index()); }
  
  void delete_file(std::string filename);  

  
  std::string get_current_dir();

  bool fexists(const char *filename);
  bool increase_the_time_for_board(size_t index);
  void set_event_number(size_t ievent, size_t eventnumber);
  void set_event_written(size_t ievent, bool written);

  void set_event_type(size_t ievent, std::string type);
  bool board_has_channel(size_t iboard, size_t ichannel, bool print=false);

  
  void wait ( double seconds );
  bool board_has_coincidence(size_t iboard, double tzero, bool print=false);

  bool is_program_running(std::string asker, std::string commandname);
  
  
};

#endif

