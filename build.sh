#!/bin/bash
mkdir cmakedist
cd cmakedist
rm -rf *
cmake ..
make
make package
