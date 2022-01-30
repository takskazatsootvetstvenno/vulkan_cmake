# vulkan_cmake
This is a test project for Vulkan api

For Windows OS: use WidnowsDebug.cmd or WindowsRelease.cmd scripts

This project requires:
Cmake, 
Visual studio 2019,
Conan,
video driver with vulkan support.

Optionaly: 

Layer "VK_LAYER_KHRONOS_validation" in system.
# Building
 
Create new directory (for example "test_dir"),
 
cd "test_dir", 
 
gitclone https://github.com/takskazatsootvetstvenno/vulkan_cmake.git,
 
cd vulkan_cmake
 
./WidnowsDebug.cmd or ./WindowsRelease.cmd 
 
in test_dir you will found build and install directory
# Used materials/libs
 
Used materials from https://vulkan-tutorial.com/

Also used:

Volk vulkan loader: https://github.com/zeux/volk

GLFW: https://github.com/glfw/glfw