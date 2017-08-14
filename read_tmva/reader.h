#include <TMVA/Reader.h>
typedef map<string,float> dict_s2f;
typedef map<string,double> dict_s2d;


bool debug = 0;
TTree * GetTree(string tag)
{
  string path = "./data/";
  TFile * f = TFile::Open((path+tag+".root").c_str());
  TTree * t = (TTree*) f->Get("Nominal");
  return t;
}
void BookTree(TTree* tree,  dict_s2d& tree_vars  )
{
  vector<string> all_vars = { "MET","HT","dPhiMETdijet","pTB1","pTB2","mBB","dRBB","dEtaBB","pTJ3","mBBJ" ,"nJ","EventNumberMod2","EventWeight"};
  for (string var : all_vars){
    tree_vars[var] = 0.;
    tree -> SetBranchAddress(var.c_str(),&tree_vars[var]);
  }
}

void BookReader(TMVA::Reader* reader,  dict_s2f& reader_vars  )
{
  vector<string> vars = { "MET","HT","dPhiMETdijet","pTB1","pTB2","mBB","dRBB","dEtaBB","pTJ3","mBBJ" };
  vector<string> spec = {"nJ","EventNumberMod2"};
  for(string var : vars){
    reader -> AddVariable(var, &reader_vars[var]);
  }
  for(string var : spec){
    reader -> AddSpectator(var, &reader_vars[var]);
  }
  reader -> BookMVA("BDT","/afs/cern.ch/user/c/chenc/public/forChangqiao/TMVAClassification_BDTCategories_mva28.weights.xml"); 
}

void GetEntry(TTree* tree, int n, dict_s2f & reader_vars,dict_s2d& tree_vars )
{
  vector<string> vars = { "MET","HT","dPhiMETdijet","pTB1","pTB2","mBB","dRBB","dEtaBB","pTJ3","mBBJ" ,"nJ"};
  tree->GetEntry(n);
  for (string var : vars){
    reader_vars[var] = (float) tree_vars[var];
  }
  reader_vars["EventNumberMod2"] = ((tree_vars["EventNumberMod2"] == 0.) ? 2. : 1.);
}

template <class T>
void Show( T & vars)
{
  for (auto item : vars){
    cout <<item.first << "=" << item.second <<endl;
  }
}
void ShowInfo(TTree* tree, int n, dict_s2f & reader_vars,dict_s2d& tree_vars )
{
  tree->Show(n);
  cout <<"-------------------\n"<<endl;
  cout <<"reader vars:"<<endl;
  Show<dict_s2f>(reader_vars);
  cout <<"-------------------\n"<<endl;
  cout <<"\ntree vars:"<<endl;
  Show<dict_s2d>(tree_vars);
  cout <<"-------------------\n"<<endl;
}
void CreateOutputStream(TFile* &file, TH1F* &hist, string tag)
{
  file = new TFile (("./data/out_"+tag+".root").c_str(),"recreate");
  file -> cd();
  hist = new TH1F(("h_"+tag).c_str(),("h_"+tag).c_str(),20000,-2,2);
}

void init_mva(string tag,TMVA::Reader *& reader, TTree *& tree, TFile *& file, TH1F *& hist, dict_s2f & reader_vars,dict_s2d & tree_vars,string input, string xml)
{
  reader = new TMVA::Reader();
  string path = "./data/";
  TFile * f = TFile::Open((path+tag+".root").c_str());
  tree = (TTree*) f->Get("Nominal"); 

  BookTree(tree,tree_vars);  
  BookReader(reader,reader_vars);
  file = new TFile (("./data/out_"+tag+".root").c_str(),"recreate");
  hist = new TH1F(("h_"+tag).c_str(),("h_"+tag).c_str(),20000,-2,2);
  
}
void fill_mva(string tag,TMVA::Reader *& reader, TTree *& tree, TFile *& file, TH1F *& hist, dict_s2f & reader_vars,dict_s2d & tree_vars)
{
    int N = tree -> GetEntries();
  for (int n=0;n<N;n++) {
    GetEntry(tree,n,reader_vars,tree_vars);
    if (n==0){
      ShowInfo(tree,n,reader_vars,tree_vars);
    }   
    float mva = reader -> EvaluateMVA("BDT");
    hist -> Fill(mva,tree_vars["EventWeight"]);
    //cout << mva << tree_vars["EventWeight"] << endl;
  }
  file->cd();
  hist->Write();
  file->Close();
}

#define P(var) cout << #var << "   " << var <<endl
void init_roc(TFile* &out,TH1F* &roc,TH1F* &sig,TH1F* &bkg)
{
  TFile * fsig = TFile::Open("./data/out_signal.root");  
  TFile * fbkg = TFile::Open("./data/out_background.root");  
  
  sig = (TH1F*)fsig -> Get("h_signal");
  bkg = (TH1F*)fbkg -> Get("h_background");

  out = new TFile("./data/out_roc.root","recreate");
  roc = new TH1F("roc","roc",100,0,1);

  P(out);
  P(roc);
  P(fsig);
  P(fbkg);
  P(sig);
  P(bkg);
  
}
TH1F* reverse_h(TH1F* h)
{
  TH1F * res = (TH1F*)h -> Clone();
  int N = h -> GetNbinsX();
  for (int n=0;n<N;n++)
  {
    res->SetBinContent(N-n,h->GetBinContent(n+1));
    res->SetBinError(N-n,h->GetBinError(n+1));
  }
  return res;
}

void fill_roc(TFile* out,TH1F* roc,TH1F* sig,TH1F* bkg,bool reverse=false)
{
  // print the pointers
 /* 
  P(out);
  P(roc);
  P(sig);
  P(bkg);
  */
  
  float sum_sig = sig -> Integral();
  float sum_bkg = bkg -> Integral();

  int nBin = 0;
  int N = sig -> GetNbinsX();
  if(debug)  cout << " num of bins : "<< N <<endl;


  TH1F * temp_sig;
  TH1F * temp_bkg;

  if(reverse){
    temp_sig = reverse_h(sig);
    temp_bkg = reverse_h(bkg);
  } else {
    temp_sig = sig;
    temp_bkg = bkg;
  }

  for(int n=N;n>0;n--)
  {
    float bin_center = nBin*0.01 + 0.005;

    float sig_eff_1 = temp_sig -> Integral(n,N)/sum_sig;
    float bkg_rej_1 = temp_bkg -> Integral(0,n-1)/sum_bkg;
    float diff_1 = abs(bin_center - sig_eff_1);

    float sig_eff_2 = temp_sig -> Integral(n-1,N)/sum_sig;
    float bkg_rej_2 = temp_bkg -> Integral(0,n-2)/sum_bkg;
    float diff_2 = abs(bin_center - sig_eff_2);
    if (debug) {
      P( bin_center );
      P( sig_eff_1 );
      P( bkg_rej_1 );
      P( diff_1 );
      P( sig_eff_2 );
      P( bkg_rej_2 );
      P( diff_2 );
      cout << "-------------------------"<<endl;
      
    }
    if (diff_2 > diff_1){
      roc -> SetBinContent(nBin+1,bkg_rej_1);
      roc -> SetBinError(nBin+1,0);
      nBin++;
    }
  }
  out -> cd();
  roc -> Write();
//  out -> Close(); 
}

