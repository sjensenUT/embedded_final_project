import os
import subprocess

ifn = '../bhive/throughput/skl.csv'
ofn = 'daniels_laptop.csv'
bbs_per_run = 500
total_runs = 628

for x in range(total_runs):
  p = subprocess.Popen("sudo ./bb_measure " + ifn + " " + ofn + " -n " + str(bbs_per_run)
    + " -s " + str(x*bbs_per_run), stdout=subprocess.PIPE, stderr=subprocess.PIPE, shell=True)
  out, err = p.communicate()
  print(out)



