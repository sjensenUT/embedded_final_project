#!/usr/bin/env bash

$DYNAMORIO_HOME/bin64/drrun -c ./build/bin/libbb_count.so ../counts/cactusADM_counts.csv -- $BENCHMARKS_DIR/436.cactusADM/exe/cactusADM_base.amd64-m64-gcc43-nn $BENCHMARKS_DIR/436.cactusADM/exe/benchADM.par 

$DYNAMORIO_HOME/bin64/drrun -c ./build/bin/libbb_count.so ../counts/mcf_counts.csv -- $BENCHMARKS_DIR/429.mcf/exe/mcf_base.amd64-m64-gcc43-nn $BENCHMARKS_DIR/429.mcf/data/test/input/inp.in

$DYNAMORIO_HOME/bin64/drrun -c ./build/bin/libbb_count.so ../counts/bzip2_counts.csv -- $BENCHMARKS_DIR/401.bzip2/exe/bzip2_base.amd64-m64-gcc43-nn $BENCHMARKS_DIR/401.bzip2/data/test/input/dryer.jpg 1

$DYNAMORIO_HOME/bin64/drrun -c ./build/bin/libbb_count.so ../counts/hmmer_counts.csv -- $BENCHMARKS_DIR/456.hmmer/exe/hmmer_base.amd64-m64-gcc43-nn --fixed 0 --mean 325 --num 5000 --sd 200 --seed 0 $BENCHMARKS_DIR/456.hmmer/data/test/input/bombesin.hmm


