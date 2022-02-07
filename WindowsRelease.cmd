cd ..
if exist build rmdir /s /q build
if exist install rmdir /s /q install
mkdir build
conan install vulkan_cmake -if build --build missing -s build_type=Release -s compiler="Visual Studio" -r conancenter
cd build
cmake ../vulkan_cmake -G "Visual Studio 16 2019" -A x64 -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release
cmake --install .
