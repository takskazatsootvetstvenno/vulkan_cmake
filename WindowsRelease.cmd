cd ..
if exist build rmdir /s /q build
if exist install rmdir /s /q install
mkdir build
conan install VulkanEngine -if build --build missing -s build_type=Release -r conancenter
cd build
cmake ../VulkanEngine -G "Visual Studio 16 2019" -A x64 -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release
cmake --install .
