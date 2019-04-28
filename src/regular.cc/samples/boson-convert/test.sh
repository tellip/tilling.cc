#!/usr/bin/env bash

cd $(dirname $0)

../../cmake-build-debug/boson-convert source.txt target.txt
