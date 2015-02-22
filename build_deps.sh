#!/bin/bash

cd custom_glv
PREFIX=$(pwd)/ make
PREFIX=$(pwd)/ make install
cd ..

cd freeglut
cmake .
make
cd ..

