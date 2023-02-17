//
#version 450

layout(location = 0) out vec2 texCoords_out;

vec2 positions[3] = vec2[](
	vec2(-1.f, -3.f),
	vec2( 3.f,  1.f),
	vec2(-1.f,  1.f)
);

void main(){
	const vec2 pos = positions[gl_VertexIndex];
	texCoords_out = pos * 0.5 + vec2(0.5f);
	gl_Position = vec4(pos, 0.f, 1.f);
}