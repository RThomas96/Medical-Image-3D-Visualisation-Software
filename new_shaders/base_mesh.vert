#version 150
#extension GL_ARB_explicit_attrib_location : enable

layout(location=0) in vec3 in_position;
layout(location=1) in vec3 in_normal;
layout(location=2) in vec2 in_texture;

out vec4 position; // WARNING ! This will be the 'raw' 3D position !!!
out vec4 normal;
out vec2 texture;

uniform mat4 proj;
uniform mat4 view;
uniform mat4 model;
uniform vec4 camera_pos;

void main() {
	mat3 begin_normal_matrix = mat3(view[0].xyz, view[1].xyz, view[2].xyz);
	mat3 normal_matrix_3 = inverse(transpose(begin_normal_matrix));
	position = model * vec4(in_position, 1.f);
	gl_Position = proj * view * position;
	normal = vec4(in_normal, .0f);
	texture = in_texture;
}
