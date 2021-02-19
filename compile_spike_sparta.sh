#/bin/bash!

sparta_connector_files=( "SpikeConnector" "SpikeInstructionStream" "MemoryInstruction" "FixedLatencyInstruction" "BaseInstruction" "StateInstruction")

for f in "${sparta_connector_files[@]}"
do
    cp $SPIKE_MODEL_SRC/$f.cpp $f.cc
done

../configure --prefix=/home/bscuser/local/riscvv08/gnu/ --no-create --no-recursion
make -j 2 && sudo make install

for f in "${sparta_connector_files[@]}"
do
    rm $f.cc
done
