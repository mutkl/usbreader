#include "Riostream.h"
#include "TString.h"
#include "iostream"
#include "sstream"
#include <TObjArray.h>
#include <vector>


void convert_data_to_display() {

  static const int LARGE_NUMBER=128;

  int _nsignals;
  int _nchannels_per_event[100];
  int _packet_number[100];
  bool _type[100];
  int _event_number[100];
  int _board_address[100];
  double _time[100];
  int _channels[100][LARGE_NUMBER];

  // look for input file
  string fname="data.root";
  if (gSystem->AccessPathName( fname.c_str() )) {
    clog << " file " << fname << " is missing " << endl;
    exit(0);
  }

  TFile *input = new TFile(fname.c_str(),"READ");

  // read tree from file
  TTree *data_tree = (TTree*)(input->Get("data_tree"));
  if( !data_tree ){
    clog << " problem: cannot read the tree; quitting " << endl;
    exit(0);
  }
  data_tree->SetBranchAddress("nsignals",&_nsignals);
  data_tree->SetBranchAddress("nchannels_per_event",_nchannels_per_event);
  data_tree->SetBranchAddress("event_number",_event_number);
  data_tree->SetBranchAddress("packet_number",_packet_number);
  data_tree->SetBranchAddress("type",_type);
  data_tree->SetBranchAddress("board_address",_board_address);
  data_tree->SetBranchAddress("time",_time);
  data_tree->SetBranchAddress("channels",_channels);
  TBranch *b_nsignals = data_tree->GetBranch("nsignals");
  TBranch *b_nchannels_per_event = data_tree->GetBranch("nchannels_per_event");
  TBranch *b_event_number = data_tree->GetBranch("event_number");
  TBranch *b_packet_number = data_tree->GetBranch("packet_number");
  TBranch *b_type = data_tree->GetBranch("type");
  TBranch *b_board_address = data_tree->GetBranch("board_address");
  TBranch *b_time = data_tree->GetBranch("time");
  TBranch *b_channels = data_tree->GetBranch("channels");

  ofstream output_list;
  output_list.open("event_list.txt");

  // loop on tree
  for ( int ient = 0; ient < data_tree-> GetEntries(); ient++ ) {

    // read event
    b_nsignals->GetEvent(ient);
    b_nchannels_per_event->GetEvent(ient);
    b_event_number->GetEvent(ient);

    b_packet_number->GetEvent(ient);
    b_type->GetEvent(ient);
    b_board_address->GetEvent(ient);
    b_time->GetEvent(ient);
    b_channels->GetEvent(ient);

    //output_list << "\n";
    std::cout << "output_list << _event_number[0]: " << _event_number[0] << std::endl;    
    output_list << _event_number[0];
    std::cout << "_nsignals: " << _nsignals << std::endl;
    //for(size_t is=0; is<_nsignals; is++){
    for(size_t is=0; is<_nsignals; is++){
      std::cout << "is: " << is << std::endl;
      std::cout << "_nchannels_per_event[is]: " << _nchannels_per_event[is] << std::endl;
      for(size_t ich=0; ich<_nchannels_per_event[is]; ich++){
	output_list << "," << _channels[is][ich];
      }
    }
    output_list << "\n";

  } // finish looping on tree

  output_list.close();

  exit(0);


}

