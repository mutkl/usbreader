#include "Riostream.h"
#include "TString.h"
#include "iostream"
#include "sstream"
#include <TObjArray.h>
#include <vector>


void compare(){

  // look for input file
  string fname_cosmic="output/output.castle_cosmic_nodelay_nopattern/result.root";
  string fname_topx0="output/output.castle_sr90_topx0/result.root";
  string fname_topx1="output/output.castle_sr90_top_x1/result.root";
  string fname_sidex0="output/output.castle_sr90_side_x0/result.root";
  string fname_sidex1="output/output.castle_sr90_side_x1/result.root";


  // define bunch of parameters
  static const int LARGE_NUMBER=128;
  int ndigits = 20;
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
  int _nsignals;
  int _packet_number[LARGE_NUMBER];
  bool _type[LARGE_NUMBER];
  int _event_number[LARGE_NUMBER];
  int _board_address[LARGE_NUMBER];
  double _time[LARGE_NUMBER];
  int _channels[LARGE_NUMBER];
  // two different plots
  double low_times_min=0.;
  double low_times_max=0.01;
  double high_times_min=0.001;
  double high_times_max=1.;

  TH2F *detector_accumulate_cosmic = get_histo(fname_cosmic);
  TH2F *detector_accumulate_topx0 = get_histo(fname_topx0);
  TH2F *detector_accumulate_topx1 = get_histo(fname_topx1);
  TH2F *detector_accumulate_sidex0 = get_histo(fname_sidex0);
  TH2F *detector_accumulate_sidex1 = get_histo(fname_sidex1);


  clog << " cosmic " << detector_accumulate_cosmic->GetEntries()
       << " topx0 " << detector_accumulate_topx0->GetEntries()
       << " topx1 " << detector_accumulate_topx1->GetEntries()
       << " sidex0 " << detector_accumulate_sidex0->GetEntries()
       << " sidex1 " << detector_accumulate_sidex0->GetEntries()
       << endl;

  clog << " nlayers_x " << nlayers_x
       << " nlayers_y " << nlayers_y << endl;

  TObjArray *horlines = new TObjArray(nlayers_y);
  TObjArray *verlines = new TObjArray(nlayers_x);

  for(size_t i=0; i<nlayers_y; i++){
    TLine *l = new TLine(detxmin-0.5,-0.5+i,detxmax+0.5,-0.5+i);
    l->SetLineColor(kBlue);
    l->SetLineWidth(2);
    horlines->Add(l);
  }
  for(size_t i=0; i<nlayers_x; i++){
    TLine *l = new TLine(0.5+i,detymin-0.5,0.5+i,detymax+0.5);
    l->SetLineColor(kBlue);
    l->SetLineWidth(2);
    verlines->Add(l);
  }


  detector_accumulate_topx0->Divide(detector_accumulate_cosmic);
  detector_accumulate_topx1->Divide(detector_accumulate_cosmic);
  detector_accumulate_sidex0->Divide(detector_accumulate_cosmic);
  detector_accumulate_sidex1->Divide(detector_accumulate_cosmic);
  detector_accumulate_cosmic->Divide(detector_accumulate_cosmic);


  c1.Divide(2,2,0,0);
  c1.cd(1);
  detector_accumulate_topx0->SetMarkerColor(kBlack);
  detector_accumulate_topx0->Draw("text,colz");
  horlines->Draw("same");
  verlines->Draw("same");
  c1.cd(2);
  detector_accumulate_topx1->SetMarkerColor(kBlack);
  detector_accumulate_topx1->Draw("text,colz");
  horlines->Draw("same");
  verlines->Draw("same");
  c1.cd(3);
  detector_accumulate_sidex0->SetMarkerColor(kBlack);
  detector_accumulate_sidex0->Draw("text,colz");
  horlines->Draw("same");
  verlines->Draw("same");
  c1.cd(4);
  detector_accumulate_sidex1->SetMarkerColor(kBlack);
  detector_accumulate_sidex1->Draw("text,colz");
  horlines->Draw("same");
  verlines->Draw("same");
  c1.Update();
  c1.Print("compare.eps");


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


size_t interpret(vector<size_t> expected_channels, vector<size_t> expected_channels_x, vector<size_t> expected_channels_y, int c[], size_t n){
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

  //  clog << " look for x " << x << " y " << y << endl;
  for(size_t ic=0; ic<expected_channels.size(); ic++){
    //    clog << " ... " << ic << " : " << expected_channels_x[ic] << " , " << expected_channels_y[ic] << endl;
    if( expected_channels_x[ic] == x &&
	expected_channels_y[ic] == y ) 
      return expected_channels[ic];
  }

  return -1;

}


bool is_on(vector<size_t> expected_channels, vector<size_t> expected_channels_x, vector<size_t> expected_channels_y, int c[], size_t n, size_t x, size_t y){

  int cofc = channel_of_cell(expected_channels, expected_channels_x,  expected_channels_y, x, y);

  if( cofc < 0 ){
    clog << " cannot locate x " << x << " y " << y << endl;
    return false;
  }

  for(size_t i=0; i<n; i++){
    if( c[i] == (int)cofc ) return true;
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


bool vertical_track(vector<size_t> expected_channels, vector<size_t> expected_channels_x, vector<size_t> expected_channels_y, int c[], size_t n){

  size_t detxmin=*min_element(expected_channels_x.begin(), expected_channels_x.end());
  size_t detxmax=*max_element(expected_channels_x.begin(), expected_channels_x.end());
  size_t detymin=*min_element(expected_channels_y.begin(), expected_channels_y.end());
  size_t detymax=*max_element(expected_channels_y.begin(), expected_channels_y.end());


  //  clog << " check if track is vertical " << endl;

  for(size_t i=detymin; i<detymax; i++){

    //    clog << " ... check if it has y = " << i ;
    if( !horizontal_layer(expected_channels, expected_channels_x,  expected_channels_y, c,n, i) ){
      //      clog << " no y = " << i << endl;
      return false;
    }
    //    clog << " yes " << endl;
  }

  //  clog << " yes track is vertical !!!!! " << endl;

  return true;
}


bool horizontal_track(vector<size_t> expected_channels, vector<size_t> expected_channels_x, vector<size_t> expected_channels_y, int c[], size_t n){

  size_t detxmin=*min_element(expected_channels_x.begin(), expected_channels_x.end());
  size_t detxmax=*max_element(expected_channels_x.begin(), expected_channels_x.end());
  size_t detymin=*min_element(expected_channels_y.begin(), expected_channels_y.end());
  size_t detymax=*max_element(expected_channels_y.begin(), expected_channels_y.end());

  for(size_t i=detxmin; i<=detxmax; i++)
    if( !vertical_layer(expected_channels, expected_channels_x,  expected_channels_y, c,n, i) ) return false;

  return true;
}



bool vertical_layer(vector<size_t> expected_channels, vector<size_t> expected_channels_x, vector<size_t> expected_channels_y, int c[], size_t n, size_t xlayer){

  size_t detxmin=*min_element(expected_channels_x.begin(), expected_channels_x.end());
  size_t detxmax=*max_element(expected_channels_x.begin(), expected_channels_x.end());
  size_t detymin=*min_element(expected_channels_y.begin(), expected_channels_y.end());
  size_t detymax=*max_element(expected_channels_y.begin(), expected_channels_y.end());


  for(size_t i=detymin; i<detymax; i++)
    if( is_on(expected_channels, expected_channels_x,  expected_channels_y, c,n,xlayer,i) )
      return true;
    
  return false;
}


bool horizontal_layer(vector<size_t> expected_channels, vector<size_t> expected_channels_x, vector<size_t> expected_channels_y, int c[], size_t n, size_t ylayer){

  size_t detxmin=*min_element(expected_channels_x.begin(), expected_channels_x.end());
  size_t detxmax=*max_element(expected_channels_x.begin(), expected_channels_x.end());
  size_t detymin=*min_element(expected_channels_y.begin(), expected_channels_y.end());
  size_t detymax=*max_element(expected_channels_y.begin(), expected_channels_y.end());

  for(size_t i=detxmin; i<=detxmax; i++){
    //    clog << " ... ... check x = " << i << " y = " << ylayer;
    if( is_on(expected_channels, expected_channels_x,  expected_channels_y, c,n, i, ylayer) ){      
      //      clog << " is on " << endl;
      return true;
    }
    //    clog << " is not on " << endl;
  }

  //  clog << " so y = " << ylayer << " is off " << endl;
  
  return false;
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

TH2F * get_histo(string fname){

  TFile *input = new TFile(fname.c_str(),"READ");

  TH2F *detector_accumulate = (TH2F*)input->Get("detector_accumulate");

  return detector_accumulate;

}
