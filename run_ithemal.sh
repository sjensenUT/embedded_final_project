# must be connected to docker to run
cd "${ITHEMAL_HOME}"
EXPERIMENT_NAME="test-experiment"
EXPERIMENT_TIME="$(python -c 'import time,datetime; print(datetime.datetime.fromtimestamp(time.time()).isoformat())')"
echo "Experiment Time = "
echo $EXPERIMENT_TIME
DATA_FILE="ffmpeg.pkl"
NUM_TRAINERS="1" # to actually train, we used 6, but 1 is better to make sure that it all works
#wget https://github.com/ithemal/bhive/raw/cd5f9db/sources/ffmpeg.csv # get the code strings
#wget https://github.com/ithemal/bhive/raw/cd5f9db/throughput/hsw.csv # get the HSW timings
python -c "import subprocess, torch; hsw = {k: float(v) for (k, v) in map(lambda x: x.split(','), open('hsw.csv').readlines())}; l = [(i, hsw[l], None, subprocess.check_output(['data_collection/build/bin/tokenizer', l.split(',')[0], '--token'], universal_newlines=True).strip()) for i, (l, _) in enumerate(map(lambda x: x.split(','), open('ffmpeg.csv').readlines())) if l in hsw]; torch.save(l, '${DATA_FILE}')"
python -c "import torch; torch.save([x for x in torch.load('${DATA_FILE}') if len(x[-1]) > 20], '${DATA_FILE}')"
python learning/pytorch/ithemal/run_ithemal.py --data ${DATA_FILE} --use-rnn train --experiment-name ${EXPERIMENT_NAME} --experiment-time ${EXPERIMENT_TIME} --sgd --threads 4 --trainers ${NUM_TRAINERS} --weird-lr --decay-lr --epochs 1
