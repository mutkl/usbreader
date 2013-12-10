/* -*- mode: c++ -*- */

#include "usbfunctions.h"
#include "Word.h"
#include "Event.h"
#include "Packet.h"
#include "Level.h"
#include <algorithm>
#include <ios>
#include <pqxx/pqxx>

using namespace std;

unsigned int MAX_NUM_PACKETS = 1000;

std::string int_to_string(size_t i){
  char c[20];
  sprintf(c,"%d",(int)i);
  string sval(c);
  return sval;
}

std::string get_file_number(size_t i){

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

std::string sqlInsert(std::string hex){
  std::stringstream ss;
  ss << "INSERT INTO packets VALUES(NULL, '" << hex << "');";
  // std::cout << "sending command: " << std::endl;
  //std::cout << ss.str() << std::endl;
  //std::cout << std::endl;
  return (ss.str());
}


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
  /*
  size_t filenumber=0;
  string sfilenumber=get_file_number(filenumber);
  ofstream data_file;
  string fname = "raw_data_"+board_address+"_"+sfilenumber+".txt";
  data_file.open(fname.c_str(), ios_base::out);
  */

  //postgresql methods
  
  pqxx::connection c("user=postgres host=127.0.0.1 password=f dbname=boards");

  /*
  int rc;
  char *error;
 
  // Open Database
  cout << "Opening data.db ..." << endl;
  sqlite3 *db;
  const char *filename = "board_x_data.db";
  
  rc = sqlite3_open(filename, &db);
  
  if (rc) {
    std::cout << "Error opening SQLite3 database: " << sqlite3_errmsg(db) << std::endl;
    sqlite3_close(db);
    std::exit(0);
  }
  else {
    std::cout << "Opened board_x_data.db" << std::endl;
  }
  cout << "Creating table ..." << endl;
  const char *sqlCreateTable = "CREATE TABLE packets (id INTEGER PRIMARY KEY, value STRING);";
  
  rc = sqlite3_exec(db, sqlCreateTable, NULL, NULL, &error);
  
  if (rc) {
    std::cout<< "Error executing SQLite3 statement: " << sqlite3_errmsg(db) << std::endl;
    sqlite3_free(error);
  } else {
    std::cout << "Created table..." << std::endl;
  }
  */
  clog << " will collect " << MAX_NUM_PACKETS << " packets, timeout = " << timeout << ", n of boards: " << handles.size() << ", n of packets to read together = " << n_packets_to_read_together << " from board " << board_address << " write " << n_packets_per_file << " packets per file " << endl; fflush(stdout);
  
  
  // collect and write events
  size_t ip = 0;
  size_t i_together=0;
  //vector<Packet> stored_packets;
  vector<std::string> stored_packets;
  
  //this is the main loop
  while( ip < MAX_NUM_PACKETS ){
    //Packet local_packet;
    std::string local_packet;
    bool this_ok = read_hex_from_board(handle, timeout, &local_packet);
    //std::cout << local_packet << std::endl;
    //bool this_ok = read_raw_packet_from_board(handle, timeout, &local_packet);
    if( !this_ok ) break;
    stored_packets.push_back(local_packet);
    
    ip++;
    i_together++;
    
    if( i_together == n_packets_to_read_together ){
      i_together=0;
      //      for(vector<Packet>::iterator iev=stored_packets.begin(); iev != stored_packets.end(); ++iev){
      pqxx::work txn(c); 
      for(vector<std::string>::iterator iev=stored_packets.begin(); iev != stored_packets.end(); ++iev){
	//iev->Write_raw(data_file);
	//(*iev).packet_hex();
	/*
	  rc = sqlite3_exec(db, sqlInsert((*iev)).c_str(), NULL, NULL, &error);	
	  if (rc) {
	  cerr << "Error executing SQLite3 statement: " << sqlite3_errmsg(db) << endl << endl;
	  sqlite3_free(error);
	  } else {
	  ;
	  //std:: cout << "Inserted a value into packets table." << std::endl;
	  }*/

	txn.exec(
		 "INSERT into board_1(word) VALUES ("
		 + txn.quote(*iev) +
		 ")");	
      }
      txn.commit();
      stored_packets.clear();
    }
  }
  /*
  std::cout << "Closing board_x_data.db ..." << std::endl;
  sqlite3_close(db);
  std::cout << "Closed board_x_data.db" << std::endl;
  */
  std::cout << "closing psql database" << std::endl;
  //txn.commit();	
  std::cout << " collected " << ip << " packets from board " << board_address  << std::endl;

  std::exit(0);
  
	
  return 1;
}

