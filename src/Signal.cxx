#include "Signal.h"

Signal::Signal()
{
  board_address_="";
  channel_=0;
  active_=true;
}

// constructor
Signal::Signal(std::string board_address, size_t channel, bool active)
{
  board_address_=board_address;
  channel_=channel;
  active_=active;
}


//! destructor
Signal::~Signal(){}

void Signal::dump (){
  
  std::cout << " board " << board_address_ << " channel " << channel_ << " active " << active_ << std::endl;
  
  return;
}

