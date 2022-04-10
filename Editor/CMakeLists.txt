cmake_minimum_required(VERSION 3.12)

set(EDITOR_PROJECT_NAME Editor)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

set(EDITOR_INCLUDES
)
set(EDITOR_SOURCES
	sources/main.cpp
)
add_executable(${EDITOR_PROJECT_NAME}
	${EDITOR_INCLUDES}
	${EDITOR_SOURCES}
)
set_property( DIRECTORY PROPERTY VS_STARTUP_PROJECT Editor )
target_compile_features(${EDITOR_PROJECT_NAME} PUBLIC cxx_std_20)
target_include_directories(${EDITOR_PROJECT_NAME} PUBLIC includes)
target_include_directories(${EDITOR_PROJECT_NAME} PRIVATE src)

target_link_libraries(${EDITOR_PROJECT_NAME} ${CONAN_LIBS})
target_link_libraries(${EDITOR_PROJECT_NAME} VulkanEngine)

execute_process(COMMAND ${PROJECT_SOURCE_DIR}/compile_shaders.cmd  WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
if(MSVC)
add_custom_command(TARGET ${EDITOR_PROJECT_NAME}
    POST_BUILD 
    COMMAND ${PROJECT_SOURCE_DIR}/compile_shaders.cmd  WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
endif()
add_custom_command(TARGET ${EDITOR_PROJECT_NAME}
    POST_BUILD 
    COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/data/Shaders/fragment.frag ${PROJECT_BINARY_DIR}/Editor/Shaders/fragment.frag
    COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/data/Shaders/fragment.frag.spv ${PROJECT_BINARY_DIR}/Editor/Shaders/fragment.frag.spv
    COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/data/Shaders/vertex.vert ${PROJECT_BINARY_DIR}/Editor/Shaders/vertex.vert
    COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/data/Shaders/vertex.vert.spv ${PROJECT_BINARY_DIR}/Editor/Shaders/vertex.vert.spv
)
if(CMAKE_BUILD_TYPE STREQUAL "Release")
	add_compile_definitions(CMAKE_BUILD_TYPE_RELEASE)
	message("It is Release!" )
else()
	add_compile_definitions(CMAKE_BUILD_TYPE_DEBUG)
	message("It is Debug!" )
endif()

install(TARGETS ${EDITOR_PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX}/install)