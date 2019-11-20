This is The code for our embedded systems final project


# Data Collection

To run the data collection script:

```
cd timing-harness
make NDEBUG:=1

cd ../data_collection
make
```

To run data collection:
`sudo ./bb_measure [input filename] [output filename]`

Where `[input filename]` is the reference file for throughput data, e.g. `../bhive/throughput/skl.csv`

To run data collection only for the first `N` basic blocks in the input file:
`sudo ./bb_measure [input filename] [output filename] -n N`

The script will print out a progress indicator every 100 iterations.

See `run_bb_measure.sh` for an example of how to invoke the script.
