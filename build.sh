#!/usr/bin/env bash

cd $(dirname $(readlink -f $0))

mkdir cmake-build -p
cd cmake-build
rm -rf *
cmake ../
make
