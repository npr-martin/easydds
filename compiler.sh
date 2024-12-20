#!/bin/bash
mkdir foonathan_memory_vendor/build
cd foonathan_memory_vendor/build
cmake .. -DCMAKE_INSTALL_PREFIX=../../install -DBUILD_SHARED_LIBS=ON
cmake --build . --target install -- $1

cd ../../

mkdir Fast-CDR/build
cd Fast-CDR/build
cmake .. -DCMAKE_INSTALL_PREFIX=../../install
cmake --build . --target install -- $1

cd ../../
mkdir Fast-DDS/build
cd Fast-DDS/build
cmake ..  -DCMAKE_INSTALL_PREFIX=../../install
cmake --build . --target install -- $1

cd ../../
mkdir easydds/build
cd easydds/build
cmake ..  -DCMAKE_INSTALL_PREFIX=../../install
cmake --build . --target install -- $1

cd ../../
mkdir Easy-DDS-Monitor/build
cd Easy-DDS-Monitor/build
qmake ../EDMonitor  
make $1
cp ./Easy-DDS-Monitor ../../install/bin


cd ../../
mkdir Easy-DDS-Test/build
cd Easy-DDS-Test/build
qmake ../EDTest  
make $1
cp ./EDTest ../../install/bin
cp ../EDTest/easydds.sh ../../install/bin
