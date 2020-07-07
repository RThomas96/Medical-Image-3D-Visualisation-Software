#version 400 core

// VAO inputs :
layout(location=0) in vec4 vertexPosition;
layout(location=1) in vec4 vertexNormal;
layout(location=2) in uvec4 voxelIndex; // in this shader : does nothing

// VShader outputs world space (suffixed by _WS_VS) :
out vec4 vPos_WS_VS;
out vec4 vNorm_WS_VS;
out vec3 texCoord_VS;
// VShader outputs camera space (suffixed by _CS_VS) :
out vec4 vPos_CS_VS;
out vec4 vNorm_CS_VS;
out vec4 lightDir_CS_VS;
out vec4 eyeDir_CS_VS;

// Scene parameters :
uniform mat4 mMatrix;
uniform mat4 vMatrix;
uniform mat4 pMatrix;
uniform vec4 lightPos; // will always be worldspace here !

// Voxel grid data :
uniform vec3 voxelGridOrigin;
uniform vec3 voxelGridSize;
uniform vec3 voxelSize;

// Cutting planes for the grid, will
// always have components in [0, 1]:
uniform vec3 cutPlaneMin;
uniform vec3 cutPlaneMax;

void main(void) {
	// Inverse of transpose of model matrix for normals, computed once :
	mat4 iMatrix = inverse(mMatrix);
	mat4 minverse = transpose(iMatrix);
	// ModelViewProjection matrix :
	mat4 mvp = pMatrix * vMatrix * mMatrix;

	// Camera space position of the light source :
	vec4 lightPos_CS = vMatrix * lightPos;
	vNorm_WS_VS = minverse * (normalize(vertexNormal));
	vNorm_CS_VS = vMatrix * minverse * normalize(vertexNormal);

	// Contains the position of the vertex after transform :
	vec4 vPos = vec4(.0,.0,.0,.0);

	// Scaling matrix, unit cube at first and voxel
	// grid sized parallelepiped afterwards :
	vec3 s = voxelGridSize * voxelSize;

	// Offset the grid by (origin + cutPlaneMin*gridSize).
	// Here, cutPlaneMin's coordinates serves as scalars
	// for the original displacement of the grid :
	vec3 origin = voxelGridOrigin + vec3(
		cutPlaneMin.x * s.x,
		cutPlaneMin.y * s.y,
		cutPlaneMin.z * s.z
	);

	// The voxel grid should only be scaled from :
	//     cutPlaneMin.x * s.x to cutPlaneMax.x * s.x,
	//     cutPlaneMin.y * s.y to cutPlaneMax.y * s.y,
	//     cutPlaneMin.z * s.z to cutPlaneMax.z * s.z
	s.x = (1.-(cutPlaneMin.x + (1. - cutPlaneMax.x))) * s.x;
	s.y = (1.-(cutPlaneMin.y + (1. - cutPlaneMax.y))) * s.y;
	s.z = (1.-(cutPlaneMin.z + (1. - cutPlaneMax.z))) * s.z;

	// Display the texture that should be displayed only, with cutting planes :
	texCoord_VS = vec3(
		(vertexPosition.x < 0.5) ? cutPlaneMin.x : cutPlaneMax.x,
		(vertexPosition.y < 0.5) ? cutPlaneMin.y : cutPlaneMax.y,
		(vertexPosition.z < 0.5) ? cutPlaneMin.z : cutPlaneMax.z
	);
	// The transformation matrix to resize the cube to the grid's size :
	vec4 tx = vec4(s.x, .0, .0, .0);
	vec4 ty = vec4(.0, s.y, .0, .0);
	vec4 tz = vec4(.0, .0, s.z, .0);
	vec4 tw = vec4(origin.x, origin.y, origin.z, 1.);
	mat4 transform = mat4(tx, ty, tz, tw);
	vPos = (transform * vertexPosition);

	gl_Position = mvp * vPos;
	vPos_WS_VS = mMatrix * vPos;
	vPos_CS_VS = vMatrix * mMatrix * vPos;

	eyeDir_CS_VS = (vPos_CS_VS - vec4(.0,.0,.0,.0));
	lightDir_CS_VS = lightPos_CS + eyeDir_CS_VS;
}
