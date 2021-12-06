#version 150
#extension GL_ARB_explicit_attrib_location : enable

in vec4 position;
in vec4 normal;
in vec2 texture;

out vec4 color;

void main() {
	color = normal;
}
