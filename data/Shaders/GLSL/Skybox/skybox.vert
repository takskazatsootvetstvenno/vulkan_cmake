//
#version 450

layout(location = 0) in vec3 position_in;

layout(location = 0) out vec3 texCoords_out;

layout(set = 0, binding = 0) uniform GlobalUbo
{
	mat4 projectionMatrix;
	mat4 viewMatrix;
} globalUBO;

void main(){          
	vec3 pos_in = position_in;
	/*mat4 new_view;
	new_view[0] = vec4(globalUBO.viewMatrix[0].xyz, 0.f);
	new_view[1] = vec4(globalUBO.viewMatrix[1].xyz, 0.f);
	new_view[2] = vec4(globalUBO.viewMatrix[2].xyz, 0.f);
	new_view[3] = vec4(0.f, 0.f, 0.f, 0.f);
	*/
	vec4 pos = globalUBO.projectionMatrix * vec4(mat3(globalUBO.viewMatrix) * pos_in.xyz, 1.f);
	texCoords_out = position_in;
	gl_Position = pos.xyww;
}