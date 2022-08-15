//
#version 450
layout (triangles) in;
layout (line_strip, max_vertices = 6) out;

layout(location = 0) in GS_IN {
    vec3 normal;
} gs_in[];

layout(set = 0, binding = 0) uniform GlobalUbo
{
	mat4 projectionMatrix;
	mat4 viewMatrix;
	vec3 cameraPosition;
} globalUBO;

layout(set = 0, binding = 1) uniform NormalUboInfo
{
    mat4 model;
	float magnitude;
} normalTestUBO;

layout(location = 0) out vec4 color;

void GenerateLine(int index)
{
    gl_Position = globalUBO.projectionMatrix * gl_in[index].gl_Position;
    color = vec4(1.f, 0.f, 0.f, 0.f);
    EmitVertex();
    gl_Position = globalUBO.projectionMatrix * (gl_in[index].gl_Position + 
                                vec4(gs_in[index].normal, 0.0) * normalTestUBO.magnitude);
    color = vec4(0.f, 0.f, 1.f, 0.f);
    EmitVertex();
    EndPrimitive();
}

void main()
{
    GenerateLine(0); // first vertex normal
    GenerateLine(1); // second vertex normal
    GenerateLine(2); // third vertex normal
}