#version 400 core

in vec4 vPos_GS;
in vec4 vNorm_GS;
in vec3 texCoord_GS;

out vec4 color;

uniform usampler3D texData; // texture generated by the voxel grid generation
uniform sampler2D colorScale; // Color scale generated by the program

/// Takes a uvec3 of an R8UI-based texture and spits out an RGB color by looking up the color scale
vec4 R8UItoColorScale(in uvec3 ucolor);

void main(void)
{
	// vec4 basecolor=vec4(.5, .5, .5, .5);
	uvec3 tex = texture(texData, texCoord_GS).xyz;
	vec4 colorTex = R8UItoColorScale(tex);

	color = colorTex + vec4(.5, .5, .5, 1.);
}

vec4 R8UItoColorScale(in uvec3 ucolor) {
	// Only the red component contains the data needed for the color scale :
	uint column = ucolor.r % 16;
	uint row = (ucolor.r - column) / 16;
	float index = float(ucolor.r);
	vec4 color = texture(colorScale, uvec2(row, column));
	color.a = 1.f;
	return color;
}
