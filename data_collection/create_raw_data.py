# Creates a raw data file to train Ithemal
# Input: A CSV file containing a list of BB's and timing numbers measured by the profiler
# Output: A file that contains the raw data used to train ithemal. In addition to the timing numbers, it also stores the tokenized representation of each BB as well as a human-readable string.

import os
import sys
import subprocess
import torch

TOKENIZER_PATH = "../ithemal/data_collection/build/bin/tokenizer"

timing_file = sys.argv[1]
output_file = sys.argv[2]

lines = []
with open(timing_file, 'r') as f:
    lines = f.readlines()

print("Read {} lines from input file.\n".format(len(lines)))

def call_tokenizer(bb, action):
    p = subprocess.Popen(TOKENIZER_PATH + " " + bb + " " + action,
           stdout=subprocess.PIPE, stderr=subprocess.PIPE, shell=True)
    out, err = p.communicate()
    if len(err) > 0:
        return None
    else:
        return out.rstrip()

def get_assembly_code(bb):
    return call_tokenizer(bb, '--intel')

def get_tokenization(bb):
    return call_tokenizer(bb, '--token')    

id = 0
raw_data = []
for line in lines:

    # Get the basic block and its throughput value
    bb_str, throughput = line.rstrip().split(',')
    throughput = float(throughput)

    # Get the human-readable assembly representation of the BB (intel syntax)
    assembly_code = get_assembly_code(bb_str)

    # Get the tokenized representation of the BB
    tokenization = get_tokenization(bb_str)

    # Construct the tuple and add it to the list
    if (assembly_code is None) or (tokenization is None):
        print("Tokenizer failed on BB {}, skipping.".format(id))
    else:
        bb_info = (id, throughput, assembly_code, tokenization)
        raw_data.append(bb_info)

    # TEMP
    if (id < 10):
      print(str(bb_info))

    # Progress indicator
    id = id + 1
    if (id % 1000 == 0):
        print("Progress: i = {}".format(id))

print("torch.saving data to output file...")
torch.save(raw_data, output_file)
print("Done.")
