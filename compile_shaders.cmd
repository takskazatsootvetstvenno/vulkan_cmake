glslc.exe data\shaders\GLSL\PBR\PBR.vert -o data\shaders\GLSL\PBR\PBR.vert.spv
glslc.exe data\shaders\GLSL\PBR\PBR.frag -o data\shaders\GLSL\PBR\PBR.frag.spv

glslc.exe data\shaders\GLSL\Phong\phong.vert -o data\shaders\GLSL\Phong\phong.vert.spv
glslc.exe data\shaders\GLSL\Phong\phong.frag -o data\shaders\GLSL\Phong\phong.frag.spv

glslc.exe -x hlsl -fshader-stage=vert data\shaders\HLSL\Phong\phong_vert.hlsl -o data\shaders\HLSL\Phong\phong.vert.spv
glslc.exe -x hlsl -fshader-stage=frag data\shaders\HLSL\Phong\phong_frag.hlsl -o data\shaders\HLSL\Phong\phong.frag.spv

glslc.exe -x hlsl -fshader-stage=vert data\shaders\HLSL\PBR\PBR_vert.hlsl -o data\shaders\HLSL\PBR\PBR.vert.spv
glslc.exe -x hlsl -fshader-stage=frag data\shaders\HLSL\PBR\PBR_frag.hlsl -o data\shaders\HLSL\PBR\PBR.frag.spv
