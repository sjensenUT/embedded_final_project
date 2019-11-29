export START_DIR=`pwd`
pushd ../ithemal/learning/pytorch/ithemal

export COUNTS_PATH=../../../../data_collection/counts
# TODO: replace with our model
export MODEL_PATH=../../../../Ithemal-models/paper/skylake

#python $START_DIR/print_bbs.py $COUNTS_PATH/bzip2_counts.csv | python predict.py --model $MODEL_PATH/predictor.dump --model-data $MODEL_PATH/trained.mdl --raw-stdin > $START_DIR/bzip2_predictions.csv

#python $START_DIR/print_bbs.py $COUNTS_PATH/mcf_counts.csv | python predict.py --model $MODEL_PATH/predictor.dump --model-data $MODEL_PATH/trained.mdl --raw-stdin > $START_DIR/mcf_predictions.csv

#python $START_DIR/print_bbs.py $COUNTS_PATH/hmmer_counts.csv | python predict.py --model $MODEL_PATH/predictor.dump --model-data $MODEL_PATH/trained.mdl --raw-stdin > $START_DIR/hmmer_predictions.csv

python $START_DIR/print_bbs.py $COUNTS_PATH/cactusADM_counts.csv | python predict.py --model $MODEL_PATH/predictor.dump --model-data $MODEL_PATH/trained.mdl --raw-stdin > $START_DIR/cactusADM_predictions.csv

popd
