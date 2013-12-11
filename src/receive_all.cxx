/* -*- mode: c++ -*- */

#include "usbfunctions.h"
#include "Word.h"
#include "Event.h"
#include "Packet.h"
#include "Level.h"
#include <algorithm>
#include <map>
#include "TTree.h"
#include "TFile.h"
#include <stdlib.h> 

int MAX_NUM_PACKETS = 1000;

int main(int argc, char *argv[]){

  if( argc <= 2 ){
    std::cout << " please provide device number and n of events " << std::endl;
    exit(0);
  }

  MAX_NUM_PACKETS=atoi(argv[1]);

  size_t n_boards = argc-2;
  
  // initialize parameters
  unsigned int timeout = (unsigned int)read_parameter_from_file("params.inp","timeout");
  bool print = (bool)(read_parameter_from_file("params.inp","print"));
  // print verbosity
  size_t n_packets_to_read_together = (size_t)(read_parameter_from_file("params.inp","n_packets_to_read_together")+0.5);
  size_t n_packets_per_file = (size_t)(read_parameter_from_file("params.inp","n_packets_per_file")+0.5);
  size_t n_coincidences = (size_t)(read_parameter_from_file("params.inp","n_coincidences")+0.5);
  double window = read_parameter_from_file("params.inp","window");


  // initialize root structures from convert_data_too_root.C
  static int NUM_CHANNELS=32;
  // (n channels) * (n boards) = 32 * 4 = 128
  static const int LARGE_NUMBER=128;
  static const int MAX_COINCIDENCES_NUMBER=100;
  static const int MAX_N_FILES=5000;
  int _nchannels_per_event[MAX_COINCIDENCES_NUMBER];  // n of channels per hit
  int _nsignals;  // n of hits per coincidence event
  int _packet_number[MAX_COINCIDENCES_NUMBER];
  bool _type[MAX_COINCIDENCES_NUMBER];
  int _event_number[MAX_COINCIDENCES_NUMBER];
  int _board_address[MAX_COINCIDENCES_NUMBER];
  double _time[MAX_COINCIDENCES_NUMBER];
  int _channels[MAX_COINCIDENCES_NUMBER][LARGE_NUMBER];
  
  // read all usb devices and get cypress ones
  static std::vector<libusb_device_handle*> handles;
  handles = retrieve_usb_devices();
  if( handles.size() == 0 ){
    std::cout << " problem: no usb devices " << std::endl;
    exit(0);
  }

  static std::vector<libusb_device_handle*> handles_to_usb_boards;

  for(size_t iboard = 0; iboard<n_boards; iboard++){

    std::string devicename=argv[iboard+2];
    std::string board_address = read_address_from_file("data_params.txt",devicename);

    // read one packet from each board and find board with chosen address
    bool found = false;
    for(size_t i=0; i<handles.size(); i++){
      Packet local_packet;
      bool this_ok = read_packet_from_board(handles[i], timeout, &local_packet);
      if( !this_ok ) break;
      if( local_packet.board_address() == board_address ){
	found = true;
	handles_to_usb_boards.push_back(handles[i]);
	break;
      }
    }
    if( !found ){
      std::cout << " cannot find board with address " << board_address << " quitting " << std::endl;
      exit(0);
    }
  }

    
  for(std::vector<libusb_device_handle*>::iterator ih = handles.begin(); ih!=handles.end(); ++ih){
    if( std::find(handles_to_usb_boards.begin(), handles_to_usb_boards.end(), *ih) == handles_to_usb_boards.end() ){
      libusb_release_interface(*ih,0);
      libusb_close(*ih);
    }
  }
    
  std::cout << " freed all non-cypress devices " << std::endl;
    
  
  std::cout << " will collect " << MAX_NUM_PACKETS << " packets, timeout = " << timeout << ", n of boards: " << handles_to_usb_boards.size() << ", n of packets to read together = " << n_packets_to_read_together << std::endl; fflush(stdout);


  // collect and write events
  size_t ip = 0;
  size_t i_together=0;
  size_t i_file=0;

  // packets of each board address
  //map<std::string, std::vector<Packet> > stored_packets;
  
  //Packet *stored_packets = (Packet*) malloc(n_boards*sizeof(Packet));

  Packet stored_packets[100];

  //xx
  // initialize asticella == level now
  std::cout << "initialize level" << std::endl;
  Level level(handles_to_usb_boards.size(), window);


  //Fill first entry in stored_packets std::vector with the first packet
  Packet local_packet;
  for(std::vector<libusb_device_handle*>::iterator ih = handles_to_usb_boards.begin(); ih!=handles_to_usb_boards.end(); ++ih){
    
    bool this_ok = read_raw_packet_from_board(*ih, timeout, &local_packet);

    if( !this_ok ) break;
    
    stored_packets[ih - handles_to_usb_boards.begin()] = local_packet;
    //    stored_packets[local_packet.board_address()].push_back(local_packet);

    ip++;      
  }

  level.init(stored_packets, n_boards);
  ///xx

  std::vector<size_t> indexes;
  std::vector<size_t>::iterator idx;
  size_t n_coincidences_local;

  while( ip < MAX_NUM_PACKETS || MAX_NUM_PACKETS < 0. ){
    //Packet local_packet;

    level.get_earliest_board();

    ip ++;
    
    n_coincidences_local = level.n_coincidences(&indexes,true);
    
    if( n_coincidences_local >= n_coincidences ) {
      //put event in tree
      //Event event = level.get_actual_event();
      //indexes of boards who are in coincidence
      std::cout << " n_coincidences_local " << n_coincidences_local << " min " << n_coincidences << std::endl;
   
      _nsignals = indexes.size();

      std::cout << " _nsignals " << _nsignals << std::endl;

      for(idx = indexes.begin();idx!=indexes.end();++idx){
	Event e = level.get_actual_event(*idx);
	
	_time[*idx]                 = e.time();
	_nchannels_per_event[*idx]  = e.channels().size();
	//_board_address[*idx]        = atoi(e.board_address());
	_packet_number[*idx]        = e.packet_number();
	//_type[*idx]                 = e.type()
	_event_number[*idx]         = e.event_number();
	
	
	
	std::cout << " _time[*idx] " << _time[*idx] << std::endl;
	
	
	// int _nchannels_per_event[MAX_COINCIDENCES_NUMBER];  // n of channels per hit
	// int _nsignals;  // n of hits per coincidence event
	// int _packet_number[MAX_COINCIDENCES_NUMBER];
	// bool _type[MAX_COINCIDENCES_NUMBER];
	// int _event_number[MAX_COINCIDENCES_NUMBER];
	// int _board_address[MAX_COINCIDENCES_NUMBER];
	// double _time[MAX_COINCIDENCES_NUMBER];
	// int _channels[MAX_COINCIDENCES_NUMBER][LARGE_NUMBER];
	
      }


      
      
    }
   
    if(!level.increase_the_time()){
      bool this_ok = read_raw_packet_from_board(handles_to_usb_boards[level.minimum_index()], timeout, &local_packet);        
      stored_packets[level.minimum_index()] = local_packet;
      level.init(stored_packets, n_boards);
    
    }
    
    //std::cout << " ip " << ip << std::endl;
    //    level.get_actual_event(0).dump();
    //    level.get_actual_event(1).dump();
  
  
  }
  
  //std::cout << " collected " << ip << " packets " <<std::endl;
  //std::cout << " writting root file" << std::endl;
  
  exit(0);
  
  return 1;
}

