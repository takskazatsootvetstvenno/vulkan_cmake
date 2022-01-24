#version 450

layout(location = 0) in vec2 position;
layout(location = 0) out vec2 out_pos;

void main(){
	out_pos = position;
	gl_Position = vec4(position, 0.0, 1.0);
}