#version 150
#extension GL_ARB_explicit_attrib_location : enable

in vec4 out_position;
in vec4 out_normal;

uniform vec4 sphere_color;

out vec4 color;

void main() {
	color = sphere_color;
}
