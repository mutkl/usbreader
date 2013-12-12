/* -*- mode: c++ -*- */

#ifndef USBFUNCTIONS_H
#define USBFUNCTIONS_H

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
#include "functions_usb.h"
#include "matrix/Matrix.h"
#include <bitset>
#include <sstream>
#include <vector>
#include <deque>
#include "Packet.h"
#include "Level.h"
#include "Signal.h"
#include <unistd.h>

using namespace std;

/* Frees memory and closes out library upon error or exit.*/
void handleExit(libusb_device ** devs,libusb_device_handle *handle, 
		int *firstUse, unsigned char *buf)
{
  if (handle != NULL)
    {
      libusb_release_interface(handle,0);
      libusb_close(handle);
    }
  if (devs != NULL)
    libusb_free_device_list(devs, 1);
  *firstUse = 0;
  free(buf);
  libusb_exit(NULL);
}

/* Returns 0 if device is desired Cypress module, < 0 otherwise*/
int isCypress(libusb_device *dev)
{

  /* Get descriptive data for device */
  struct libusb_device_descriptor desc;
  if (libusb_get_device_descriptor(dev,&desc) < 0)
    {
      printf("Error getting USB descriptor.\n");
      return -2;
    }

  /* Check to see if what we're checking is Cypress product */
  if ((desc.idVendor == 1204) && (desc.idProduct == 4100))
    return 0;
  
  return -1;
}

/* Does USB bulk transfer read.  Returns number of bytes transferred
 * on success, or a negative number on error or timeout.*/
int read(libusb_device_handle *handle,unsigned char bufR[],
	 unsigned int to)
{

  //  clog << " reading " << endl; fflush(stdout);

  int ok;
  int transferred;
  int end_point = 134;
	
  //	ok = libusb_bulk_transfer(handle,0x86,bufR,512,&transferred,to);
  ok = libusb_bulk_transfer(handle,end_point,bufR,512,&transferred,to);
  if (ok < 0)
    return ok;
	
  return transferred;
}

/* USB bulk transfer write.  Returns 0 on success or (negative) error 
 * code on error.*/
int write(libusb_device_handle *handle,unsigned char bufW[],int buflen,
	  unsigned int to)
{
  int ok;
  int transferred;
  ok = libusb_bulk_transfer(handle,0x02,bufW,buflen,&transferred,to);
  if (ok < 0)
    {
      printf("Bulk write error %d\n", ok);
      return ok;
    }

  return transferred;
}

/* Attempts to reset all library configurations for USB device.*/
void reset(libusb_device ** devs, libusb_device_handle *handle, 
	   int * firstUse, unsigned char *buf)
{
  if (libusb_reset_device(handle) < 0)
    {
      handleExit(devs,handle,firstUse,buf);
      printf("Error resetting device.\n");
    }
  if (libusb_set_configuration(handle, 1) < 0)
    {
      handleExit(devs,handle,firstUse,buf);
      printf("libusb_set_configuration error\n");
    }
  if (libusb_kernel_driver_active(handle,0) != 0)
    {
      if (libusb_detach_kernel_driver(handle,0) < 0)
	{
	  handleExit(devs,handle,firstUse,buf);
	  printf("Error detaching kernel\n");
	}
    }
  if (libusb_claim_interface(handle, 0) < 0)
    {
      handleExit(devs,handle,firstUse,buf);
      printf("libusb_claim_interface error\n");
    }
  if (libusb_set_interface_alt_setting(handle,0,0)<0)
    {
      handleExit(devs,handle,firstUse,buf);
      printf("libusb_set_interface_alt_setting \n");
    }
}


/* Takes in an input mxArray of numbers and returns array with
 * its valus stored in hex (unsigned char) array */
#if 0
unsigned char *getNumToHexArray(const mxArray *prs)
{
  unsigned char *mem = NULL;
  double *act = mxGetData(prs);
  if (prs != NULL)
    //	  mem = (unsigned char*)malloc(2*prs.size());
    mem = malloc(mxGetM(prs)*2,sizeof(char));

  if (mem != NULL)	
    {
      int i = 0;
      for (i;i<mxGetM(prs);i++)
	{
	  int here = act[i];
	  mem[2*i] = (unsigned char) (here >> 8);
	  mem[2*i+1] = (unsigned char) (here & 255);
	}
    }
  return mem;
}
#endif
/* Display possible commands on request or incorrect usage.*/
void usage()
{
  printf("Usage:  \n");
  printf("usbLib('regWrite',addr,data) where addr and data are\n");
  printf("		  input integers - ie hex2dec('ffff')\n");
  printf("usbLib('regRead',addr,NumberOfRegsToRead) moves data\n");
  printf("		  from addr to USB to be available for reading\n");
  printf("usbLib('usbRead',NumberOfTimesToRead) does a bulk \n");
  printf("		  transfer from USB to computer an int number\n");
  printf("	      of times.\n");
  printf("usbLib('reset') resets the USB device.\n");
  printf("usbLib('exit') frees memory and closes everything; \n");
  printf("		  call this function after you've finished.\n");
}



string convert_hex_to_bin(char *s){

  //  string input(s);
  char output[256];

  int i;
  string a,b;

  //decoding might be fast with array, map, something here
  for(i=0;i<2;i++)
    {
      if( i == 1 ) a=string(output);

      switch(s[i])
	{
	case '0':
	  sprintf(output,"0000");
	  break;
	case '1':
	  sprintf(output,"0001");
	  break;
	case '2':
	  sprintf(output,"0010");
	  break;
	case '3':
	  sprintf(output,"0011");
	  break;
	case '4':
	  sprintf(output,"0100");
	  break;
	case '5':
	  sprintf(output,"0101");
	  break;
	case '6':
	  sprintf(output,"0110");
	  break;
	case '7':
	  sprintf(output,"0111");
	  break;
	case '8':
	  sprintf(output,"1000");
	  break;
	case '9':
	  sprintf(output,"1001");
	  break;
	case 'a':
	case 'A':
	  sprintf(output,"1010");
	  break;
	case 'b':
	case 'B':
	  sprintf(output,"1011");
	  break;
	case 'c':
	case 'C':
	  sprintf(output,"1100");
	  break;
	case 'd':
	case 'D':
	  sprintf(output,"1101");
	  break;
	case 'e':
	case 'E':
	  sprintf(output,"1110");
	  break;
	case 'f':
	case 'F':
	  sprintf(output,"1111");
	  break;
	default:
	  printf("Number Is Not Hexadecimal");
	  exit(0);
	}
      
    }
  

  b=string(output);

  //  clog << " input " << input << " output " << a+b << endl;

  return a+b;


}


bool get_channel_number(size_t iword, size_t ichar, size_t * ichannel, char a){

  if( a=='0' ) // even if it's a channel, it's off
    return false;

  // if it's a channel, it's on
  switch(iword){
  case 0:{
    if( ichar==0 ) return false;
    if( ichar==1) return false;
    *ichannel = 33-ichar;  // 31 30 ... 18
    return true;
  }
  case 1:{
    if( ichar==0 ) return false ;
    *ichannel = 18-ichar; // 17 16 ... 3
    return true;
  }
  case 2:{
    if( ichar==0 ) return false ;
    if( ichar>3 ) return false ;
    *ichannel = 3-ichar; // 2 1 0
    return true;
  }
  default:
    return false ;
  }

  return false ;

}


bool time_stamp(size_t iword, size_t ichar){

  switch(iword){
  case 0:{
    return false;
  }
  case 1:{
    return false;
  }
  case 2:{
    if( ichar <= 3 ) return false ;
    return true;
  }
  case 3:{
    if( ichar == 0 ) return false ;
    return true;
  }
  case 4:{
    if( ichar == 0 ) return false ;
    return true;
  }
  case 5:{
    if( ichar == 0 ) return false ;
    return true;
  }
  default:
    return false ;
  }

  return false ;

}

string db_hex_to_binary(int num_bytes, char buf[]){
  string output="";
  size_t ialternate=0;
  string first,second;
  for (size_t k = 0;k<num_bytes;k++)
    {
      char b[2] = {(char)buf[2*k],(char)buf[2*k+1]};

      if( ialternate==0){
	second=convert_hex_to_bin(b);
	ialternate=1;
      }
      else{
	first=convert_hex_to_bin(b);
	ialternate=0;
	output += first+second;
      }
      
    }
  

  return output;
}


string hexagonal_to_binary(int num_bytes, unsigned char *buf){
  string output="";
  //  clog << " hexagonal: ";
  size_t ialternate=0;
  string first,second;
  for (size_t k = 0;k<num_bytes;k++)
    {
      //      printf("%02x",buf[k]);
      
      char b[256];
      int n = sprintf(b,"%02x",buf[k]);
      
      if( ialternate==0){
	second=convert_hex_to_bin(b);
	ialternate=1;
      }
      else{
	first=convert_hex_to_bin(b);
	ialternate=0;
	output += first+second;
      }
      
    }

  //  clog << endl; clog << " binary: " << output << endl;

  return output;
}

static vector<libusb_device_handle*> retrieve_usb_devices(){

  unsigned char *buf = NULL;
  static libusb_device **devs = NULL;

  /* The handle is what we'll actually use to open usb for R/W*/
  static libusb_device_handle *handle = NULL;
  static vector<libusb_device_handle*> handles;
  
  /* Initialize USB device if first call to this mex file*/
  static int firstUse = 0;
  if (firstUse == 0)
    {
      ssize_t cnt;
      
      /* Initializes libusb library and gets list of usb devices */
      if (libusb_init(NULL) < 0)
	printf("Unable to initialize library");
      //clog << " initialized library " << endl; fflush(stdout);
     
      cnt = libusb_get_device_list(NULL, &devs);
      if (cnt < 0)
	printf("Unable to get device list");
      
      /* Go through all USB devices and open the correct one*/
      libusb_device *dev;
      int i = 0;
      while ((dev = devs[i++]) != NULL) 
	{
	  //clog << " ... device " << i ; fflush(stdout);
	  /* If it's cypress device, open and break out of loop*/
	  if (isCypress(dev) == 0)
	    {
	      //clog << " is cypress " << endl; fflush(stdout);
	      /* Open USB for reading*/
	      
	      if (libusb_open(dev,&handle) < 0)
		{
		  handleExit(devs,handle,&firstUse,buf);
		  printf("Unable to open USB");
		}
	      //clog << " opened device" << endl; fflush(stdout);
	      //	      if( handles.size() < NUM_BOARDS )
	      handles.push_back(handle);
	      
	      //	     reset(devs,handle,&firstUse,buf);
	      //	      break;
	    }
	}
      
      
      /* If it got through loop, it couldn't detect correct device.*/
      if (handles.size() == 0)
	{
	  handleExit(devs,handle,&firstUse,buf);
	  printf("Cannot find Cypress device. \n");
	  return handles;
	}
      
      /* Note that static stays in memory between mex calls */
      firstUse++;
    }

  return handles;

}

bool read_hex_from_board(libusb_device_handle* handle, double timeout, std::string *packet){
  //std::stringstream ss;
  unsigned char *buf = NULL;
  buf=(unsigned char*)malloc(NUM_BYTES);
  
  // read buffer from usb device
  int t = read(handle,buf,(int)(timeout+0.5));

  if (t < 0){
    clog << " timeout has returned value: " << t << endl; fflush(stdout);
    free(buf);
    return false;
  }
  char bb[1024];
  for (size_t k = 0;k<NUM_BYTES;k++){
    char b[256]; 
    int n = sprintf(b,"%02x",buf[k]); //magic here, std::cout unable to read buf[k]
    bb[2*k  ] = b[0];
    bb[2*k+1] = b[1];
    //    ss << b;
  }
  
  *packet = (string)bb;
  //*packet = ss.str();
  free(buf);
  return true;
}

bool read_packet_from_board(libusb_device_handle* handle, double timeout, Packet *packet){
  //std::stringstream ss;
  unsigned char *buf = NULL;

  buf=(unsigned char*)malloc(NUM_BYTES);
  
  // read buffer from usb device
  int t = read(handle,buf,(int)(timeout+0.5));
  if (t < 0){
    clog << " timeout has returned value: " << t << endl; fflush(stdout);
    free(buf);
    return false;
  }
  
  string packet_string=hexagonal_to_binary(NUM_BYTES, buf); 
  *packet= Packet(packet_string);
  (*packet).packet_to_physical_parameters();
  
  free(buf);
  
  
  return true;
  
}

bool read_raw_packet_from_board(libusb_device_handle* handle, double timeout, Packet * packet){

  unsigned char *buf = NULL;
  
  buf=(unsigned char*)malloc(NUM_BYTES); 
  
  // read buffer from usb device
  int t = read(handle,buf,(int)(timeout+0.5));
  if (t < 0){
    clog << " timeout has returned value: " << t << endl; fflush(stdout);
    free(buf);
    return false;
  }
  
    *packet = Packet(buf);
  
    /*
  // convert hexagonal output to binary
  string packet_string=hexagonal_to_binary(NUM_BYTES, buf);
  *packet = Packet(packet_string);
  free(buf);
    */
  free(buf);
  return true;

}


float read_parameter_from_file(string filename, string paramname){

  FILE * pFile = fopen (filename.c_str(),"r");

  char name[256];
  float value;

  while( EOF != fscanf(pFile, "%s %e", name, &value) ){
    if( paramname.compare(name) == 0 ){
      return value;
    }
  }

  cout << " warning: could not find parameter " << paramname << " in file " << filename << endl;

  exit(0);

  return 0.;


}

string read_address_from_file(string filename, string paramname){

  FILE * pFile = fopen (filename.c_str(),"r");

  char name[256];
  float value;

  while( EOF != fscanf(pFile, "%s %e", name, &value) ){

    if( paramname.compare(name) == 0 ){
      string address;
      char c[20];
      sprintf(c,"%d",(int)value);
      string extra(c);
      if( value < 10 ){
	address="00000";
      }
      else{
	address="0000";
      }
      return address+extra;
    }
  }

  clog << " warning: could not find address " << paramname << " in file " << filename << endl;

  exit(0);

  return "";


}


string integer_to_binary(unsigned n){

  const size_t size = sizeof(n) * 8;
  char res[size];
  unsigned ind = size;
  do{
    res[--ind] = '0' + (n & 1);
  } while (n >>= 1);

  return std::string(res + ind, res + size);

}

vector<string> get_possible_board_addresses(int NUM_BOARDS){

  vector<string> possible_board_address;
  for(int i=0; i<NUM_BOARDS; i++){
    string extra = integer_to_binary(i);
    string address="";
    for(size_t j=0; j<(NUM_BITS_BOARD_ADDRESS-extra.size()); j++)
      address += "0";
    possible_board_address.push_back((address+extra));
  }

  return possible_board_address;
}

size_t index_of_board_address(string address){

  return (size_t)bitset<6>(address).to_ulong();

}


void read_pattern_file(string filename, vector <Signal> * signals){

  signals->clear();

  FILE * pFile = fopen (filename.c_str(),"r");

  int active;
  char boardWord[256];
  char board_address[256];
  char channelWord[256];
  int channelnumber;

  string board="board";
  string channel="channel";

  while( EOF != fscanf(pFile, "%d %s %s %s %d", &active, boardWord, board_address, channelWord, &channelnumber) ){

    if( board.compare(boardWord) || channel.compare(channelWord)  ){
      clog << " unrecognized pattern signal: " << boardWord << " " << channelWord << endl;
      if( board.compare(boardWord) ) clog << " board " << board << " is " << boardWord << endl;
      if( channel.compare(channelWord) ) clog << " channel " << channel << " is " << channelWord << endl;
      exit(0);
    }


    Signal s(board_address,channelnumber, (bool)active);
    signals->push_back(s);

  }



  return;
}

#endif
