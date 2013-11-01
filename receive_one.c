#include "usbfunctions.h"
#include "Word.h"
#include "Event.h"
#include "Packet.h"
#include "Level.h"
#include <algorithm>

using namespace std;

unsigned int MAX_NUM_PACKETS = 1000;

string get_file_number(size_t i);
string int_to_string(size_t i);

int main(int argc, char *argv[]){

  if( argc <= 2 ){
    clog << " please provide device number and n of events " << endl;
    exit(0);
  }
  string devicename=argv[1];
  MAX_NUM_PACKETS=atoi(argv[2]);

  // initialize parameters
  unsigned int timeout = (unsigned int)read_parameter_from_file("params.inp","timeout");
  bool print = (bool)(read_parameter_from_file("params.inp","print"));
  // print verbosity
  size_t n_packets_to_read_together = (size_t)(read_parameter_from_file("params.inp","n_packets_to_read_together")+0.5);
  string board_address = read_address_from_file("data_params.txt",devicename);
  size_t n_packets_per_file = (size_t)(read_parameter_from_file("params.inp","n_packets_per_file")+0.5);


  // read all usb devices and get cypress ones
  static vector<libusb_device_handle*> handles;
  handles = retrieve_usb_devices();
  if( handles.size() == 0 ){
    clog << " problem: no usb devices " << endl;
    exit(0);
  }

  // read one packet from each board and find board with chosen address
  bool found = false;
  size_t index_of_board_with_good_address;
  static libusb_device_handle* handle;
  for(size_t i=0; i<handles.size(); i++){
    Packet local_packet;
    bool this_ok = read_packet_from_board(handles[i], timeout, &local_packet);
    if( !this_ok ) break;
    if( local_packet.board_address() == board_address ){
      found = true;
      handle = handles[i];
      index_of_board_with_good_address = i;
      break;
    }
  }
  if( !found ){
    clog << " cannot find board with address " << board_address << " quitting " << endl;
    exit(0);
  }

  for(vector<libusb_device_handle*>::iterator ih = handles.begin(); ih!=handles.end(); ++ih){
    if( ih - handles.begin() != index_of_board_with_good_address ){
      libusb_release_interface(*ih,0);
      libusb_close(*ih);
    }
  }

  clog << " freed devices for board " << board_address << endl; fflush(stdout);

  // open data file for outputting
  size_t filenumber=0;
  string sfilenumber=get_file_number(filenumber);
  ofstream data_file;
  string fname = "raw_data_"+board_address+"_"+sfilenumber+".txt";
  data_file.open(fname.c_str(), ios_base::out);


  clog << " will collect " << MAX_NUM_PACKETS << " packets, timeout = " << timeout << ", n of boards: " << handles.size() << ", n of packets to read together = " << n_packets_to_read_together << " from board " << board_address << " write " << n_packets_per_file << " packets per file " << endl; fflush(stdout);





  // collect and write events
  size_t ip = 0;
  size_t i_together=0;
  size_t i_file=0;
  vector<Packet> stored_packets;

  while( ip < MAX_NUM_PACKETS ){
    Packet local_packet;
    bool this_ok = read_raw_packet_from_board(handle, timeout, &local_packet);
    if( !this_ok ) break;

    stored_packets.push_back(local_packet);

    
    ip++;
    i_together++;
    i_file++;

    if( i_together == n_packets_to_read_together ){
      i_together=0;
      for(vector<Packet>::iterator iev=stored_packets.begin(); iev != stored_packets.end(); ++iev)
	iev->Write_raw(data_file);
      stored_packets.clear();
    }

    if( i_file >= n_packets_per_file ){
      data_file.close();
      filenumber ++;
      sfilenumber=get_file_number(filenumber);
      fname = "raw_data_"+board_address+"_"+sfilenumber+".txt";
      data_file.open(fname.c_str());
      i_file = 0;
    }

  }

  if( n_packets_to_read_together >= MAX_NUM_PACKETS ){
    for(vector<Packet>::iterator iev=stored_packets.begin(); iev != stored_packets.end(); ++iev)
      iev->Write_raw(data_file);
    stored_packets.clear();
  }

  // close data file
  data_file.close();


  clog << " collected " << ip << " packets from board " << board_address  << endl;
  exit(0);
  
  return 1;
}

string get_file_number(size_t i){

  size_t limit = 6;
  string numb=int_to_string(i);
  if( numb.size() > limit ){
    clog << " problem: file number " << i << " exceeds limit " << endl;
    exit(0);
  }
  size_t n_extra_zeros=6-numb.size();
  string output="";
  for(size_t i=0; i<n_extra_zeros; i++)
    output += '0';
  return output+numb;

}


string int_to_string(size_t i){
  char c[20];
  sprintf(c,"%d",(int)i);
  string sval(c);
  return sval;
}

