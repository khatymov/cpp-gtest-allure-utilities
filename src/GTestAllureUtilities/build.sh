#!/bin/bash

# Get the directory where the script is located
# script_dir="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

# Change to the project root directory
# cd "${script_dir}/.."

build_dir="build"

if [ -f "CMakeUserPresets.json" ]; then
  rm "CMakeUserPresets.json" || exit $?
fi

if [ -d "${build_dir}" ]; then
  rm -rf "${build_dir}" || exit $?
fi

cmake_build_type="Release"

# Install dependencies
conan install . --output-folder="${build_dir}" --build=missing -s build_type=${cmake_build_type} || exit $?

cd "${build_dir}" || exit $?

# Build
cmake .. -G "Ninja"  -DCMAKE_TOOLCHAIN_FILE="conan_toolchain.cmake" -DCMAKE_BUILD_TYPE=${cmake_build_type} || exit $?
cmake --build . || exit $?
