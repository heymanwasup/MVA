# BDT Training

A easy framework to inveestigate the variables ranking in VHBB 0leptopn analysis.

## Usage:
You can launch the MVA(BDT) training parallelly:
```
git clone https://github.com/heymanwasup/MVA.git
cd MVA
python launch_training.py
```
In the main function of `launch_training.py` you can chose run on batch or locally, each time you pass a binary str, in which `1` represents the corresponding variable is used during the training, '0' means not used. The variables ranking is defined at easy_train.py

There are some scripts in `./read_mva` could be used for drawing the BDT distribution of signal\background and ROC curve.  
