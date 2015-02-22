#!/bin/bash

echo "-- building minilisp"

cd minilisp
./buildlib.sh
cd ..

echo "-- building GLV"

cd custom_glv
PREFIX=$(pwd)/ make
PREFIX=$(pwd)/ make install
cd ..

echo "-- building FreeGLUT"

cd freeglut
cmake .
make
cd ..

echo "-- done!"

