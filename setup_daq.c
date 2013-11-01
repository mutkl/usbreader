#include "usbfunctions.h"
#include "Word.h"
#include "Event.h"
#include "Packet.h"
#include "Level.h"
#include <algorithm>

using namespace std;

int main(int argc, char *argv[]){

  // initialize parameters
  unsigned int timeout = (unsigned int)read_parameter_from_file("params.inp","timeout");


  // read all usb devices and get cypress ones
  static vector<libusb_device_handle*> handles;
  handles = retrieve_usb_devices();
  if( handles.size() == 0 ){
    clog << " problem: no usb devices " << endl;
    exit(0);
  }


  // get possible board addresses
  vector<string> possible_board_addresses = get_possible_board_addresses(handles.size());
  clog << " n of boards found: " << handles.size() << endl;
  std::clog << "Here I am1" << std::endl;
  // open data file for outputting
  ofstream data_params_file;
  data_params_file.open("data_params.txt",ios_base::out);
  data_params_file << " NUM_BOARDS " << handles.size() << endl;


  // read one packet from each board and find their addresses
  static libusb_device_handle* handle;
  for(int i=handles.size()-1; i>= 0; i--){
    Packet local_packet;
    bool this_ok = read_packet_from_board(handles[i], timeout, &local_packet);
    if( !this_ok ) break;
    clog << " device " << i << " has address " << local_packet.board_address() << endl;

    // check that address is ok
    bool found = false;
    for(vector<string>::iterator ia=possible_board_addresses.begin(); ia!= possible_board_addresses.end(); ia++){
      if( !strcmp(ia->c_str(),  bitset<6>(local_packet.board_address()).to_string().c_str() )){
	found = true;
	break;
      }
    }
    if( !found ){
      clog << " problem: board address not recognized:" << local_packet.board_address() <<  " allowed addresses are: " << endl;
      for(vector<string>::iterator ia=possible_board_addresses.begin(); ia!= possible_board_addresses.end(); ia++){
	clog << *ia << endl;
      }
      exit(0);
    }

    

    char c[20];
    sprintf(c,"%d",i);
    string nc(c);
    string devname="dev"+nc;
    data_params_file << devname << "   " << local_packet.board_address() << endl;
  }

  for(vector<libusb_device_handle*>::iterator ih = handles.begin(); ih!=handles.end(); ++ih){
    libusb_release_interface(*ih,0);
    libusb_close(*ih);
  }


  // close data file
  data_params_file.close();


  exit(0);
  
  return 1;
}
