#include <iostream>
#include <math.h>
#include <TROOT.h>
#include <TChain.h>
#include "TH2.h"
#include "TH1.h"
#include "TPaveLabel.h"
#include "TPaveText.h"
#include <string>
#include <sstream>
#include <vector>

void setRoot(void){

  gROOT->Reset();
  gROOT->SetStyle("Plain");
  gStyle->SetPalette(1,0);

  gStyle->SetOptStat(0);
  gStyle->SetOptTitle(0);


}

void draw(TH1F* h, string name, string format){

  TLegend leg(0.6,0.8,1,1);

  h->SetLineColor(1);
  leg.AddEntry(h);
  h->Draw();
  c1->Update();
  string title="fig/"+name+format;

  c1->Print(title.c_str());


  return;
}

void draw(TH1F* h, string name){

  draw(h,name,"eps");

  return;
}

void draw(TH1F* h, string name, bool log){

  TLegend leg(0.6,0.8,1,1);

  h->SetLineColor(1);
  leg.AddEntry(h);
  h->Draw();
  c1->Update();
  string title="fig/"+name+".eps";

  if( log ){
    double max = h->GetMaximum();
    h->GetYaxis()->SetRange(0.5,max*1.3);
    c1->SetLogy(1);
  }

  c1->Print(title.c_str());

  if( log )
    c1->SetLogy(0);


  return;
}



void drawTwo(TH1F* h, TH1F *g, string name, bool NormalizeArea, bool log, bool divide, double rangemin = 0., double rangemax = 0.){

  TCanvas canvas("canvas","canvas",0,0,800,500);

  TLegend leg(0.4,0.7,0.9,0.9);
  leg.SetFillColor(0);

  double maxxh = h->GetXaxis()->GetXmax();
  double maxxg = g->GetXaxis()->GetXmax();
  double maxx = min(maxxh, maxxg);
  double amaxx = max(maxxh, maxxg);
  double minxh = h->GetXaxis()->GetXmin();
  double minxg = g->GetXaxis()->GetXmin();
  double minx = max(minxh, minxg);
  double aminx = min(minxh, minxg);
  

  if( divide ){
#if 0
    canvas.Divide(1,2);
    canvas.cd(1);
#else
    double topxmin = 0.;
    double topxmax = 1.;
    double topymin = 0.30;
    double topymax = 1.;
    double botxmin = 0.;
    double botxmax = 1.*(maxx - minx)/(amaxx-aminx);
    double botymin = 0.;
    double botymax = 0.29;
    TPad* toppad = new TPad("toppad","toppad",topxmin, topymin, topxmax, topymax);
    TPad* botpad = new TPad("botpad","botpad",botxmin, botymin, botxmax, botymax);
    toppad->Draw();
    toppad->cd();
#endif
  }



  h->SetLineColor(kBlue);
  h->SetMarkerColor(kBlue);
  g->SetLineColor(kRed);

  if( NormalizeArea ){
    if( g->Integral() != 0 )
      g->Scale(h->Integral()/g->Integral());
  }

  h->Sumw2();
  g->Sumw2();

  double max;
  double maxh = h->GetMaximum();
  double maxg = g->GetMaximum();

  if( maxh >= maxg ){
    max = maxh;
  } else {
    max = maxg;
  }


  double min, max;
  if( log ){

    double minh = getNonZeroMinimum(h);
    double ming = getNonZeroMinimum(g);
    

    if( minh <= ming ){
      min = minh;
    }
    else
      min = ming;
    
    min = min/2.;
  }


  if( maxh > maxg ){
    if( log ){
      canvas.SetLogy(1);
      if( divide )
	toppad->SetLogy(1);
      h->GetYaxis()->SetRangeUser(min,1.5*max);
    }
    h->Draw();
    g->Draw("same E");
  }
  else{
    if( log ){
      canvas.SetLogy(1);
      if( divide )
	toppad->SetLogy(1);
      g->GetYaxis()->SetRangeUser(min,1.5*max);
    }
    g->Draw("E");
    h->Draw("same");
  }

  canvas.Update();
    
  leg.AddEntry(h, h->GetTitle(), "l");
  leg.AddEntry(g, g->GetTitle(), "le");

  leg.Draw("same");


  if( divide ){
    Double_t  chi2;
    Int_t  ndf;
    Int_t  igood;

    if( rangemin == 0. )
      rangemin = h->GetXaxis()->GetXmin();
    if( rangemax == 0. )
      rangemax = h->GetXaxis()->GetXmax();

    size_t fb = h->FindBin(rangemin);
    size_t lb = h->FindBin(rangemax);
    size_t nbins = h->GetXaxis()->GetNbins();

    TH1F ch("ch","",lb-fb+1,rangemin,rangemax);
    TH1F cg("cg","",lb-fb+1,rangemin,rangemax);

    for(size_t i=fb; i<=lb; i++){
      ch.SetBinContent(i+1,h->GetBinContent(i));
      cg.SetBinContent(i+1,g->GetBinContent(i));
    }

    ch.Chi2TestX(&cg,chi2, ndf, igood, "UW P");

    double prob = TMath::Prob(chi2, ndf);

    string sndof  = double_to_string((double)ndf);
    string schi2 = double_to_string(chi2);
    string sprob = double_to_string(prob);
  
    double pt_xmin = 0.45;
    double pt_xmax = 0.8;
    double pt_ymin = 0.6;
    double pt_ymax = 0.7;
    TPaveText ptTop(pt_xmin, pt_ymin, pt_xmax, pt_ymax, "NDC");
    ptTop.SetFillColor(4100); // text is black on white
    ptTop.SetTextSize(0.06);
    ptTop.SetTextAlign(12);
    string text="#chi^{2} /ndof = "+schi2+" / "+sndof+",    prob = "+sprob;
    ptTop.AddText(text.c_str());

    ptTop.Draw("same");

#if 0
    canvas.cd(2);
#else
    canvas.cd(0);
    botpad->Draw();
    botpad->cd();
#endif

    TPaveText *pt = new TPaveText(0.1,0.7,0.8,0.9, "NDC"); // NDC sets coords
    // relative to pad dimensions
    pt->SetFillColor(0); // text is black on white
    pt->SetTextSize(0.1); 
    pt->SetTextAlign(12);
    pt->SetBorderSize(1);

    string tith=h->GetTitle();
    string titg=g->GetTitle();
    string title="ratio: "+titg+" / "+tith;
    //  string title="ratio";

    pt->AddText(title.c_str());

    TH1F copyh("copyh",h->GetTitle(), h->GetXaxis()->GetNbins(),minx, maxx);
    TH1F copyg("copyg",g->GetTitle(), h->GetXaxis()->GetNbins(),minx, maxx);
    for(size_t i=0; i<copyh.GetXaxis()->GetNbins()+1; i++){
      copyh.SetBinContent(i, h->GetBinContent(h->GetXaxis()->FindBin(copyh.GetBinCenter(i))));
    }
    for(size_t i=0; i<copyg.GetXaxis()->GetNbins()+1; i++){
      copyg.SetBinContent(i, g->GetBinContent(g->GetXaxis()->FindBin(copyg.GetBinCenter(i))));
    }

    if( NormalizeArea ){
      if( copyg.Integral() != 0 )
	copyg.Scale(copyh.Integral()/copyg.Integral());
    }


    TH1F *ratio = copyh.Clone(); 
    ratio->SetLineColor(kBlack);
    ratio->GetXaxis()->SetLabelSize(0.1);
    ratio->GetYaxis()->SetLabelSize(0.1);
    ratio->SetName("ratio"); // Clone one of the histograms
    ratio->Divide(&copyg,&copyh,1.,1.);
    
    ratio->Draw("E");
    pt->Draw("same");


  }

  canvas.Update();


  string title="fig/"+name;

  canvas.Print(title.c_str());


  return;
}


void drawTwo(TH1F* h, TH1F *g, string name, bool NormalizeArea, bool log){

  drawTwo(h, g, name, NormalizeArea, log, false);

  return;


}


void drawTwo(TH1F* h, TH1F *g, string name, bool NormalizeArea){

  drawTwo(h, g, name, NormalizeArea, false);

  return;
}

void drawThree(TH1F* h, TH1F *g, TH1F *l, string name, bool NormalizeArea){

  TLegend leg(0.6,0.8,1,1);

  h->SetLineColor(1);
  leg.AddEntry(h);
  g->SetLineColor(2);
  leg.AddEntry(g);
  l->SetLineColor(4);
  leg.AddEntry(l);


  if( NormalizeArea ){
    if( g->GetEntries() != 0 )
      g->Scale(h->GetEntries()/g->GetEntries());
    if( l->GetEntries() != 0 )
      l->Scale(h->GetEntries()/l->GetEntries());
  }

  double maxh = h->GetMaximum();
  double maxg = g->GetMaximum();
  double maxl = l->GetMaximum();

  if( maxh > maxg ){
    if( maxh > maxl){
      h->Draw();
      g->Draw("same");
      l->Draw("same");
    } else {
      l->Draw();
      g->Draw("same");
      h->Draw("same");
    } 
  } else {
    if( maxg > maxl ){
      g->Draw();
      l->Draw("same");
      h->Draw("same");
    } else {
      l->Draw();
      g->Draw("same");
      h->Draw("same");
    }
  }


  leg.Draw("same");
  c1->Update();
  string title="fig/"+name+".gif";

  c1->Print(title.c_str());


  return;
}

void drawTwoD(TH2F* h, string name){

  //  TLegend leg(0.6,0.8,1,1);

  h->SetLineColor(1);
  //  leg.AddEntry(h);
  h->Draw("colz");
  //  leg.Draw("same");
  c1->Update();
  string title="fig/"+name+".eps";

  c1->Print(title.c_str());


  return;
}



void drawDivTwo(TH1F* h, TH1F *g, string name){

  h->Sumw2(); 
  g->Sumw2(); // store sum of squares of weights (if not already done)
  TH1F *ratio = h->Clone(); 
  ratio->SetName("ratio"); // Clone one of the histograms
  ratio->Divide(g,h,1.,1.);


  TLegend leg(0.,0.8,1.,1);

  string tith=h->GetName();
  string titg=g->GetName();
  string title="ratio: "+titg+" / "+tith;
  //  string title="ratio";

  leg.AddEntry(ratio, title.c_str());

  ratio->Draw();
  leg.Draw("same");

  string pic = "fig/ratio_"+name+".jpg";

  c1->Print(pic.c_str());


  return;
}


Double_t FX ( Double_t *x, Double_t *parm) { 

  double val = parm[0] *exp(-x[0])*cos(parm[1]*x[0]);

return val; 

} 


Double_t FV ( Double_t *x, Double_t *parm) { 

  // 0 3 A
  // 1 0 b
  // 2 1 omega
  // 3 2 k
  // 4 4 c

  double val = -parm[0] *exp(-parm[1]*x[0])*parm[1]*cos(parm[2]*x[0]);

return val; 

} 

Double_t FT ( Double_t *x, Double_t *parm) { 

  // 0 3 A
  // 1 0 b
  // 2 1 omega
  // 3 2 k
  // 4 4 c

  double val = parm[0]*parm[0]*parm[3]*parm[3]/(2*parm[4]) *(parm[1]*parm[1]/(parm[3]*parm[3]) + 1 -   exp(-2*parm[1]*x[0])*(parm[1]*parm[1]/(parm[3]*parm[3])*cos(2*parm[2]*x[0]) + parm[2]*parm[1]/(parm[3]*parm[3])*sin(2*parm[2]*x[0]) + 1 ));
;



return val; 

} 


Double_t DC ( Double_t *x, Double_t *parm) { 

  // 0 A

  double val = parm[0]*pow(x[0],3./2.);

return val; 

} 

Double_t DQ ( Double_t *x, Double_t *parm) { 

  // 0 A

  double newx[1];
  double val = 0.;

  for(size_t i=0; i<6; i++){
    double ix = (double)i*2.;
    newx[0] = ix;
    val += 1.3*DC(newx,parm)*exp(-0.5*((x[0]-ix)/0.1)**2);
  }

return val; 

} 




Double_t FSDOT ( Double_t *x, Double_t *parm) { 

  // 0 3 A
  // 1 0 b
  // 2 1 omega
  // 3 2 k
  // 4 4 c
  // 5 5 mass
  // 6 6 max value of histogram
  // 7 7 T iniziale

  
  double T = FT(x, parm);
  double v = FV(x, parm);

  return 2*parm[5]*parm[1]*v*v/(parm[7] + T);


} 



Double_t FS ( Double_t *x, Double_t *parm) { 

  // 0 3 A
  // 1 0 b
  // 2 1 omega
  // 3 2 k
  // 4 4 c
  // 5 5 mass
  // 6 6 max value of histogram
  // 7 7 T iniziale

  size_t nsteps = 1000;
  double valmax = parm[6];  // max of histogram
  double step_length = valmax / nsteps;
  double integral = 0.;
  size_t imax = (size_t)(x[0]/step_length + 0.5);

  if( imax < 2 ) return 0.;

  double newx[1];

  for(size_t i=1; i<nsteps; i++){
    if( i > imax ) break;
    newx[0] = i*step_length;
    integral += FSDOT(newx,parm);

  }

  integral *= step_length;

  return integral;


} 


Double_t fascio_geodetiche ( Double_t *x, Double_t *parm) { 

  // 0 theta0

  double val = 0.;
  double param = parm[0];

  if( parm[0] == 0. ){
    if((int)(x[0]*100) % 2 == 0)
      param = 1.;
    else
      param = -1.;
  }


  val = 3*param/(3. + param*x[0]);


  return val; 

} 





Double_t single_beta_spectrum ( Double_t *x, Double_t *parm) { 

  // 0 == C
  // 1 == mass
  // 2 == Q

  double val = parm[0]*pow(pow(x[0],2) + 2.*x[0]*parm[1]
			   ,0.5)*pow(parm[2]-x[0],2)*pow(x[0]+parm[1],2);

  return val;

} 

Double_t double_beta_spectrum ( Double_t *x, Double_t *parm) { 

  // 0 == C
  // 1 == mass
  // 2 == Q

  double val = parm[0]*x[0]*pow(parm[2]-x[0],5)*
    (1. + 2.*x[0]+4.*pow(x[0],2)/3. + pow(x[0],3)/3.+pow(x[0],4)/30.);

  return val;

} 

Double_t double_beta_spectrum_smeared ( Double_t *x, Double_t *parm) { 

  // 0 == C
  // 1 == mass
  // 2 == Q
  // 3 == smearing sigma
  // 4 == smearing amplitude

  double local_val = 0.;
  //  size_t nsteps=100*(size_t)(parm[3]/parm[2]+0.5);
  size_t nsteps=1000;
  //  cout << " Q " << parm[2] << " s " << parm[3] << " n " << nsteps << endl;
  for(size_t i=0; i<nsteps; i++){
    double ix = parm[2]/nsteps*i;
    double val = parm[0]*ix*pow(parm[2]-ix,5)*
      (1. + 2.*ix+4.*pow(ix,2)/3. + pow(ix,3)/3.+pow(ix,4)/30.);
    double quad = pow(x[0] - ix,2)/(2.*pow(parm[3],2));
    local_val += exp(-quad)/(2.5*parm[3])*parm[4]*val;

    //    cout << " ix " << ix << " val " << val << " smeared " << local_val << endl;
  }

  return local_val;

} 

double fit(TH1F* h){

  double sigma, error;

  fit(h, &sigma, &error);

  return sigma;


}


void fit_gaus_with_params(TH1F* h, string *sigma, string *sigmaerr, int N){

  double s, e;

  TF1 *func = new TF1("func", "gaus");

  h->Fit(func,"N");

  std::ostringstream sigmas, sigmaerrors;

  s=func->GetParameter(2);
  e=func->GetParError(2);

  sigmas << rndup(s, N);
  sigmaerrors << rndup(e, N);
  *sigma =  sigmas.str();
  *sigmaerr =  sigmaerrors.str();


  return ;


}


double getNonZeroMinimum(TH1F* h){

  size_t nbins = h->GetXaxis()->GetNbins();

  double min = h->GetMaximum() + 1.;
  double val;
  for(size_t i=1; i<nbins+1; i++){

    val = h->GetBinContent(i);
    if( (val < min ) && (val > 0.) )
      min = val;
  }
  
  return min;
  

}



void fit(TH1F* h, double *sigma_, double *sigmaerr_){

  string name="";
  fit(h, sigma_, sigmaerr_, name);


  return;


}

void fit(TH1F* h, double *sigma_, double *sigmaerr_, string title){

  string direc="fig/";

  TF1 *func = new TF1("func", "gaus");

  func->SetLineColor(2);

  double sigma = 0.;
  double mean = 0.;
  double meanerror=0.;
  double error = 0.;

  for(size_t i=0; i<3; i++){

    if( i==0 )
      h->Fit(func,"Q");
    else
      h->Fit(func,"QR","",mean-2.*sigma,mean+2.*sigma);

    sigma=func->GetParameter(2);
    mean=func->GetParameter(1);
    meanerror=func->GetParError(1);  // error on mean                                                                 
    error=func->GetParError(2);  // error on sigma                                                                    
  }

  string name = h->GetName();

  TLegend leg(0.6,0.7,0.9,1.);
  leg.AddEntry(h,title.c_str());
  leg.AddEntry(func,"gaussian fit");

  std::string ssigma, serr;;

  // convert double to string                                                                                                                                       
  { std::ostringstream ssa;
    ssa << sigma;
    ssigma = ssa.str();
  }

  { std::ostringstream ssb;
    ssb << error;
    serr = ssb.str();
  }


  string text="sigma = "+ssigma+" +- "+serr;

  leg.SetHeader(text.c_str());
  leg.SetTextSize(0.03);


  h->Draw();
  leg.Draw("same");

  cout << " fitting histogram: " << name << endl;
  cout << "   mean = " << mean << " +- " << meanerror << endl;
  cout << "   sigma = " << sigma << " +- " << error << endl;

  *sigma_ = sigma;
  *sigmaerr_ = error;


  c1->Print((direc+name+"_"+title+"Fit.gif").c_str());


  return;


}

double rndup(double n)//round up a float type and show 3 decimal places
{
  bool positive=true;
  if( n < 0 )
    positive = false;


  double hn = fabs(n)*1000;
  size_t shn = (size_t)(hn + 0.5);
  double rn = shn/1000.;

  if( !positive )
    rn *= -1.;

  return rn;

}          

double rndup(double n, int N)//round up a float type and show N decimal place
{
  bool positive=true;
  if( n < 0 )
    positive = false;

  double factor = pow(10,N);

  double hn = fabs(n)*factor;
  size_t shn = (size_t)(hn + 0.5);
  double rn = shn/factor;

  if( !positive )
    rn *= -1.;

  return rn;

}          


void fitTwoG(TH1F* h, double *sigma1_, double *sigmaerr1_, double *sigma2_, double *sigmaerr2_, string title){

  string direc="fig/";

  Double_t par[6];
  TF1 *g1    = new TF1("g1","gaus");
  g1->SetLineColor(1);


  double sigma = 0.;
  double mean = 0.;
  double meanerror=0.;
  double error = 0.;

  for(size_t i=0; i<3; i++){

    if( i==0 )
      h->Fit(g1,"QN0");
    else
      h->Fit(g1,"QRN0","",mean-3.*sigma,mean+3.*sigma);

    sigma=g1->GetParameter(2);
    mean=g1->GetParameter(1);
    meanerror=g1->GetParError(1);  // error on mean                                                                 
    error=g1->GetParError(2);  // error on sigma                                                                    

    cout << " 1st gaussian: step " << i << " mean " << mean << " +- " << meanerror << "  sigma = " << sigma << " +- " << error << endl;

  }
  g1->GetParameters(&par[0]);


  TF1 *g2    = new TF1("g2","gaus",-par[2]/20.,par[2]/20.);
  g2->SetLineColor(2);
  h->Fit(g2,"QRN0");
  g2->GetParameters(&par[3]);
  cout << " 2nd gaussian:  mean " << g2->GetParameter(1) << " +- " << g2->GetParError(1) << "  sigma = " << g2->GetParameter(2) << " +- " << g2->GetParError(2) << endl;

  c1->Clear();

  TF1 *total = new TF1("total","gaus(0)+gaus(3)",-3.*par[2],3.*par[2]);
  total->SetLineColor(4);
  total->SetParameters(par);
  h->Fit(total,"R");
  total->GetParameters(&par[0]);

  Double_t* ErrParams = total->GetParErrors();
  //  Double_t *temp = total->GetParErrors();
  //  ErrParams.insert(ErrParams.begin(), temp, temp + 6);

  if( par[2] < 0 ){
    cout << " warning: 1st sigma is " << par[2] << endl;
    par[2] = fabs(par[2]);
  }

  if( par[5] < 0 ){
    cout << " warning: 2nd sigma is " << par[5] << endl;
    par[5] = fabs(par[5]);
  }

  string name = h->GetName();

  TLegend leg(0.6,0.8,0.9,0.9);
  leg.AddEntry(h,title.c_str());
  leg.AddEntry(total,"double gaussian fit");

  std::string ssigma1, serr1;
  std::string ssigma2, serr2;

  // convert double to string                                                                                                                                       
  { std::ostringstream ssa1;
    ssa1 << rndup(par[2]);
    ssigma1 = ssa1.str();
  }

  { std::ostringstream ssb1;
    ssb1 << rndup(ErrParams[2]);
    serr1 = ssb1.str();
  }

  { std::ostringstream ssa2;
    ssa2 << rndup(par[5]);
    ssigma2 = ssa2.str();
  }

  { std::ostringstream ssb2;
    ssb2 << rndup(ErrParams[5]);
    serr2 = ssb2.str();
  }


  TPaveText *pt = new TPaveText(0.6,0.7,0.9,0.8, "NDC"); // NDC sets coords
  // relative to pad dimensions
  pt->SetFillColor(0); // text is black on white
  pt->SetTextSize(0.03); 
  pt->SetTextAlign(12);

  //  string text="sigma1 = "+ssigma1+" +- "+serr1+"\n sigma2 = "+ssigma2+" +- "+serr2;
  string text1="sigma large = "+ssigma1+" +- "+serr1 + " mm";
  string text2="sigma small = "+ssigma2+" +- "+serr2 + " mm";

  //  leg.SetHeader(text.c_str());
  //  leg.SetTextSize(0.03);

  pt->AddText(text1.c_str());
  pt->AddText(text2.c_str());



  h->Draw();
  leg.Draw("same");
  pt->Draw("same");       //to draw your text object
  h->Draw("same");

  c1->Update();

  cout << " fitting histogram: " << name << endl;
  cout << "   mean1 = " << par[1] << " +- " << ErrParams[1] << endl;
  cout << "   sigma1 = " << par[2] << " +- " << ErrParams[2] << endl;
  cout << "   mean2 = " << par[4] << " +- " << ErrParams[4] << endl;
  cout << "   sigma2 = " << par[5] << " +- " << ErrParams[5] << endl;


  *sigma1_ = par[2];
  *sigmaerr1_ = ErrParams[2];
  *sigma2_ = par[5];
  *sigmaerr2_ = ErrParams[5];


  c1->Print((direc+name+"_"+title+"Fit.gif").c_str());


  return;


}

void drawFour(TH1F* a, TH1F *b, TH1F *c, TH1F *d, string name, bool NormalizeArea, bool log){

  TLegend leg(0.6,0.6,1,0.8);

  a->SetLineColor(1);
  leg.AddEntry(a);
  b->SetLineColor(2);
  leg.AddEntry(b);
  c->SetLineColor(4);
  leg.AddEntry(c);
  d->SetLineColor(6);
  leg.AddEntry(d);


  if( NormalizeArea ){

    if( b->Integral() != 0 )
      b->Scale((a->Integral()*0.5)/(b->Integral()));
    if( c->Integral() != 0 )
      c->Scale((a->Integral()*2.)/(c->Integral()));
    if( d->Integral() != 0 )
      d->Scale((a->Integral()*3.)/(d->Integral()));
  }

  double maxa = a->GetMaximum();
  double maxb = b->GetMaximum();
  double maxc = c->GetMaximum();
  double maxd = d->GetMaximum();

  double max, min = 0;

  if( log ){

    double mina = getNonZeroMinimum(a);
    double minb = getNonZeroMinimum(b);
    double minc = getNonZeroMinimum(c);
    double mind = getNonZeroMinimum(d);
    

    if( mina <= minb && mina <= minc && mina <= mind ){
      min = mina;
    }
    else if( minb <= mina && minb <= minc && minb <= mind ){
      min = minb;
    }
    else if( minc <= mina && minc <= minb && minc <= mind ){
      min = minc;
    }
    else{ 
      min = mind;
    }
    
    min = min/2.;
  }

  if( maxa >= maxb && maxa >= maxc && maxa >= maxd ){
    max = maxa;
    a->GetYaxis()->SetRangeUser(min,1.5*maxa);
    a->GetYaxis()->SetTitle("");
    a->Draw();
  }
  else if( maxb >= maxa && maxb >= maxc && maxb >= maxd ){
    max = maxb;
    b->GetYaxis()->SetRangeUser(min,1.5*maxb);
    b->GetYaxis()->SetTitle("");
    b->Draw();
  }
  else if( maxc >= maxa && maxc >= maxb && maxc >= maxd ){
    max = maxc;
    c->GetYaxis()->SetRangeUser(min,1.5*maxc);
    c->GetYaxis()->SetTitle("");
    c->Draw();
  }
  else{ 
    max = maxd;
    d->GetYaxis()->SetRangeUser(min,1.5*maxd);
    d->GetYaxis()->SetTitle("");
    d->Draw();
  }

  if( log ){
    c1->SetLogy(1);
  }

  a->Draw("same");
  b->Draw("same");
  c->Draw("same");
  d->Draw("same");

  leg.Draw("same");
  //  c1->SetLogy(1);
  c1->Update();
  string title="fig/"+name+".gif";

  c1->Print(title.c_str());

  if( log )
    c1->SetLogy(0);

  return;
}


void drawFive(TH1F* a, TH1F *b, TH1F *c, TH1F *d, TH1F *e, string name, bool NormalizeArea, bool log){

  //  TLegend leg(0.6,0.55,1,0.75);
  TLegend leg(0.6,0.75,1,0.95);

  a->SetLineColor(1);
  leg.AddEntry(a);
  b->SetLineColor(2);
  leg.AddEntry(b);
  c->SetLineColor(4);
  leg.AddEntry(c);
  d->SetLineColor(6);
  leg.AddEntry(d);
  e->SetLineColor(7);
  leg.AddEntry(e);


  if( NormalizeArea ){

    if( b->Integral() != 0 )
      b->Scale((a->Integral()*0.5)/(b->Integral()));
    if( c->Integral() != 0 )
      c->Scale((a->Integral()*2.)/(c->Integral()));
    if( d->Integral() != 0 )
      d->Scale((a->Integral()*3.)/(d->Integral()));
    if( e->Integral() != 0 )
      e->Scale((a->Integral()*3.)/(e->Integral()));
  }

  double maxa = a->GetMaximum();
  double maxb = b->GetMaximum();
  double maxc = c->GetMaximum();
  double maxd = d->GetMaximum();
  double maxe = e->GetMaximum();

  double max, min = 0;

  if( log ){

    double mina = getNonZeroMinimum(a);
    double minb = getNonZeroMinimum(b);
    double minc = getNonZeroMinimum(c);
    double mind = getNonZeroMinimum(d);
    double mine = getNonZeroMinimum(e);
    

    if( mina <= minb && mina <= minc && mina <= mind && mina <= mine){
      min = mina;
    }
    else if( minb <= mina && minb <= minc && minb <= mind  && minb <= mine ){
      min = minb;
    }
    else if( minc <= mina && minc <= minb && minc <= mind  && minc <= mine){
      min = minc;
    }
    else if( mind <= mina && mind <= minb && mind <= minc  && mind <= mine){
      min = mind;
    }
    else{ 
      min = mine;
    }
    
    min = min/2.;
  }

  if( maxa >= maxb && maxa >= maxc && maxa >= maxd && maxa >= maxe){
    max = maxa;
    a->GetYaxis()->SetRangeUser(min,1.5*maxa);
    a->GetYaxis()->SetTitle("");
    a->Draw();
  }
  else if( maxb >= maxa && maxb >= maxc && maxb >= maxd && maxb >= maxe){
    max = maxb;
    b->GetYaxis()->SetRangeUser(min,1.5*maxb);
    b->GetYaxis()->SetTitle("");
    b->Draw();
  }
  else if( maxc >= maxa && maxc >= maxb && maxc >= maxd && maxc >= maxe){
    max = maxc;
    c->GetYaxis()->SetRangeUser(min,1.5*maxc);
    c->GetYaxis()->SetTitle("");
    c->Draw();
  }
  else if( maxd >= maxa && maxd >= maxb && maxd >= maxc && maxd >= maxe){
    max = maxd;
    d->GetYaxis()->SetRangeUser(min,1.5*maxd);
    d->GetYaxis()->SetTitle("");
    d->Draw();
  }
  else{ 
    max = maxe;
    e->GetYaxis()->SetRangeUser(min,1.5*maxe);
    e->GetYaxis()->SetTitle("");
    e->Draw();
  }

  if( log ){
    c1->SetLogy(1);
  }

  a->Draw("same");
  b->Draw("same");
  c->Draw("same");
  d->Draw("same");
  e->Draw("same");

  leg.Draw("same");
  //  c1->SetLogy(1);
  c1->Update();
  string title="fig/"+name+".gif";

  c1->Print(title.c_str());

  if( log )
    c1->SetLogy(0);

  return;
}



Double_t FScalaPolverePiatto ( Double_t *x, Double_t *parm) { 
  // 0 a_0
  // 1 t_0
  double val = parm[0]*pow(x[0],2./3.);
  return val; 
} 



Double_t FScalaRadiazionePiatto ( Double_t *x, Double_t *parm) { 
  // 0 a_0
  // 1 t_0
  double val = parm[0]*sqrt(x[0]);
  return val; 
} 


Double_t FScalaVuotoPiatto ( Double_t *x, Double_t *parm) { 
  // 0 a_0
  // 1 t_0
  // 2 nu_H
  double val = parm[0]*exp(parm[2]*x[0]);
  return val; 
} 


Double_t FScalaPolvereSferico ( Double_t *x, Double_t *parm) { 
  // 0 a_0
  // 1 t_0
  // 2 tmax

  double eta = 0;
  double min = 999999999999.;
  for(size_t istep=0; istep<1000.; istep++){
    double local_eta = istep*parm[2]/(500.);
    double local_diff = fabs(parm[0]/2.*( local_eta - sin(local_eta)) - x[0]);

    if( local_diff < min )
      {
	min = local_diff;
	eta = local_eta;
      }
  }

  double val = parm[0]/2.*(1 - cos(eta));

  return val; 
} 



Double_t FScalaRadiazioneSferico ( Double_t *x, Double_t *parm) { 
  // 0 a_0
  // 1 t_0
  double val = parm[0]*sqrt(1 - pow(1 - x[0]/parm[0],2.));

  return val; 
} 


Double_t FScalaVuotoSferico ( Double_t *x, Double_t *parm) { 
  // 0 a_0
  // 1 t_0
  // 2 nu_H
  double val = parm[0]*(exp((x[0]/parm[0] + parm[1])) + exp(-(x[0]/parm[0] + parm[1])))/2.;
  return val; 
} 

Double_t FScalaPolvereIperbolico ( Double_t *x, Double_t *parm) { 
  // 0 a_0
  // 1 t_0
  // 2 tmax

  double eta = 0;
  double min = 999999999999.;
  for(size_t istep=0; istep<1000.; istep++){
    double local_eta = istep*parm[2]/(500.);
    double local_diff = fabs(parm[0]/2.*( (exp(local_eta) - exp(-local_eta))/2. - local_eta - x[0]));

    if( local_diff < min )
      {
	min = local_diff;
	eta = local_eta;
      }
  }



  double val = parm[0]/2.*((exp(eta) + exp(-eta))/2. - 1);

  return val; 
} 



Double_t FScalaRadiazioneIperbolico ( Double_t *x, Double_t *parm) { 
  // 0 a_0
  // 1 t_0
  double val = parm[0]*sqrt(pow(1 + x[0]/parm[0],2.) - 1);

  return val; 
} 


Double_t FScalaVuotoIperbolico ( Double_t *x, Double_t *parm) { 
  // 0 a_0
  // 1 t_0
  // 2 nu_H
  double val = parm[0]*(exp(x[0]/parm[0]) - exp(-x[0]/parm[0]))/2.;
  return val; 
} 

Double_t FFOT ( Double_t *x, Double_t *parm) { 
  // 0  L
  // 1 r_s
  // x = r / r_s

  double val = parm[0]*parm[0]/(2*x[0]*x[0]*x[0]*parm[1]*parm[1]*parm[1])*(x[0] - 1)*parm[1];

  return val; 

} 


Double_t BEdist ( Double_t *x, Double_t *parm) { 
  // 0   eing/kT

  double val = 1./(exp(parm[0]*(x[0]-1.)) - 1.);

  if( x[0] <= 1. ) val = 999; // sopra il limite perche non si veda lo zero

  return val; 

} 


Double_t FMASSIVE ( Double_t *x, Double_t *parm) { 
  // 0  L
  // 1 r_s
  // 2 c
  // x = r / r_s

  if( x[0] == 0. ) return 0.;

  double val = 1./2.*(1 - 1./x[0])*(parm[2]*parm[2] + parm[0]*parm[0]/(x[0]*x[0]*parm[1]*parm[1]));

  return val; 

} 

Double_t MSferaRuot ( Double_t *x, Double_t *parm) { 

  double val1 = 1./pow(x[0],2);
  double val2 = (1. - pow(x[0],2))/(2.*pow(x[0],3));
  double val3 = (1. + x[0])/(1. - x[0]);

  return 3./2.*(val1 - val2*log(val3));


} 


string double_to_string(double val){
  char c[20];
  sprintf(c,"%.3g",val);
  string sval(c);
  return sval;
}

string get_diam_title(double diameterMil){

  return double_to_string(diameterMil);
}


double g(){
  return 9.8;
}

double pi(){
  return 3.1417;
}


vector<string> read_string_file(string filename){

  vector<string> list;

  string word;
  ifstream wordfile(filename.c_str());
  while (wordfile >> word) {
    list.push_back(word);
  }

  return list;

}

string get_wires_title(string ftitle, double diameterMil){

  string DiamTitle = get_diam_title(diameterMil);

  string title = "../fig/"+ftitle+"_"+DiamTitle;

  return title;
}

double get_yield_weight(double diameterMil, double factor){
  double diameter = diameterMil*factor;
  double yield_strength_expected = 5.20e8;
  double yield_weight = yield_strength_expected*pi()*diameter*diameter*1e-6/(4*g());
  return yield_weight;
}



double get_ultimate_weight(double diameterMil, double factor){
  double diameter = diameterMil*factor;
  double ultimate_tensile_strength_expected = 8.60e8;
  double ultimate_weight = ultimate_tensile_strength_expected*pi()*diameter*diameter*1e-6/(4*g());
  return ultimate_weight;
}


Int_t do_wires(double diameterMil, string xt, string yt, string ftitle, string formula, string filen, double factor, size_t id, std::vector<string> dates){

  TCanvas c2("c2","c2",100,100,800,600);

  double leg_xmin = 0.8;
  double leg_xmax = 1.;
  double leg_ymin = 0.1;
  double leg_ymax = 0.3;
  if( diameterMil == 5.){
    leg_xmin = 0.1;
    leg_xmax = 0.3;
    leg_ymin = 0.7;
    leg_ymax = 0.9;
  }
  TLegend leg(leg_xmin, leg_ymin, leg_xmax, leg_ymax);
  leg.SetFillColor(4000);

  TPaveText pt(0.3, 0.9, 0.9, 1., "NDC");
  pt.SetFillColor(kOrange); // text is black on white
  pt.SetTextSize(0.03);
  pt.SetTextAlign(12);
  string ptitle = "elongation_vs_weight for wire of thickness "+get_diam_title(diameterMil)+" mil";

  if( diameterMil == 10.001 )
    ptitle = "elongation_vs_weight for Malin wire of thickness 10 mil";
  if( diameterMil == 10. )
    ptitle = "elongation_vs_weight for Fairbanks wire of thickness 10 mil";
  if( diameterMil == 10.002 )
    ptitle = "elongation_vs_weight for Fairbanks wire of thickness 10 mil";
  if( diameterMil == 15.001 )
    ptitle = "elongation_vs_weight for Fairbanks wire of thickness 15 mil";
  if( diameterMil == 15.002 )
    ptitle = "elongation_vs_weight for Fairbanks wire of thickness 15 mil";

  string title = get_wires_title(ftitle, diameterMil)+".cumulative.jpeg";
  pt.AddText(ptitle.c_str());

  double diameter = diameterMil*factor;
  double yield_strength_expected = 5.20e8;
  double yield_weight = get_yield_weight(diameterMil,factor);
  
  string syield = double_to_string(yield_weight);


  double ultimate_weight = get_ultimate_weight(diameterMil,factor);

  string sultimate  = double_to_string(ultimate_weight);

  cout << " expected elasticity point " << yield_weight << " kg,  ultimate deformation at " << ultimate_weight << " kg " << endl;

  TH2F histos[dates.size()];
  TGraph graphs[dates.size()];
  TH2F histo_all;
  TGraph graph_all;

  vector<double> xsall;
  vector<double> ysall;

  double max_histos = 0;
  double max_y_histos = 0;
  double idatemax = 0;

  for(size_t idate=0; idate<dates.size(); idate++){
    cout << " doing date " << dates[idate] << endl; fflush(stdout);

    string date = dates[idate];

    vector<double> xs;
    vector<double> ys;

    get_wire_data(diameterMil, filen, factor, date, &xs, &ys);

    size_t s = xs.size();
    
    double L = initial_length(diameterMil, date);

    for(size_t i=0; i<s; i++){
      xsall.push_back(xs[i]);
      ysall.push_back(ys[i]);
    }

    for(size_t i=0; i<s; i++){
      cout << " x " << xs[i] << " y " << ys[i] << endl;
    }
    
    double max_x = get_ultimate_weight(diameterMil,factor);

    double min = xs[0];
    double max = xs[s-1];
    double max_histo = max;

    if( max_histo > max_histos ){
      max_histos = max_histo;
    }

    if( ys[s-1] > max_y_histos ){
      max_y_histos = ys[s-1];
      idatemax = idate;
    }

    cout << " size " << s << " last xs " << xs[s-1] << " last ys " << ys[s-1] << endl;

    TH2F histo;
    
    fill_xy_histo(diameterMil, xt, yt, ftitle, factor, id, date, &histo, xs, ys);
    string title = get_wires_title(ftitle, diameterMil)+"."+date+".jpeg";
    histos[idate] = histo;

    TGraph graph;
    copy_to_graph(diameterMil, xt, yt, ftitle, factor, id, date, &graph, max_x, xs, ys);
    graphs[idate] = graph;

    Color_t mycolor = get_wire_color(idate, date, diameterMil);
    Style_t mystyle = get_wire_style(idate, date, diameterMil);
    Size_t mysize = get_wire_size(idate, date, diameterMil);

    histos[idate].SetMarkerColor(mycolor);
    histos[idate].SetMarkerStyle(mystyle);
    histos[idate].SetMarkerSize(mysize);

    graphs[idate].SetLineColor(mycolor);
    graphs[idate].SetLineStyle(mystyle);
    graphs[idate].SetLineWidth(mysize);

    leg.AddEntry(&graphs[idate], dates[idate].c_str(),"L");
  }


  histos[idatemax].Draw();


  TLine elastic_point(yield_weight, 0, yield_weight, 100);
  elastic_point.SetLineStyle(2);
  elastic_point.SetLineColor(kPink - 2);
  elastic_point.SetLineWidth(10);
  if( yield_weight > histos[0].GetXaxis()->GetXmin() ){
    elastic_point.Draw("same");
  }

  leg.AddEntry(&elastic_point, ("exp elasticity at "+syield+" kg").c_str(), "l");
  TLine breaking_point(ultimate_weight, 0, ultimate_weight, 100);
  breaking_point.SetLineStyle(2);
  breaking_point.SetLineColor(kGray);
  breaking_point.SetLineWidth(10);

  breaking_point.Draw("same");

  for(size_t idate=0; idate<dates.size(); idate++){
    histos[idate].Draw("same");
    graphs[idate].Draw("same L");
  }

  leg.AddEntry(&breaking_point, ("exp ult def at "+sultimate+" kg").c_str(), "l");
  pt.Draw();
  leg.Draw();
  c2.SetTitle(title.c_str());
  c2.Update();
  c2.Print(title.c_str());

  fill_xy_histo(diameterMil, xt, yt, ftitle, factor, id, date, &histo_all, xsall, ysall);

  fit_wires(diameterMil, xt, yt, ftitle, formula, filen, factor, id, histo_all);
  
  return 1;

}

Color_t get_color(size_t index, Color_t mycolor){
  
  if( index < 5)
    mycolor += index;
  else if( index < 15 )
    mycolor -= (index - 5);
  else if( index < 20 )
    mycolor += (index - 15);
  else if( index < 30 )
    mycolor -= (index - 20);
  
  return mycolor;
  
}

Color_t get_wire_color(size_t index, string date, double diameterMil){
  
  Color_t mycolor = kOrange;
  if( epoch(diameterMil, date, "highlight") )
    mycolor = kBlue;;

  if( index < 10)
    mycolor += index;
  else
    mycolor -= (index - 10);

  
  return mycolor;
  
}

Size_t get_wire_size(size_t index, string date, double diameterMil){
  
  Size_t mysize = 1;
  if( epoch(diameterMil, date, "highlight") )
    mysize = 1;
  
  return mysize;
  
}

Style_t get_wire_style(size_t index, string date, double diameterMil){
  
  Style_t mystyle = 33;
  if( epoch(diameterMil, date, "highlight") )
    mystyle = 20;
  
  return mystyle;
  
}

bool epoch(double diameterMil, string date, string epoch){

  if( !strcmp(epoch.c_str(), "no_wrapping_around_brass")){
    if( diameterMil == 10. && 
	( !strcmp(date.c_str(), "Oct20a") || !strcmp(date.c_str(), "Oct20b") || !strcmp(date.c_str(), "Oct20c") || !strcmp(date.c_str(), "Oct31a") || !strcmp(date.c_str(), "Oct31b") || !strcmp(date.c_str(), "Oct31c") || !strcmp(date.c_str(), "Oct31d") )	){
      cout << " wire " << diameterMil << " date " << date << " is in epoch " << epoch << endl;
      return true;
    }
    
    return false;
  }
  if( !strcmp(epoch.c_str(), "inchscale")){
    if( diameterMil == 5. ){
      cout << " wire " << diameterMil << " date " << date << " is in epoch " << epoch << endl;
      return true;
    }
    
    if( diameterMil == 10.001 && (!strcmp(date.c_str(), "Aug31") || !strcmp(date.c_str(), "Sep27d"))){
      cout << " wire " << diameterMil << " date " << date << " is in epoch " << epoch << endl;
      return true;
    }
    
    if( diameterMil == 10.002 && (!strcmp(date.c_str(), "Nov03a") 
				  || !strcmp(date.c_str(), "Nov03b")  
				  || !strcmp(date.c_str(), "Nov03e")  
				  || !strcmp(date.c_str(), "Nov03g")  
				  || !strcmp(date.c_str(), "Nov03h") 
				  || !strcmp(date.c_str(), "Nov03i") 
				  || !strcmp(date.c_str(), "Nov03l") 
				  || !strcmp(date.c_str(), "Nov03m") 
				  || !strcmp(date.c_str(), "Nov03n")  ) ){
      cout << " wire " << diameterMil << " date " << date << " is in epoch " << epoch << endl;
      return true;
    }
    
    if( diameterMil == 15. && strcmp(date.c_str(), "Sep27d")){
      cout << " wire " << diameterMil << " date " << date << " is in epoch " << epoch << endl;
      return true;
    }
    
    if( diameterMil == 15.001 ){
      cout << " wire " << diameterMil << " date " << date << " is in epoch " << epoch << endl;
      return true;
    }

    if( diameterMil == 15.002 ){
      
      if( !strcmp(date.c_str(), "Nov10e") ||
	  !strcmp(date.c_str(), "Nov10g") ||
	  !strcmp(date.c_str(), "Nov10i") ){
	
	return false;
      }
      
      return true;
    }
  }
  if( !strcmp(epoch.c_str(), "small_base")){
    if( diameterMil == 5.){
      cout << " wire " << diameterMil << " date " << date << " is in epoch " << epoch << endl;
      return true;
    }
    
    if( diameterMil == 10.002 ){
      cout << " wire " << diameterMil << " date " << date << " is in epoch " << epoch << endl;
      return true;
    }
    
    return false;
  }
  if( !strcmp(epoch.c_str(), "middle_base")){
    if( diameterMil == 10.001 ){
      cout << " wire " << diameterMil << " date " << date << " is in epoch " << epoch << endl;
      return true;
    }

    if( diameterMil == 10.){
      cout << " wire " << diameterMil << " date " << date << " is in epoch " << epoch << endl;
      return true;
    }

    if( diameterMil == 15.001  ){
      return false;
    }

    if( diameterMil == 15.002  ){
      cout << " wire " << diameterMil << " date " << date << " is in epoch " << epoch << endl;
      return true;
    }

    return false;
  }
  if( !strcmp(epoch.c_str(), "big_base")){
    if( diameterMil == 15.){
      cout << " wire " << diameterMil << " date " << date << " is in epoch " << epoch << endl;
      return true;
    }

    if( diameterMil == 15.001  ){
      cout << " wire " << diameterMil << " date " << date << " is in epoch " << epoch << endl;
      return true;
    }

    if( diameterMil == 15.002  ){
      return false;
    }

    return false;
  }
  if( !strcmp(epoch.c_str(), "highlight")){
    return epoch(diameterMil, date, "no_wrapping_around_brass");
  }
  
  return false;  
  
}


double get_base_weight(double diameterMil, string date){

  double offset_x = 0.;

  bool done = false;

  if( epoch(diameterMil,date,"small_base") ){
    done = true;
    offset_x = 0.3315;
  }
  if( epoch(diameterMil,date,"middle_base") ){
    if( done ){
      cout << " warning wire " << diameterMil << " date " << date << " has two bases " << endl;
      exit(0);
    }
    done = true;
    offset_x = 1.0375;
  }
  if( epoch(diameterMil,date,"big_base") ){
    if( done ){
      cout << " warning wire " << diameterMil << " date " << date << " has two bases " << endl;
      exit(0);
    }
    done = true;
    //    offset_x = 5.45;
    offset_x = 5.9345;
  }


  return offset_x;

}

void get_wire_data(double diameterMil, string filen, double factor, string date, vector<double>* xs, vector<double>* ys){ 

  xs->clear();
  ys->clear();

  double diameter = diameterMil*factor;

  string DiamTitle = get_diam_title(diameterMil);

  double L = initial_length(diameterMil, date);

  string filename = filen+"_"+DiamTitle+"."+date+".txt";
  std::ifstream inFile(filename.c_str());
  
  // Make sure the file stream is good
  if(!inFile) {
    cout << "Failed to open file " << filename << endl;
    return 0;
  }

  bool skip_first_point = false;
  if( diameterMil == 10.002 ){
    skip_first_point = true;
    ;
  }

  double local_x = 0;
  double local_y = 0;
  size_t cou = 0;
  double offset_x = get_base_weight(diameterMil, date);
  double offset_y = 0.;

  double weight_correction = 1.;

#if 1
  weight_correction = 1.15;
  // weights are wrong; nominal 0.1 kg corresponds to 0.115 kg
#endif

  while(!inFile.eof()) {
    inFile >> local_x;
    local_x *= weight_correction;

    local_x += offset_x;
    if( cou > 0 || ! skip_first_point)
      xs->push_back(local_x);

    inFile >> local_y;
    double tmo2 = local_y;
    if( inch_scale(diameterMil, date) ){
      local_y *= factor;
    }
    if( cou == 1 && skip_first_point){
      double tmp = offset_y;
      offset_y -= local_y;
    }

    local_y += offset_y;
    local_y /= L;
    if( cou > 0 || ! skip_first_point)
      ys->push_back(local_y);

    cou++;
  }

  size_t s = xs->size();

  xs->erase(xs->begin()+s-1);  
  ys->erase(ys->begin()+s-1);  
  
  s = xs->size();

  if( s != ys->size() ){
    cout << " xs size " << s << " ys size " << ys->size() << endl;
    return 0;
  }
  

  return;
}

bool inch_scale(double diameterMil, string date){

  return epoch(diameterMil, date, "inchscale");

}


void get_time_data(double diameterMil, double factor, string date, vector<double>* xs, vector<double>* ys){ 

  xs->clear(); // time
  ys->clear(); // length

  double diameter = diameterMil*factor;

  string DiamTitle = get_diam_title(diameterMil);

  double L = initial_length(diameterMil, date);

  string filen="data/EvsT";

  string filename = filen+"_"+DiamTitle+"."+date+".txt";

  std::ifstream inFile(filename.c_str());
  
  // Make sure the file stream is good
  if(!inFile) {
    cout << "Failed to open file " << filename << endl;
    return 0;
  }

  double local_x = 0;
  double local_y = 0;
  double offset_y = 0.;

  bool is = inch_scale(diameterMil, date);


  while(!inFile.eof()) {
    inFile >> local_x;
    xs->push_back(local_x);

    inFile >> local_y;
    double tmo2 = local_y;
    if( is ){
      local_y *= factor;
    }

    local_y += offset_y;
    local_y /= L;
    ys->push_back(local_y);

  }

  size_t s = xs->size();

  xs->pop_back();
  ys->pop_back();
  
  s = xs->size();

  if( s != ys->size() ){
    cout << " xs size " << s << " ys size " << ys->size() << endl;
    return 0;
  }
  

  return;
}

bool inch_scale(double diameterMil, string date){

  return epoch(diameterMil, date, "inchscale");

}

double initial_length(double diameterMil, string date){

  double L = 0.;

  if( !strcmp(date.c_str(), "Aug15" ) ) 
    L = 2.325;
  else if( !strcmp(date.c_str(), "Aug30" ) ){
    L = 2.690;
  }
  else if( !strcmp(date.c_str(), "Aug31" ) ){
    if( diameterMil == 5.)
      L = 2.270;
    else if( diameterMil == 10.001)
      L = 2.240;
    else if( diameterMil == 15.)
      L = 2.080;
  }
  else if( !strcmp(date.c_str(), "Sep27a" ) ){
    if( diameterMil == 10.001)
      L = 2.150;
    else if( diameterMil == 15.)
      L = 2.140;
  }
  else if( !strcmp(date.c_str(), "Sep27b" ) ){
    if( diameterMil == 10.001)
      L = 2.150;
    else if( diameterMil == 15.)
      L = 2.140;
  }
  else if( !strcmp(date.c_str(), "Sep27c" ) ){
    if( diameterMil == 10.001)
      L = 2.135;
    else if( diameterMil == 15.)
      L = 2.170;
  }
  else if( !strcmp(date.c_str(), "Sep27d" ) ){
    if( diameterMil == 10.001)
      L = 2.110;
    else if( diameterMil == 15.)
      L = 2.210;
  }
  else if( !strcmp(date.c_str(), "Sep28a" ) ){
    if( diameterMil == 10.001)
      L = 2.170;
    else if( diameterMil == 15.)
      L = 2.250;
  }
  else if( !strcmp(date.c_str(), "Sep28b" ) ){
    if( diameterMil == 10.001)
      L = 2.050;
    else if( diameterMil == 15.)
      L = 2.050;
  }
  else if( !strcmp(date.c_str(), "Sep28c" ) ){
    if( diameterMil == 10.001)
      L = 2.080;
    else if( diameterMil == 15.)
      L = 2.160;
  }
  else if( !strcmp(date.c_str(), "Oct17a" ) ){
    L = 2.060;
  }
  else if( !strcmp(date.c_str(), "Oct17b" ) ){
    L = 2.160;
  }
  else if( !strcmp(date.c_str(), "Oct17c" ) ){
    L = 2.070;
  }
  else if( !strcmp(date.c_str(), "Oct17d" ) ){
    L = 2.150;
  }
  else if( !strcmp(date.c_str(), "Oct18a" ) ){
    L = 2.075;
  }
  else if( !strcmp(date.c_str(), "Oct18b" ) ){
    L = 2.145;
  }
  else if( !strcmp(date.c_str(), "Oct18c" ) ){
    L = 2.060;
  }
  else if( !strcmp(date.c_str(), "Oct18d" ) ){
    L = 2.045;
  }
  else if( !strcmp(date.c_str(), "Oct18e" ) ){
    L = 2.185;
  }
  else if( !strcmp(date.c_str(), "Oct18f" ) ){
    L = 2.155;
  }
  else if( !strcmp(date.c_str(), "Oct20a" ) ){
    L = 1.995;
  }
  else if( !strcmp(date.c_str(), "Oct20b" ) ){
    L = 2.165;
  }
  else if( !strcmp(date.c_str(), "Oct20c" ) ){
    L = 2.080;
  }
  else if( !strcmp(date.c_str(), "Oct31a" ) ){
    L = 2.190;
  }
  else if( !strcmp(date.c_str(), "Oct31b" ) ){
    L = 2.115;
  }
  else if( !strcmp(date.c_str(), "Oct31c" ) ){
    L = 2.175;
  }
  else if( !strcmp(date.c_str(), "Oct31d" ) ){
    L = 2.170;
  }
  else if( !strcmp(date.c_str(), "Nov03a" ) ){
    L = 2.140;
  }
  else if( !strcmp(date.c_str(), "Nov03b" ) ){
    L = 2.135;
  }
  else if( !strcmp(date.c_str(), "Nov03c" ) ){
    L = 2.290;
  }
  else if( !strcmp(date.c_str(), "Nov03d" ) ){
    L = 2.290;
  }
  else if( !strcmp(date.c_str(), "Nov03e" ) ){
    L = 2.290; // directly from screw at bottom 
  }
  else if( !strcmp(date.c_str(), "Nov03f" ) ){
    L = 2.280; 
  }
  else if( !strcmp(date.c_str(), "Nov03g" ) ){
    L = 2.250; 
  }
  else if( !strcmp(date.c_str(), "Nov03h" ) ){
    L = 2.240; 
  }
  else if( !strcmp(date.c_str(), "Nov03i" ) ){
    L = 2.240; 
  }
  else if( !strcmp(date.c_str(), "Nov03l" ) ){
    L = 2.250; 
  }
  else if( !strcmp(date.c_str(), "Nov03m" ) ){
    L = 2.380; 
  }
  else if( !strcmp(date.c_str(), "Nov03n" ) ){
    L = 2.330; 
  }
  else if( !strcmp(date.c_str(), "Nov10a" ) ){
    L = 2.160; 
  }
  else if( !strcmp(date.c_str(), "Nov10b" ) ){
    L = 2.180; 
  }
  else if( !strcmp(date.c_str(), "Nov10c" ) ){
    L = 2.260; 
  }
  else if( !strcmp(date.c_str(), "Nov10d" ) ){
    L = 2.090; 
  }
  else if( !strcmp(date.c_str(), "Nov10e" ) ){
    L = 2.250; 
  }
  else if( !strcmp(date.c_str(), "Nov10f" ) ){
    L = 2.210; 
  }
  else if( !strcmp(date.c_str(), "Nov10g" ) ){
    L = 2.310; 
  }
  else if( !strcmp(date.c_str(), "Nov10h" ) ){
    L = 2.150; 
  }
  else if( !strcmp(date.c_str(), "Nov10i" ) ){
    L = 2.120; 
  }

  return L;
}

void copy_to_graph(double diameterMil, string xt, string yt, string ftitle, double factor, size_t id, string date, TGraph* graph, double max_x, vector<double> xs, vector<double> ys){

  double diameter = diameterMil*factor;

  size_t s = xs.size();

  double yerr = 0.01; // mm                                                  
  if( inch_scale(diameterMil, date) )
    yerr = 1*factor;  // 1 mil                                             

  string DiamTitle = get_diam_title(diameterMil);
  if( diameterMil = 10.001 )
    DiamTitle="10";
  if( diameterMil = 10.002 )
    DiamTitle="10";

  double max = 0;
  double min = 9999999999.;
  for(size_t i=0; i<s; i++){
    if( xs[i] > max )
      max = xs[i];
    if( xs[i] < min )
      min = xs[i];
  }

  double min_y = 9999999999.;
  double max_y = 0;
  for(size_t i=0; i<s; i++){
    if( ys[i] > max_y )
      max_y = ys[i];
    if( ys[i] < min_y )
      min_y = ys[i];
  }

  double max_histo = max;
  double min = xs[0];
  double min_y = ys[0];

  string htit = "thickness = "+DiamTitle+" mil";

  Double_t* x = (Double_t*)malloc(sizeof(Double_t)*s);
  Double_t* y = (Double_t*)malloc(sizeof(Double_t)*s);

  for(size_t i=0; i<s; i++){
    x[i] = xs[i];
    y[i] = ys[i];
  }

  TGraph YvsX(xs.size(), x, y);
  
  YvsX.SetMarkerColor(kBlue);
  YvsX.SetLineColor(kBlue);
  YvsX.SetMarkerStyle(33);
  YvsX.SetMarkerSize(1);

  TGraph *tmp = YvsX.Clone("graph");
  *graph = YvsX;

  free(x);
  free(y);

  return;


}

Int_t fill_xy_histo(double diameterMil, string xt, string yt, string ftitle, double factor, size_t id, string date, TH2F* histo, vector<double> xs, vector<double> ys){

  double diameter = diameterMil*factor;

  size_t s = xs.size();

  double yerr = 0.01; // mm                                                  
  if( inch_scale(diameterMil, date) )
    yerr = 1*factor;  // 1 mil                                             

  string DiamTitle = get_diam_title(diameterMil);
  if( diameterMil = 10.001 )
    DiamTitle="10";
  if( diameterMil = 10.002 )
    DiamTitle="10";

  if( diameterMil = 15.001 )
    DiamTitle="15";
  if( diameterMil = 15.002 )
    DiamTitle="15";

  double max = 0;
  double min = 9999999999.;
  for(size_t i=0; i<s; i++){
    if( xs[i] > max )
      max = xs[i];
    if( xs[i] < min )
      min = xs[i];
  }

  double min_y = 9999999999.;
  double max_y = 0;
  for(size_t i=0; i<s; i++){
    if( ys[i] > max_y )
      max_y = ys[i];
    if( ys[i] < min_y )
      min_y = ys[i];
  }

  double max_histo = max;

  string htit = "thickness = "+DiamTitle+" mil";

  TH2F YvsX("YvsX", htit.c_str(), 100, min, max_histo, 100, min_y, max_y);
  
  YvsX.GetXaxis()->SetTitle(xt.c_str());
  YvsX.GetXaxis()->SetTitleSize(0.04);
  YvsX.GetYaxis()->SetTitle(yt.c_str());
  YvsX.GetYaxis()->SetTitleSize(0.04);
  YvsX.SetMarkerColor(kBlue);
  YvsX.SetLineColor(kBlue);
  YvsX.SetMarkerStyle(33);
  YvsX.SetMarkerSize(1);
  
  for(size_t i=0; i<s; i++){
    YvsX.Fill(xs[i], ys[i]);
  }
  
  TH2F *tmp = YvsX.Clone("histo");
  *histo = *tmp;

  return 1;
}


Int_t fit_wires(double diameterMil, string xt, string yt, string ftitle, string formula, string filen, double factor, size_t id, TH2F YvsX){



  TCanvas c1("c1","c1",100,100,800,600);

  TLegend leg(0.1,0.8,0.5,1.);
  leg.SetFillColor(kOrange);
  leg.SetTextSize(0.03);
  
  double diameter = diameterMil*factor;

  double yerr = 0.01; // mm                                                  
  //  if( inch_scale(diameterMil, date) )
  //    yerr = 1*factor;  // 1 mil                                             

  string DiamTitle = get_diam_title(diameterMil);

  YvsX.Draw("e");

  double min = YvsX.GetXaxis()->GetXmin();
  double max = YvsX.GetXaxis()->GetXmax();

  TF1 line("line",formula.c_str(),min,max);
  line.SetLineColor(kRed);
  line.SetLineStyle(2);
  if( formula=="[0] + [1]*x"){
    line.SetParameter(0,0);
    line.SetParameter(1,YvsX.GetYaxis()->GetXmax()/max);
  }
  else if( formula=="[2] + [0]*(1 - exp(-x/[1]))"){
    line.SetParameter(0,ys[s-1]);
    line.SetParameter(1,xs[s-1]);
    line.SetParameter(2,ys[1]);
    double mymin = 0.99*ys[0];
    double mymax = 1.015*ys[s-1];
    cout << " mymin " << mymin << " mymax " << mymax << endl;
    YvsX.GetYaxis()->SetRangeUser(mymin, mymax);
  }
  double max_fit = get_max_fit(diameterMil);
  

  YvsX.Fit("line","R","",min,max_fit);

  leg.AddEntry(&YvsX, YvsX.GetTitle(), "l");
  leg.AddEntry(&line, "linear fit", "l");
    
  Int_t ndof = get_entries_below(max_fit, YvsX);

  double p0 = line.GetParameter(0);
  double p1 = line.GetParameter(1);
  double ep0 = line.GetParError(0);
  double ep1 = line.GetParError(1);
  double chi2 = line.GetChisquare();
  double prob = TMath::Prob(chi2,ndof);
  if( formula=="[2] + [0]*(1 - exp(-x/[1]))"){
    cout << " time constant = " << p1/60. << " +- " << ep1/60. << " min, so wait for " << 6*p1/60. << " min " << endl;
  }
  cout << " chi2 " << chi2 << " ndof " << ndof << " prob " << prob << endl;
  string sndof  = double_to_string(ndof);
  
  string schi2 = double_to_string(chi2);
  
  string sprob = double_to_string(prob);
  
  string sp0 = double_to_string(p0);
  string sp1 = double_to_string(p1);
  
  string sep0 = double_to_string(ep0);
  string sep1 = double_to_string(ep1);
  
  double pt_xmin = 0.1;
  double pt_xmax = 0.55;
  double pt_ymin = 0.65;
  double pt_ymax = 0.8;
  
  
  double yield_strength_expected = 5.20e8;
  double yield_weight = get_yield_weight(diameterMil,factor);
  
  string syield = double_to_string(yield_weight);


  double ultimate_weight = get_ultimate_weight(diameterMil,factor);

  string sultimate  = double_to_string(ultimate_weight);

  TLine elastic_point(yield_weight, 0, yield_weight, 100);
  elastic_point.SetLineStyle(2);
  elastic_point.SetLineColor(kPink - 2);
  elastic_point.SetLineWidth(10);
  elastic_point.Draw("same");

  leg.AddEntry(&elastic_point, ("exp elasticity at "+syield+" kg").c_str(), "l");
  TLine breaking_point(ultimate_weight, 0, ultimate_weight, 100);
  breaking_point.SetLineStyle(2);
  breaking_point.SetLineColor(kGray);
  breaking_point.SetLineWidth(10);

  breaking_point.Draw("same");
  leg.AddEntry(&breaking_point, ("exp breaking point at "+sultimate+" kg").c_str(), "l");



  TPaveText pt(pt_xmin, pt_ymin, pt_xmax, pt_ymax, "NDC");
  pt.SetFillColor(4100); // text is black on white
  pt.SetTextSize(0.03);
  pt.SetTextAlign(12);
  string text0="p_{0} = "+sp0+" +- "+sep0;
  string text1="p_{1} = "+sp1+" +- "+sep1;
  string text2="#chi^{2} = "+schi2+" / ndof = "+sndof;
  string text3="prob = "+sprob;
  pt.AddText(text0.c_str());
  pt.AddText(text1.c_str());
  pt.AddText(text2.c_str());
  pt.AddText(text3.c_str());
  if( formula=="[0] + [1]*x"){
    if( p1 != 0. ){
      double young=4*g()/(pi()*diameter*diameter*p1*1.e-9);
      double eyoung = young*ep1/(p1*p1);
      
      cout << " young module = " << young << " +- " << eyoung << " Pa " << endl;
      
      string syoung = double_to_string(young);
      string seyoung = double_to_string(eyoung);
      
      string text4="Young modulus = "+syoung+" +- "+seyoung+" Pa";      
      pt.AddText(text4.c_str());
    }
  }
  pt.Draw();
  leg.Draw();

  c1.Update();
  
  string title = get_wires_title(ftitle, diameterMil)+".all.jpeg";
  
  c1.Print(title.c_str());
  
  return 1;

}

bool exists_file(string filen, double DiameterMil, string date){


  string DiamTitle = get_diam_title(DiameterMil);

  string filename = filen+"_"+DiamTitle+"."+date+".txt";

  std::ifstream inp;
  inp.open(filename.c_str(), ifstream::in);
  inp.close();
  if(inp.fail())
    {
      cout << " file " << filename << " does not exist " << endl;
      return false;
    }

  cout << " file " << filename << " exists " << endl;

  return true;

}


double get_max_fit(double diameterMil){

  double weight_correction = 1.;
  // weights are wrong; nominal 0.1 kg corresponds to 0.115 kg

#if 1
  weight_correction = 1.15;
#endif

  double val;

  if( diameterMil == 5.){
    val = 0.79;
  }
  else if( diameterMil == 10.001 || diameterMil == 10. || diameterMil == 10.002){
    val = 2.2;
  }
  else if( diameterMil == 15. || diameterMil == 15.001 || diameterMil == 15.002){
    val = 6.1;
  }
  else{
    cout << " problem: unknown max fit for diametermil " << diameterMil << endl;
    exit(0);
  }

  return val;
  

}

Int_t get_entries_below(double max, TH2F h){

  Int_t n = 0;

  for(size_t i=0; i<h.GetXaxis()->GetNbins(); i++){
    for(size_t j=0; j<h.GetXaxis()->GetNbins(); j++){
      if( h.GetXaxis()->GetBinCenter(i) < max )
	n += h.GetBinContent(i,j);
    }
  }

  cout << " entries : " << h.GetEntries() << " entries below " << max << " : " << n << endl;

  return n;

}


void elong_vs_time_plot(double factor){

  double DiameterMil = 15.001;
  string date="Nov10a";

  vector<double> ts;
  vector<double> es;
  get_time_data(DiameterMil, factor, date, &ts, &es);

  size_t s = ts.size();

  TCanvas c2("c2","c2",100,100,800,600);

  double leg_xmin = 0.1;
  double leg_xmax = 0.4;
  double leg_ymin = 0.8;
  double leg_ymax = 1.;
  
  TLegend leg(leg_xmin, leg_ymin, leg_xmax, leg_ymax);
  leg.SetFillColor(4000);
  
  double pave_xmin = 0.5;
  double pave_xmax = 1.;
  double pave_ymin = 0.1;
  double pave_ymax = 0.4;

  TPaveText pt(pave_xmin, pave_ymin, pave_xmax, pave_ymax, "NDC");
  pt.SetFillColor(kOrange); // text is black on white
  pt.SetTextSize(0.03);
  pt.SetTextAlign(12);
  string sd = get_diam_title(DiameterMil);
  if( DiameterMil == 15.002 || DiameterMil == 15.001 )
    sd ="15";
  string ptitle = "elongation_vs_time for wire of thickness "+sd+" mil";
  pt.AddText(ptitle.c_str());
  string qtitle = "mass = 0.8 kg";
  pt.AddText(qtitle.c_str());

  double xmin = 0.;
  double xmax = 1.1*ts[s-1];
  double ymin = 0.9*es[0];
  double ymax = 1.1*es[s-1];

  TH2F YvsX("YvsX", ptitle.c_str(), 100, xmin, xmax, 100, ymin, ymax);
  
  string xt="time (s)";
  string yt="relative elongation (mm/m)";

  YvsX.GetXaxis()->SetTitle(xt.c_str());
  YvsX.GetXaxis()->SetTitleSize(0.04);
  YvsX.GetYaxis()->SetTitle(yt.c_str());
  YvsX.GetYaxis()->SetTitleSize(0.04);
  YvsX.SetMarkerColor(kBlue);
  YvsX.SetLineColor(kBlue);
  YvsX.SetMarkerStyle(33);
  YvsX.SetMarkerSize(1);
  
  for(size_t i=0; i<s; i++){
    YvsX.Fill(ts[i], es[i]);
  }
  


  string formula="[2] + [0]*(1 - exp(-x/[1]))";

  TF1 line("line",formula.c_str(),xmin,xmax);
  line.SetLineColor(kRed);
  line.SetLineStyle(2);
  line.SetParameter(0,es[s-1]);
  line.SetParameter(1,ts[s-1]);
  line.SetParameter(2,es[1]);

  YvsX.Fit("line");

  leg.AddEntry(&YvsX, "data", "l");
  leg.AddEntry(&line, "exponential fit", "l");

  Int_t ndof = YvsX.GetEntries();

  double p0 = line.GetParameter(0);
  double p1 = line.GetParameter(1);
  double ep0 = line.GetParError(0);
  double ep1 = line.GetParError(1);
  double chi2 = line.GetChisquare();
  double prob = TMath::Prob(chi2,ndof);
  cout << " time constant = " << p1/60. << " +- " << ep1/60. << " min, so wait for " << 6*p1/60. << " min " << endl;
  cout << " chi2 " << chi2 << " ndof " << ndof << " prob " << prob << endl;
  string sndof  = double_to_string(ndof);
  
  string schi2 = double_to_string(chi2);
  
  string sprob = double_to_string(prob);
  
  string sp0 = double_to_string(p0);
  string sp1 = double_to_string(p1);
  
  string sep0 = double_to_string(ep0);
  string sep1 = double_to_string(ep1);
  
  string text0="p_{0} = "+sp0+" +- "+sep0;
  string text1="p_{1} = "+sp1+" +- "+sep1;
  string text2="#chi^{2} = "+schi2+" / ndof = "+sndof;
  string text3="prob = "+sprob;
  pt.AddText(text0.c_str());
  pt.AddText(text1.c_str());
  pt.AddText(text2.c_str());
  pt.AddText(text3.c_str());

  string DiamTitle = get_diam_title(DiameterMil);
  string pictitle="../fig/elongation_vs_time_"+DiamTitle+".all.jpeg";

  YvsX.Draw();
  pt.Draw();
  leg.Draw();
  c2.SetTitle(pictitle.c_str());
  c2.Update();
  c2.Print(pictitle.c_str());


  return;

}

bool get_meaningful_hot_zones(std::vector<double> data, std::vector<double> &lz, std::vector<double> &hz, std::vector<double> &ratio){

  bool print = false;

  std::vector<double> oldlz;
  std::vector<double> oldhz; 
  std::vector<double> oldratios; 
  std::vector<double> newlz;
  std::vector<double> newhz;
  std::vector<double> newratios;
  double absmin = *( std::min_element( data.begin(), data.end() ) );
  double absmax = *( std::max_element( data.begin(), data.end() ) );
  double min = absmin;
  double max = absmax;
  double CYCLE_LENGTH = 25.e-9;  // clock cycle (s)
  double ncycles = 1000;

  oldlz.clear();
  oldhz.clear();
  oldratios.clear();
  oldlz.push_back(min);
  oldhz.push_back(max);
  bool ok = false;
  size_t counter = 0;
  double min_duration = 999999999999999.;
  while( !ok ){
    newlz.clear();
    newhz.clear();
    newratios.clear();

    for(size_t i=0; i<oldlz.size(); i++){
      ok = get_clever_hot_zones(data, oldlz[i], oldhz[i], newlz, newhz, newratios) || ok;
    }

    oldlz.clear();
    oldhz.clear();
    oldratios.clear();
    if( newlz.size() != newhz.size() ||
	newlz.size() != newratios.size() ){
      cout << " problem: newlz size " << newlz.size() << " newhz " << newhz.size() << " rato " << newratios.size() << endl;
      exit(0);
    }

    if( print )
      cout << " step " << counter << " zones " << newlz.size() << " are: " << endl;
    for(size_t i=0; i<newlz.size(); i++){
      if( print )
	cout << " ... zone " << i << "[" << newlz[i] << ", " << newhz[i] << "] ratio " << newratios[i] << endl; fflush(stdout);
      
      if( (oldlz.size()>0) && (newlz[i] == newhz[i-1]) ){
	oldhz.pop_back();
	oldhz.push_back(newhz[i]);
	double olr = oldratios.back();
	oldratios.pop_back();
	oldratios.push_back(newratios[i] + olr);
      }
      else{
	oldlz.push_back(newlz[i]);
	oldhz.push_back(newhz[i]);
	oldratios.push_back(newratios[i]);
      }

      if( min_duration > oldhz.back() - oldlz.back() ){
	min_duration = oldhz.back() - oldlz.back();
      }
    }
    
    if( min_duration < ncycles*CYCLE_LENGTH ){
      if( print )
	cout << " zone has duration " << min_duration << " minimum " << ncycles*CYCLE_LENGTH << endl;
      ok = ok || true;
    }

    counter ++;
  }
 

    if( oldlz.size() != oldhz.size() ||
	oldlz.size() != oldratios.size() ){
      cout << " problem: oldlz size " << oldlz.size() << " oldhz " << oldhz.size() << " rato " << oldratios.size() << endl;
      exit(0);
    }
    
  lz.clear();
  hz.clear();
  ratio.clear();
  for(size_t i=0; i<oldlz.size(); i++){
    lz.push_back(oldlz[i]);
    hz.push_back(oldhz[i]);
    ratio.push_back(oldratios[i]);
  }

  return true;
}

bool get_clever_hot_zones(std::vector<double> data, double absmin, double absmax, std::vector<double> &lz, std::vector<double> &hz, std::vector<double> &ratios){

  bool print = false;

  if( print )
    cout << " get clever hot zones between " << absmin << " and " << absmax << endl; fflush(stdout);

  bool ok = false;
  int nbins = 10;
  double limit = 0.95;


  double binsize = (absmax - absmin)/nbins;

  TH1F* test = new TH1F("test","test",nbins,absmin, absmax);
  
  for(size_t i=0; i<data.size(); i++)
    if( data[i] >= absmin && data[i] <= absmax ) 
      test->Fill(data[i]);

  double supertotal = data.size();
  double total = test->Integral();

  if( total <= 0. ){
    cout << " problem: total integral is " << total << endl;
    exit(0);
  }

  size_t nbin=0;

  for(size_t i=0; i<nbins; i++){
    double local = test->GetBinContent(i+1);
    double ratio = local/total;
    bool thisbin=(ratio > 1. - limit);
    bool oldbin=false;
    if( i > 0 && test->GetBinContent(i)/total > 1. - limit )
      oldbin = true;

    if( thisbin ){
      nbin++;
      if( oldbin ){
	if( lz.size() <= 0 ) { 
	  cout << " problem: popping back bin " << i << " but lz size " << lz.size() << endl; 
	  exit(0);
	}
	hz.pop_back();
	hz.push_back(test->GetXaxis()->GetBinUpEdge(i+1));
	double olr = ratios.back();
	ratios.pop_back();
	ratios.push_back(local/supertotal + olr);
      }
      else{
	lz.push_back(test->GetXaxis()->GetBinLowEdge(i+1));
	hz.push_back(test->GetXaxis()->GetBinUpEdge(i+1));
	ratios.push_back(local/supertotal);
      }
      if( print )
	cout << " bin " << i << " min " << lz.back() << " max " << hz.back()<< " local " << local << " supertotal " << supertotal << " ratio " << local/supertotal << endl;
    }
  }

  if( print ){
    cout << " min " << absmin << " max " << absmax << " binsize " << binsize << " n of positive bins " << nbin << " n of zones " << lz.size() << endl;
    for(size_t i=0; i<lz.size(); i++){
      cout << " zone " << i << "[" << lz[i] << ", " << hz[i] << "] ratio" << ratios[i] << endl;
    }
  }

  if( nbin == nbins ){
    ok = true;
  }

  delete test;

  return ok;

}




bool get_meaningful_min_max(std::vector<double> data, double* Min, double* Max){

  double min = 0.;
  double max = min;
  bool ok = false;
  while( !ok ){
    ok = get_clever_min_max(data, &min, &max, min, max) ;
    if( ok ){
      ok = get_clever_min_max(data, &min, &max, min, max) ;
      ok = get_clever_min_max(data, &min, &max, min, max) ;
    }
  }

  *Min = min;
  *Max = max;
  return true;

}

bool get_clever_min_max(std::vector<double> data, double* Min, double* Max, double min = 0., double max = 0.){

  double limit = 0.90;

  if( min == 0. && max == 0. ){
    max = *( std::max_element( data.begin(), data.end() ) );
    min = *( std::min_element( data.begin(), data.end() ) );
  }

  //  cout << " look in window " << min << " - " << max;

  size_t nbins = 1000;
  double hmin  =0.99*min;
  double hmax  =1.01*max;
  double bwidth = (hmax-hmin)/nbins;

  TH1F* test = new TH1F("test","test",nbins,hmin, hmax);

  for(size_t i=0; i<data.size(); i++)
    if( data[i] >= min && data[i] <= max ) 
      test->Fill(data[i]);

  double total_integral=test->Integral(1, nbins);

  double ave = test->GetMean();
  int iave=test->GetXaxis()->FindBin(ave);

  //  cout << " integral " << total_integral << " ave " << ave << " iave " << iave << endl;

  for(size_t i=0; i<test->GetXaxis()->GetNbins(); i++){
    int hl = iave + 1 + i;
    int ll = iave - 1 - i;
    if( hl > nbins ) hl = nbins;
    if( ll <= 0 ) ll = 0;
    *Min = test->GetXaxis()->GetBinLowEdge(ll);
    *Max = test->GetXaxis()->GetBinUpEdge(hl);
    double local_integral=test->Integral(ll, hl);
    double ratio = local_integral / total_integral;

    //    cout << " .. add " << i + 1 << " bins, wind " << *Min << " - " << *Max << " ratio " << ratio << endl;

    if( ratio > limit ){
      if( hl - ll <= 5 ){
	delete test;
	return false;
      }
      delete test;
      return true;
    }
  
  }

  cout << " could not find meaningful min and max " << endl;
  exit(0);

}



string get_property(string name, string property){
  // extract property from name:
  // name = lungo_15__largo_20__alto_30


  size_t found = name.find(property.c_str());
  size_t first = 0;

  if( found == string::npos ){
    cout << " problem: property " << property << " not found in name " << name << endl;
    exit(0);
  }
    
  name = name.substr(found+property.size()+1);

  found = name.find("__");
  // good also if end of word


  return name.substr(first,found);

}



bool is_program_running(string asker, string commandname){
  // Check if process is running via command-line
  
  clog << asker << " is checking if " << commandname << " is running " << endl;
  
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
    clog << commandname << " is running, so " << asker << " will wait " << endl; fflush(stdout);
    return true;
  }

  return false;
}
  
  
