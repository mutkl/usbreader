
#include <../functions.C>

plot_channels(){

  //  setRoot();

  string name="channels.txt";

  TH1F* channels = new TH1F("channels",name.c_str(),32,-0.5,31.5);
  
  TString filename(name.c_str());
  
  ifstream in(filename.Data());
  if (!in || in.bad()) return 1;
  
  double local_ch = 0.;
  while (!in.eof()) {
    in >> local_ch;
    channels->Fill(local_ch);
  }
  in.close();
  
  channels->GetXaxis()->SetTitle(" channel ");
  channels->SetFillColor(1);
  channels->Draw();
  
  string savename="channel.C";
  string resavename="channel.eps";
  
  c1->Print(savename.c_str());
  c1->Print(resavename.c_str());
  
  delete channels;


  exit(0);
  return;



}
