cmake_minimum_required(VERSION 3.12)

set(CORE_PROJECT_NAME VulkanEngine)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

set(CORE_INCLUDES
	includes/App.h
	includes/Device.h
	includes/Pipeline.h
	includes/SwapChain.h
	includes/Window.h
)
set(CORE_SOURCES
	sources/App.cpp
	sources/Device.cpp
	sources/main.cpp
	sources/Pipeline.cpp
	sources/SwapChain.cpp
	sources/Window.cpp
)
add_executable(${CORE_PROJECT_NAME}
	${CORE_INCLUDES}
	${CORE_SOURCES}
)
set_property( DIRECTORY PROPERTY VS_STARTUP_PROJECT VulkanEngine )
target_compile_features(${CORE_PROJECT_NAME} PUBLIC cxx_std_20)
target_include_directories(${CORE_PROJECT_NAME} PUBLIC includes)
target_include_directories(${CORE_PROJECT_NAME} PRIVATE src)

target_link_libraries(${CORE_PROJECT_NAME} ${CONAN_LIBS})

add_custom_command(TARGET ${CORE_PROJECT_NAME}
    POST_BUILD 
    COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/data/Shaders/fragment.frag ${PROJECT_BINARY_DIR}/VulkanEngine/Shaders/fragment.frag
    COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/data/Shaders/fragment.frag.spv ${PROJECT_BINARY_DIR}/VulkanEngine/Shaders/fragment.frag.spv
    COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/data/Shaders/vertex.vert ${PROJECT_BINARY_DIR}/VulkanEngine/Shaders/vertex.vert
    COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/data/Shaders/vertex.vert.spv ${PROJECT_BINARY_DIR}/VulkanEngine/Shaders/vertex.vert.spv
)
if(CMAKE_BUILD_TYPE STREQUAL "Release")
	add_compile_definitions(CMAKE_BUILD_TYPE_RELEASE)
	message("It is Release!" )
else()
	add_compile_definitions(CMAKE_BUILD_TYPE_DEBUG)
	message("It is Debug!" )
endif()

install(TARGETS ${CORE_PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX}/install)
install(DIRECTORY ${PROJECT_SOURCE_DIR}/data/Shaders DESTINATION ${CMAKE_INSTALL_PREFIX}/install)