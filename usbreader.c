#include <iostream>
#include <cstring>
#include <fstream> 
#include <cmath>
#include <stdlib.h>
#include <stdio.h>
#include <string>       
#include <stdio.h>
#include <libusb-1.0/libusb.h>
#include <usb.h>
#include "functions_usb.c"


using namespace std;


int main() {

  const int CYPRESS_VENDOR_ID = 1204;
  const int CYPRESS_PRODUCT_ID = 4100;
  const uint32_t  TIMEOUT = 500;
  const uint32_t NBYTES = 512;
  //  const int CYPRESS_VENDOR_ID = 7531;
  //  const int CYPRESS_PRODUCT_ID = 1;


  cout << " USING LIBUSB " << endl ;
  /// using libusb

  // look at all usb devices
  print_usb_devices();

  int endpoint = libusb_speak_to_cypress(CYPRESS_VENDOR_ID, CYPRESS_PRODUCT_ID,TIMEOUT, NBYTES);



  cout << endl;
  cout << " USING USB " << endl ;

  /// using usb

  usb_init();
  usb_set_debug(3);


  // look at all buses
  print_buses();



  // look for Cypress chip
  struct usb_dev_handle *cypress_handle = locate_cypress(CYPRESS_VENDOR_ID, CYPRESS_PRODUCT_ID);
  if( cypress_handle == 0 )
    {
      cout << " could not find the Cypress device " << endl;
      return (-1);
    }  


  speak_to_cypress(cypress_handle, endpoint, TIMEOUT, NBYTES);

  usb_release_interface(cypress_handle, 0);

  usb_close(cypress_handle);
  
  return 1;

}
