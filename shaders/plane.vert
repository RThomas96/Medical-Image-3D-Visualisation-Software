#version 400 core

// VAO inputs :
layout(location=0) in vec4 vertexPosition;
layout(location=1) in vec4 vertexNormal;
layout(location=2) in vec3 vertexTexCoord; // In this shader : does nothing

// VShader outputs world space (suffixed by _WS_VS) :
out vec4 vPos;
out vec4 vNorm;
out vec3 texCoord;

// Model, view, and projection matrices :
uniform mat4 model_Mat;
uniform mat4 view_Mat;
uniform mat4 projection_Mat;

// The transform used by the grid to change space from grid to world
uniform mat4 gridTransform;
// The origin of the grid's bounding box
uniform vec3 gridPosition;
// The size of the grid's bounding box in world space :
uniform vec3 gridSize;
// The dimensions of the grid :
uniform vec3 gridDimensions;

// Plane number : 1 (x), 2 (y), 3 (z)
uniform int currentPlane;

// Positions of all planes, along the direction they show :
uniform vec3 planePosition;

// Get a displacement to apply to the plane's vertices for a given plane index (1, 2, 3 -> x, y, z)
vec4 planeIdxToPlanePosition(int idx) {
	// displacement to apply :
	vec4 displ = vec4(.0, .0, .0, .0);
	if (idx == 1) { displ.x = planePosition.x; }
	if (idx == 2) { displ.y = planePosition.y; }
	if (idx == 3) { displ.z = planePosition.z; }
	return displ;
}

// Get a size multiplier to apply to the plane's vertices for a given plane index (1, 2, 3 -> x, y, z)
vec4 planeIdxToPlaneSize(int idx) {
	// size :
	vec4 s = vec4(.0, .0, .0, .0);
	if (idx == 1) { s.y = float(gridSize.y); s.z = float(gridSize.z); }
	if (idx == 2) { s.x = float(gridSize.x); s.z = float(gridSize.z); }
	if (idx == 3) { s.x = float(gridSize.x); s.y = float(gridSize.y); }
	return s;
}

void main(void) {
	mat4 norMat = inverse(transpose(model_Mat));
	vec4 gridSize4 = vec4(gridSize.x, gridSize.y, gridSize.z, 1.f);
	vec4 gridPosition4 = vec4(gridPosition.x, gridPosition.y, gridPosition.z, .0);
	vec4 gridDimensions4 = vec4(gridDimensions.x, gridDimensions.y, gridDimensions.z, 1.f);
	/*
	Vertex position will always be normalized (i.e., in [0, 1]). We need to apply the correct size multiplier and
	the correct displacement in order to get the 'real' position of a vertex within that plane.
	*/
	vec4 vPos_ws = (vertexPosition * gridSize4) + gridPosition4 + planeIdxToPlanePosition(currentPlane);
	vec4 tempP = (vertexPosition * gridSize4) + planeIdxToPlanePosition(currentPlane);
	vec4 vPos_gs = inverse(gridTransform) * vPos_ws;
	vec4 vPos_ts = vPos_gs / gridDimensions4;

	vPos = vPos_ws;
	vNorm = norMat * vertexNormal;
	texCoord = vPos_ts.xyz;

	gl_Position = projection_Mat * view_Mat * model_Mat * vPos;
}
