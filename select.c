#include "usbfunctions.h"
#include "Word.h"
#include "Event.h"
#include "Packet.h"
#include "Level.h"
#include "Signal.h"
#include <algorithm>

using namespace std;

int main(int argc, char *argv[]){

  // read parameters
  double window = read_parameter_from_file("params.inp","window");
  double delay = read_parameter_from_file("params.inp","delay");
  size_t n_coincidences = (size_t)(read_parameter_from_file("params.inp","n_coincidences")+0.5);
  bool print = (bool)(read_parameter_from_file("params.inp","print")); // print verbose
  bool pattern = (bool)(read_parameter_from_file("params.inp","pattern"));
  int NUM_BOARDS = (int)(read_parameter_from_file("data_params.txt", "NUM_BOARDS")+0.5);
  size_t n_packets_per_file = (size_t)(read_parameter_from_file("params.inp","n_packets_per_file")+0.5);

  ofstream data_params_file;
  data_params_file.open("data_params.txt", ios_base::app);
  data_params_file << "window " << window << endl;
  data_params_file << "delay " << delay << endl;
  data_params_file << "n_coincidences " << n_coincidences << endl;
  data_params_file << "pattern " << pattern << endl;
  data_params_file.close();

  bool single_board = false;
  if( NUM_BOARDS < 2 ){
    single_board = true;
    clog << " allowing only " << NUM_BOARDS << " board, single = " << single_board << endl;
  }

  clog << " Pattern is " << pattern << endl;
  vector<Signal> WantedPattern;
  if( pattern ){
    if( argc <= 1 ){
      clog << " please provide pattern file " << endl;
      exit(0);
    }
    string patternfile=argv[1];
    read_pattern_file(patternfile, &WantedPattern);
    clog << " will look for pattern : " << endl;
    for(vector<Signal>::iterator is=WantedPattern.begin(); is!=WantedPattern.end(); ++is)
      is->dump();
  }

  // get possible board addresses
  vector<string> possible_board_address = get_possible_board_addresses(NUM_BOARDS);


  // initialize asticella
  clog << "initialize asticella" << std::endl;
  Level asticella(NUM_BOARDS, window);
  for(int iii=0;iii<possible_board_address.size();iii++){
    clog << " possible_board_address(es): " << possible_board_address.at(iii) << std::endl;
  }

  asticella.open_data_files(possible_board_address);
  asticella.recalculate_minima();
  asticella.set_time_to_minimum();
  if( print ) {
    clog << " first asticella: "; asticella.dump();
  }

  // open output data file
  size_t filenumber=0;
  string sfilenumber=asticella.get_file_number(filenumber);
  //std::clog << "sfilenumber: " << sfilenumber << std::endl;
  ofstream data_file;
  string fname = "coincidence_data_"+sfilenumber+".txt";
  data_file.open(fname.c_str(), ios_base::out);


  // loop on data and write coincidences
  size_t n_input_signals = 0;
  size_t nevents = 0;
  bool ok = true;
  size_t i_file=0;
  vector<size_t> indexes;
  while( ok ){
    //clog << "in while ok: " << ok << endl;
    ok = asticella.increase_the_time();
    //clog << "set ok:      " << ok << endl;
    if( ok ){
      if( print ){
	asticella.dump();
      }
      //      clog << "clearing indexes" << endl;
      indexes.clear();
      bool selected=false;
      
      if( single_board ){
	selected = true;
	indexes.push_back(0);
      }else{
	//std::clog << "selected: " << selected << std::endl;
	//std::clog << "pattern : " << pattern  << std::endl;
	if( !pattern ){
	  // select signals in coincidence
	  size_t nc = asticella.n_coincidences(&indexes, print);
	  selected = ( nc >= n_coincidences );
	  //std::clog << "selected = (nc >= n_coincidences)" << selected << " = (" << nc << ">=" << n_coincidences << ")" << std::endl;
	} else {
	  // select signals in coincidence following pattern
	  selected = asticella.select(&indexes, WantedPattern, print);
	}
      }
      
      if( selected ){
	//std::clog << "selected is: " << selected << " in if( selected) " << std::endl;
	bool newevent=true;
	for(vector<size_t>::iterator ii=indexes.begin(); ii!=indexes.end(); ++ii){
	  //std::clog << "in loop" << std::endl;
	  if( asticella.get_actual_event(*ii).written() ){
	    // keep writing to same event if one of the signals
	    // in coincidence has already been written
	    newevent=false;
	  }else{
	    asticella.set_event_written(*ii, true);
	    asticella.set_event_number(*ii, nevents);
	    if( print ){
	      clog << " writing signal: ";  asticella.get_actual_event(*ii).dump();
	    }
	    //clog << " writing signal: ";  asticella.get_actual_event(*ii).dump();
	    asticella.get_actual_event(*ii).Write(data_file);
	  }
	}
	//std::clog << " newevent " << newevent << " nevents " << nevents << std::endl;
	//std::clog << "if( newevent && delay == 0): " << "if( " << newevent << " && " << delay << " == 0)" << std::endl;
	//if( newevent && delay == 0.){
	//if( newevent && delay == 0){
	if( newevent == 0 && delay == 0){
	  // if we are going to look for delay, don't increase ev number yet
	  //std::clog << "increment i_file" << std::endl;
	  i_file ++;
	}else{
	  nevents ++;
	}
      
      }
      
      
      if( selected && delay != 0. ){
	clog << " delay: " << delay << std::endl;
	// open window of time delay and store 1st signal in window
	double tzero=asticella.time();
	while( asticella.increase_the_time() && 
	       asticella.time() < tzero+delay ){
	  size_t minindex = asticella.minimum_index();
	  if( !asticella.get_actual_event(minindex).written() ){
	    asticella.set_event_written(minindex, true);
	    asticella.set_event_type(minindex, "SLOW");
	    asticella.set_event_number(minindex, nevents);
	    if( print ){
	      clog << " writing slow signal: dt " << asticella.get_actual_event(minindex).time("s") - tzero << " ";  asticella.get_actual_event(minindex).dump();
	    }
	    asticella.get_actual_event(minindex).Write(data_file);
	    break;
	  }
	}
	nevents ++;
	i_file ++;
      }
      
      
    }
    //end if(ok)
    
    n_input_signals ++;
    
    //std::clog << " i_file: " << i_file  << " n_packets_per_file " << n_packets_per_file << std::endl;
    if( i_file >= n_packets_per_file ){
      clog << " selector closing file " << fname << endl;
      data_file.close();
      filenumber ++;
      sfilenumber=asticella.get_file_number(filenumber);
      fname = "coincidence_data_"+sfilenumber+".txt";
      data_file.open(fname.c_str());
      i_file = 0;
    }

    //std::clog << " ninputs " << n_input_signals << " ifile " << i_file << " ok2 " << ok << std::endl;

  }


  asticella.close();

  data_file.close();
  
  clog << " n of read signals: " << n_input_signals << endl;
  clog << " n of selected macro-events: " << nevents << endl;
  
  exit(0);

  
  return 1;
}

