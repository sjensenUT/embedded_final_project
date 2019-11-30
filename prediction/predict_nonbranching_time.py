# Predicts total non-branching time of a workload given its BB counts and BB execution time predictions

import sys

counts_file = sys.argv[1]
predictions_file = sys.argv[2]

bb_counts = {}
counts_lines = []
preds_lines  = []

unaccounted_bbs = 0
accounted_bbs = 0
total_cycles = 0.0

with open(counts_file, 'r') as f:
    counts_lines = f.readlines()
    counts_lines = [x.rstrip() for x in counts_lines]

with open(predictions_file, 'r') as f:
    preds_lines = f.readlines()
    preds_lines = [x.rstrip() for x in preds_lines]

for line in counts_lines:
    bb, count = line.split(",")
    if bb in bb_counts:
        # Handle duplicates
        bb_counts[bb] = bb_counts[bb] + int(count)
    else:
        bb_counts[bb] = int(count)

for line in preds_lines:
    bb, pred = line.split(",")

    if bb not in bb_counts:
        # This shouldn't happen. But just in case...
        continue

    # Unfortunately, the tokenizer fails on certain BBs during prediction
    # This appears to count for < 1% of all BBs on average, so hopefully
    # it doesn't make our model significantly worse. Maybe one day we'll
    # figure out why.
    if pred == "fail":
        unaccounted_bbs = unaccounted_bbs + bb_counts[bb]
    else:
        accounted_bbs = accounted_bbs + bb_counts[bb]
        # Make sure to divide the prediction by 100, as each prediction 
        # is for a BB unrolled 100 times.
        total_cycles = total_cycles + (bb_counts[bb] * (float(pred)/100.0))

print("Total estimated non-branching time: {} cycles.".format(total_cycles))
print("Unfortunately, {} out of {} ({}%) BBs were unaccounted for.".format (
          unaccounted_bbs,
          unaccounted_bbs + accounted_bbs, 
          float(unaccounted_bbs) * 100 / (unaccounted_bbs + accounted_bbs) )
     )
    
