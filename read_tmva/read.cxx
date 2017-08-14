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
#define cate0 1
#define cate1 (t_vars.nJ>1.5)&&(t_vars.nJ<2.5)&&(t_vars.EventNumberMod2%2>-0.5)&&(t_vars.EventNumberMod2%2<0.5) 
#define cate2 (t_vars.nJ>1.5)&&(t_vars.nJ<2.5)&&(t_vars.EventNumberMod2%2>0.5)&&(t_vars.EventNumberMod2%2<1.5)
#define cate3 (t_vars.nJ>2.5)&&(t_vars.nJ<3.5)&&(t_vars.EventNumberMod2%2>-0.5)&&(t_vars.EventNumberMod2%2<0.5)
#define cate4 (t_vars.nJ>2.5)&&(t_vars.nJ<3.5)&&(t_vars.EventNumberMod2%2>0.5)&&(t_vars.EventNumberMod2%2<1.5)
//based on Fill(...), added the categories
void Fill_split(TTree* tree, vector<TH1F*> & sig,vector<TH1F*> & bkg,string  wfile) 
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
      sig[0] -> Fill(mva,t_vars.weight);
      if((t_vars.nJ>1.5)&&(t_vars.nJ<2.5)&&(int(t_vars.EventNumberMod2)%2>-0.5)&&(int(t_vars.EventNumberMod2)%2<0.5)) {
        sig[1] -> Fill(mva,t_vars.weight);
      } else if((t_vars.nJ>1.5)&&(t_vars.nJ<2.5)&&(int(t_vars.EventNumberMod2)%2>0.5)&&(int(t_vars.EventNumberMod2)%2<1.5)) {
        sig[2] -> Fill(mva,t_vars.weight);
      } else if((t_vars.nJ>2.5)&&(t_vars.nJ<3.5)&&(int(t_vars.EventNumberMod2)%2>-0.5)&&(int(t_vars.EventNumberMod2)%2<0.5)) {
        sig[3] -> Fill(mva,t_vars.weight);
      } else if((t_vars.nJ>2.5)&&(t_vars.nJ<3.5)&&(int(t_vars.EventNumberMod2)%2>0.5)&&(int(t_vars.EventNumberMod2)%2<1.5)) {
        sig[4] -> Fill(mva,t_vars.weight);
      } else {
        cout <<"WARNING!!! event not into any categories"<<endl;
        tree->Show(n);
      }
    }
    else if (t_vars.classID == 1 && string(t_vars.className) == string("Background")){
      bkg[0] -> Fill(mva,t_vars.weight);
      if((t_vars.nJ>1.5)&&(t_vars.nJ<2.5)&&(int(t_vars.EventNumberMod2)%2>-0.5)&&(int(t_vars.EventNumberMod2)%2<0.5)) {
        bkg[1] -> Fill(mva,t_vars.weight);
      } else if((t_vars.nJ>1.5)&&(t_vars.nJ<2.5)&&(int(t_vars.EventNumberMod2)%2>0.5)&&(int(t_vars.EventNumberMod2)%2<1.5)) {
        bkg[2] -> Fill(mva,t_vars.weight);
      } else if((t_vars.nJ>2.5)&&(t_vars.nJ<3.5)&&(int(t_vars.EventNumberMod2)%2>-0.5)&&(int(t_vars.EventNumberMod2)%2<0.5)) {
        bkg[3] -> Fill(mva,t_vars.weight);
      } else if((t_vars.nJ>2.5)&&(t_vars.nJ<3.5)&&(int(t_vars.EventNumberMod2)%2>0.5)&&(int(t_vars.EventNumberMod2)%2<1.5)) {
        bkg[4] -> Fill(mva,t_vars.weight);
      } else {
        cout <<"WARNING!!! event not into any categories"<<endl;
        tree->Show(n);
      }
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

//default: to perform the xml on some tmva output
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


//write the roc from the RNN distribution
void write_roc_split_for_RNN()
{
  TFile * input = TFile::Open("../RNN_data/RNN_categorites.root");
  TFile * out   = new TFile("../data/RNN_rocs.root","recreate");
  string njets[2] = {"2","3"};
  string parities[2] = {"E","O"};
  
  out -> cd();
  map<string,TH1F*> rocs;
  for (string nj : njets){
    for(string parity : parities){
      string s_sig = "RNN_S_TEST_" + nj + "j" + parity;
      string s_bkg = "RNN_B_TEST_" + nj + "j" + parity;
      string s_roc = "roc_" + nj + "j" + parity;
      TH1F * h_sig = (TH1F*)input -> Get(s_sig.c_str());
      TH1F * h_bkg = (TH1F*)input -> Get(s_bkg.c_str());
      rocs[s_roc] = new TH1F(s_roc.c_str(),s_roc.c_str(),100,0,1);

      fill_roc(out,rocs[s_roc],h_sig,h_bkg,true);
    }
  }
  for(auto roc : rocs){
    roc.second->Write();
  }
  out->Close();
}

//test no weight
//
void write_roc_split_for_RNN_test()
{
  TFile * input = TFile::Open("../RNN_data/RNN_2jE.root");
  TFile * out   = new TFile("../data/RNN_rocs_test.root","recreate");
  string njets[1] = {"2"};
  string parities[1] = {"E"};
  string wtag[2] = {"N_",""};
  
  out -> cd();
  map<string,TH1F*> rocs;
  for (string nj : njets){
    for(string parity : parities){
      for(string w : wtag){
        string s_sig = "RNN_S_TEST_" + w + nj + "j" + parity;
        string s_bkg = "RNN_B_TEST_" + w + nj + "j" + parity;
        string s_roc = "roc_" + w + nj + "j" + parity;
        TH1F * h_sig = (TH1F*)input -> Get(s_sig.c_str());
        TH1F * h_bkg = (TH1F*)input -> Get(s_bkg.c_str());
        cout << nj << "  " << parity << endl;
        cout << "h_sig = " << h_sig << endl;
        cout << "h_bkg = " << h_bkg << endl;
        rocs[s_roc] = new TH1F(s_roc.c_str(),s_roc.c_str(),100,0,1);

        fill_roc(out,rocs[s_roc],h_sig,h_bkg,true);
      }
    }
  }
  for(auto roc : rocs){
    roc.second->Write();
  }
  out->Close();
}

//based on write_mva_roc_by_xml, added MVAs and ROCs in 4 categories
void write_mva_roc_split_by_xml(string wfile="TMVA_28.xml",string xmltag="28")
{
  cout <<"started"<<endl;
  string infile = "/afs/cern.ch/user/c/chenc/mva_vhbb0l/mva_output/TMVA_v24.root";
  TFile * input = TFile::Open(infile.c_str());

  TTree * TestTree = (TTree*)input -> Get("TestTree");
  TTree * TrainTree = (TTree*)input -> Get("TrainTree");

  string oname = "../data/out_"+xmltag+"xml_on_24tree_split.root";
  TFile * out = new TFile(oname.c_str(),"RECREATE");
  
  vector<TH1F*> train_sig;
  vector<TH1F*> train_bkg;

  vector<TH1F*> test_sig;
  vector<TH1F*> test_bkg;

  vector<TH1F*> roc;
  vector<TH1F*> train_roc;
  vector<TH1F*> test_roc;

  vector<TH1F*> sig;
  vector<TH1F*> bkg;

  string splits[5] = {"","_2jE", "_2jO", "_3jE", "_3jO"};
  for(int n=0;n<5;n++) {
    string s_train = "BDT_S_TRAIN" + splits[n];
    string b_train = "BDT_B_TRAIN" + splits[n];

    string s_test = "BDT_S_TEST" + splits[n];
    string b_test = "BDT_B_TEST" + splits[n];

    string s = "BDT_S" + splits[n];
    string b = "BDT_B" + splits[n];

    string r_train = "ROC_TRAIN" + splits[n];
    string r_test  = "ROC_TEST" + splits[n];
    string r       = "ROC" + splits[n];

    train_sig.push_back(new TH1F(s_train.c_str(),s_train.c_str(),30000,-1.5,1.5));
    train_bkg.push_back(new TH1F(b_train.c_str(),b_train.c_str(),30000,-1.5,1.5));

    test_sig.push_back(new TH1F(s_test.c_str(),s_test.c_str(),30000,-1.5,1.5));
    test_bkg.push_back(new TH1F(b_test.c_str(),b_test.c_str(),30000,-1.5,1.5));

    sig.push_back(new TH1F(s.c_str(),s.c_str(),30000,-1.5,1.5));
    bkg.push_back(new TH1F(b.c_str(),b.c_str(),30000,-1.5,1.5));

    roc.push_back(new TH1F(r.c_str(),r.c_str(),100,0,1));
    train_roc.push_back(new TH1F(r_train.c_str(),r_train.c_str(),100,0,1));
    test_roc.push_back(new TH1F(r_test.c_str(),r_test.c_str(),100,0,1));


  }
  Fill_split(TrainTree,train_sig,train_bkg,wfile); 
  Fill_split(TestTree,test_sig,test_bkg,wfile);

  for (int n=0;n<5;n++){
    sig[n] -> Add(train_sig[n]);
    sig[n] -> Add(test_sig[n]);

    bkg[n] -> Add(train_bkg[n]);
    bkg[n] -> Add(test_bkg[n]);

    fill_roc(out,roc[n],sig[n],bkg[n]);
    fill_roc(out,train_roc[n],train_sig[n],train_bkg[n]);
    fill_roc(out,test_roc[n],test_sig[n],test_bkg[n]);

    sig[n] -> Write();
    bkg[n] -> Write();

    train_sig[n] -> Write();
    train_bkg[n] -> Write();
    test_sig[n] -> Write();
    test_bkg[n] -> Write();

  }
  out -> cd();

  out -> Close();
}


//performing the xml(defalut is xml28) on charles tree, w.r.t. training and test, get the roc and tmva score
void write_mva_roc_by_xml(string wfile="TMVA_28.xml",string xmltag="28")
{
  cout <<"started"<<endl;
  string infile = "/afs/cern.ch/user/c/chenc/mva_vhbb0l/mva_output/TMVA_v24.root";
  TFile * input = TFile::Open(infile.c_str());

  TTree * TestTree = (TTree*)input -> Get("TestTree");
  TTree * TrainTree = (TTree*)input -> Get("TrainTree");

  string oname = "../data/out_"+xmltag+"xml_on_24tree.root";
  TFile * out = new TFile(oname.c_str(),"RECREATE");

  TH1F * train_sig = new TH1F("BDT_S_TRAIN","BDT_S_TRAIN",30000,-1.5,1.5);
  TH1F * train_bkg = new TH1F("BDT_B_TRAIN","BDT_B_TRAIN",30000,-1.5,1.5);

  TH1F * test_sig = new TH1F("BDT_S_TEST","BDT_S_TEST",30000,-1.5,1.5);
  TH1F * test_bkg = new TH1F("BDT_B_TEST","BDT_B_TEST",30000,-1.5,1.5);

  TH1F * sig = new TH1F("BDT_S","BDT_S",30000,-1.5,1.5);
  TH1F * bkg = new TH1F("BDT_B","BDT_B",30000,-1.5,1.5);
   
  TH1F * h_roc = new TH1F("ROC","ROC",100,0,1);
  TH1F * h_roc_train = new TH1F("ROC_TRAIN","ROC_TRAIN",100,0,1);
  TH1F * h_roc_test = new TH1F("ROC_TEST","ROC_TEST",100,0,1);

  Fill(TrainTree,train_sig,train_bkg,wfile);
  Fill(TestTree,test_sig,test_bkg,wfile);

  sig -> Add(train_sig);
  sig -> Add(test_sig);

  bkg -> Add(train_bkg);
  bkg -> Add(test_bkg);
  
  fill_roc(out,h_roc,sig,bkg);
  fill_roc(out,h_roc_train,train_sig,train_bkg);
  fill_roc(out,h_roc_test,test_sig,test_bkg);

  out -> cd();
  sig -> Write();
  bkg -> Write();
  train_sig -> Write();
  train_bkg -> Write();
  test_sig -> Write();
  test_bkg -> Write();
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
void split_trees_sigbkg_traintest()
{
  TFile * input = TFile::Open("../mva_output/TMVA_v24.root");
  TTree * TestTree = (TTree*)input -> Get("TestTree");
  TTree * TrainTree = (TTree*)input -> Get("TrainTree");
  
  TFile * out = new TFile("../inputs/splited_traintest_mva24.root","RECREATE");
  TTree * train_sig = new TTree("train_sig","train_sig");
  TTree * train_bkg = new TTree("train_bkg","train_bkg");

  TTree * test_sig = new TTree("test_sig","test_sig");
  TTree * test_bkg = new TTree("test_bkg","test_bkg");

  FillTree (TestTree,test_sig,test_bkg);
  FillTree (TrainTree,train_sig,train_bkg);
  out -> cd();
  test_sig -> Write();
  test_bkg -> Write();

  train_sig -> Write();
  train_bkg -> Write();
}
void read()
{
  //fill_tmva_local("v1","v1","TMVACAT_TREE24_v1.root","TMVAClassification_BDTCategories_v1.weights.xml");
  //fill_tmva_local("v1","28","TMVACAT_TREE24_v1.root","TMVA_28.xml");
  //split_trees_sigbkg_traintest();
//  write_mva_roc_byxml("TMVA_28.xml");
//  write_mva_roc_by_xml("TMVAClassification_BDT_test_1111111111_NominalSets.weights.xml","v0");
//  write_mva_roc_split_by_xml();
//  write_roc_split_for_RNN();
  write_roc_split_for_RNN_test();
}
