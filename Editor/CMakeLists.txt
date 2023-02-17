cmake_minimum_required(VERSION 3.12)

set(EDITOR_PROJECT_NAME Editor)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

set(EDITOR_INCLUDES
)
set(EDITOR_SOURCES
	sources/main.cpp
)
set(SHADERS
    ../data/Shaders/GLSL/PBR/PBR.frag
    ../data/Shaders/GLSL/PBR/PBR.vert
)
add_executable(${EDITOR_PROJECT_NAME}
	${EDITOR_INCLUDES}
	${EDITOR_SOURCES}
	${SHADERS}
)
set_property( DIRECTORY PROPERTY VS_STARTUP_PROJECT Editor )
target_compile_features(${EDITOR_PROJECT_NAME} PUBLIC cxx_std_20)
target_include_directories(${EDITOR_PROJECT_NAME} PUBLIC includes)
target_include_directories(${EDITOR_PROJECT_NAME} PRIVATE src)

target_link_libraries(${EDITOR_PROJECT_NAME} ${CONAN_LIBS})
target_link_libraries(${EDITOR_PROJECT_NAME} VulkanEngine)

add_custom_command(TARGET ${EDITOR_PROJECT_NAME}
    POST_BUILD 

    COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/data/Shaders/GLSL/Negative/Negative.frag        ${PROJECT_BINARY_DIR}/Editor/data/Shaders/GLSL/Negative/Negative.frag
    COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/data/Shaders/GLSL/Negative/Negative.vert        ${PROJECT_BINARY_DIR}/Editor/data/Shaders/GLSL/Negative/Negative.vert

    COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/data/Shaders/GLSL/Fullscreen/Fullscreen.frag        ${PROJECT_BINARY_DIR}/Editor/data/Shaders/GLSL/Fullscreen/Fullscreen.frag
    COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/data/Shaders/GLSL/Fullscreen/Fullscreen.vert        ${PROJECT_BINARY_DIR}/Editor/data/Shaders/GLSL/Fullscreen/Fullscreen.vert

    COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/data/Shaders/GLSL/PBR/PBR.frag        ${PROJECT_BINARY_DIR}/Editor/data/Shaders/GLSL/PBR/PBR.frag
    COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/data/Shaders/GLSL/PBR/PBR.vert        ${PROJECT_BINARY_DIR}/Editor/data/Shaders/GLSL/PBR/PBR.vert
	
    COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/data/Shaders/GLSL/Phong/phong.frag     ${PROJECT_BINARY_DIR}/Editor/data/Shaders/GLSL/Phong/phong.frag
    COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/data/Shaders/GLSL/Phong/phong.vert     ${PROJECT_BINARY_DIR}/Editor/data/Shaders/GLSL/Phong/phong.vert
    
    COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/data/Shaders/GLSL/Normal/normal.frag   ${PROJECT_BINARY_DIR}/Editor/data/Shaders/GLSL/Normal/normal.frag
    COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/data/Shaders/GLSL/Normal/normal.vert   ${PROJECT_BINARY_DIR}/Editor/data/Shaders/GLSL/Normal/normal.vert	
    COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/data/Shaders/GLSL/Normal/normal.geom   ${PROJECT_BINARY_DIR}/Editor/data/Shaders/GLSL/Normal/normal.geom	

    COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/data/Shaders/GLSL/Skybox/skybox.frag   ${PROJECT_BINARY_DIR}/Editor/data/Shaders/GLSL/Skybox/skybox.frag
    COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/data/Shaders/GLSL/Skybox/skybox.vert   ${PROJECT_BINARY_DIR}/Editor/data/Shaders/GLSL/Skybox/skybox.vert
	
    COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/data/Shaders/HLSL/Phong/phong_frag.hlsl ${PROJECT_BINARY_DIR}/Editor/data/Shaders/HLSL/Phong/phong_frag.hlsl
    COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/data/Shaders/HLSL/Phong/phong_vert.hlsl ${PROJECT_BINARY_DIR}/Editor/data/Shaders/HLSL/Phong/phong_vert.hlsl

    COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/data/Shaders/HLSL/PBR/PBR_frag.hlsl ${PROJECT_BINARY_DIR}/Editor/data/Shaders/HLSL/PBR/PBR_frag.hlsl
    COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/data/Shaders/HLSL/PBR/PBR_vert.hlsl ${PROJECT_BINARY_DIR}/Editor/data/Shaders/HLSL/PBR/PBR_vert.hlsl

    COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/data/Shaders/HLSL/Skybox/skybox_frag.hlsl ${PROJECT_BINARY_DIR}/Editor/data/Shaders/HLSL/Skybox/skybox_frag.hlsl
    COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/data/Shaders/HLSL/Skybox/skybox_vert.hlsl ${PROJECT_BINARY_DIR}/Editor/data/Shaders/HLSL/Skybox/skybox_vert.hlsl

    COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/data/Skybox ${PROJECT_BINARY_DIR}/Editor/data/Skybox
    COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/data/brdfLUT.png ${PROJECT_BINARY_DIR}/Editor/data/brdfLUT.png
)
if(CMAKE_BUILD_TYPE STREQUAL "Release")
	add_compile_definitions(CMAKE_BUILD_TYPE_RELEASE)
	message("It is Release!" )
else()
	add_compile_definitions(CMAKE_BUILD_TYPE_DEBUG)
	message("It is Debug!" )
endif()

install(TARGETS ${EDITOR_PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX}/install)
install(DIRECTORY ${PROJECT_SOURCE_DIR}/data DESTINATION ${CMAKE_INSTALL_PREFIX}/install)
