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
`sudo ./bb_measure.py`

Open `bb_measure.py` and change filenames/numbers as necessary

To generate the raw data file:
- Make sure you have python3 installed with packages `subprocess` and `torch`
- Make sure Dynamorio is installed and built (see Ithemal README)
- Set the `$DYNAMORIO_HOME` environment variable to where Dynamorio was built
- Run `setup.sh` in this repo's top-level directory as follows: `. ./setup.sh`
- in `data_collection/`, run (for example):
`python3 create_raw_data.py inputfile.csv outputfile.data

