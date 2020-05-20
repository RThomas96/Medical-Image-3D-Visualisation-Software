#version 400 core

layout(location=0) in vec4 vertexPosition;
layout(location=1) in vec4 vertexNormal;
layout(location=2) in uvec4 voxelIndex;

out vec4 vPos_WS;
out vec4 vNorm_WS;
out vec3 texCoord;

out vec4 vPos_CS;
out vec4 vNorm_CS;
out vec4 lightDir_CS;
out vec4 eyeDir_CS;

uniform mat4 mMatrix;
uniform mat4 vMatrix;
uniform mat4 pMatrix;
uniform vec4 lightPos; // will always be worldspace here !

uniform uvec3 imageSize;
uniform ivec3 neighborOffset;

void main(void) {
	// ModelViewProjection matrix :
	mat4 mvp = pMatrix * vMatrix * mMatrix;
	// Inverse of transpose of model matrix for normals, computed once :
	mat4 minverse = transpose(inverse(mMatrix));

	vNorm_WS = minverse * (normalize(vertexNormal));

	vPos_CS = vMatrix * mMatrix * vertexPosition;
	vNorm_CS = vMatrix * minverse * normalize(vertexNormal);

	eyeDir_CS = (vPos_CS - vec4(.0,.0,.0,.0));
	// Camera space position of the light source :
	vec4 lightPos_CS = vMatrix * lightPos;
	lightDir_CS = lightPos_CS + eyeDir_CS;

	// Float versions of program data :
	float fimgx = float(imageSize.x);
	float fimgy = float(imageSize.y);
	float fimgz = float(imageSize.z);

	float fidxx = float(voxelIndex.x);
	float fidxy = float(voxelIndex.y);
	float fidxz = float(voxelIndex.z);

	// Contains the position of the vertex after transform :
	vec4 vPos = vec4(.0,.0,.0,.0);

	if (gl_InstanceID == 0) {
		// For the texture cube, at instance 0, we display the whole texture :
		texCoord = vertexPosition.xyz;
		// The transformation matrix for it to show the true texture size :
		vec4 tx = vec4(fimgx*fidxx, .0, .0, .0);
		vec4 ty = vec4(.0, fimgy*fidxy, .0, .0);
		vec4 tz = vec4(.0, .0, fimgz*fidxz, .0);
		vec4 tw = vec4(.0, .0, .0, 1.);
		mat4 transform = mat4(tx, ty, tz, tw);
		vPos = transform * vertexPosition;
	} else {
		// Float versions of ivec3's coordinates :
		float fnbx = float(neighborOffset.x);
		float fnby = float(neighborOffset.y);
		float fnbz = float(neighborOffset.z);

		// Each cube is one pixel wide. Which means the tex coordinates at the vertex given here are :
		float texCoordX = 1.f / fimgx;
		float texCoordY = 1.f / fimgy;
		float texCoordZ = 1.f / fimgz;
		// Base tex coordinate of the neighborhood 'cube' :
		vec3 neighborhoodBaseTexCoord = vec3(fnbx * texCoordX, fnby * texCoordY, fnbz * texCoordZ);
		// Tex offset within the neighborhood 'cube' :
		vec3 baseTexCoord = neighborhoodBaseTexCoord + vec3(fidxx * texCoordX, fidxy * texCoordY, fidxz * texCoordZ);
		// Tex coordinate of the vertex :
		texCoord = vec3(baseTexCoord.x + vertexPosition.x * texCoordX,
				baseTexCoord.y + vertexPosition.y * texCoordY,
				baseTexCoord.z + vertexPosition.z * texCoordZ);
		// NOTE	: the displacement for a pixel is given by vec3(1,1,1) * mMatrix + displacementSlice (TODO : fix this, might be wrong)

		// The unit of displacement (size of a single cube) :
		vec4 dis = vec4(1., 1., 1., 0.);
		// The base position of the neighbor grid as a whole :
		vec4 basePos = vec4(fnbx * dis.x, fnby * dis.y, fnbz * dis.z, 0.);
		// Position of the cube within the grid :
		vec4 posInGrid = vec4(dis.x * fidxx, dis.y * fidxy, dis.z * fidxz, 0.);
		// Final vertex position :
		vPos = (basePos + posInGrid + vertexPosition) * mMatrix;
	}

	gl_Position = mvp * vPos;
	vPos_WS = mMatrix * vPos;
}
