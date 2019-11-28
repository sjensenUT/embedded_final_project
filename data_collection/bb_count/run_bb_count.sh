#!/usr/bin/env bash

$DYNAMORIO_HOME/bin64/drrun -c ./build/bin/libbb_count.so cactus_counts.csv -- $BENCHMARKS_DIR/436.cactusADM/exe/cactusADM_base.amd64-m64-gcc43-nn $BENCHMARKS_DIR/436.cactusADM/exe/benchADM.par 
