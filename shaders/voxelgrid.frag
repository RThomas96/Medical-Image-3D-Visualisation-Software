#version 150 core
#extension GL_ARB_explicit_attrib_location : require

#pragma optimize(off)
#pragma debug(on)

// Signals we're in the main shader, for any shaders inserted into this one.
#define MAIN_SHADER_UNIT

in vec4 vPos_WS;
in vec4 vNorm_WS;
in vec3 texCoord;

in vec4 vPos_CS;
in vec4 vNorm_CS;
in vec4 lightDir_CS;
in vec4 eyeDir_CS;

layout(location = 0) out vec4 color;
layout(location = 1) out vec4 worldPosition;

// Draw modes :
//    - 0u : texture/cube/polygons only
//    - 1u : 0u, but with wireframe on top
//    - 2u : wireframe only, transparent
uniform uint drawMode;

uniform vec4 lightPos;
uniform usampler3D texData; // texture generated by the voxel grid generation
uniform sampler1D colorScale; // Color scale generated by the program

uniform vec3 sceneBBPosition;
uniform vec3 sceneBBDiagonal;
uniform vec3 planePositions;
uniform vec3 planeDirections;

// Grid-related information :
uniform vec3 voxelGridOrigin;
uniform vec3 voxelGridSize;
uniform vec3 voxelSize;

uniform vec2 colorBounds;		// Min/max values to compute the color scale
uniform vec2 textureBounds;		// Min/max values of the texture to show.
uniform vec2 colorBoundsAlternate;	// Min/max values to compute the color scale
uniform vec2 textureBoundsAlternate;	// Min/max values of the texture to show.

uniform vec3 color0;	// Start of the color segment
uniform vec3 color1;	// End of the color segment
uniform vec3 color0Alternate;	// Start of the color segment
uniform vec3 color1Alternate;	// End of the color segment

uniform uint rgbMode;	// Show only R, only G, or RG

uniform uint r_channelView;	// The coloration function chosen
uniform uint r_selectedChannel;	// The currently selected channel
uniform uint r_nbChannels;	// nb of channels in the image in total (R, RG, RGB ?)
uniform uint g_channelView;	// The coloration function chosen
uniform uint g_selectedChannel;	// The currently selected channel
uniform uint g_nbChannels;	// nb of channels in the image in total (R, RG, RGB ?)

// Get a plane's coordinate in its axis.
float planeIdxToPlanePosition(int id);
// Checks a fragment is visible, according to the plane positions and directions.
bool isFragmentVisible();

bool checkAndColorizeVoxel(in uvec3 voxel, out vec4 return_color);

#pragma include_color_shader;

#line 2072

void main(void)
{
	// Compute the cutting plane position so that we can threshold the fragments :
	if (isFragmentVisible() == false) { worldPosition.w = .0f; discard; }

	float epsilon = .03;
	/*
	float distMin = min(barycentricCoords.x/largestDelta.x, min(barycentricCoords.y/largestDelta.y, barycentricCoords.z/largestDelta.z));
	*/

	vec4 basecolor;
	uvec3 ui = texture(texData, texCoord).xyz;
	worldPosition.w = 1.f;

	// computed color :
	vec4 compColor = vec4(.1,.1, .1, 1.);
	if (checkAndColorizeVoxel(ui, compColor) == false) {
		compColor = vec4(.8, .8, .8, 1.);
		worldPosition.w = .0f;
	}

	color = compColor;
	worldPosition.xyz = (voxelGridOrigin + texCoord * voxelGridSize * voxelSize);
}


float planeIdxToPlanePosition(int id) {
	// displacement to apply :
	vec3 diff = planePositions - sceneBBPosition;
	if (id == 1) { return diff.x; }
	if (id == 2) { return diff.y; }
	if (id == 3) { return diff.z; }
	return 0.f;
}

bool isFragmentVisible() {
	float epsilon = .01f;
	if (((vPos_WS.x - planePositions.x) * planeDirections.x + epsilon) < .0f) { return false; }
	if (((vPos_WS.y - planePositions.y) * planeDirections.y + epsilon) < .0f) { return false; }
	if (((vPos_WS.z - planePositions.z) * planeDirections.z + epsilon) < .0f) { return false; }
	return true;
}

bool checkAndColorizeVoxel(in uvec3 voxel, out vec4 return_color) {
	float voxVal;
	vec2 cB, tB;
	bool isRed = false;
	bool canSwitch = false;

	vec2 vis = vec2(.0f, .0f);

	voxVal = float(voxel.r);
	if (voxVal >= textureBounds.x && voxVal < textureBounds.y) {vis.r = 1.f;}
	voxVal = float(voxel.g);
	if (voxVal >= textureBoundsAlternate.x && voxVal < textureBoundsAlternate.y) {vis.g = 1.f;}

	return_color = voxelIdxToColor(voxel, vis);
	if (return_color.a < .0f) { return false; }
	return true;
}
