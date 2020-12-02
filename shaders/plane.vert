#version 400 core

// VAO inputs :
layout(location=0) in vec4 vertexPosition;
layout(location=1) in vec4 vertexNormal;

// VShader outputs world space (suffixed by _WS_VS) :
out vec4 vPos_WS_VS;
out vec4 vNorm_WS_VS;
out vec3 texCoord_VS;
// VShader outputs camera space (suffixed by _CS_VS) :
out vec4 vPos_CS_VS;
out vec4 vNorm_CS_VS;
out vec4 lightDir_CS_VS;
out vec4 eyeDir_CS_VS;

// Model, view, and projection matrices :
uniform mat4 mMatrix;
uniform mat4 vMatrix;
uniform mat4 pMatrix;

// The transform used by the grid to change space from grid to world
uniform mat4 gridTransform;
// The origin of the grid's bounding box
uniform vec3 gridPosition;
// The size of the grid  we have to intersect :
uniform uvec3 gridSize;

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
	// For gl_Position :
	mat4 mvp = pMatrix * vMatrix * mMatrix;

	/*
	Vertex position will always be normalized (i.e., in [0, 1]). We need to apply the correct size multiplier and
	the correct displacement in order to get the 'real' position of a vertex within that plane.
	*/

	vec4 vPos_planeSpace = vertexPosition;
	vec4 vPos_worldSpace = vPos_planeSpace * planeIdxToPlaneSize(currentPlane) + planeIdxToPlanePosition(currentPlane);
	vec4 vPos_gridSpace = vPos_planeSpace * gridTransform;
	vec4 vPos_texSpace = vPos_gridSpace;
	vPos_texSpace.x = vPos_texSpace.x / float(gridSize.x);
	vPos_texSpace.y = vPos_texSpace.y / float(gridSize.y);
	vPos_texSpace.z = vPos_texSpace.z / float(gridSize.z);

	vNorm_WS_VS = vertexNormal;
	texCoord_VS = vPos_texSpace.xyz;

	gl_Position = vPos_worldSpace;
}
