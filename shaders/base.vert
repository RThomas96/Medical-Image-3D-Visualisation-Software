#version 400 core

// TODO : marked in EOL

layout(location=0) in vec4 vertexPosition;
layout(location=1) in vec4 vertexNormal;
layout(location=2) in vec3 vertexTexCoord; // TODO : remove this
layout(location=3) in uvec4 voxelIndex; // TODO : make this indexed for glDrawElementsIndexed (see glVertexAttribDivisor for that)
					// TODO : if last one is 0, all other are scale factors, else positions

// _WS is world space, _CS is camera space

out vec4 vPos_WS;
out vec4 vNorm_WS;
out vec3 texCoord; // TODO : be computed on the fly with new uniform

out vec4 vPos_CS;
out vec4 vNorm_CS;
out vec4 lightDir_CS;
out vec4 eyeDir_CS;

uniform mat4 mMatrix;
uniform mat4 vMatrix;
uniform mat4 pMatrix;
uniform vec4 lightPos; // will always be worldspace here !

uniform uvec3 imageSize; // TODO : make this linked to the program !
uniform vec3 neighborOffset; // TODO : make this uvec3, to be computed for real position
// TODO : add new float uniform (todo : make sure of float) for heightmap difference between image slices

void main(void) {
	// Inverse of transpose of model matrix for normals, computed once :
	mat4 minverse = transpose(inverse(mMatrix));

	// TODO : compute float position of vertex based on the voxelIndex given in argument
	// NOTE	: the displacement for a pixel is given by vec3(1,1,1) * mMatrix + displacementSlice (TODO : fix this, might be wrong)

	// How much do we need to offset the inspecting cube :
	vec4 posOffset = vec4(neighborOffset.x * float(imageSize.x), neighborOffset.y * float(imageSize.y), neighborOffset.z * float(imageSize.z), .0);
	// The position of the vertex of the offset cube :
	vec4 vPos = vertexPosition + posOffset;
	// The texture coordinate we need to fetch
	vec4 vTex = vertexTexCoord + neighborOffset;

	mat4 mvp = pMatrix * vMatrix * mMatrix;
	gl_Position = mvp * vPos;
	vPos_WS = mMatrix * vPos;
	vNorm_WS = minverse * (normalize(vertexNormal));
	texCoord = vTex;

	vPos_CS = vMatrix * vertexPosition;
	vNorm_CS = vMatrix * minverse * normalize(vertexNormal);

	eyeDir_CS = (vPos_CS - vec4(.0,.0,.0,.0));
	vec4 lightPos_CS = vMatrix * lightPos;
	lightDir_CS = lightPos_CS + eyeDir_CS;
}
