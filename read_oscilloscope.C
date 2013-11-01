
#include <../functions.C>

void read_scope_data(string filename, vector <double> & times, vector<double> & voltages){

  times.clear();
  voltages.clear();

  TString Filename(filename.c_str());
  
  ifstream in(Filename.Data());
  if (!in || in.bad()) return 1;
  
  double local_time = 0.;
  double local_voltage = 0.;
  while (!in.eof()) {
    in >> local_time >> local_voltage;
    times.push_back(local_time);
    voltages.push_back(local_voltage);
  }
  in.close();

  if( times.size() != voltages.size() ){
    cout << " problem: " << times.size() << " times and " << voltages.size() << " voltages " << endl;
    exit(0);
  }

  return;
}


void read_oscilloscope(string filename, string secondfilename="unknown"){

  //  setRoot();

  bool secondf=false;

  cout << " reading from file " << filename << endl;

  if( secondfilename!="unknown" ){
    cout << " and file " << secondfilename << endl;
    secondf=true;
  }


  vector<double> times;
  vector<double> voltages;
  vector<double> stimes;
  vector<double> svoltages;
  TCanvas canvas("canvas","canvas",800,600);
  TLegend legend(0.7,0.8,1.,1.,"NDC");
  legend.SetFillColor(0);
  double YTitleOffset = 1.2;


  read_scope_data(filename, times, voltages);
  if( secondf )
    read_scope_data(secondfilename, stimes, svoltages);


  int nbinsT=10000;
  double minT, maxT;
  double sminT, smaxT;
  get_meaningful_min_max(times, &minT, &maxT);
  if( secondf )
    get_meaningful_min_max(stimes, &sminT, &smaxT);

  
  TH1F* waveform = new TH1F("waveform",filename.c_str(),nbinsT, minT, maxT);
  TH1F* swaveform = new TH1F("swaveform",secondfilename.c_str(),nbinsT, sminT, smaxT);

  for(size_t ibint=1; ibint <= times.size(); ibint++){
    waveform->Fill(times[ibint], voltages[ibint]);
  }

  if( secondf ){
    for(size_t ibint=1; ibint <= stimes.size(); ibint++){
      swaveform->Fill(stimes[ibint], svoltages[ibint]);
    }
  }




  double minT, maxT, minV, maxV;
  get_meaningful_min_max(times, &minT, &maxT);
  //  get_meaningful_min_max(voltages, &minV, &maxV);
  int nbinsT=100;
  //  int nbinsV=100;

  
  gStyle->SetOptStat(0);
  canvas.Update();

  waveform->GetXaxis()->SetTitle(" time (s) ");
  waveform->GetYaxis()->SetTitle(" amplitude (V) ");
  waveform->GetYaxis()->SetTitleOffset(YTitleOffset);
  waveform->SetLineColor(1);
  waveform->SetFillColor(1);
  swaveform->GetXaxis()->SetTitle(" time (s) ");
  swaveform->GetYaxis()->SetTitle(" amplitude (V) ");
  swaveform->SetLineColor(2);
  swaveform->SetFillColor(2);
  legend.AddEntry(waveform,waveform->GetTitle(),"F");
  if( secondf )
    legend.AddEntry(swaveform,swaveform->GetTitle(),"F");
  waveform->Draw();
  swaveform->Draw("same");
  legend.Draw();
  canvas.Update();
  
  string savename="channel.C";
  string resavename="channel.eps";

  
  canvas.Print(savename.c_str());
  canvas.Print(resavename.c_str());
  
  delete waveform;
  if( secondf )
    delete swaveform;

  exit(0);
  return;



}
