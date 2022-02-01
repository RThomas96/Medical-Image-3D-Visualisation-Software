#version 150
#extension GL_ARB_explicit_attrib_location : enable

layout(location=0) in vec3 in_position;
layout(location=1) in vec3 in_normal;

out vec4 out_position;
out vec4 out_normal;

uniform mat4 proj;
uniform mat4 view;
uniform float scale;
uniform vec3 position;

void main() {
	gl_Position = proj * view * vec4((in_position * scale) + position, 1.f);
	out_position = gl_Position;
	out_normal = vec4(in_normal, .0f);
}
