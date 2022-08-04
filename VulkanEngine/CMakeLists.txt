cmake_minimum_required(VERSION 3.12)

set(CORE_PROJECT_NAME VulkanEngine)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

set(CORE_INCLUDES
	includes/Renderer.h
	includes/App.h
	includes/Device.h
	includes/Pipeline.h
	includes/SwapChain.h
	includes/Window.h
	includes/Camera.h
	includes/Model.h
	includes/Logger.h
	includes/Buffer.h
	includes/Mesh.h
	includes/Descriptors.h
	includes/MeshMGR.h	
	includes/VulkanHelpUtils.h
	includes/Event.h
	includes/Shader.h
	includes/Texture.h
)
set(CORE_SOURCES
	sources/Renderer.cpp
	sources/App.cpp
	sources/Device.cpp
	sources/Pipeline.cpp
	sources/SwapChain.cpp
	sources/Window.cpp
	sources/Camera.cpp
	sources/Model.cpp
	sources/Logger.cpp
	sources/Buffer.cpp
	sources/Mesh.cpp
	sources/Descriptors.cpp
	sources/MeshMGR.cpp
	sources/Shader.cpp
	sources/Texture.cpp
)
add_library(${CORE_PROJECT_NAME} STATIC
	${CORE_INCLUDES}
	${CORE_SOURCES}
	../../bindings/imgui_impl_glfw.cpp
	../../bindings/imgui_impl_vulkan.cpp
	../../bindings/imgui_impl_glfw.h
	../../bindings/imgui_impl_vulkan.h
)
set_property( DIRECTORY PROPERTY VS_STARTUP_PROJECT VulkanEngine )
target_compile_features(${CORE_PROJECT_NAME} PUBLIC cxx_std_20)
target_include_directories(${CORE_PROJECT_NAME} PUBLIC includes)
target_include_directories(${CORE_PROJECT_NAME} PRIVATE src)

target_link_libraries(${CORE_PROJECT_NAME} ${CONAN_LIBS})

install(TARGETS ${CORE_PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX}/install)
