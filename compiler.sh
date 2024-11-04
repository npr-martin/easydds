#!/bin/bash
mkdir foonathan_memory_vendor/build
cd foonathan_memory_vendor/build
cmake .. -DCMAKE_INSTALL_PREFIX=../../install -DBUILD_SHARED_LIBS=ON
cmake --build . --target install

cd ../../

mkdir Fast-CDR/build
cd Fast-CDR/build
cmake .. -DCMAKE_INSTALL_PREFIX=../../install
cmake --build . --target install

cd ../../
mkdir Fast-DDS/build
cd Fast-DDS/build
cmake ..  -DCMAKE_INSTALL_PREFIX=../../install
cmake --build . --target install

cd ../../
mkdir easydds/build
cd easydds/build
cmake ..  -DCMAKE_INSTALL_PREFIX=../../install
make 

