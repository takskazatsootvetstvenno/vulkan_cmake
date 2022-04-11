#!/bin/bash
#sudo pip install conan
#sudo apt-get -y install cmake
#sudo apt-get install libgl1-mesa-dev
#sudo apt-get install libglu1-mesa-dev

cd ..
rm -r build
rm -r install
mkdir build
export CONAN_SYSREQUIRES_MODE=enabled
conan install vulkan_cmake -if build --build missing -s build_type=Debug -r conancenter
cd build
cmake ../vulkan_cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Debug
cmake --build . --config Debug
cmake --install .
