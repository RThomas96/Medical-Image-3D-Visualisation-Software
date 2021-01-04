#version 400 core

#extension GL_ARB_separate_shader_objects : enable

/****************************************/
/**************** Inputs ****************/
/****************************************/
layout(location = 0) in vec4 vertexPosition;	// Vertex position
layout(location = 1) in vec4 vertexNormal;	// In this shader : does nothing
layout(location = 2) in vec3 vertexTexCoord;	// In this shader : does nothing

/****************************************/
/*************** Outputs ****************/
/****************************************/
layout(location = 0) out vec4 vPos;		// The vertex's positions
layout(location = 1) out vec3 vTexCoord;	// The vertex's texture coordinates

/****************************************/
/*************** Uniforms ***************/
/****************************************/
/* For the FB/BB dimensions, the X coordinate will be width, and Y will be height. */
uniform vec2 fbDims;		// Framebuffer dimensions
uniform vec2 bbDims;		// Grid bounding box dimensions on this plane
uniform uint planeIndex;	// The identifier of the currently drawn plane.
uniform mat4 gridTransform;	// The grid's world-space → grid-space transform
uniform vec4 gridDimensions;	// The grid dimensions (used to compute tex-space coordinates)
uniform vec4 gridBBDiagonal;	// The grid's world-space bounding box diagonal
uniform vec4 gridBBPosition;	// The grid's world-space bounding box position
uniform float depth;		// The depth along the plane's direction, normalized (for texture fetching)

/****************************************/
/*********** Function headers ***********/
/****************************************/
// Takes the vertex positions of the planes, and returns them to the 'right' plane for viewing.
vec4 planeCoordsToGLPosition(in vec4 position);
// Computes the displacement of a plane to be at 'depth' position in the grid.
vec4 planeDisplacementCompute(in uint idx);
// Takes a vec3 and spits out a corresponding vec4. Will always set omega to 0.
vec4 toVec4(in vec3 inputPos, in float wComponent);

/****************************************/
/***************** Main *****************/
/****************************************/
void main(void) {
	/* This shader will do the following :
		- determine which plane is currently being drawn,
		- compute a multiplier for the unit plane which will
		  allow it to cover the whole grid in world-space,
		- apply that multiplier to the currently drawn plane,
		- compute the vertex positions in world space, then
		  grid space, then texture space,
		- output a set of vertex coordinates & attributes to
		  draw it in the fragment shader.
	*/
	vec4 gridDiag = gridBBDiagonal; gridDiag.w = 1.;
	vec4 gridPos = gridBBPosition; gridPos.w = 1.;
	vec2 multiplier = vec2(1., 1.); // used to compute values
	vec4 planeMultiplier = multiplier.xyxy; // used to apply it to the plane later on
	vec4 gridHalfSize = gridBBDiagonal / 2.f;
	vec4 gridCenter = gridBBPosition + gridHalfSize;

	// Ratios of the framebuffer and the bounding box :
	float ratio_fb = fbDims.x / fbDims.y;
	float ratio_bb = bbDims.x / bbDims.y;

	if (ratio_bb > ratio_fb) {
		// If the bounding box is wider relative to the framebuffer :
		multiplier.x = 1.f; // The BB width will take the whole FB
		multiplier.y = ratio_fb / ratio_bb;
	} else {
		// If the framebuffer is wider relative to the bounding box :
		multiplier.y = 1.f; // The bb height will be displayed whole
		float ratio_bb_inv = bbDims.y / bbDims.x;
		float ratio_fb_inv = fbDims.y / fbDims.x;
		multiplier.x = ratio_fb_inv / ratio_bb_inv;
	}
	/*if (planeIndex == 1) { planeMultiplier.yz = multiplier; } // Applied to Y and Z axes
	if (planeIndex == 2) { planeMultiplier.xz = multiplier; } // Applied to X and Z axes
	if (planeIndex == 3) { planeMultiplier.xy = multiplier; } // Applied to X and Y axes
	*/
	planeMultiplier.xy = multiplier;

	/* TESTING : computing the coordinates same way as plane.vert, but resize plane to fit the FB ratio
	and shift the BB to recenter the view (shift/recenter done in second iteration) */

	vec4 vPosWS = gridBBPosition + (vertexPosition * gridBBDiagonal) + planeDisplacementCompute(planeIndex);
	vec4 vPosGS = gridTransform * vPosWS;
	vec4 vPosTS = vPosGS / gridDimensions;

	vPos = planeCoordsToGLPosition(vertexPosition) * planeMultiplier * 2.f - vec4(multiplier.x, multiplier.y, 1.f, .0f);
	vPos.w = 1.;
	vTexCoord = vPosTS.xyz;

	gl_Position = vPos;
}

/****************************************/
/************** Functions ***************/
/****************************************/
vec4 toVec4(in vec3 inputPos, in float wComponent) {
	vec4 outputPos;
	outputPos.xyz = inputPos.xyz;
	outputPos.w = wComponent;
	return outputPos;
}

vec4 planeCoordsToGLPosition(in vec4 position) {
	if (planeIndex == 1) { return position.yzxw; }
	if (planeIndex == 2) { return position.xzyw; }
	return position.xyzw;
}

vec4 planeDisplacementCompute(in uint idx) {
	vec4 displ = vec4(.0f, .0f, .0f, .0f);
	if (idx == 1) { displ.x = depth * gridBBDiagonal.x; }
	if (idx == 2) { displ.y = depth * gridBBDiagonal.y; }
	if (idx == 3) { displ.z = depth * gridBBDiagonal.z; }
	return displ;
}
