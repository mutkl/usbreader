#include "Riostream.h"
#include "TString.h"
#include "iostream"
#include "sstream"
#include <TObjArray.h>
#include "TPaveLabel.h"
#include "TPaveText.h"
#include <time.h>
#include <vector>

using namespace std;

void viewer() {

  // define bunch of parameters
  // (n channels) * (n boards) = 32 * 4 = 128
  //  const int LARGE_NUMBER=128;
  int _nsignals;
  int _nchannels_per_event[100];
  int _event_number[100];
  double _time[100];
  int _channels[100][128];
  bool print = (bool)(read_parameter_from_file("params.inp","print")); // print verbose
  size_t filenumber=0;
  TFile *input;
  int modulo=100000;
  string sfilenumber, fname, sfilenumbernext, fnamenext;

  // load detector shape
  vector<size_t> expected_channels;
  vector<size_t> expected_channels_x;
  vector<size_t> expected_channels_y;
  read_detector("detector.inp",&expected_channels, &expected_channels_x, &expected_channels_y);
  size_t detxmin=*min_element(expected_channels_x.begin(), expected_channels_x.end());
  size_t detxmax=*max_element(expected_channels_x.begin(), expected_channels_x.end());
  size_t detymin=*min_element(expected_channels_y.begin(), expected_channels_y.end());
  size_t detymax=*max_element(expected_channels_y.begin(), expected_channels_y.end());
  size_t nlayers_x = get_n_layers(expected_channels_x);
  size_t nlayers_y = get_n_layers(expected_channels_y);


  int ndigits = 20;

  TFile *output = new TFile("data.root","RECREATE");

  TChain chain("data_tree");

  while(1){

    // if input file is not there, stay in eternal loop
    // must quit with ctrl+c
    sfilenumber=get_file_number(filenumber);
    fname = "coincidence_data_"+sfilenumber+".root";
    sfilenumbernext = get_file_number(filenumber+1);
    fnamenext = "coincidence_data_"+sfilenumbernext+".root";
    if( gSystem->AccessPathName( fname.c_str() ) ||  gSystem->AccessPathName( fnamenext.c_str() ) ) {
    
      string asker="viewer";
      string commandname="./select";
      //clog << asker << " needs file " << fname << " or the next, but they are not there yet, check if " << commandname << " is running " << endl;

      bool running=true;
      bool good;
      while(running){
	running=is_lockfile_running(asker.c_str(), "converter");
	  //	running=is_program_running(asker.c_str(), commandname.c_str());
	good = !gSystem->AccessPathName( fname.c_str() )  && !gSystem->AccessPathName( fnamenext.c_str() );
	if( good ) break;
      }
      
      if( !running ){
	clog << asker << " will stop " << endl;
	break;
      }

      clog << asker << " will continue " << endl;
    }
  
  
    // read new input root file
    sfilenumber=get_file_number(filenumber);
    fname = "coincidence_data_"+sfilenumber+".root";
    input = new TFile(fname.c_str(),"READ");
    if( input->IsZombie() ){
      clog << " viewer needs file " << fname << " but it's a zombie, quitting " << endl;
      break;
    }
    clog << " viewer adding file " << fname << endl;


    // add file to chain
    chain.Add(fname.c_str());

    // read tree from file
    TTree *data_tree = (TTree*)(input->Get("data_tree"));
    data_tree->SetBranchAddress("nsignals",&_nsignals);
    data_tree->SetBranchAddress("nchannels_per_event",_nchannels_per_event);
    data_tree->SetBranchAddress("event_number",_event_number);
    data_tree->SetBranchAddress("time",_time);
    data_tree->SetBranchAddress("channels",_channels);

    TBranch *b_nsignals = data_tree->GetBranch("nsignals");
    TBranch *b_nchannels_per_event = data_tree->GetBranch("nchannels_per_event");
    TBranch *b_event_number = data_tree->GetBranch("event_number");
    TBranch *b_time = data_tree->GetBranch("time");
    TBranch *b_channels = data_tree->GetBranch("channels");

    // loop on tree events
    for ( int ient = 0; ient < data_tree-> GetEntries(); ient++ ) {

      // read event
      b_nsignals->GetEvent(ient);
      b_nchannels_per_event->GetEvent(ient);
      b_event_number->GetEvent(ient);
      b_time->GetEvent(ient);
      b_channels->GetEvent(ient);
    
    }
    

   // finish looping on tree
  
  delete input;
  filenumber ++;
  
  // write chain into output file
  chain.Merge(output->GetName());
}
  // finish waiting for files
  
  exit(0);
  
  
}


float read_parameter_from_file(string filename, string paramname){

  FILE * pFile = fopen (filename.c_str(),"r");

  char name[256];
  float value;

  while( EOF != fscanf(pFile, "%s %e", name, &value) ){
    if( paramname.compare(name) == 0 ){
      fclose(pFile);
      return value;
    }
  }

  clog << " warning: could not find parameter " << paramname << " in file " << filename << endl;
  fclose(pFile);

  exit(0);

  return 0.;

}


string get_file_number(size_t i){
  
  size_t limit = 6;
  string numb=int_to_string((int)i);
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

  string int_to_string(int i){
    char c[20];
    sprintf(c,"%d",i);
    string sval(c);
    return sval;
  }


string double_to_string(double val){
  char c[20];
  sprintf(c,"%.9g",val);
  string sval(c);
  return sval;
}

  void wait ( double seconds )
  {
    clock_t endwait;
    endwait = clock () + seconds*1e6;
    while (clock() < endwait) {}
  }

void read_detector(string filename, vector<size_t> * expected_channels, vector<size_t> * expected_channels_x, vector<size_t> * expected_channels_y){

  FILE * pFile = fopen (filename.c_str(),"r");

  int c;
  int x;
  int y;

  while( EOF != fscanf(pFile, "%d %d %d", &c, &x, &y) ){
    expected_channels->push_back(c);
    expected_channels_x->push_back((size_t)x);
    expected_channels_y->push_back((size_t)y);
  }

  fclose(pFile);

  return;
}


bool is_lockfile_running(string asker, string commandname){
  // check lock file

  //  clog << asker << " is checking if " << commandname << " has lockfile " << endl;

  ifstream lockfile_;
  string lockfile_name="__"+commandname+"_lock_file";
  lockfile_.open(lockfile_name.c_str());
  if( lockfile_.good() ){
    //    clog << commandname << " is running, so " << asker << " will hold on " << endl; fflush(stdout);
    return true;
  }
  return false;
}



bool is_program_running(string asker, string commandname){
  // Check if process is running via command-line
  
  //  clog << asker << " is checking if " << commandname << " is running " << endl;
  
  char child[100]; 
  char die[100];	

  string name1="pidof -x ";
  string name2=" > /dev/null ";
  // Check if process is running via command-line

  strcpy (child, name1.c_str());
  strcat (child, commandname.c_str());
  strcat (child, name2.c_str());

  // loop to execute kill
  if(0 == system(child)) {
    //    clog << commandname << " is running, so " << asker << " will hold on " << endl; fflush(stdout);
    return true;
  }

  return false;
}
  
size_t get_n_layers(vector<size_t> expected_channels_){
  size_t counter = 0;
  vector<size_t> unique;
  for(size_t i=0; i<expected_channels_.size(); i++){
    vector<size_t>::iterator ic = std::find(unique.begin(), unique.end(), expected_channels_[i]);
    if( ic == unique.end() )
      unique.push_back(expected_channels_[i]);
  }

  return unique.size();

}
