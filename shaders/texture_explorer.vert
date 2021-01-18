#version 400 core

#extension GL_ARB_separate_shader_objects : enable

/****************************************/
/**************** Inputs ****************/
/****************************************/
layout(location = 0) in vec4 vertexPosition;	// Vertex position
layout(location = 1) in vec4 vertexNormal;	// In this shader : does nothing
layout(location = 2) in vec3 vertexTexCoord;	// Texture coordinate used for the position in the scene BB

/****************************************/
/*************** Outputs ****************/
/****************************************/
layout(location = 0) out vec4 vPos;		// The vertex's positions
layout(location = 1) out vec4 vNorm;
layout(location = 2) out vec3 vTexCoord;	// The vertex's texture coordinates

/****************************************/
/*************** Uniforms ***************/
/****************************************/
/* For the FB/BB dimensions, the X coordinate will be width, and Y will be height. */
uniform vec2 fbDims;		// Framebuffer dimensions
uniform vec2 bbDims;		// Scene's bounding box dimensions on this plane
uniform uint planeIndex;	// The identifier of the currently drawn plane.
uniform mat4 gridTransform;	// The grid's world-space → grid-space transform
uniform vec4 gridDimensions;	// The grid dimensions (used to compute tex-space coordinates)
uniform vec4 sceneBBDiagonal;	// The grid's world-space bounding box diagonal
uniform vec4 sceneBBPosition;	// The grid's world-space bounding box position
uniform vec3 planePositions;	// The plane positions, in world space
uniform uint heading;		// The plane's 'up' orientation (up [default] = 0, right = 1, down = 2, left = 3)
uniform float zoom;		// The zoom factor for the current view

/****************************************/
/*********** Function headers ***********/
/****************************************/
// Takes the vertex positions of the planes, and returns them to the 'right' plane for viewing.
vec4 planeCoordsToGLPosition(in vec4 position);
// Computes the displacement of a plane to be at 'depth' position in the grid.
vec4 planeDisplacementCompute(in uint idx);
// Computes the proper multiplier to apply to a plane in order to fit it within the device coordinates
vec2 computeMultiplier(in uint head);

/****************************************/
/***************** Main *****************/
/****************************************/
void main(void) {
	/*
	This shader will do the following :
		- determine which plane is currently being drawn,
		- compute a multiplier for the unit plane which will
		  allow it to cover the whole scene in world-space,
		- apply that multiplier to the currently drawn plane,
		- compute the vertex positions in world space, then
		  grid space, then texture space,
		- output a set of vertex coordinates & attributes to
		  draw it in the fragment shader.
	*/
	vec2 bb = bbDims.xy;
	vec2 fb = fbDims.xy;
	if (heading == 1 || heading == 3) { bb = bb.yx; }

	// Reminder : FB/BB dimensions are expressed as { width, height }
	float ratio_fb = fb.x / fb.y;
	float ratio_bb = bb.x / bb.y;
	// Multiplier to apply to the gl_Position to make the
	// bounding box fit within the frambuffer :
	vec3 multiplier = vec3(.0, .0, 1.);

	if (ratio_bb > ratio_fb) {
		multiplier.x =1.f;
		multiplier.y = ratio_fb / ratio_bb;
	} else {
		// If the framebuffer is wider relative to the bounding box :
		multiplier.y = 1.f; // The bb height will be displayed whole
		float ratio_bb_inv = bb.y / bb.x;
		float ratio_fb_inv = fb.y / fb.x;
		multiplier.x = ratio_fb_inv / ratio_bb_inv;
	}

	vec4 vPos_WS = sceneBBPosition + (vertexPosition * sceneBBDiagonal) + planeDisplacementCompute(planeIndex);
	vec4 vPos_GS = (gridTransform) * vPos_WS;
	vec4 vPos_TS = vPos_GS / gridDimensions;

	gl_Position = vec4(.0, .0, .0, 1.);
	gl_Position.xyz = multiplier * vertexTexCoord;
	gl_Position.xyz *= zoom;

	vPos = vertexPosition;
	vNorm = normalize(vertexNormal);
	vTexCoord = vPos_TS.xyz;
}

/****************************************/
/************** Functions ***************/
/****************************************/
vec4 planeCoordsToGLPosition(in vec4 position) {
	if (planeIndex == 1) { return position.yzxw; }
	if (planeIndex == 2) { return position.xzyw; }
	return position.xyzw;
}

vec4 planeDisplacementCompute(in uint idx) {
	vec4 displ = vec4(.0f, .0f, .0f, .0f);
	vec3 diff=  planePositions - sceneBBPosition.xyz;
	if (idx == 1) { displ.x = diff.x; }
	if (idx == 2) { displ.y = diff.y; }
	if (idx == 3) { displ.z = diff.z; }
	return displ;
}

vec2 computeMultiplier(in uint head) {
	// Ratios of the framebuffer and the bounding box :
	float ratio_fb = fbDims.x / fbDims.y;
	float ratio_bb = bbDims.x / bbDims.y;
	//if (head == 1 || head == 3) { ratio_bb = 1/ratio_bb; }
	vec2 multiplier = vec2(1., 1.);

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

	return multiplier;
}

void oldMainFunction() {
	/*
	This shader will do the following :
		- determine which plane is currently being drawn,
		- compute a multiplier for the unit plane which will
		  allow it to cover the whole grid in world-space,
		- apply that multiplier to the currently drawn plane,
		- compute the vertex positions in world space, then
		  grid space, then texture space,
		- output a set of vertex coordinates & attributes to
		  draw it in the fragment shader.
	*/
	/*
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
	planeMultiplier.xy = multiplier;

	vec4 vPosWS = gridBBPosition + (vertexPosition * gridBBDiagonal) + planeDisplacementCompute(planeIndex);
	vec4 vPosGS = gridTransform * vPosWS;
	vec4 vPosTS = vPosGS / gridDimensions;

	vPos = planeCoordsToGLPosition(vertexPosition) * planeMultiplier * 2.f - vec4(multiplier.x, multiplier.y, 1.f, .0f);
	vPos.w = 1.;
	vTexCoord = vPosTS.xyz;
	*/
}
