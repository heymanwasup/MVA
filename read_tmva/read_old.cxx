#include <TMVA/Reader.h>
#include <reader.h>

struct Vars {
 float MET;
 float HT;
 float dPhiMETdijet;
 float pTB1;
 float pTB2;
 float mBB;
 float dRBB;
 float dEtaBB;
 float pTJ3;
 float mBBJ;
 float nJ;
 float EventNumberMod2;
 float weight;
 float BDTCategories;
 int   classID;
 char className[11];
};

void BookTree( TTree* tree, Vars &vars)
{
 
   tree->SetBranchAddress("classID", &vars.classID);
   tree->SetBranchAddress("className", vars.className);
   tree->SetBranchAddress("MET", &vars.MET);
   tree->SetBranchAddress("HT", &vars.HT);
   tree->SetBranchAddress("dPhiMETdijet", &vars.dPhiMETdijet);
   tree->SetBranchAddress("pTB1", &vars.pTB1);
   tree->SetBranchAddress("pTB2", &vars.pTB2);
   tree->SetBranchAddress("mBB", &vars.mBB);
   tree->SetBranchAddress("dRBB", &vars.dRBB);
   tree->SetBranchAddress("dEtaBB", &vars.dEtaBB);
   tree->SetBranchAddress("pTJ3", &vars.pTJ3);
   tree->SetBranchAddress("mBBJ", &vars.mBBJ);
   tree->SetBranchAddress("nJ", &vars.nJ);
   tree->SetBranchAddress("EventNumberMod2", &vars.EventNumberMod2);
   tree->SetBranchAddress("weight", &vars.weight);
   tree->SetBranchAddress("BDTCategories", &vars.BDTCategories);
  
}
void SetBranch( TTree* tree, Vars &vars)
{
   tree->Branch("classID", &vars.classID);
   tree->Branch("MET", &vars.MET);
   tree->Branch("HT", &vars.HT);
   tree->Branch("dPhiMETdijet", &vars.dPhiMETdijet);
   tree->Branch("pTB1", &vars.pTB1);
   tree->Branch("pTB2", &vars.pTB2);
   tree->Branch("mBB", &vars.mBB);
   tree->Branch("dRBB", &vars.dRBB);
   tree->Branch("dEtaBB", &vars.dEtaBB);
   tree->Branch("pTJ3", &vars.pTJ3);
   tree->Branch("mBBJ", &vars.mBBJ);
   tree->Branch("nJ", &vars.nJ);
   tree->Branch("EventNumberMod2", &vars.EventNumberMod2);
   tree->Branch("EventWeight", &vars.weight);
   tree->Branch("BDTCategories", &vars.BDTCategories);
}
void BookReader(TMVA::Reader * reader,  map<string,float> &r_vars,string wfile)
{
  vector<string> train_vars = {"MET","HT","dPhiMETdijet","pTB1","pTB2","mBB","dRBB","dEtaBB","pTJ3","mBBJ" };
  vector<string> spec_vars = {"nJ","EventNumberMod2"};
  
  for(auto var : train_vars){
    r_vars[var] = 0;
    reader -> AddVariable(var,&r_vars[var]);
  }
  for(auto var : spec_vars){
    r_vars[var] = 0;
    reader -> AddSpectator(var,&r_vars[var]);
  }
  reader -> BookMVA("BDT","/afs/cern.ch/user/c/chenc/mva_vhbb0l/weights/"+wfile);
}
#define T(var) r_vars[#var] = t_vars.var
void GetEntry(TTree* tree, Vars & t_vars, map<string,float> & r_vars,int n,bool trans=false)
{
  tree -> GetEntry(n);
  T(MET);
  T(HT);
  T(dPhiMETdijet);
  T(pTB1);
  T(pTB2);
  T(mBB);
  T(dRBB);
  T(dEtaBB);
  T(pTJ3);
  T(mBBJ);
  T(nJ);
  if(!trans) {
    T(EventNumberMod2);
  } else {
    r_vars["EventNumberMod2"] = (t_vars.EventNumberMod2==0? 2 : 1);
  }
}
void Loop(TMVA::Reader* reader, TTree* tree, Vars & t_vars,map<string,float>& r_vars,int NLimit,TH1F* hist = NULL)
{
  int N = tree -> GetEntries();
  float totalW = 0.;
  for (int n=0;n<N;n++){
    GetEntry(tree,t_vars,r_vars,n);
    if (t_vars.classID == 0 || string(t_vars.className) == string("Signal")){
      totalW += t_vars.weight;
      if (NULL!=hist){
        hist -> Fill(t_vars.weight);
      }
      /*
      float mva = reader -> EvaluateMVA("BDT");
      cout << "----------------"<< endl;
      cout << "mva readout  = "<< mva<<endl;;
      cout << "classID = " << t_vars.classID << endl;
      cout << "className = " << t_vars.className << endl;
      tree -> Show(n);
      */
    }
  }
  if (hist!=NULL){
    hist->Scale(1./totalW);
  }
  //cout << "total weight = "<< totalW << endl;
}
void read_tmvaout(TH1F* hist=NULL,string wfile="")
{
  TMVA::Reader * reader = new TMVA::Reader();
  TFile * input = TFile::Open("../mva_output/TMVA-TEST1111111111.root");
  TTree * tree = (TTree*)input -> Get("TestTree");

  Vars t_vars;
  map<string,float> r_vars;
  
  BookTree(tree,t_vars);
  BookReader(reader,r_vars,wfile);
  
  Loop(reader,tree,t_vars,r_vars,10,hist);
}


void get_total_w(string b_name,TH1F* hist=NULL){
  TFile * f = TFile::Open("/afs/cern.ch/user/c/chenc/mva_vhbb0l/data/signal.root");
  TTree * tree = (TTree*)f -> Get("Nominal");
  double w = 0;
  tree -> SetBranchAddress(b_name.c_str(), &w);
  int N = tree -> GetEntries();
  double total_w = 0;
  for (int n=0;n<N;n++){
    tree -> GetEntry(n);
     
    total_w += w;
    if (hist!=NULL){
      hist -> Fill(w);
    }
  }
  if(hist!=NULL){
    hist -> Scale(1./total_w);
  }
}
void get_weights()
{
  TFile * out = new TFile("f_weights.root","recreate");
  TH1F* h_mvaout = new TH1F("hfinal","hfinal",100,-0.05,0.05);
  TH1F* h_original = new TH1F("hinit","hinit",100,-0.05,0.05);
  read_tmvaout(h_mvaout,"TMVAClassification_BDTCategories.weights.xml");
  get_total_w("EventWeight",h_original);
  
  out -> cd();
  h_mvaout -> Write();
  h_original -> Write();
  out -> Close();
  
}
void FillTree(TTree* tree, TTree* sig,TTree* bkg) 
{
  Vars t_vars;
  
  BookTree(tree,t_vars);
  SetBranch(sig,t_vars);
  SetBranch(bkg,t_vars);
  
  int N = tree -> GetEntries();
  for (int n=0;n<N;n++){
    tree -> GetEntry(n);
    if (t_vars.classID == 0 && string(t_vars.className) == string("Signal")){
      sig -> Fill();
    }
    else if (t_vars.classID == 1 && string(t_vars.className) == string("Background")){
      bkg -> Fill();
    }
    else {
      cout <<"ERROR class name , class ID "<< t_vars.classID <<"  ,  "<<string(t_vars.className)<<endl;
    }
  }
}

void Fill(TTree* tree, TH1F* sig,TH1F* bkg,string wfile) 
{
  TMVA::Reader * reader = new TMVA::Reader();
  Vars t_vars;
  map<string,float> r_vars;
  
  BookTree(tree,t_vars);
  BookReader(reader,r_vars,wfile);
  
  int N = tree -> GetEntries();
  for (int n=0;n<N;n++){
    GetEntry(tree,t_vars,r_vars,n,false);
    float mva = reader -> EvaluateMVA("BDT");
    if (t_vars.classID == 0 && string(t_vars.className) == string("Signal")){
      sig -> Fill(mva,t_vars.weight);
    }
    else if (t_vars.classID == 1 && string(t_vars.className) == string("Background")){
      bkg -> Fill(mva,t_vars.weight);
    }
    else {
      cout <<"ERROR class name , class ID "<< t_vars.classID <<"  ,  "<<string(t_vars.className)<<endl;
    }
  }
}
void fill_tmva_v24()
{
  TFile * input = TFile::Open("../mva_output/TMVA_v24.root");
  TTree * TestTree = (TTree*)input -> Get("TestTree");
  TTree * TrainTree = (TTree*)input -> Get("TrainTree");
  
  TFile * out = new TFile("../data/out_mva24_28.root","RECREATE");

  TH1F * sig = new TH1F("h_sig","h_sig",10000,-1.5,1.5);
  TH1F * bkg = new TH1F("h_bkg","h_bkg",10000,-1.5,1.5);
  Fill(TestTree,sig,bkg,"TMVA_28.xml");
  Fill(TrainTree,sig,bkg,"TMVA_28.xml");
  out -> cd();
  sig -> Write();
  bkg -> Write();
  out -> Close();
}

//default: to perform the xml on some  tmva output
void fill_tmva_local(string mtag,string wtag,string mfile,string wfile)
{
  cout <<"staring"<<endl;
  string path = "/afs/cern.ch/user/c/chenc/mva_vhbb0l/mva_output/";
  TFile * input = TFile::Open((path+mfile).c_str());

  TTree * TestTree = (TTree*)input -> Get("TestTree");
  TTree * TrainTree = (TTree*)input -> Get("TrainTree");

  string oname = "../data/out_"+wtag+"xml_on_"+mtag+"tree.root";
  TFile * out = new TFile(oname.c_str(),"RECREATE");

  TH1F * sig = new TH1F("h_sig","h_sig",10000,-1.5,1.5);
  TH1F * bkg = new TH1F("h_bkg","h_bkg",10000,-1.5,1.5);
  Fill(TestTree,sig,bkg,wfile);
  Fill(TrainTree,sig,bkg,wfile);



  TH1F * h_roc = new TH1F("roc","roc",100,0,1);
  fill_roc(out,h_roc,sig,bkg);


  out -> cd();
  sig -> Write();
  bkg -> Write();
  out -> Close();
}

void split_trees()
{
  TFile * input = TFile::Open("../mva_output/TMVA_v24.root");
  TTree * TestTree = (TTree*)input -> Get("TestTree");
  TTree * TrainTree = (TTree*)input -> Get("TrainTree");
  
  TFile * out = new TFile("splited_mva24.root","RECREATE");
  TTree * sig = new TTree("t_sig","t_sig");
  TTree * bkg = new TTree("t_bkg","t_bkg");
  FillTree (TestTree,sig,bkg);
  FillTree (TrainTree,sig,bkg);
  out -> cd();
  sig -> Write();
  bkg -> Write();
}
void read()
{
  fill_tmva_local("v1","v1","TMVACAT_TREE24_v1.root","TMVAClassification_BDTCategories_v1.weights.xml");
  //fill_tmva_local("v1","28","TMVACAT_TREE24_v1.root","TMVA_28.xml");
}
