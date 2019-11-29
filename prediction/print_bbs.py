import sys

ifname = sys.argv[1]

lines = []
with open(ifname, "r") as f:
    lines = f.readlines()
    
for line in lines:
    print(line.rstrip().split(",")[0])
