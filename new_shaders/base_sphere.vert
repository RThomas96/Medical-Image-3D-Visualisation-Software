#version 150
#extension GL_ARB_explicit_attrib_location : enable

layout(location=0) in vec3 in_position;
layout(location=1) in vec3 in_normal;

out vec4 out_position;
out vec4 out_normal;

uniform mat4 proj;
uniform mat4 view;
uniform mat4 model;
uniform float scale;
uniform vec3 position;

void main() {
	vec4 p4 = model * vec4(position, 1.f);
	gl_Position = proj * view * (p4 + vec4(in_position * scale, 0.f));
	out_position = p4;
	out_normal = vec4(in_normal, .0f);
}
