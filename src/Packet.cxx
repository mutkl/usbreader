#include <Packet.h>


Packet::Packet()
{
  packet_="";
  events_.clear();
  packet_number_=0.;
  board_address_="";
}


// constructor from binary string
Packet::Packet(std::string packet)
{
  if( packet.size() != NUM_BITS ){
    std::clog << " warning: a packet has length " << packet.size() << " instead of " << NUM_BITS << " packet: " << packet << " stored as: " << packet_ << std::endl;
    std::exit(0);
  }
  events_.clear();
  packet_number_=0.;
  board_address_="";
  
  packet_=packet;
}

Packet::Packet(unsigned char* packet)
{
  packet_hex_ = packet;
  events_.clear();
  packet_number_=0.;
  board_address_="";
}

// constructor from HEX string
Packet::Packet(std::string packet, bool useless)
{
  packet_ = packet;
  events_.clear();
  packet_number_=0.;
  board_address_="";
}


Packet::~Packet(){}

void Packet::dump(){
  
  std::cout << " packet number " << packet_number() << " board address " << board_address() << " events: " << std::endl;
  for(std::vector<Event>::iterator iw=events_.begin(); iw!= events_.end(); ++iw){
    std::cout << " event " << iw - events_.begin() << " :"; iw->dump();
  }
  std::cout << " " << std::endl;

  
}

void Packet::set(std::string packet)
{
  packet_ = std::string(packet,0,NUM_BITS);
  if( packet.size() != NUM_BITS ){
    std::cout << " warning: a packet has length " << packet.size() << " instead of " << NUM_BITS << " packet: " << packet << " stored as: " << packet_ << std::endl;
    std::exit(0);
  }
  
}

void Packet::packet_to_physical_parameters(void){
  //x

  double dpn = 0; // packet number
  double olddpn = 0;
  char pn[NUM_PN_BITS];


  for(size_t i=0; i<NUM_PN_BITS; i++){
    size_t uu=0;
    uu=NUM_WORDS_PER_PACKET*NUM_BITS_PER_WORD+i;
    std::string same_packet=packet_;
    pn[i]=same_packet[uu];

  }

 
  std::string spn(pn);

  olddpn = dpn;
  dpn = (double)std::bitset<48>(spn).to_ulong();
  

  packet_number_=dpn;
  
 
  for(size_t iev=0; iev<NUM_EVENTS; iev++){
    std::string event=std::string(packet_,NUM_BITS_PER_EVENT*iev,NUM_BITS_PER_EVENT);
    Event E(event);
    E.event_to_physical_parameters();
    //E.dump();
    events_.push_back(E);
  }

  char ze[NUM_ZEROS];
  for(size_t i=0; i<NUM_ZEROS; i++){
    ze[i]=packet_.at(NUM_WORDS_PER_PACKET*NUM_BITS_PER_WORD+NUM_PN_BITS+i);
  }
  std::string sze(ze);
  if( std::bitset<10>(sze).to_ulong() != 0 ){
    std::cout << " Problem: ten zeros are:" << sze << std::endl;
    std::exit(0);
  }
  //std::cout << std::endl;
  std::cout << "5" << std::endl;
  //    std::cout << " board address: ";
  char bad[NUM_BITS_BOARD_ADDRESS];
  for(size_t i=0; i<NUM_BITS_BOARD_ADDRESS; i++){
    bad[i]=packet_.at(NUM_WORDS_PER_PACKET*NUM_BITS_PER_WORD+58+i);
    //      std::cout << packet_.at(NUM_WORDS_PER_PACKET*NUM_BITS_PER_WORD+58+i);
  }
  board_address_ = std::string(bad,0,NUM_BITS_BOARD_ADDRESS);
  std::cout << "6" << std::endl;
  for(std::vector<Event>::iterator iev=events_.begin(); iev!=events_.end(); iev++){
    iev->set_packet_number(packet_number());
    iev->set_board_address(board_address());
  }
  std::cout << "7" << std::endl;
  return;
}

void Packet::Write(std::ostream & data_file){

  for(std::vector<Event>::iterator iev=events_.begin(); iev!= events_.end(); ++iev){
    iev->Write(data_file);
  }

  return;
}

void Packet::Write_raw(std::ostream & data_file){

  data_file << packet_;
  data_file << std::endl;
    
  return;
}

bool Packet::overlap(Packet p){

  std::cout << " checking packets: " << std::endl;
  dump();
  std::cout << " and " << std::endl;
  p.dump();

  std::cout << " ref times: " << first_time() << " and " << last_time() << std::endl;
  std::cout << " proposed times: " << p.first_time() << " and " << p.last_time() << std::endl;

  if( first_time() < p.last_time() &&
      last_time() > p.first_time() ) {
    std::cout << " overlap! " << std::endl;
    return  true;
  }

  std::cout << " no overlap " << std::endl;

  return false;
}


void Packet::get_coincident_events(Packet p, double window, std::vector<Event> *evs){

  for(std::vector<Event>::iterator iev=events_.begin(); iev!=events_.end(); iev++){
    for(std::vector<Event>::iterator ievp=p.events_.begin(); ievp!=p.events_.end(); ievp++){

      if( iev->coincidence(*ievp, window) ){
	if( !iev->written() ){
	  evs->push_back(*iev);
	  iev->set_written(true);
	}
	if( ! ievp->written() ){
	  evs->push_back(*ievp);
	  ievp->set_written(true);
	}
      }
    }
  }

  return;
}

void Packet::add_to_list(std::vector<Event> * all_events){

  for(std::vector<Event>::iterator iev=events_.begin(); iev!= events_.end(); ++iev){

    all_events->push_back(*iev);
      
  }
    
  return;
}

bool Packet::goto_next_event(Event *e){
  for(std::vector<Event>::iterator iev=events_.begin(); iev!=events_.end(); ++iev){
    if( iev->analyzed() ) continue;
    iev->set_analyzed(true);
    *e = *iev;
    return true;
  }

  return false;

}

