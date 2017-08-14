from ROOT import *
from array import array
gROOT.SetBatch(1)
path = './plots/'
debug = True
C = {
  1:1,
  2:2,
  3:3,
  4:4,
  5:kYellow+2,
  6:6,
  7:7,
  8:kBlue-7,
  9:kCyan,
  10:kCyan+1,
  11:kPink+10,
}
def cmp(hists,name,title=None,yrange=None):
  canvas = TCanvas('canvas','canvas',800,800)
  leg    = TLegend(0.175,0.20,0.60,0.65)
#  leg    = TLegend(0.6,0.6,0.85,0.75)
  for hs in hists:
    hs[0].SetLineColor(hs[1])
    hs[0].SetStats(0)
    if not yrange == None:
      hs[0].GetXaxis().SetRangeUser(yrange[0],yrange[1])
    leg.AddEntry(hs[0],hs[2])
    if not title == None:
      hs[0].SetTitle(title)
    hs[0].Draw('same')
  leg.Draw()
  canvas.Print(path+name+'.png')

def get_h_local(vtag,flab={}):
  fname = './mva_output/TMVACAT_TREE24_%s.root'%(vtag)
  f = TFile(fname)
  h = f.Get('Method_Category/{0:}/MVA_{0:}_rejBvsS'.format('BDTCategories_%s'%(vtag)))
  flab[fname] = f
  print 'f',f
  print 'hname','Method_Category/{0:}/MVA_{0:}_rejBvsS'.format('BDTCategories_%s'%(vtag))
  print 'h',h
  return h


    
def _cmp_mva28_on_tree24():      
  tf_24 = TFile('./mva_output/TMVA_v24.root')
  tf_mva28 = TFile('./data/out_roc_24_28.root')
  
  method = 'BDTCategories'
  h_24 = tf_24.Get('Method_Category/{0:}/MVA_{0:}_rejBvsS'.format(method))
  h_mva28 = tf_mva28.Get('roc')
  
  hists = [
    [h_24,kBlue,'roc from charles'],
    [h_mva28,kRed,'tmva28 xml'],
  ]
  cmp(hists,'validation','roc comparison')
def trans(h,tag):
  ranges = {
    'MET':[0,500],
    'HT' :[0,800],
    'pTB1':[0,500],
    'pTB2':[0,200],
    'mBB':[0,500],
    'pTJ3':[0,100],
    'mBBJ':[0,800],
    'dRBB':[0,5],
    'dPhiMETdijet':[0,3.2],
    'dEtaBB':[0,5],
  }
  N = 50
  r = ranges[tag]
  step = (r[1] - r[0])/float(N)
  bins = [r[0] + n*step for n in range(N+1)]
  abins = array('d',bins)
  return h.Rebin(N,'',abins)

  
def __cmp(trees, tag ):
  t1 = trees[0][0]
  l1 = trees[0][1]
  t2 = trees[1][0]
  l2 = trees[1][1]
  
  canvas = TCanvas('canvas','canvas',800,800)
  branches = ["MET","HT","dPhiMETdijet","pTB1","pTB2","mBB","dRBB","dEtaBB","pTJ3","mBBJ"] 
  for b in branches:
      

    t1.Draw('{0:}>>h1_{0:}{1:}'.format(b,tag),'EventWeight')
    t2.Draw('{0:}>>h2_{0:}{1:}'.format(b,tag),'EventWeight')

#    exec('h2_{0:}{1:} = h1_{0:}{1:}.Clone();#h2_{0:}{1:}.Reset()'.format(b,tag))
    exec('h1 = h1_{0:}{1:}'.format(b,tag))
    exec('h2 = h2_{0:}{1:}'.format(b,tag))
    h1 = trans(h1,b)
    h2 = trans(h2,b)
    s1 = h1.Integral()
    s2 = h2.Integral()
    if debug:
      print b
      print 't1',t1 
      print 't2',t2 
      print 'h1',h1 
      print 'h2',h2 
      print 's1',s1 
      print 's2',s2 
    h1_scale = h1.Clone()
    h2_scale = h2.Clone()
      
    h1_scale.Scale(1./s1)
    h2_scale.Scale(1./s2)

    hists = [
      [h1,kBlue,l1],
      [h2,kRed,l2],
    ]  
    hists_s = [
      [h1_scale,kBlue,l1+', scaled'],
      [h2_scale,kRed,l2+', scaled'],
    ]  

    fname = '{0:}_{1:}'.format(tag,b)
    title = '{0:}, {1:}'.format(tag,b)
    cmp(hists, fname,title)
    cmp(hists_s, fname+'_scaled',title+', scaled')
def _cmp_localtree_w_tree24():
  f_lt_s = TFile('./data/signal.root')
  f_lt_b = TFile('./data/background.root')

  f_24 = TFile('./data/splited_mva24.root')
  
  sigtree = [
    [f_lt_s.Get('Nominal'), 'local'],
    [f_24.Get('t_sig'), 'charles'],
  ]
  bkgtree= [
    [f_lt_b.Get('Nominal'), 'local'],
    [f_24.Get('t_bkg'), 'charles'],
  ]
  __cmp(sigtree,'sig')
  __cmp(bkgtree,'bkg')
  
def _cmp_local_xml():
  f28 = TFile('./data/out_roc_24_28.root')
  h28 = f28.Get('roc')
  hists = [
    [h28,1,'xml-mva28'],
  ]
  flab = {}
  for n in range(4):
    h = get_h_local('v%s'%(n),flab)
    print 'h',h 
    hs = [h,n+2,'xml-local %s'%(n)]
    hists.append(hs)

  cmp(hists,'local_cmp','roc comparison')
def _make_args(args,fs):
  hists = []
  n=1
  for arg in args:
    fn = './data/out_{0:}xml_on_{1:}tree.root'.format(arg[0],arg[1])
    f = TFile(fn)
    fs[fn] = f
    h = f.Get('roc')
    hists.append([h,C[n],'{0:} xml'.format(arg[0],arg[1])])
    #hists.append([h,C[n],'{0:} xml on {1:} tree'.format(arg[0],arg[1])])
    n+=1
  return hists

def _cmp_diy_rocs():#trees
  fs = {}
  args = [
    ['28','24'],
    ['28','v1'],
  ] 
  hists = _make_args(args,fs)
  cmp(hists,'diy_roc_validation','diy roc validation')

def _cmp_rocdiy_tmvaout(vtag):
  fs = {}
  
  args = [ [vtag,vtag] ]
  hists = _make_args(args,fs)
  f = TFile('./mva_output/TMVACAT_TREE24_%s.root'%(vtag))
  h = f.Get('Method_Category/{0:}/MVA_{0:}_rejBvsS'.format('BDTCategories_%s'%(vtag)))
  hists.append([h,2,'{0:} roc in tmvaout'.format(vtag)])
  cmp(hists,'diyroc_tmvaout','compare diy roc and tmvaout')

def _cmp_rocs():#trees
  fs = {}
  args = [
    ['28','v1'],
    ['v0','v0'],
    ['v1','v1'],
    ['v2','v2'],
    ['v3','v3'],
  ] 
  hists = _make_args(args,fs)
  cmp(hists,'roc_comparison','roc comparison')
def _cmp_rocs_tmvaout():
  args = [
    [0b1111111111,'NominalSets','test'],
  ]
  f_base = TFile('./mva_output/TMVA_v24.root')
  method_base = 'BDTCategories'
  method_temp = 'Method_Category/{0:}/MVA_{0:}_rejBvsS'
  h_base = f_base.Get(method_temp.format(method_base))
  fs = {}
  s = h_base.Integral()
  hists = [ [h_base,1,'Charles, I={0:.2f}'.format(s)]]
  
  for arg in args:
    postfix = '{0:}_{1:b}_{2:}'.format(arg[2],arg[0],arg[1])
    fname = './mva_output/TMVA_{0:}.root'.format(postfix)

    fs[fname] = TFile(fname)
    hs = fs[fname].Get(method_temp.format('BDT_{0:}'.format(postfix)))
    s = hs.Integral()
    hists.append([hs,2,'local training, I={0:.2f}'.format(s)])
  cmp(hists,'roc_comp_mvaout','roc comparison')


def get_hists_by_var(svar,fs):
    method_temp = 'Method_Category/{0:}/MVA_{0:}_rejBvsS'
    postfix = 'test_{0:}_NominalSets'.format(svar)
    fname = './mva_output/TMVA_{0:}.root'.format(postfix)
    
    fs[fname] = TFile(fname)
    hs = fs[fname].Get(method_temp.format('BDT_{0:}'.format(postfix)))
    return hs

def _cmp_roc_vars(varStr):
  usedVars = ["MET","HT","dPhiMETdijet","pTB1","pTB2","mBB","dRBB","dEtaBB","pTJ3","mBBJ"] 
  fs = {}
  hists = [] 
  hbase = get_hists_by_var('1111111111',fs)
  s = hbase.Integral()
  hists.append([hbase,None,'full vars, I={0:.2f}'.format(s),s])
  for n,c in enumerate(varStr):
    if c=='0':
      continue
    svar = varStr[0:n] + '0' + varStr[n+1:]
    hs = get_hists_by_var(svar,fs)
    s = hs.Integral()
    hists.append([hs,None,'w/o {0:}, I={1:.2f}'.format(usedVars[n],s),s])
  hists = sorted(hists,key=lambda x:x[3])
  for n,h in enumerate(hists):
    h[1] = C[n+1]
  cmp(hists,'roc_comp_vars','roc comparison')


def _cmp_rnn_bdt():
  njets = [2,3]
  parities = ["E","O"]
  t = 'roc_test_{0:}'
  t_n = 'roc_test_nw_{0:}'
  
  f_bdt = TFile('./data/roc_bdt.root')
  f_rnn = TFile('./data/roc_rnn.root')
  
  trans = {'E':'Even','O':'Odd'}

  for nj in njets:
    for parity in parities:
      cate = '{0:}j{1:}'.format(nj,parity)
      h_bdt = f_bdt.Get(t.format(cate))
      h_rnn = f_rnn.Get(t.format(cate))
      h_rnn_n = f_rnn.Get(t_n.format(cate))
      hists = [
        [h_bdt, kBlue, 'BDT, I={0:.2f}'.format(h_bdt.Integral()), '{0:} jets, {1:}'.format(nj,trans[parity])],
        [h_rnn, kRed, 'RNN, I={0:.2f}'.format(h_rnn.Integral()), '{0:} jets, {1:}'.format(nj,trans[parity])],
        #[h_rnn_n, kBlack, 'RNN,no Wei, I={0:.2f}'.format(h_rnn_n.Integral()), '{0:} jets, {1:}'.format(nj,trans[parity])]
      ]
      cmp(hists,'roc_{0:}'.format(cate),'{0:} jets, {1:}'.format(nj,trans[parity]))
      



def main():
#  _cmp_mva28_on_tree24()
#  _cmp_diy_rocs()  
#  _cmp_rocdiy_tmvaout('v1')
#  _cmp_rocs()
#  _cmp_rocs_tmvaout()
#  _cmp_roc_vars('1111111111')
  _cmp_rnn_bdt()

if __name__ == '__main__':
  main()
