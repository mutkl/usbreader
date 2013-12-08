#include "usbfunctions.h"
#include "Word.h"
#include "Event.h"
#include "Packet.h"
#include "Level.h"
#include <algorithm>


int main(int argc, char *argv[]){
  std::cout << "start" << std::endl;
  // initialize parameters
  unsigned int timeout = (unsigned int)read_parameter_from_file("params.inp","timeout");

  // read all usb devices and get cypress ones
  static std::vector<libusb_device_handle*> handles;
  
  handles = retrieve_usb_devices();  
  
  if( handles.size() == 0 ){
    std::cout << " problem: no usb devices " << std::endl;
    std::exit(0);
  }


  // get possible board addresses
  std::vector<std::string> possible_board_addresses = get_possible_board_addresses(handles.size());
  std::cout << " n of boards found: " << handles.size() << std::endl;
  std::cout << "Here I am1" << std::endl;
  // open data file for outputting
  ofstream data_params_file;
  data_params_file.open("data_params.txt",ios_base::out);
  data_params_file << " NUM_BOARDS " << handles.size() << std::endl;


  // read one packet from each board and find their addresses


  static libusb_device_handle* handle;
  
  for(int i=handles.size()-1; i>= 0; i--){
    
    Packet local_packet;
    
    bool this_ok = read_packet_from_board(handles[i], timeout, &local_packet);
    
    if( !this_ok ) break;
    std::cout << " device " << i << " has address " << local_packet.board_address() << std::endl;

    // check that address is ok
    bool found = false;
    for(std::vector<std::string>::iterator ia=possible_board_addresses.begin(); ia!= possible_board_addresses.end(); ia++){
      if( !strcmp(ia->c_str(),  bitset<6>(local_packet.board_address()).to_string().c_str() )){
	found = true;
	break;
      }
    }
    if( !found ){
      std::cout << " problem: board address not recognized:" << local_packet.board_address() <<  " allowed addresses are: " << std::endl;
      for(std::vector<std::string>::iterator ia=possible_board_addresses.begin(); ia!= possible_board_addresses.end(); ia++){
	std::cout << *ia << std::endl;
      }
      std::exit(0);
    }

    

    char c[20];
    sprintf(c,"%d",i);
    std::string nc(c);
    std::string devname="dev"+nc;
    data_params_file << devname << "   " << local_packet.board_address() << std::endl;
  }

  for(std::vector<libusb_device_handle*>::iterator ih = handles.begin(); ih!=handles.end(); ++ih){
    libusb_release_interface(*ih,0);
    libusb_close(*ih);
  }


  // close data file
  data_params_file.close();


  std::exit(0);
  
  return 1;
}
