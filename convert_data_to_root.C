#include <../functions.C>
#include "Riostream.h"
#include "TString.h"
#include "iostream"
#include "sstream"
#include <time.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

void convert_data_to_root() {
//  Read data from an ascii file and create a root file with an histogram and an ntuple.


  // create lock file (for viewer to check)
  ofstream lockfile_;
  string lockfile_name="__converter_lock_file";
  lockfile_.open(lockfile_name.c_str());
  lockfile_ << " converter active " << endl;
  lockfile_.close();


  TString dir = gSystem->UnixPathName(gInterpreter->GetCurrentMacroName());
  dir.ReplaceAll("read_data.C","");
  dir.ReplaceAll("/./","/");


  // initialize a bunch of parameters
  static int NUM_CHANNELS=32;
  // (n channels) * (n boards) = 32 * 4 = 128
  static const int LARGE_NUMBER=128;
  static const int MAX_COINCIDENCES_NUMBER=100;
  static const int MAX_N_FILES=5000;
  double default_value=-666.;
  int default_value_int=-666;
  bool print = (bool)(read_parameter_from_file("params.inp","print",lockfile_name)); // print verbose
  double NUM_BOARDS = read_parameter_from_file("data_params.txt", "NUM_BOARDS",lockfile_name);
  int NUM_ALL_CHANNELS=(int)(NUM_BOARDS+0.5)*NUM_CHANNELS;
  size_t n_packets_per_file = (size_t)(read_parameter_from_file("params.inp","n_packets_per_file",lockfile_name)+0.5);
  double mintime=999999999999999999.;
  double maxtime=0.;
  double minpn=99999999999999999.;
  double maxpn=0.;
  size_t i_file=0;
  size_t linec = 0;
  std::string lineData;
  size_t old_evn=0;
  int nchannels_per_event=0;
  std::vector<double> row;
  ifstream in;
  int in_number = 0;
  bool good_in=true;
  string fnum, fname;
  // important: expected data structure
  // evn, boardaddress, packetnumber, type, time
  size_t n_variables_before_channels=5; 

  // initialize root structures
  int _nchannels_per_event[MAX_COINCIDENCES_NUMBER];  // n of channels per hit
  int _nsignals;  // n of hits per coincidence event
  int _packet_number[MAX_COINCIDENCES_NUMBER];
  bool _type[MAX_COINCIDENCES_NUMBER];
  int _event_number[MAX_COINCIDENCES_NUMBER];
  int _board_address[MAX_COINCIDENCES_NUMBER];
  double _time[MAX_COINCIDENCES_NUMBER];
  int _channels[MAX_COINCIDENCES_NUMBER][LARGE_NUMBER];

  size_t filenumber=0;
  string sfilenumber=get_file_number(filenumber,lockfile_name);
  string fname = "coincidence_data_"+sfilenumber+".root";
  TFile *f = new TFile(fname.c_str(),"RECREATE");
  string snumbnext;
  string fnamenext;
  
  TTree *data_tree = new TTree("data_tree","ntuple of data");
  data_tree->Branch("nsignals",&_nsignals,"nsignals/I");
  data_tree->Branch("nchannels_per_event",(_nchannels_per_event),"nchannels_per_event[nsignals]/I");
  data_tree->Branch("event_number",(_event_number),"event_number[nsignals]/I");
  data_tree->Branch("packet_number",(_packet_number),"packet_number[nsignals]/I");
  data_tree->Branch("type",(_type),"type[nsignals]/O");
  data_tree->Branch("board_address",(_board_address),"board_address[nsignals]/I");
  data_tree->Branch("time",(_time),"time[nsignals]/D");
  data_tree->Branch("channels",(_channels),"channels[nsignals][128]/I");


  // loop on input txt files
  bool ok = true;
  while(ok){
 
    // if next input txt file is not there, quit
    // otherwise open it
    fnum=get_file_number(in_number,lockfile_name);
    fname="coincidence_data_"+fnum+".txt";
    snumbnext = get_file_number(in_number+1,lockfile_name);
    fnamenext = "coincidence_data_"+snumbnext+".txt";
    if( gSystem->AccessPathName( fname.c_str() ) ||  gSystem->AccessPathName( fnamenext.c_str() ) ) {

      string asker="converter";
      string commandname="./select";
      cout << asker << " needs file " << fname << " or the next, but it's not there yet, check if " << commandname << " is running " << endl;

      bool running=true;
      bool good;
      while(running){
	running=is_program_running(asker.c_str(), commandname.c_str());
	good = !gSystem->AccessPathName( fname.c_str() ) && !gSystem->AccessPathName( fnamenext.c_str() );
	if( good ) break;
      }
      
      if( !running ){
	clog << asker << " will stop " << endl;
	ok=false;
      }

    }
    in.open(fname.c_str());
    if (!in.good()){
      in.close();
      break;
    }
    //    cout << " converter opening file " << fname << endl;

    // delete already processed files
    if( in_number > 2 ){
      string fnumprev=get_file_number(in_number-2,lockfile_name);
      string fnameprev="coincidence_data_"+fnumprev+".txt";
      if (!gSystem->AccessPathName( fnameprev.c_str() )) {
	delete_file(fnameprev);
      }
    }

    _nsignals=0;
    // read input txt file
    while (getline(in, lineData)) {
      if (!in.good()){
	in.close();
	break;
      }
      linec++;
      
      // read one signal
      double local_d;
      row.clear();
      std::stringstream lineStream(lineData);
      while(lineStream >> local_d ){
	row.push_back(local_d);
      }
      if( !lineStream.eof() ){
	cout << "problem: no eof" << endl;
	remove(lockfile_name.c_str());
	exit(0);
      }
      //Vic start here
      if( row.size() < n_variables_before_channels ){
	cout << " problem: row size is " << row.size() << endl;
	remove(lockfile_name.c_str());
	exit(0);
      }
      
      nchannels_per_event=0;
      // if the new signal belongs to a new event
      // write all accumulated signals to one event
      if( _nsignals >= MAX_COINCIDENCES_NUMBER - 1 || (size_t)(row[0] + 0.5) != old_evn ){
	i_file ++;

	if( print ){
	  clog << " " << endl;
	  clog << " writing to tree: nsignals " << _nsignals;
	  for(size_t jj=0; jj<_nsignals; jj++){
	    //for(size_t jj=0; jj<100; jj++){
	    clog << " signal " << jj << " nchannels " << _nchannels_per_event[jj]  << " _event_number " << _event_number[jj] << " _board_address " << _board_address[jj] << " _packet_number " << _packet_number[jj] << "  _time " << _time[jj]  << std::endl;
	    for(size_t kk=0; kk<_nchannels_per_event[jj]; kk++){
	      //for(size_t kk=0; kk<128; kk++){
	    std::clog << " channel " << kk  << " _channels " << _channels[jj][kk] << std::endl;
	    }
	  }
	}
      
      
	data_tree->Fill();

	nchannels_per_event=0;
	_nsignals=0;
	for(size_t i=0; i<MAX_COINCIDENCES_NUMBER; i++){
	  _nchannels_per_event[i]=0;
	  _event_number[i] = default_value_int;
	  _board_address[i] = default_value_int;
	  _packet_number[i] = default_value_int;
	  _type[i] = false;
	  _time[i] = default_value;
	  for(size_t j=0; j<LARGE_NUMBER; j++){
	    _channels[i][j]=default_value_int;
	  }
	}
      }
      
      // load signal into root variables
      // Vic jk it's here
      for(size_t i=n_variables_before_channels; i<row.size(); i++){
        //std::clog << "row.size(): " << row.size() << std::endl;
	_event_number[_nsignals] =  (size_t)(row[0] + 0.5);
	_board_address[_nsignals] = (size_t)binaryToBase10((int)(row[1] + 0.5));
	_packet_number[_nsignals] = (size_t)(row[2] + 0.5);
	_type[_nsignals] = (bool)row[3];
	_time[_nsignals] = row[4];
	_channels[_nsignals][nchannels_per_event]=NUM_CHANNELS*_board_address[_nsignals]+(size_t)(row[i]+0.5);

	if( print ){
	  std::clog << "" << std::endl;
	  clog << " reading evn " << _event_number[_nsignals] <<  " b " << _board_address[_nsignals] << " p " << _packet_number[_nsignals] << " t " << _time[_nsignals] << " c " << _channels[_nsignals][nchannels_per_event] << endl;
	}
	
	if( maxpn < row[2] ) maxpn = row[2];
	if( minpn > row[2] ) minpn = row[2];
	if( row[4] < mintime ) mintime=row[4];
	if( row[4] > maxtime ) maxtime=row[4];

	
	nchannels_per_event ++;
	if( nchannels_per_event >= LARGE_NUMBER ){
	  clog << " problem: nchannels_per_event " << nchannels_per_event << " LARGE NUMBER " << LARGE_NUMBER << endl;
	  nchannels_per_event=LARGE_NUMBER-1;
	}
	_nchannels_per_event[_nsignals]=nchannels_per_event;

      }


      // write into new root file
      if( i_file >= n_packets_per_file ){
	f->Write();
	f->Close();
	delete f;
	filenumber ++;
	sfilenumber=get_file_number(filenumber,lockfile_name);
	fname = "coincidence_data_"+sfilenumber+".root";
	f = new TFile(fname.c_str(),"RECREATE");
	data_tree = new TTree("data_tree","ntuple of data");
	data_tree->Branch("nsignals",&_nsignals,"nsignals/I");
	data_tree->Branch("nchannels_per_event",(_nchannels_per_event),"nchannels_per_event[nsignals]/I");
	data_tree->Branch("event_number",(_event_number),"event_number[nsignals]/I");
	data_tree->Branch("packet_number",(_packet_number),"packet_number[nsignals]/I");
	data_tree->Branch("type",(_type),"type[nsignals]/O");
	data_tree->Branch("board_address",(_board_address),"board_address[nsignals]/I");
	data_tree->Branch("time",(_time),"time[nsignals]/D");
	data_tree->Branch("channels",(_channels),"channels[nsignals][128]/I");
	i_file = 0;
      }

      _nsignals++;
      old_evn=(size_t)(row[0] + 0.5);

      
    } // end of reading txt file

    in.close();
    in_number ++;
  } // end of looping on txt files
  

  // store min time, max time, min pknumber, max pknumber for future plotting
  ofstream data_params_file;
  data_params_file.open("data_params.txt",ios_base::out | ios_base::app);
  data_params_file << " mintime " << mintime << endl;
  data_params_file << " maxtime " << maxtime << endl;
  data_params_file << " minpn " << minpn << endl;
  data_params_file << " maxpn " << maxpn << endl;
  data_params_file.close();

  // save root file
  //data_tree->Fill();
  f->Write();
  f->Close();
  
   
  remove(lockfile_name.c_str());
  exit(0);


}


int binaryToBase10(int n)
{
  int output = 0;

  int m = 1;

  for(int i=0; n > 0; i++) {

    if(n % 10 == 1) {
      output += m;
    }
    n /= 10;
    m <<= 1;
  }

  return output;
}


float read_parameter_from_file(string filename, string paramname, string lockfile_name){

  FILE * pFile = fopen (filename.c_str(),"r");

  char name[256];
  float value;

  while( EOF != fscanf(pFile, "%s %e", name, &value) ){
    if( paramname.compare(name) == 0 ){
      return value;
    }
  }

  cout << " warning: could not find parameter " << paramname << " in file " << filename << endl;

  remove(lockfile_name.c_str());
  exit(0);

  return 0.;


}



string get_file_number(size_t i, string lockfilename){
  
  size_t limit = 6;
  string numb=int_to_string((int)i);
  if( numb.size() > limit ){
    clog << " problem: file number " << i << " exceeds limit " << endl;
    remove(lockfile_name.c_str());
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


  void wait ( double seconds )
  {
    clock_t endwait;
    endwait = clock () + seconds * 1e6 ;
    while (clock() < endwait) {}
  }

void delete_file(string filename){

  TString path = gDirectory->pwd();
  string strpath=path;
  const char spath[80]=strpath.c_str();
  strcat(spath, filename.c_str());
  unlink(spath);

  return;
}

string get_current_dir(){

  char cCurrentPath[100];
  
  if (!getcwd(cCurrentPath, sizeof(cCurrentPath) / sizeof(char)))
    {
      return 0;
    }
  
  cCurrentPath[sizeof(cCurrentPath) - 1] = '\0'; /* not really required */
  
  string answer=cCurrentPath;

  return answer;
  
}



bool is_program_running(string asker, string commandname){
  // Check if process is running via command-line
  
  //  clog << asker << " is checking if " << commandname << " is running " << endl;
  
  char child[100]; 
  char die[100];	

  string name1="/bin/pidof -x ";
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
  
  
