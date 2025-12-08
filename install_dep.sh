#!/bin/bash
set -x
#TODO reuse code
if [[ ${USER} = "root" ]] || [[ -z ${USER} ]]; then
    apt-get update -y && apt install build-essential gcc g++ python3 cmake make lcov python3-pip valgrind -y
    python3 -m pip install conan
else
    sudo apt-get update -y && sudo apt install build-essential gcc g++ python3 cmake make lcov python3-pip valgrind -y
    sudo python3 -m pip install conan
fi

conan profile detect
