
from multiprocessing import Pool
from ROOT import *
gROOT.LoadMacro('read.cxx')
def get_tuple(vtag):
  return (vtag,vtag,"TMVACAT_TREE24_%s.root"%(vtag),"TMVAClassification_BDTCategories_%s.weights.xml"%(vtag))
def fun(args):
  fill_tmva_local(*tuple(args))



data = [
  get_tuple('v0'),
]



p = Pool(2)
p.map(fun,data)
#print fill_tmva_local
#for d in data:    
#  print d
#  p.apply_async(fill_tmva_local,d)
#p.close()
#p.join()


