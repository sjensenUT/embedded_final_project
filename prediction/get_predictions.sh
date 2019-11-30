export START_DIR=`pwd`
pushd ../ithemal/learning/pytorch/ithemal

export COUNTS_PATH=../../../../data_collection/counts
export MODEL_PATH=$START_DIR/../trained_model_files

#python $START_DIR/print_bbs.py $COUNTS_PATH/bzip2_counts.csv | python predict.py --model $MODEL_PATH/predictor.dump --model-data $MODEL_PATH/trained.mdl --raw-stdin > $START_DIR/predictions/bzip2_predictions.csv

#python $START_DIR/print_bbs.py $COUNTS_PATH/mcf_counts.csv | python predict.py --model $MODEL_PATH/predictor.dump --model-data $MODEL_PATH/trained.mdl --raw-stdin > $START_DIR/predictions/mcf_predictions.csv

#python $START_DIR/print_bbs.py $COUNTS_PATH/hmmer_counts.csv | python predict.py --model $MODEL_PATH/predictor.dump --model-data $MODEL_PATH/trained.mdl --raw-stdin > $START_DIR/predictions/hmmer_predictions.csv

#python $START_DIR/print_bbs.py $COUNTS_PATH/cactusADM_counts.csv | python predict.py --model $MODEL_PATH/predictor.dump --model-data $MODEL_PATH/trained.mdl --raw-stdin > $START_DIR/predictions/cactusADM_predictions.csv

#python $START_DIR/print_bbs.py $COUNTS_PATH/gcc_counts.csv | python predict.py --model $MODEL_PATH/predictor.dump --model-data $MODEL_PATH/trained.mdl --raw-stdin > $START_DIR/predictions/gcc_predictions.csv

python $START_DIR/print_bbs.py $COUNTS_PATH/libquantum_counts.csv | python predict.py --model $MODEL_PATH/predictor.dump --model-data $MODEL_PATH/trained.mdl --raw-stdin > $START_DIR/predictions/libquantum_predictions.csv

popd
