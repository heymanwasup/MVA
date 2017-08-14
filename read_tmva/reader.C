#include <TMVA/Reader.h>
#include "reader.h"

void get_mva(string sfile)
{
  TMVA::Reader * reader = NULL;
  TTree * tree = NULL;

  TFile * file = NULL;
  TH1F * hist = NULL;

  map<string,float>  reader_vars;
  map<string,double> tree_vars;

  init_mva(sfile,reader,tree,file,hist,reader_vars,tree_vars);
  fill_mva(sfile,reader,tree,file,hist,reader_vars,tree_vars);
}

void get_roc()
{
  TFile * f_roc = NULL; 
  TH1F * h_sig = NULL;
  TH1F * h_bkg = NULL;
  TH1F * h_roc = NULL;

  init_roc(f_roc,h_roc,h_sig,h_bkg);
  fill_roc(f_roc,h_roc,h_sig,h_bkg);
}

void reader()
{
//  get_mva("signal");
//  get_mva("background");

  get_roc();
}
