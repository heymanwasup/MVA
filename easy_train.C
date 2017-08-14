#include <cstdlib>
#include <algorithm>
#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <bitset>

#include "TChain.h"
#include "TFile.h"
#include "TTree.h"
#include "TString.h"
#include "TObjString.h"
#include "TSystem.h"
#include "TROOT.h"

#include "TMVA/Factory.h"
#include "TMVA/Tools.h"
#include "TMVA/TMVAGui.h"
#include "TMVA/MethodCategory.h"




using namespace std;

const int N = 10;

void get_used_variables(int vars,vector<string> & used_variables, string &vars2j, string & vars3j)
{
  vector<string> all_variables = {"MET","HT","dPhiMETdijet","pTB1","pTB2","mBB","dRBB","dEtaBB","pTJ3","mBBJ"};
  vector<int>    iused;
  int nused = 0;
  for (int n=N-1;n>=0;n--)
  {
    bool used = vars%2;
    vars = vars>>1;
    if(used){
      used_variables.push_back(all_variables[n]);
      nused++;
    }
    iused.push_back(used);
  }
  reverse(iused.begin(),iused.end());
  reverse(used_variables.begin(),used_variables.end());

  vars2j = "";
  vars3j = "";
  cout << endl;
  for(int n=0;n<N;n++){
    string var = all_variables[n];
    printf("var%3d:   %15s %d\n",n,var.c_str(),iused[n]);
    if(iused[n]){
      vars3j = vars3j + var + ":";
      if((var!="pTJ3")&&(var!="mBBJ")){
        vars2j = vars2j + var + ":";
      }
    }
  }
  printf("<%d> variables are used\n",nused);
  cout << endl;
  vars2j.pop_back();
  vars3j.pop_back();
}


int easy_train(int vars, string cfg, string ctag, string name)
{
  

  TMVA::Tools::Instance();

  string s_vars = std::bitset<N>(vars).to_string();
  string postfix = name+"_"+s_vars+"_"+ctag;
  TString outfileName( "/afs/cern.ch/user/c/chenc/mva_vhbb0l/mva_output/TMVA_"+postfix+".root" );

  //redirect stderr and stdout
  string out_log = "/afs/cern.ch/user/c/chenc/mva_vhbb0l/logs/"+postfix+".log";
  string err_log = "/afs/cern.ch/user/c/chenc/mva_vhbb0l/logs/"+postfix+".err";
  freopen(out_log.c_str(),"w",stdout);
  freopen(err_log.c_str(),"w",stderr);

  system("pwd");

  TFile* outputFile = TFile::Open( outfileName, "RECREATE" );

  //initializing used variables, book variables
  vector<string> usedVars;
  string vars2j,vars3j;
  get_used_variables(vars,usedVars,vars2j,vars3j);

  TMVA::Factory *factory = new TMVA::Factory( "TMVAClassification", outputFile,
      "!V:!Silent:Color:DrawProgressBar:AnalysisType=Classification" );

  factory->AddSpectator("nJ",          'I'); 
  factory->AddSpectator("EventNumberMod2",   'F'); 
  for (string var : usedVars) {
    factory->AddVariable(var.c_str(),'F');
  }


  //book trees
  string temp_path = "/eos/atlas/user/c/chenc/temp" ;
  string input0 = "/afs/cern.ch/user/c/chenc/mva_vhbb0l/inputs/splited_traintest_mva24.root";
  string input1 = temp_path + "/tree_" + postfix + ".root";
  string rm_cp = "rm "+input1+" ; cp " + input0 + " " + input1; 
  cout << rm_cp << endl; 
  system(rm_cp.c_str());
  TFile * input = TFile::Open(input1.c_str());
  cout << "input: (" << input << "): " << input1 <<endl;

  TTree * test_sig = (TTree*) input -> Get("test_sig");
  TTree * test_bkg = (TTree*) input -> Get("test_bkg");

  TTree * train_sig = (TTree*) input -> Get("train_sig");
  TTree * train_bkg = (TTree*) input -> Get("train_bkg");

  factory -> AddSignalTree(train_sig,1.,TMVA::Types::kTraining);
  factory -> AddSignalTree(test_sig,1.,TMVA::Types::kTesting);

  factory -> AddBackgroundTree(train_bkg,1.,TMVA::Types::kTraining);
  factory -> AddBackgroundTree(test_bkg,1.,TMVA::Types::kTesting); 

  factory->SetWeightExpression("EventWeight");

  //book categroies
  TMVA::MethodCategory* mJetCategories = 0;
  TMVA::MethodBase* bdtJetCategories = factory->BookMethod( TMVA::Types::kCategory, ("BDT_"+postfix).c_str(),"" );
  mJetCategories = dynamic_cast<TMVA::MethodCategory*>(bdtJetCategories);

  string cate0 = "(nJ>1.5)&&(nJ<2.5)&&(EventNumberMod2%2>-0.5)&&(EventNumberMod2%2<0.5)"; 
  string cate1 = "(nJ>1.5)&&(nJ<2.5)&&(EventNumberMod2%2>0.5)&&(EventNumberMod2%2<1.5)";
  string cate2 = "(nJ>2.5)&&(nJ<3.5)&&(EventNumberMod2%2>-0.5)&&(EventNumberMod2%2<0.5)";
  string cate3 = "(nJ>2.5)&&(nJ<3.5)&&(EventNumberMod2%2>0.5)&&(EventNumberMod2%2<1.5)";

  mJetCategories->AddMethod(cate0.c_str(), vars2j, TMVA::Types::kBDT, "BDTCat_2j_1o2", cfg);
  mJetCategories->AddMethod(cate1.c_str(), vars2j, TMVA::Types::kBDT, "BDTCat_2j_2o2", cfg);
  mJetCategories->AddMethod(cate2.c_str(), vars3j, TMVA::Types::kBDT, "BDTCat_3j_1o2", cfg);
  mJetCategories->AddMethod(cate3.c_str(), vars3j, TMVA::Types::kBDT, "BDTCat_3j_2o2", cfg);


  //perform training 
  
  factory->OptimizeAllMethods("SigEffAt001","Scan");
  factory->OptimizeAllMethods("ROCIntegral","FitGA");

  factory->TrainAllMethods();
  factory->TestAllMethods();
  factory->EvaluateAllMethods();

  outputFile->Close();

  std::cout << "==> Wrote root file: " << outputFile->GetName() << std::endl;
  std::cout << "==> TMVAClassification is done!" << std::endl;

  string wfile = "TMVAClassification_BDT_" + postfix + ".weights.xml";
  string local_path = "/afs/cern.ch/user/c/chenc/mva_vhbb0l";
  string cmp = "[[ $PWD == \""+local_path+ "\" ]] || cp ./weights/"+wfile+" "+local_path+"/weights/";
  string rm_temp = "rm " + input1;
  cout << cmp << endl;
  cout << rm_temp << endl;
  
  system(cmp.c_str());
  system(rm_temp.c_str());

  delete factory;

  if (!gROOT->IsBatch()) TMVA::TMVAGui( outfileName );

  return 0;
}
