/* -*- mode: c++ -*- */
#ifndef ISIGNAL
#define ISIGNAL

#include <iostream>
#include <string>
#include <vector>

using namespace std;

class Signal {

  string board_address_;
  size_t channel_;
  bool active_;

public:

  // default constructor
  Signal()
    {
      board_address_="";
      channel_=0;
      active_=true;
    }

  // constructor
  Signal(string board_address, size_t channel, bool active)
    {
      board_address_=board_address;
      channel_=channel;
      active_=active;
    }


  //! destructor
  virtual ~Signal(){};

  virtual void dump (ostream & a_out         = cout,
		     const string & a_title  = "",
		     const string & a_indent = ""){
    {
      string indent;
      if (! a_indent.empty ()) indent = a_indent;
      if (! a_title.empty ())
	{
	  a_out << indent << a_title << endl;
	}

      clog << " board " << board_address_ << " channel " << channel_ << " active " << active_ << endl;

      return;
    }
  }


  void set_board_address( string board_address ) {
    board_address_ = board_address;
  }
  
  void set_channel( size_t channel ) {
    channel_ = channel;
  }
  
  void set_active( bool active ) {
    active_ = active;
  }
  
  string board_address(void)
  {
    return board_address_;
  }

  size_t channel(void)
  {
    return channel_;
  }

  bool active(void)
  {
    return active_;
  }


};

#endif

