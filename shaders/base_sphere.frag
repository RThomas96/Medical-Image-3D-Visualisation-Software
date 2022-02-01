#version 150
#extension GL_ARB_explicit_attrib_location : enable

in vec4 out_position;
in vec4 out_normal;

out vec4 color;

void main() {
	vec3 base_normal = abs(out_normal.xyz / 1.5f);
	color = vec4(base_normal, 1.f);
}
