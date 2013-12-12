/* -*- mode: c++ -*- */
#include <iostream>
#include <iomanip>
#include <cstring>
#include <fstream> 
#include <cmath>
#include <stdlib.h>
#include <stdio.h>
#include <string>       
#include <stdio.h>
#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <stdint.h>
#include <libusb-1.0/libusb.h>
#include <usb.h>
//#include "functions_usb.h"
#include "usbfunctions.h"
//#include "matrix/Matrix.h"
#include <bitset>
#include <sstream>
#include <vector>
#include <deque>
#include <unistd.h>
#include "Word.h"
#include "Event.h"
#include "Packet.h"
#include "Level.h"


//static int NUM_BYTES = 512;
static struct libusb_device_handle *devh = NULL;



int completed = 0; 
const int CYPRESS_VENDOR_ID = 1204;
const int CYPRESS_PRODUCT_ID = 4100;
const uint32_t  TIMEOUT = 1000;
const uint32_t NBYTES = 512;


void cback(struct libusb_transfer *transfer){
  //  std::cout << "In cback, found" << std::endl;
  //std::cout << "actual_length: " << transfer->actual_length << std::endl;
  //  int *completed = transfer->user_data;
  //completed = 1;
  int *p; 
  p=(int*)transfer->user_data; 
  *p=1; //  set completed 
  //std::cout << "set p" << std::endl;
}
//bool read_hex_from_board(libusb_device_handle* handle, double timeout, std::string *packet){
bool read_hex_from_boardd(libusb_device_handle* cypp, struct libusb_transfer *transfer,  std::string *packet){
  
  
  int enddpoint = 134;
  unsigned char ed = (unsigned char)enddpoint;
  
  unsigned char *buf = NULL;
  buf=(unsigned char*)malloc(NUM_BYTES);

  completed = 0;
  
  libusb_fill_bulk_transfer(transfer,cypp,ed,buf,NBYTES,cback,&completed,TIMEOUT);
  int r = libusb_submit_transfer(transfer);  
 
  completed = 0;
  while (!completed) 
    { 
      r = libusb_handle_events_completed(NULL,&completed); 
    }
  char bb[1024];
  for (size_t k = 0;k<NUM_BYTES;k++)
    {
      char b[256];
      //      int n = sprintf(b,"%02x",buf[k]);
      int n = sprintf(b,"%02x",buf[k]);
      //std::cout << b[0] << b[1] << " "; 
      bb[2*k  ] = b[0];
      bb[2*k+1] = b[1];
      
    }

  *packet = (string)bb;
  return true;

 
}

int main(){
  
  print_usb_devices();  
  std::cout << std::endl;
  std::cout << " USING USB " << std::endl ;
  
  //libusb_device_handle *cypp = cypress(CYPRESS_VENDOR_ID, CYPRESS_PRODUCT_ID,TIMEOUT, NBYTES);
  //int enddpoint = endpoint(CYPRESS_VENDOR_ID, CYPRESS_PRODUCT_ID,TIMEOUT, NBYTES);
  vector<libusb_device_handle*> cyppp = retrieve_usb_devices();

  
  
  //struct libusb_transfer *transfer;
  // transfer = libusb_alloc_transfer(0);

  std::string local_packet;
  
  //std::vector<std::string> packets1;
  std::vector<std::string> packets2;
  
  for(int i = 0 ; i < 1000 ; i++){ 
    //    for(auto cypp : cyppp){
    bool oker = read_hex_from_board(cyppp[0], TIMEOUT, &local_packet);
    packets2.push_back(local_packet);
    local_packet.clear();
    //}
    
  }
  
  //  int claim = libusb_claim_interface(cypp,0);
  //std::cout << "claim:" << claim << std::endl;



  // for(int i = 0 ; i < 100 ; i++){
  //   bool oker = read_hex_from_boardd(cypp,transfer,&local_packet);
  //   //bool oker = read_hex_from_board(cypp, TIMEOUT, &local_packet);
  //   packets1.push_back(local_packet);
  //   local_packet.clear();
  // }
  

  

  double current[2], before[2];
  
  
  // std::cout << packets1[0] << std::endl;
  // for(std::vector<std::string>::iterator itr = packets1.begin(); itr != packets1.end(); ++itr){
  //   Packet *p = new Packet(db_hex_to_binary(512,(char*)(*itr).c_str()));
  //   p->packet_to_physical_parameters();
  //   current = p->packet_number();
  //   std::cout << "diff " << current - before << " , ";
  //   //std::cout << "current = p->packet_number(): " << std::setprecision(14) << p->packet_number() << std::endl;
  //   before = current;
  // }
  // std::cout << "\n" << std::endl;
  

   std::cout << packets2[0] << std::endl;
  size_t i = 1;
  for(std::vector<std::string>::iterator itr = packets2.begin(); itr != packets2.end(); ++itr){
    Packet *p = new Packet(db_hex_to_binary(512,(char*)(*itr).c_str()));
    p->packet_to_physical_parameters();
   
    //if(i == 0) i = 1;
    //else i = 0;
    //current[i] = p->packet_number();
    //p->dump();
    std::cout << "current = p->packet_number(): " << std::setprecision(14) << p->packet_number() << std::endl;    
    //std::cout << "diff " << current[i] - before[i] << " , ";
    //before[i] = current[i];
  }
  std::cout << "\n" << std::endl;
  
//bbuuff.push_back(buf2);	  
  // 	  
  // while (!completed) 
  //   { 
  //     r = libusb_handle_events_completed(NULL,&completed); 
  //     if (r < 0) 
  // 	{ 
  // 	  if (r == LIBUSB_ERROR_INTERRUPTED)continue; 
  // 	  libusb_cancel_transfer(transfer); 
  // 	  while (!completed) if (libusb_handle_events_completed(NULL,&completed) < 0) break; 
  // 	  break; 
  // 	} 
  //     if( completed ) 
  // 	{ 

  // 	  free(buf);
  // 	  unsigned char*  buf2 = (unsigned char*)malloc(NBYTES);
  // 	  handeledTransfers++; 
  // 	  i++; 
  // 	  bytesRead+=transfer->actual_length; 
		  
  // 	  libusb_fill_bulk_transfer(transfer,cypp,ed,buf2,NBYTES,cback,&completed,TIMEOUT);
  // 	  status=libusb_submit_transfer(transfer); 
  // 	  bbuuff.push_back(buf2);	  
  // 	  free(buf2);
  // 	  if(i>=1)   // loopcounter. just no to have it running for ever. 
  // 	    { 
  // 	      completed=1; 
  // 	    } 
  // 	  else completed=0; 
  // 	} 
  //     else handeledOther++; 
  //   } 

  //std::cout << "i: " << i << " handeledTransfers: " << handeledTransfers << " bytesRead: " << bytesRead << std::endl;
  // do cleanup 

  // std::cout << "bbuuff.size(): " << bbuuff.size() << std::endl;
  // std::cout << "length: " << transfer->length << std::endl;
  // std::cout << "timeout: " << transfer->timeout << std::endl;
  // std::cout << "reading buffer" << std::endl;

  // for(int i=0;i<bbuuff.size();++i){
  // for (size_t k = 0;k<NUM_BYTES;k++)
  //   {
  //     char b[256];
  //     //      int n = sprintf(b,"%02x",buf[k]);
  //     int n = sprintf(b,"%02x",bbuuff.at(i)[k]);
  //     std::cout << b[0] << b[1] << " "; 
  //   }
  // std::cout << "\n\n next" << std::endl;
  // }


  /*  std::cout << "\n\n~~~~~~~~~~Using Sync Tfer~~~~~~~~~~" << std::endl;
  unsigned char* bufR = (unsigned char*)malloc(NBYTES);
  bool ok;
  int trans;
  ok = libusb_bulk_transfer(cypp,ed,bufR,512,&trans,TIMEOUT);
  std::cout << "ok: " << std::endl;

  for (size_t k = 0;k<NUM_BYTES;k++)
    {
      char b[256];
      int n = sprintf(b,"%02x",bufR[k]);
      std::cout << b[0] << b[1] << " "; 
    }
  std::cout << std::endl;
  
  //  clog << endl; clog << " binary: " << output << endl;


  


  
  */

  return 0;
}
