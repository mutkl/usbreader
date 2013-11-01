#include "Riostream.h"
#include "TString.h"
#include "iostream"
#include "sstream"
#include <TObjArray.h>
#include <vector>


void analyze_data() {

  // define bunch of parameters
  static const int NUM_CHANNELS=32;
  // (n channels) * (n boards) = 32 * 4 = 128
  static const int LARGE_NUMBER=128;
  double NUM_BOARDS = read_parameter_from_file("data_params.txt", "NUM_BOARDS");
  int NUM_ALL_CHANNELS=(int)(NUM_BOARDS+0.5)*NUM_CHANNELS;
  bool print = (bool)(read_parameter_from_file("params.inp","print")); // print verbose
  int ndigits = 20;
  double mintime=read_parameter_from_file("data_params.txt","mintime")*0.99;
  double maxtime=read_parameter_from_file("data_params.txt","maxtime")*1.01;
  double minpn=read_parameter_from_file("data_params.txt","minpn")*0.99;
  double maxpn=read_parameter_from_file("data_params.txt","maxpn")*1.01;
  double oldtime=0.;
  double muon_tzero=0.;
  vector<size_t> expected_on;

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

  double delay  = read_parameter_from_file("params.inp","delay");

  // root structures
  TCanvas c1("c1","canvas",0,0,600,600);
  TFile *output = new TFile("result.root","RECREATE");
  int _nsignals;
  int _nchannels_per_event[100];
  int _packet_number[100];
  bool _type[100];
  int _event_number[100];
  int _board_address[100];
  double _time[100];
  int _channels[100][LARGE_NUMBER];
  // two different plots
  double low_times_min=0.;
  double low_times_max=0.01;
  double high_times_min=0.001;
  double high_times_max=1.;

  // book histograms
  TH1F *channels = new TH1F("channels","channels distribution",NUM_ALL_CHANNELS,-0.5,NUM_ALL_CHANNELS-0.5);
  channels->GetXaxis()->SetTitle(" channel ");
  channels->GetXaxis()->SetTitleSize(0.045);
  channels->GetXaxis()->SetLabelSize(0.045);
  channels->SetFillColor(1);

  TH1F *channels_fast = new TH1F("channels_fast","fast channels distribution",NUM_ALL_CHANNELS,-0.5,NUM_ALL_CHANNELS-0.5);
  channels_fast->GetXaxis()->SetTitle(" fast channel ");
  channels_fast->GetXaxis()->SetTitleSize(0.045);
  channels_fast->GetXaxis()->SetLabelSize(0.045);
  channels_fast->SetFillColor(1);

  TH1F *channels_slow = new TH1F("channels_slow","slow channels distribution",NUM_ALL_CHANNELS,-0.5,NUM_ALL_CHANNELS-0.5);
  channels_slow->GetXaxis()->SetTitle(" slow channel ");
  channels_slow->GetXaxis()->SetTitleSize(0.045);
  channels_slow->GetXaxis()->SetLabelSize(0.045);
  channels_slow->SetFillColor(1);

  TH1F *channels_expected = new TH1F("channels_expected","expected channels distribution",NUM_ALL_CHANNELS,-0.5,NUM_ALL_CHANNELS-0.5);
  channels_expected->GetXaxis()->SetTitle(" expected channel ");
  channels_expected->GetXaxis()->SetTitleSize(0.045);
  channels_expected->GetXaxis()->SetLabelSize(0.045);
  channels_expected->SetFillColor(2);

  TH1F *time = new TH1F("time","time of each event", 1000, mintime,maxtime);
  time->GetXaxis()->SetTitle(" time since the board was started (s) ");
  time->GetXaxis()->SetTitleSize(0.045);
  time->GetXaxis()->SetLabelSize(0.045);
  time->SetFillColor(4);

  TH1F *on_channels = new TH1F("on_channels","number of channels simultaneously on",NUM_ALL_CHANNELS+1,-0.5,NUM_ALL_CHANNELS+0.5);
  on_channels->GetXaxis()->SetTitle(" on channels ");
  on_channels->GetXaxis()->SetTitleSize(0.045);
  on_channels->GetXaxis()->SetLabelSize(0.045);
  on_channels->SetFillColor(2);
  on_channels->GetXaxis()->SetTitle(" number of channels simultaneously on ");

  TH1F *on_channels_expected = new TH1F("on_channels_expected","number of expected channels simultaneously on",NUM_ALL_CHANNELS+1,-0.5,NUM_ALL_CHANNELS+0.5);
  on_channels_expected->GetXaxis()->SetTitle(" on channels ");
  on_channels_expected->GetXaxis()->SetTitleSize(0.045);
  on_channels_expected->GetXaxis()->SetLabelSize(0.045);
  on_channels_expected->SetFillColor(2);
  on_channels_expected->GetXaxis()->SetTitle(" number of expected channels simultaneously on ");

  TH1F *deltatime_inside_event_lowtimes = new TH1F("deltatime_inside_event_lowtimes","delta time distribution inside one event (low times)",ntimebins(low_times_min,low_times_max),low_times_min,low_times_max);
  deltatime_inside_event_lowtimes->GetXaxis()->SetTitle(" delta t (s) ");
  deltatime_inside_event_lowtimes->GetXaxis()->SetTitleSize(0.045);
  deltatime_inside_event_lowtimes->GetXaxis()->SetLabelSize(0.045);

  TH1F *deltatime_inside_event_hightimes = new TH1F("deltatime_inside_event_hightimes","delta time distribution inside one event (high times)",ntimebins(high_times_min,high_times_max),high_times_min,high_times_max);
  deltatime_inside_event_hightimes->GetXaxis()->SetTitle(" delta t (s) ");
  deltatime_inside_event_hightimes->GetXaxis()->SetTitleSize(0.045);
  deltatime_inside_event_hightimes->GetXaxis()->SetLabelSize(0.045);

  TH1F *deltatime_between_events_lowtimes = new TH1F("deltatime_between_events_lowtimes","delta time distribution between events (low times)",ntimebins(low_times_min,low_times_max),low_times_min,low_times_max);
  deltatime_between_events_lowtimes->GetXaxis()->SetTitle(" delta t (s) ");
  deltatime_between_events_lowtimes->GetXaxis()->SetTitleSize(0.045);
  deltatime_between_events_lowtimes->GetXaxis()->SetLabelSize(0.045);

  TH1F *deltatime_between_events_hightimes = new TH1F("deltatime_between_events_hightimes","delta time distribution between events (high times)",ntimebins(high_times_min,high_times_max),high_times_min,high_times_max);
  deltatime_between_events_hightimes->GetXaxis()->SetTitle(" delta t (s) ");
  deltatime_between_events_hightimes->GetXaxis()->SetTitleSize(0.045);
  deltatime_between_events_hightimes->GetXaxis()->SetLabelSize(0.045);

  TH1F *packet_number = new TH1F("packet_number","packet_number",1000,minpn, maxpn);
  packet_number->GetXaxis()->SetTitle(" packet number ");
  packet_number->GetXaxis()->SetTitleSize(0.045);
  packet_number->GetXaxis()->SetLabelSize(0.045);

  TH1F *type = new TH1F("type","type",2,-0.5, 1.5);
  type->GetXaxis()->SetTitle(" type ");
  type->GetXaxis()->SetTitleSize(0.045);
  type->GetXaxis()->SetLabelSize(0.045);

  TH1F *muon_time = new TH1F("muon_time","muontime",ntimebins(0.,delay),0.,delay);
  muon_time->GetXaxis()->SetTitle(" delta t (s) ");
  muon_time->GetXaxis()->SetTitleSize(0.045);
  muon_time->GetXaxis()->SetLabelSize(0.045);


  TH2F *detector_accumulate = new TH2F("detector_accumulate","detector_accumulate",nlayers_x+2,-0.5,(double)nlayers_x+1.5,(double)nlayers_y+2,-0.5,(double)nlayers_y+1.5);
  detector_accumulate->SetMarkerStyle(2);
  detector_accumulate->SetMarkerSize(3);
  detector_accumulate->SetMarkerColor(kRed);

  // look for input file
  string fname="data.root";
  if (gSystem->AccessPathName( fname.c_str() )) {
    int nsec = 10;
    clog << " analyzer nedds file " << fname << " but it's not there yet, try to wait " << nsec << " seconds ... " << endl;
    if (gSystem->AccessPathName( fname.c_str() )){
      clog << " analyzer quitting " << endl;
      exit(0);
    }
  }

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

  TFile *event_output = new TFile(,"READ");


 
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

    if( print ){
      clog << " event " << ient << " nsignals " << _nsignals << endl;
      for(size_t is=0; is<_nsignals; is++){
	std::clog << " nchannels " << _nchannels_per_event[is] << endl;
	//	clog << " ... c " << _channels[is] << " t " << setprecision(ndigits) << _time[is] << " dti " << _time[is] - tzero 
	//    << " dte " << dt << " tzero " << tzero << endl;
      }
    }

    // get tzero
    double tzero=999999999999999999.;
    size_t iszero=0;
    for(size_t is=0; is<_nsignals; is++){
      //for(size_t ich=0; ich<_nchannels_per_event[is]; ich++){
	if (print ) {
	  std::clog << " _time[is] " << _time[is] << " tzero " << tzero << std::endl;
	}
       
	if( _time[is] < tzero && _time[is]>0){
	  
	  tzero = _time[is];
	  iszero=is;
	  //}
      }
    }
   
    // look for muon candidate
    double dt;
    if( ient != 0 ){
      dt = tzero - oldtime;
      if (print ) {
	std::clog << " tzero " << tzero << " oldtime " << oldtime << " dt " << dt << std::endl;
      }
      deltatime_between_events_lowtimes->Fill(dt);
      deltatime_between_events_hightimes->Fill(dt);
      for(size_t isi=0; isi<_nsignals; isi++){
	if( _type[0] == false && _type[isi] == true ){ // fast and slow
	  double mdt=_time[isi] - tzero;
	  clog << " muon decay candidate, dt " << mdt << endl;
	  muon_time->Fill(mdt);
	  break;
	}
      }
    }
    oldtime = tzero;
  

    // fill histograms
    on_channels->Fill(_nsignals);

    //    clog << " expected channels on: " << endl;
    
    expected_on.clear();
    for(size_t is=0; is<_nsignals; is++){
      for(size_t ich=0; ich<_nchannels_per_event[is]; ich++){
	// channels distributions
	channels->Fill(_channels[is][ich]);
	vector<size_t>::iterator ic = std::find(expected_channels.begin(), expected_channels.end(), _channels[is][ich]);
	if( ic != expected_channels.end() ){
	  //	clog << *ic << endl;
	  expected_on.push_back(_channels[is][ich]);
	  channels_expected->Fill(_channels[is][ich]);
	  size_t ind = ic - expected_channels.begin();
	  detector_accumulate->Fill(expected_channels_x[ind], expected_channels_y[ind]);
	}

	if( _type[is] )
	  channels_slow->Fill(_channels[is][ich]);
	else
	  channels_fast->Fill(_channels[is][ich]);
      
      }
      packet_number->Fill(_packet_number[is]);
      type->Fill(_type[is]);
      // time distributions
      time->Fill(_time[is]);
      if( is != iszero ){
	deltatime_inside_event_lowtimes->Fill(_time[is] - tzero);
	deltatime_inside_event_hightimes->Fill(_time[is] - tzero);
      }
    }
    on_channels_expected->Fill(expected_on.size());




  } // finish looping on tree
  clog << " analyzer finished looping " << endl; fflush(stdout);

  // write output
  output->Write();

  exit(0);


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

  clog << " warning: could not find parameter " << paramname << " in file " << filename << endl;

  exit(0);

  return 0.;

}

bool is_parameter_in_file(string filename, string paramname){

  FILE * pFile = fopen (filename.c_str(),"r");

  char name[256];
  float value;

  while( EOF != fscanf(pFile, "%s %e", name, &value) ){
    if( paramname.compare(name) == 0 ){
      return true;
    }
  }

  return false;

}


Int_t ntimebins(double min, double max){
  Int_t maxnbins=500000;
  double CYCLE_LENGTH = 25.e-9;  // clock cycle (s)
  Int_t nbins = (Int_t)((max - min)/CYCLE_LENGTH + 0.5);

  if( nbins > maxnbins ){
    //    clog << " warning: limiting n of bins from " << nbins << " to " << maxnbins << endl;
    nbins = maxnbins;
  }

  return nbins;
}


size_t interpret(vector<size_t> expected_channels, vector<size_t> expected_channels_x, vector<size_t> expected_channels_y, int c[][], size_t n){
  // 0 ... !A 
  // 1 ... A !C 
  // 2 ... A C F I
  // 3 ... A C F (H, J)
  // 4 ... A C (E, G)
  // 5 ... A C F 
  // 6 ... A C 

  if( !is_on(expected_channels, expected_channels_x,  expected_channels_y, c,n,1.,3.) ) return 0;

  if( !is_on(expected_channels, expected_channels_x,  expected_channels_y, c,n,1.,2.) ) return 1;

  if( is_on(expected_channels, expected_channels_x,  expected_channels_y, c,n,1.,1.) && 
      is_on(expected_channels, expected_channels_x,  expected_channels_y, c,n,1.,0.) ) 
    return 2;

  if( is_on(expected_channels, expected_channels_x,  expected_channels_y, c,n,1.,1.) && 
      ( is_on(expected_channels, expected_channels_x,  expected_channels_y, c,n,0.,0.) ||
	is_on(expected_channels, expected_channels_x,  expected_channels_y, c,n,2.,0.) ))
    return 3;
  
  if( is_on(expected_channels, expected_channels_x,  expected_channels_y, c,n,0.,1.) ||
      is_on(expected_channels, expected_channels_x,  expected_channels_y, c,n,2.,1.) )
    return 4;
  
  if( is_on(expected_channels, expected_channels_x,  expected_channels_y, c,n,1.,1.) )
    return 5;

  return 6;

}

int channel_of_cell(vector<size_t> expected_channels, vector<size_t> expected_channels_x, vector<size_t> expected_channels_y, size_t x, size_t y){

  //clog << " look for x " << x << " y " << y << endl;
  for(size_t ic=0; ic<expected_channels.size(); ic++){
    //clog << " ... " << ic << " : " << expected_channels_x[ic] << " , " << expected_channels_y[ic] << endl;
    if( expected_channels_x[ic] == x &&
	expected_channels_y[ic] == y ) 
      return expected_channels[ic];
  }

  return -1;

}


bool is_on(vector<size_t> expected_channels, vector<size_t> expected_channels_x, vector<size_t> expected_channels_y, int c[][], size_t n, size_t x, size_t y){
  int cofc = channel_of_cell(expected_channels, expected_channels_x,  expected_channels_y, x, y);
  
  if( cofc < 0 ){
    clog << " cannot locate x " << x << " y " << y << endl;
    return false;
  }

  for(size_t i=0; i<n; i++){
    if( c[0][i] == (int)cofc ) return true;
  }
  return false;
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

  if( expected_channels->size() != expected_channels_x->size() ||
      expected_channels->size() != expected_channels_y->size() ){
    cout << " problem: detector sizes " << expected_channels->size() << " " << expected_channels_x->size() << " " << expected_channels_y->size() << endl;
    exit(0);
  }


  return;
}





string int_to_string(int i){
  char c[20];
  sprintf(c,"%d",i);
  string sval(c);
  return sval;
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
