#version 150 core
#extension GL_ARB_separate_shader_objects : enable

// Signals we're in the main shader, for any shaders inserted into this one.
#define MAIN_SHADER_UNIT

/****************************************/
/**************** Inputs ****************/
/****************************************/
in vec4 vPos;
in vec4 vNorm;
in vec3 texCoord;
in vec4 vPos_PS;

/****************************************/
/*************** Outputs ****************/
/****************************************/
layout(location = 0) out vec4 color;		// This fragment's color
layout(location = 1) out vec4 worldPosition;	// This fragment's world position

/****************************************/
/*************** Uniforms ***************/
/****************************************/
uniform usampler3D texData;	// texture generated by the voxel grid generation
uniform vec3 planePositions;	// Positions of all planes, along the direction they show
uniform vec3 planeDirections;	// The directions of the cutting planes, along their axis
uniform vec3 sceneBBPosition;	// The scene's bounding box position
uniform vec3 sceneBBDiagonal;	// The scene's bounding box diagonal
uniform int currentPlane;	// Plane identifier : 1 (x), 2 (y), 3 (z)
uniform bool showTex;		// Do we show the texture on the plane, or not ?
uniform bool drawOnlyData;	// Should we draw only the data ? not any other plane ?

// The structure which defines every attributes for the color channels.
struct colorChannelAttributes {
	uint isVisible;			// /*align : ui64*/ Is this color channel enabled/visible ?
	uint colorScaleIndex;	// /*align : ui64*/ The color channel to choose
	uvec2 visibleBounds;	// /*align : vec4*/ The bounds of the visible values
	uvec2 colorScaleBounds;	// /*align : vec4*/ The value bounds for the color scale
};

uniform uint mainChannelIndex;						// The index of the main channel in the voxel data
uniform sampler1D colorScales[4];					// All the color scales available (all encoded as 1D textures)
layout(std140) uniform ColorBlock {
	colorChannelAttributes attributes[4];	// Color attributes laid out in this way : [ main, R, G, B ]
} colorChannels;

uniform bool intersectPlanes = false;	// Should the planes intersect each other ? (hide other planes)

/****************************************/
/*********** Function headers ***********/
/****************************************/
// Determines the plane position along its axis
float planeIdxToPlanePosition(int id);
// Return a color corresponding to a plane's index
vec4 planeIndexToColor();
// Checks if a plane is visible (not hidden by another plane)
bool isPlaneVisible(bool intersect);
// Checks if the plane should be drawn as a simple border, or not.
bool shouldDrawBorder();
// Check the voxel should be displayed
bool checkAndColorizeVoxel(in uvec3 color, out vec4 return_color);
// New function to colorize voxels :
vec4 fragmentEvaluationSingleChannel(in uvec3 color);

#pragma include_color_shader;

#line 2067

/****************************************/
/***************** Main *****************/
/****************************************/
void main(void)
{
	worldPosition = vec4(.0,.0,.0,.0);
	// Early discard if the plane shouldn't be shown :
	if (isPlaneVisible(intersectPlanes) == false) { discard; }

	// not in border :
	vec4 colorTex = vec4(.0, .0, .0, .0);
	if (texCoord.x > 0. && texCoord.x < 1.) {
		if (texCoord.y > .0 && texCoord.y < 1.) {
			if (texCoord.z > 0. && texCoord.z < 1.) {
				if (isPlaneVisible(true) && showTex == true) {
					uvec3 tex = texture(texData, texCoord).xyz;
					worldPosition.xyz = sceneBBPosition + texCoord * sceneBBDiagonal;
					worldPosition.w = 1.f;
					colorTex = fragmentEvaluationSingleChannel(tex);
					if (colorTex.w < 0.005f) {
						float white_shade = 245.f/255.f;
						colorTex = vec4(white_shade, white_shade, white_shade, 1.);
					}
				}
			}
		}
	}
	color = colorTex;
	if (shouldDrawBorder() == true && drawOnlyData == true) {
		color = planeIndexToColor();
	}


	if (color.a < .005f) { worldPosition.w = .0f; discard; }
}

/****************************************/
/************** Functions ***************/
/****************************************/
bool shouldDrawBorder() {
	float min = .01;
	float max = .99;
	if (currentPlane == 1) {
		if (vPos_PS.y > max || vPos_PS.z > max) { return true; }
		if (vPos_PS.y < min || vPos_PS.z < min) { return true; }
	}
	if (currentPlane == 2) {
		if (vPos_PS.x > max || vPos_PS.z > max) { return true; }
		if (vPos_PS.x < min || vPos_PS.z < min) { return true; }
	}
	if (currentPlane == 3) {
		if (vPos_PS.x > max || vPos_PS.y > max) { return true; }
		if (vPos_PS.x < min || vPos_PS.y < min) { return true; }
	}
	return false;
}

float planeIdxToPlanePosition(int id) {
	// displacement to apply :
	vec3 diff = planePositions;
	if (id == 1) { return diff.x; }
	if (id == 2) { return diff.y; }
	if (id == 3) { return diff.z; }
	return 0.f;
}

vec4 planeIndexToColor() {
	if (currentPlane == 1) { return vec4(1., .0, .0, .9); }
	if (currentPlane == 2) { return vec4(.0, 1., .0, .9); }
	if (currentPlane == 3) { return vec4(.0, .0, 1., .9); }
	return vec4(.27, .27, .27, 1.);
}

bool isPlaneVisible(bool intersect) {
	float epsilon = .001f;
	if (((vPos.x - planePositions.x) * planeDirections.x + epsilon) < .0f) { if (intersect) { return false; }}
	if (((vPos.y - planePositions.y) * planeDirections.y + epsilon) < .0f) { if (intersect) { return false; }}
	if (((vPos.z - planePositions.z) * planeDirections.z + epsilon) < .0f) { if (intersect) { return false; }}
	return true;
}

bool checkAndColorizeVoxel(in uvec3 voxel, out vec4 return_color) {
	return true;
}