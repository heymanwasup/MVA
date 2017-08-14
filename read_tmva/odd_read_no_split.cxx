
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
