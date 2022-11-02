cmake_minimum_required(VERSION 3.12)

SET(PROJECT_NAME VulkanEngine)

set(default_build_type "Release")

get_filename_component(GENERAL_FOLDER ../ ABSOLUTE)
set(CMAKE_INSTALL_PREFIX "${GENERAL_FOLDER}" CACHE PATH "${GENERAL_FOLDER}" FORCE)

project(${PROJECT_NAME})

if(MSVC)
	 add_compile_options(/MP)
endif()

add_subdirectory(VulkanEngine)
add_subdirectory(Editor)
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT Editor)