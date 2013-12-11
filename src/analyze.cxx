#include "usbfunctions.h"
#include "Signal.h"
#include "Level.h"
#include "Event.h"
#include <vector>
#include <string>
#include <pqxx/pqxx>
#include "Packet.h"
#include <algorithm> 

int main(int argc, char *argv[]){

  //Get Info  
double window = read_parameter_from_file("params.inp","window");           
double delay = read_parameter_from_file("params.inp","delay");
size_t n_coincidences = (size_t)(read_parameter_from_file("params.inp","n_coincidences")+0.5);
bool print = (bool)(read_parameter_from_file("params.inp","print")); // print verbose
bool pattern = (bool)(read_parameter_from_file("params.inp","pattern"));
int NUM_BOARDS = (int)(read_parameter_from_file("data_params.txt", "NUM_BOARDS")+0.5);
size_t n_packets_per_file = (size_t)(read_parameter_from_file("params.inp","n_packets_per_file")+0.5);


                          
 bool single_board = false;
 std::string board;

 if( NUM_BOARDS < 2 ){
   single_board = true;
   std::cout << " allowing only " << NUM_BOARDS << " board, single = " << single_board << std::endl;
 }  
 
 // vector<Signal> WantedPattern;
 // std::string patternfile = "pattern_stoppingmuon.inp";
 // read_pattern_file(patternfile, &WantedPattern);
 // clog << " will look for pattern : " << endl;
 // for(vector<Signal>::iterator is=WantedPattern.begin(); is!=WantedPattern.end(); ++is)
 //   is->dump(); 
 
 vector<std::string> possible_board_address = get_possible_board_addresses(NUM_BOARDS);
 vector<std::string>::iterator pba;

 
 //Get RAW data

 clog << "initialize leveler" << std::endl;
 Level leveler(NUM_BOARDS, window);
 
 static unsigned int nbrds = NUM_BOARDS;
 Packet stored_packets[nbrds];
 //std::vector<Packet> stored_packets;

 //loop over boards
 
 unsigned int n_events = atoi(argv[1]);
 unsigned int packet_board_counter[nbrds];
 std::fill_n(packet_board_counter,nbrds,1);
 std::cout << "Let's just see if I can read and write to a new table, I want Level to do it though at some point" << std::endl;
 pqxx::connection c_("user=postgres host=127.0.0.1 password=f dbname=boards");
 int one = 1;
 for(pba = possible_board_address.begin();pba!=possible_board_address.end();++pba){
   pqxx::work xxx(c_);
   pqxx::result r = xxx.exec(leveler.read_from_db(*pba,1).c_str());
   xxx.commit();
   std::string jj = db_hex_to_binary(NUM_BYTES,(char*)r[0][0].c_str());
   Packet *ff = new Packet(jj);
   //ff->packet_to_physical_parameters(); 
   stored_packets[pba - possible_board_address.begin()] = *ff;
   r.clear();
   //ff->dump();
 }
 
 leveler.init(stored_packets,NUM_BOARDS); 
 std::vector<size_t> indexes;
 std::vector<size_t>::iterator idx;
 size_t n_coincidences_local;

 while(true)
   {
     //H  word sits in r[0][0] access with r[0][0].c_str()
     //512=======================NUM_BYTES
     //std::cout << (unsigned char*)r[0][0].c_str() << std::endl;
     leveler.dump();
     leveler.get_earliest_board();
     n_coincidences_local = leveler.n_coincidences(&indexes,false);
     
     if( n_coincidences_local >= n_coincidences ) {
       std::cout << " n_coincidences_local " << n_coincidences_local << " min " << n_coincidences << std::endl;      //_nsignals = indexes.size();
       //std::cout << " _nsignals " << _nsignals << std::endl;
       for(idx = indexes.begin();idx!=indexes.end();++idx){
 	 Event e = leveler.get_actual_event(*idx);
 	 e.dump();
       }

     }
     
     if(!leveler.increase_the_time()) {
       size_t mindex = leveler.minimum_index();
       std::cout << "increasing board: " << mindex << std::endl;
       packet_board_counter[mindex]++;
       std::cout << "packet_board_counter[mindex]" << packet_board_counter[mindex] << std::endl;
       pqxx::work xxx(c_);
       pqxx::result r = xxx.exec(leveler.read_from_db(possible_board_address[mindex],packet_board_counter[mindex]).c_str());
       xxx.commit();
       stored_packets[mindex] = Packet(db_hex_to_binary(NUM_BYTES,(char*)r[0][0].c_str()));
       leveler.init(stored_packets, NUM_BOARDS);
       r.clear();
     }
     
     if(packet_board_counter[0] == n_events || packet_board_counter[1] == n_events)//not extendable
       break;
   }
 
}
