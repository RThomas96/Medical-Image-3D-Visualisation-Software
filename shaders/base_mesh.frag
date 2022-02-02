#version 150
#extension GL_ARB_explicit_attrib_location : enable

in vec4 position;
in vec4 normal;
in vec2 texture;

layout(location = 0) out vec4 color;
layout(location = 1) out vec4 worldPosition;

void main() {
	vec3 base_normal = abs(normal.xyz / 1.5f);
	color = vec4(base_normal, 1.f);
}
