#version 400 core

/****************************************/
/**************** Inputs ****************/
/****************************************/
layout(location=0) in vec4 vertexPosition;	// Vertex position, normalized.
layout(location=1) in vec4 vertexNormal;	// Vertex normal, normalized.
layout(location=2) in vec3 vertexTexCoord;	// Vertex texture coordinates. In this shader : does nothing

/****************************************/
/*************** Outputs ****************/
/****************************************/
out vec4 vPos;
out vec4 vNorm;
out vec3 texCoord;

/****************************************/
/*************** Uniforms ***************/
/****************************************/
uniform mat4 model_Mat;		// Model matrix
uniform mat4 view_Mat;		// View matrix
uniform mat4 projection_Mat;	// Projection matrix
uniform mat4 gridTransform;	// The transform used by the grid to change space from grid to world
uniform vec3 sceneBBPosition;	// The scene's bounding box position
uniform vec3 sceneBBDiagonal;	// The scene's bounding box diagonal
uniform vec3 gridSize;		// The size of the grid's bounding box in world space
uniform vec3 gridDimensions;	// The dimensions of the grid
uniform vec3 planePosition;	// World-space positions of all planes, along the axis they cut
uniform int currentPlane;	// Plane identifier : 1 (x), 2 (y), 3 (z)

/****************************************/
/*********** Function headers ***********/
/****************************************/
// Get a displacement to apply to the plane's vertices for a given plane identifier
vec4 planeIdxToPlanePosition(int id);
// Get a size multiplier to apply to the plane's vertices for a given plane identifier
vec4 planeIdxToPlaneSize(int id);

/****************************************/
/***************** Main *****************/
/****************************************/
void main(void) {
	mat4 norMat = inverse(transpose(model_Mat));
	vec4 gridSize4 = vec4(gridSize, 1.);
	vec4 planePosition4 = vec4(planePosition, .0);
	vec4 gridDimensions4 = vec4(gridDimensions, .0);
	vec4 sceneBBPosition4 = vec4(sceneBBPosition, .0);

	/*
	Vertex position will always be normalized (i.e., in [0, 1]). We need to apply the correct size multiplier and
	the correct displacement in order to get the 'real' position of a vertex within that plane.
	*/
	vec4 vPos_ws = sceneBBPosition4 + (vertexPosition * gridSize4) + planeIdxToPlanePosition(currentPlane);
	vec4 vPos_gs = inverse(gridTransform) * vPos_ws;
	vec4 vPos_ts = vPos_gs / gridDimensions4;

	vPos = vPos_ws;
	vNorm = norMat * vertexNormal;
	texCoord = vPos_ts.xyz;

	gl_Position = projection_Mat * view_Mat * model_Mat * vPos;
}

/****************************************/
/************** Functions ***************/
/****************************************/

vec4 planeIdxToPlanePosition(int id) {
	// displacement to apply :
	vec3 diff = planePosition - sceneBBPosition;
	vec4 displ = vec4(.0, .0, .0, .0);
	if (id == 1) { displ.x = diff.x; }
	if (id == 2) { displ.y = diff.y; }
	if (id == 3) { displ.z = diff.z; }
	return displ;
}

vec4 planeIdxToPlaneSize(int id) {
	// size :
	vec4 s = vec4(.0, .0, .0, .0);
	if (id == 1) { s.y = float(gridSize.y); s.z = float(gridSize.z); }
	if (id == 2) { s.x = float(gridSize.x); s.z = float(gridSize.z); }
	if (id == 3) { s.x = float(gridSize.x); s.y = float(gridSize.y); }
	return s;
}