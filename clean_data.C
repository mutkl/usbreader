#include <../functions.C>
#include "Riostream.h"
#include "TString.h"
#include "iostream"
#include "sstream"
#include <TObjArray.h>

void clean_data() {

  bool one_board=false;
  static const int NUM_CHANNELS=32;
  // (n channels) * (n boards) = 32 * 4 = 128
  static const int LARGE_NUMBER=128;
  double NUM_BOARDS = read_parameter_from_file("data_params.txt", "NUM_BOARDS");
  int NUM_ALL_CHANNELS=(int)(NUM_BOARDS+0.5)*NUM_CHANNELS;

  TFile *input = new TFile("data.root","READ");
  TFile *output = new TFile("data_clean.root","RECREATE");
  
  TTree *old_data_tree = (TTree*)(input->Get("data_tree"));

  Double_t _Dt;  // s
  Double_t _time;  // s
  bool _C[LARGE_NUMBER];
  int _PN;

  old_data_tree->SetBranchAddress("Dt",&_Dt);
  old_data_tree->SetBranchAddress("time",&_time);
  old_data_tree->SetBranchAddress("C",_C);
  old_data_tree->SetBranchAddress("PN",&_PN);

  TBranch *b_time = old_data_tree->GetBranch("time");
  TBranch *b_Dt = old_data_tree->GetBranch("Dt");
  TBranch *b_C = old_data_tree->GetBranch("C");
  TBranch *b_PN = old_data_tree->GetBranch("PN");

  double nentries=old_data_tree-> GetEntries();

  vector<double> t;
  double initial_time = 0.;
  double final_time = 0.;
  double summed_Dt = 0.;
  for ( int ient = 0; ient < nentries; ient++ ) {

    b_time->GetEvent(ient);
    b_Dt->GetEvent(ient);
    t.push_back(_time);
    if( ient == 0 ) initial_time = _time;
    if( ient == old_data_tree->GetEntries() - 1 ) final_time = _time;
    summed_Dt += _Dt;
  }

  double duration = final_time - initial_time;
  if( duration == 0. ){
    cout << " duration " << duration << endl;
    exit(0);
  }

  cout << " before cleaning: " << endl;
  cout << "       nevents: " << nentries << endl;
  cout << "       1st time " << initial_time << " (s) " << endl;
  cout << "       last time " << final_time << " (s) " << endl;
  cout << "       (last - 1st) " << duration << " (s) , summed Dt = " << summed_Dt << " (s) ";


  if( one_board ){
    if( fabs(duration - summed_Dt)/duration > 0.1 ){
      cout << " wrong! " << endl;
      exit(0);
    } else { cout << " ok " << endl; }
  }

  std::vector<double> lz, hz, ratio;
  get_meaningful_hot_zones(t,lz,hz,ratio);
  for(size_t i=0; i<lz.size(); i++){
    cout << " time zone " << i << "[" << lz[i] << ", " << hz[i] << "]" << " ratio " << ratio[i] << endl;
  }

  TTree *new_data_tree = new TTree("data_tree","ntuple of data");
  Double_t time, Dt;  // s 
  bool C[LARGE_NUMBER];
  int PN;
  new_data_tree->Branch("time",&time,"time/D");
  new_data_tree->Branch("C",&(C[0]),"C[128]/O");
  new_data_tree->Branch("Dt",&Dt,"Dt/D");
  new_data_tree->Branch("PN",&PN,"PN/I");


  double newmin = lz.front();
  double newmax = hz.back();
  bool first = true;
  summed_Dt = 0.;
  for ( int ient = 0; ient < nentries; ient++ ) {
    b_time->GetEvent(ient);
    b_Dt->GetEvent(ient);
    b_C->GetEvent(ient);
    b_PN->GetEvent(ient);
    if( _time < newmin || _time > newmax ) continue;
    if( first ){
      _Dt = 0;				       
      first = false;
      initial_time = _time;
    }

    if( ient == nentries - 1 ) final_time = _time;

    summed_Dt += _Dt;
    time = _time;
    for(size_t i=0; i<NUM_ALL_CHANNELS; i++)
      C[i] = _C[i];

    Dt = _Dt;
    PN = _PN;
    new_data_tree->Fill();

  }

  duration = final_time - initial_time;
  if( duration == 0. ){
    cout << " duration " << duration << endl;
    exit(0);
  }
  cout << " after cleaning: " << endl;
  cout << "       nevents: " << new_data_tree->GetEntries() << " ratio = " << new_data_tree->GetEntries()/nentries << endl;
  cout << "       1st time " << initial_time << " (s) " << endl;
  cout << "       last time " << final_time << " (s) " << endl;
  cout << "       apparent duration " << duration << " (s) , summed Dt = " << summed_Dt << " (s) ";

  if( one_board) {
    if( fabs(duration - summed_Dt)/duration > 0.1 ){
      cout << " wrong! " << endl;
      exit(0);
    } else { cout << " ok " << endl; }
    
  }

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

  cout << " warning: could not find parameter " << paramname << " in file " << filename << endl;

  exit(0);

  return 0.;


}
