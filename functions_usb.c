#include <iostream>
#include <cstring>
#include <fstream> 
#include <cmath>
#include <stdlib.h>
#include <stdio.h>
#include "math.h"
#include <string>       
#include <libusb-1.0/libusb.h>
#include <usb.h>


using namespace std;

void print_endpoint(struct usb_endpoint_descriptor *endpoint)
{
  printf(" bEndpointAddress: %02xh ", endpoint->bEndpointAddress);
  printf(" bmAttributes: %02xh ", endpoint->bmAttributes);
  printf(" wMaxPacketSize: %d ", endpoint->wMaxPacketSize);
  printf(" bInterval: %d ", endpoint->bInterval);
  printf(" bRefresh: %d ", endpoint->bRefresh);
  printf(" bSynchAddress: %d ", endpoint->bSynchAddress);
}



void print_altsetting(struct usb_interface_descriptor *interface)
{

  printf(" bInterfaceNumber: %d ", interface->bInterfaceNumber);
  printf(" bAlternateSetting: %d ", interface->bAlternateSetting);
  printf(" bNumEndpoints: %d ", interface->bNumEndpoints);
  printf(" bInterfaceClass: %d ", interface->bInterfaceClass);
  printf(" bInterfaceSubClass: %d ", interface->bInterfaceSubClass);
  printf(" bInterfaceProtocol: %d ", interface->bInterfaceProtocol);
  printf(" iInterface: %d ", interface->iInterface);
  
  for (int i = 0; i < interface->bNumEndpoints; i++)
    print_endpoint(&interface->endpoint[i]);
}


void print_interface(struct usb_interface *interface)
{

  for (int i = 0; i < interface->num_altsetting; i++)
    print_altsetting(&interface->altsetting[i]);

}


void print_configuration(struct usb_config_descriptor *config)
{

  printf(" wTotalLength: %d ", config->wTotalLength);
  printf(" bNumInterfaces: %d ", config->bNumInterfaces);
  printf(" bConfigurationValue: %d ", config->bConfigurationValue);
  printf(" iConfiguration: %d ", config->iConfiguration);
  printf(" bmAttributes: %02xh ", config->bmAttributes);
  printf(" MaxPower: %d ", config->MaxPower);
  
  for (int i = 0; i < config->bNumInterfaces; i++)
    print_interface(&config->interface[i]);

}


void print_device(libusb_device *dev) {

  libusb_device_descriptor desc;
  int r = libusb_get_device_descriptor(dev, &desc);
  if (r < 0) {
    cout<<" problem: failed to get device descriptor: "<< r << endl;
    return;
  }

  cout<<" number of possible configurations: "<<(int)desc.bNumConfigurations<<"  ";
  cout<<" device class: "<<(int)desc.bDeviceClass<<"  ";
  cout<<" vendor id: "<<desc.idVendor<<"  ";
  cout<<" product id: "<<desc.idProduct<< " ";

  libusb_config_descriptor *config;
  libusb_get_config_descriptor(dev, 0, &config);
  cout<<" number of interfaces: "<<(int)config->bNumInterfaces<<" | ";

  const libusb_interface *inter;
  const libusb_interface_descriptor *interdesc;
  const libusb_endpoint_descriptor *epdesc;

  for(int i=0; i<(int)config->bNumInterfaces; i++) {
    inter = &config->interface[i];
    cout<< " interface " << i << " number of alternate settings: "<<inter->num_altsetting<<" | ";
    for(int j=0; j<inter->num_altsetting; j++) {
      interdesc = &inter->altsetting[j];
      cout<< " setting " << j << " interface number: "<<(int)interdesc->bInterfaceNumber<<" | ";
      cout<<" number of endpoints: "<<(int)interdesc->bNumEndpoints<<" | ";
      for(int k=0; k<(int)interdesc->bNumEndpoints; k++) {
	epdesc = &interdesc->endpoint[k];
	cout<< " endpoint " << k << " descriptor type: "<<(int)epdesc->bDescriptorType<<" | ";
	cout<<" endpoint address: "<<(int)epdesc->bEndpointAddress<<" | "; fflush(stdout);
      }
    }
  }
  cout<<endl;
  libusb_free_config_descriptor(config);

  return;
}

void get_endpoint_of_device(libusb_device *dev, int *EP = 0) {

  libusb_device_descriptor desc;
  int r = libusb_get_device_descriptor(dev, &desc);
  if (r < 0) {
    cout<<"failed to get device descriptor"<<endl;
    return;
  }

  libusb_config_descriptor *config;
  libusb_get_config_descriptor(dev, 0, &config);
  const libusb_interface *inter;
  const libusb_interface_descriptor *interdesc;
  const libusb_endpoint_descriptor *epdesc;
  for(int i=0; i<(int)config->bNumInterfaces; i++) {
    inter = &config->interface[i];
    for(int j=0; j<inter->num_altsetting; j++) {
      interdesc = &inter->altsetting[j];
      for(int k=0; k<(int)interdesc->bNumEndpoints; k++) {
	epdesc = &interdesc->endpoint[k];
	*EP = (int)epdesc->bEndpointAddress;
      }
    }
  }
  libusb_free_config_descriptor(config);


  return;
}


void print_usb_dev(struct usb_bus *bus, struct usb_device *dev){

  char word[256];
  usb_dev_handle *device_handle = 0;
  device_handle = usb_open(dev);

  cout << " bus name " << bus->dirname << " filename " << dev->filename
       << " vendor id: " << dev->descriptor.idVendor
       << " product id: " << dev->descriptor.idProduct
       << " address: " << dev->filename;

  int ret = usb_get_string_simple(device_handle, dev->descriptor.iManufacturer, word, sizeof(word));
  if( ret <= 0 ){
    cout << " problem: could not get manufacturer " << endl; 
    usb_close (device_handle);
    return;
  }  
  cout << " manufacturer : " <<  word ;
  
  
  ret = usb_get_string_simple(device_handle, dev->descriptor.iProduct, word, sizeof(word));
  if( ret <= 0 ){
    cout << " unable to fetch product string" << endl;
    usb_close (device_handle);
    return;
  }
  cout << " product : " <<  word ;
  
  ret = usb_get_string_simple(device_handle, dev->descriptor.iSerialNumber, word, sizeof(word));
  if( ret <= 0 ){
    cout << " unable to fetch serial number string" << endl;
    usb_close (device_handle);
    return;
  }
  cout << " serial number : " <<  word << " printing configurations: ";

  for (int i = 0; i < dev->descriptor.bNumConfigurations; i++){
    struct usb_config_descriptor config = dev->config[i];
    print_configuration(&config);
  }

  usb_close (device_handle);

  return;

}


void print_buses(void) 
{
  struct usb_bus *bus;
  struct usb_device *dev;
  char word[256];
  int ret;
  
  usb_find_busses();
  usb_find_devices();

  cout << " looking at buses " << endl;

  for (bus = usb_busses; bus; bus = bus->next)
    {
      for (dev = bus->devices; dev; dev = dev->next)	
	{
	  print_usb_dev(bus,dev);
	}	
      cout << endl;
    }
  
  cout << endl;

  return;

}


usb_dev_handle *locate_cypress(int vendor_id, int product_id)
{
  struct usb_bus *bus;
  struct usb_device *dev;
  usb_dev_handle *device_handle = 0;
  char word[256];
  int ret;
  
  usb_find_busses();
  usb_find_devices();

  for (bus = usb_busses; bus; bus = bus->next)
    {
      for (dev = bus->devices; dev; dev = dev->next)	
	{

	  device_handle = usb_open(dev);

	  if (!device_handle) {
	    usb_close (device_handle);
	    continue;
	  }

	  if( dev->descriptor.idVendor == vendor_id &&
	      dev->descriptor.idProduct == product_id ){
	      
	    print_usb_dev(bus,dev);
	    
	    return (device_handle);  	
	  }
	}	
    }
  
  
  return (0);
  
}






void print_usb_devices(){

  libusb_device **devs; //pointer to list of devices
  libusb_context *ctx = NULL; //a libusb session
  int r; //for return values
  ssize_t cnt; //holding number of devices in list

  r = libusb_init(&ctx); //initialize a library session
  if(r < 0) {
    cout<<" problem: could not initialize libusb: "<< r <<endl;
    libusb_exit(ctx); //close the session
    return;
  }

  libusb_set_debug(ctx, 3); 
  //set verbosity level to 3


  cnt = libusb_get_device_list(ctx, &devs); //get the list of devices
  if(cnt < 0) {
    cout<<" problem: could not get device " << endl;
    libusb_free_device_list(devs, 1); //free the list, unref the devices in it
    libusb_exit(ctx); 
    return;
  }

  cout<< " there are " << cnt<<" usb devices "<<endl; 


  for(size_t i=0; i<cnt; i++)
    print_device(devs[i]); //print specs of this device

  
  libusb_free_device_list(devs, 1); //free the list, unref the devices in it
  libusb_exit(ctx); //close the session

  return;

}


int libusb_speak_to_cypress(int vendor_id, int product_id,   uint32_t timeout,   uint32_t nbyte  ){
  // returns the endpoint of requested usb device

  libusb_device **devs; //pointer to list of devices
  libusb_context *ctx = NULL; //a libusb session
  int r; //for return values
  ssize_t cnt; //holding number of devices in list
  int EP = 0;

  r = libusb_init(&ctx); //initialize a library session
  if(r < 0) {
    cout<<" problem: init error "<< r <<endl;
    libusb_exit(ctx); 
    return EP;
  }

  libusb_set_debug(ctx, 3); 


  cnt = libusb_get_device_list(ctx, &devs);
  if(cnt < 0) {
    cout<<" proble: get device error" << cnt << endl; //there was an error
    libusb_free_device_list(devs, 1); 
    libusb_exit(ctx); 
    return EP;
  }



  bool found = false;
  ssize_t cypress_index;
  for(ssize_t i = 0; i < cnt; i++) {
    libusb_device_descriptor desc;
    int r = libusb_get_device_descriptor(devs[i], &desc);
    if (r < 0) {
      continue;
    }

    if( desc.idVendor = vendor_id && desc.idProduct == product_id ){
      cypress_index = i;
      found = true;
      break;
    }
  }

  if( !found ) {
    cout << " warning: I could not find a usb device with product id " << product_id << " and vendor id " << vendor_id << endl;
    libusb_free_device_list(devs, 1); 
    libusb_exit(ctx); 
    return EP;
  }


  libusb_device_handle *cypress_handle = libusb_open_device_with_vid_pid(ctx, vendor_id, product_id); 

  if(cypress_handle == NULL || cypress_handle == 0 ){
    cout<<" warning: cannot open device with product id " << product_id << " and vendor id " << vendor_id << endl;
    libusb_free_device_list(devs, 1);
    libusb_exit(ctx);
    return EP;
  }


  cout << " device with product id " << product_id << " and vendor id " << vendor_id << " has been found " << endl;
  print_device(devs[cypress_index]);
  get_endpoint_of_device(devs[cypress_index], &EP);
  unsigned char cEP = EP;
  cout << " the endpoint is found to be " << EP << endl; fflush(stdout);

  
  int ret = 0;
  int xfer = 0;
  unsigned char* receive_data=(unsigned char*)malloc(nbyte);
  
  if(libusb_kernel_driver_active(cypress_handle, 0) == 1) { 
    //find out if kernel driver is attached
    cout<<" kernel driver is active "<<endl;
    if(libusb_detach_kernel_driver(cypress_handle, 0) == 0) 
      //detach it
      cout<<" kernel driver has been detached "<<endl;
  }
  r = libusb_claim_interface(cypress_handle, 0);
  //claim interface 0 (the first) of device 
  if(r < 0) {
    cout<<" problem: could not claim interface "<< r << endl;
    libusb_free_device_list(devs, 1);
    libusb_exit(ctx); 
    return EP;
  }
  cout<<" claimed interface; now I will read through bulk transfer with nbyte " << nbyte << " xfer " << xfer << " timeout " << timeout <<endl;  fflush(stdout);


  //do a synchronous bulk read from the device
  ret = libusb_bulk_transfer(cypress_handle, EP, receive_data, nbyte, &xfer, timeout);
  cout << " bulk transfer reading returns value: " << ret   << " xfer " << xfer << endl; fflush(stdout);

  
  cout << " now I will write " << endl; fflush(stdout);
  unsigned char *data = new unsigned char[4]; //data to write
  data[0]='a';data[1]='b';data[2]='c';data[3]='d'; //some dummy values
  xfer = 0; //used to find out how many bytes were written
  
  cout<<" data we are going to write: ->" << data << "<- " << endl; fflush(stdout);
  //just to see the data we want to write : abcd
  r = libusb_bulk_transfer(cypress_handle, EP, data, 4, &xfer, 0);
  cout << " bulk transfer writing returns value: " << r   << " xfer " << xfer << endl; fflush(stdout);

  
  r = libusb_release_interface(cypress_handle, 0); //release the claimed interface
  if(r!=0) {
    cout<<" problem: cannot release interface"<<endl;
    libusb_free_device_list(devs, 1);
    libusb_exit(ctx);
    return EP;
  }

  cout<<" released interface"<<endl;
  
  libusb_close(cypress_handle); //close the device we opened
  
  delete[] data; //delete the allocated memory for data
  


  libusb_free_device_list(devs, 1);
  libusb_exit(ctx);

  return EP;

}


void speak_to_cypress(  struct usb_dev_handle *cypress_handle, int EP, uint32_t timeout , uint32_t nbyte){

  int send_status;
  int open_status;
  char send_data=0xff;
  //  char receive_data=0;
  char* receive_data=(char*)malloc(nbyte);
  open_status = usb_set_configuration(cypress_handle,1);
  cout << "conf_stat= " << open_status << endl;
  
  open_status = usb_claim_interface(cypress_handle,0);
  cout << "claim_stat= " << open_status << endl;
  
  open_status = usb_set_altinterface(cypress_handle,0);
  cout << "alt_stat= " << open_status << endl; fflush(stdout);
  
  send_status=usb_bulk_read(cypress_handle,EP,receive_data,(int)nbyte,timeout);	
  cout << " reading at EP " << EP << " result = " << send_status << " received = " <<  *receive_data; fflush(stdout);
  string b(receive_data); cout << " b " << b << endl; fflush(stdout);
  cout << " str " << usb_strerror() << endl;

  send_status=usb_bulk_write(cypress_handle,EP,&send_data,1,timeout);
  cout << "writing, result = " << send_status << endl;
  
  

  return;

}
