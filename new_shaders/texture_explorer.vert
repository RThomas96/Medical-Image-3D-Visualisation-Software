#version 150 core

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_explicit_attrib_location : enable

/****************************************/
/**************** Inputs ****************/
/****************************************/
layout(location = 0) in vec4 vertexPosition;	// Vertex position
layout(location = 1) in vec4 vertexNormal;		// In this shader : does nothing
layout(location = 2) in vec3 vertexTexCoord;	// Actually position within the final FB : [-1;1] on axes

/****************************************/
/*************** Outputs ****************/
/****************************************/
layout(location = 0) out vec4 vPos;				// The vertex's positions
layout(location = 1) out vec3 vOriginalCoords;	// Original coords in plane-space
layout(location = 2) out vec3 vTexCoord;		// The vertex's texture coordinates
layout(location = 3) out vec2 planeMultiplier;	// The multiplier used to 'stretch' the plane

/****************************************/
/*************** Uniforms ***************/
/****************************************/
/* For the FB/BB dimensions, the X coordinate will be width, and Y will be height. */
uniform vec2 fbDims;			// Framebuffer dimensions
uniform vec2 bbDims;			// Scene's bounding box dimensions on this plane
uniform uint planeIndex;		// The identifier of the currently drawn plane.
uniform mat4 gridTransform;		// The grid's world-space → grid-space transform
uniform vec4 gridDimensions;	// The grid dimensions (used to compute tex-space coordinates)
uniform vec4 sceneBBDiagonal;	// The grid's world-space bounding box diagonal
uniform vec4 sceneBBPosition;	// The grid's world-space bounding box position
uniform vec3 planePositions;	// The plane positions, in world space
uniform vec2 offset;			// The offset from the origin, to move the viewer
uniform uint heading;			// The plane's 'up' orientation (up [default] = 0, right = 1, down = 2, left = 3)
uniform float zoom;				// The zoom factor for the current view

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
	if (heading == 1u || heading == 3u) { bb = bb.yx; }

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

	/*
	Note : here we dont add the sceneBBPosition because the offset of the grid is kept separate from
	the grid information itself. As such, we compute the coordinates as if they were at the origin.
	*/
	vec4 vPos_WS = (vertexPosition * sceneBBDiagonal) + planeDisplacementCompute(planeIndex);
	vec4 vPos_GS = (gridTransform) * vPos_WS;
	vec4 vPos_TS = vPos_GS / gridDimensions;

	gl_Position = vec4(.0, .0, .0, 1.);
	gl_Position.xyz = multiplier * vertexTexCoord;
	gl_Position.xyz *= zoom;
	gl_Position.xy += offset;

	vPos = vertexPosition;
	vOriginalCoords = vertexTexCoord;
	vTexCoord = vPos_TS.xyz;
	planeMultiplier = multiplier.xy;
}

/****************************************/
/************** Functions ***************/
/****************************************/
vec4 planeCoordsToGLPosition(in vec4 position) {
	if (planeIndex == 1u) { return position.yzxw; }
	if (planeIndex == 2u) { return position.xzyw; }
	return position.xyzw;
}

vec4 planeDisplacementCompute(in uint idx) {
	vec4 displ = vec4(.0f, .0f, .0f, .0f);
	vec3 diff=  planePositions - sceneBBPosition.xyz;
	if (idx == 1u) { displ.x = diff.x; }
	if (idx == 2u) { displ.y = diff.y; }
	if (idx == 3u) { displ.z = diff.z; }
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
