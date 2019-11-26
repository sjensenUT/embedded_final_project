pushd ithemal
export ITHEMAL_HOME=`pwd`
cd $DYNAMORIO_HOME/lib64/release
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:`pwd`
popd
