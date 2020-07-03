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

void main(void) {
	// Inverse of transpose of model matrix for normals, computed once :
	mat4 iMatrix = inverse(mMatrix);
	mat4 minverse = transpose(iMatrix);
	// ModelViewProjection matrix :
	mat4 mvp = pMatrix * vMatrix;

	// Camera space position of the light source :
	vec4 lightPos_CS = vMatrix * lightPos;
	vNorm_WS_VS = minverse * (normalize(vertexNormal));
	vNorm_CS_VS = vMatrix * minverse * normalize(vertexNormal);

	// Contains the position of the vertex after transform :
	vec4 vPos = vec4(.0,.0,.0,.0);

	vec3 s = voxelGridSize * voxelSize; // s for scale

	// Always displays the whole texture (might change in the future) :
	texCoord_VS = vertexPosition.xyz;
	// The transformation matrix to resize the cube to the grid's size :
	vec4 tx = vec4(s.x, .0, .0, .0);
	vec4 ty = vec4(.0, s.y, .0, .0);
	vec4 tz = vec4(.0, .0, s.z, .0);
	vec4 tw = vec4(.0, .0, .0, 1.);
	mat4 transform = mat4(tx, ty, tz, tw);
	vPos = (transform * vertexPosition) + vec4(voxelGridOrigin, .0); // TODO : check if we can't just embed it in the transform matrix above

	gl_Position = mvp * vPos;
	vPos_WS_VS = mMatrix * vPos;
	vPos_CS_VS = vMatrix * mMatrix * vPos;

	eyeDir_CS_VS = (vPos_CS_VS - vec4(.0,.0,.0,.0));
	lightDir_CS_VS = lightPos_CS + eyeDir_CS_VS;
}
