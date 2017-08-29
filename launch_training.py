from multiprocessing import Pool
import os
from ROOT import *
gROOT.SetBatch(1)
gROOT.LoadMacro('easy_train.C')

def fun(args):
  easy_train(*tuple(args))

cfg = "!H:!V:NTrees=200:MaxDepth=4:BoostType=AdaBoost:AdaBoostBeta=0.15:SeparationType=GiniIndex:nCuts=100:NEventsMin=100:PruneMethod=NoPruning"

def submit_diy():
  data = [
  [0b0111111111,cfg,'NominalSets','test'],
]
  fun(data[0])

p = Pool(8)
def get_args(variables):
  data = []
  for n,s in enumerate(variables):
    if s=='1':
      svar = variables[0:n] + '0' + variables[n+1:]
      nvar = int(svar,2)
      data.append([nvar,cfg,'NominalSets','test'])
  return data
def LOCAL_SUB_AUTO(variables):
  data = get_args(variables)
  p.map(fun,data)
def LOCAL_SUB(data):
  p.map(fun,data)
def BATCH_SUB_AUTO(variables):
  data = get_args(variables)
  #cmd_head = 'source '
  BATCH_SUB(data)
def BATCH_SUB(data):
  cmd_head = 'bsub -q 8nh '
  pwd = '/afs/cern.ch/user/c/chenc/mva_vhbb0l'
  cmd_tail = pwd+'/run.sh {0:} {1:} {2:} {3:}'
  for d in data:
    varStr = '{0:b}'.format(d[0])
    post_fix =  '{0:}_{1:}_{2:}'.format(d[3],varStr,d[2])
    varstr = '0b' + varStr
    b_cmd = cmd_head + cmd_tail.format(varstr,d[1],d[2],d[3])
    print b_cmd
    os.system(b_cmd)

  fun(data[0])

if __name__ == '__main__':
  #LOCAL_SUB('1111111111')
  #BATCH_SUB_AUTO('1111111111')
  data = [
    [0b1111111111,cfg,'NominalSets','test'],
  ]
  BATCH_SUB(data)

