#version 150
#extension GL_ARB_explicit_attrib_location : enable

in vec4 position;
in vec4 normal;
in vec2 texture;

layout(location = 0) out vec4 color;
layout(location = 1) out vec4 worldPosition;

void main() {
	gl_FragDepth = .001f;
	vec3 base_normal = abs(normal.xyz / 1.5f);
	color = vec4(1.f, .0f, .0f, 1.f);
	worldPosition = position;
}
