/* -*- mode: c++ -*- */
#ifndef SIGNAL_H
#define SIGNAL_H

#include <iostream>
#include <string>
#include <vector>

class Signal {

  std::string board_address_;
  size_t channel_;
  bool active_;

public:
  Signal();
  Signal(std::string board_address, size_t channel, bool active);
  ~Signal();
  
  void dump();
  
  void set_board_address( std::string board_address ) { board_address_ = board_address; }
  void set_channel( size_t channel ) { channel_ = channel; }
  void set_active( bool active ) { active_ = active; }
  std::string board_address() { return board_address_; }
  size_t channel() { return channel_; }
  bool active() { return active_; }

};

#endif

