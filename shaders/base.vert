#version 400 core

// VAO inputs :
layout(location=0) in vec4 vertexPosition;
layout(location=1) in vec4 vertexNormal;
layout(location=2) in uvec4 voxelIndex; // is supposed to change every instance, not every vertex

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
uniform vec4 voxelGridOrigin;
uniform vec3 voxelGridSize;
uniform uint voxelGridShown;

// Initial space data :
uniform uint scaledCubes;
uniform uvec3 imageSize;
uniform uvec3 neighborOffset;

// Cutting planes for the grid, will
// always have components in [0, 1]:
uniform vec3 cutPlaneMin;
uniform vec3 cutPlaneMax;

void main(void) {
	// ModelViewProjection matrix :
	mat4 mvp = pMatrix * vMatrix * mMatrix;
	// Inverse of transpose of model matrix for normals, computed once :
	mat4 iMatrix = inverse(mMatrix);
	mat4 minverse = transpose(iMatrix);

	// Camera space position of the light source :
	vec4 lightPos_CS = vMatrix * lightPos;
	vNorm_WS_VS = minverse * (normalize(vertexNormal));
	vNorm_CS_VS = vMatrix * minverse * normalize(vertexNormal);

	// Float versions of program data :
	float fimgx = float(imageSize.x);
	float fimgy = float(imageSize.y);
	float fimgz = float(imageSize.z);

	float fidxx = float(voxelIndex.x);
	float fidxy = float(voxelIndex.y);
	float fidxz = float(voxelIndex.z);

	// Contains the position of the vertex after transform :
	vec4 vPos = vec4(.0,.0,.0,.0);

	if (gl_InstanceID < scaledCubes) {
		// Cube size, if displayed entirely :
		vec3 size = vec3(fidxx, fidxy, fidxz);

		// Compute the displacement needed for the origin of the cube :
		vec3 origin = vec3(
			cutPlaneMin.x * size.x,
			cutPlaneMin.y * size.y,
			cutPlaneMin.z * size.z
		);

		// The cube needs to be shrinked down to the size dictated by
		// the cutting planes :
		size.x = (1.-(cutPlaneMin.x + (1. - cutPlaneMax.x))) * size.x;
		size.y = (1.-(cutPlaneMin.y + (1. - cutPlaneMax.y))) * size.y;
		size.z = (1.-(cutPlaneMin.z + (1. - cutPlaneMax.z))) * size.z;

		// For the texture cube, at instance 0, we display the part of the
		// texture that needs to be displayed :
		texCoord_VS = vec3(
			(vertexPosition.x < 0.5) ? cutPlaneMin.x : cutPlaneMax.x,
			(vertexPosition.y < 0.5) ? cutPlaneMin.y : cutPlaneMax.y,
			(vertexPosition.z < 0.5) ? cutPlaneMin.z : cutPlaneMax.z
		);

		// The transformation matrix for it to show the true texture size :
		vec4 tx = vec4(size.x, .0, .0, .0);
		vec4 ty = vec4(.0, size.y, .0, .0);
		vec4 tz = vec4(.0, .0, size.z, .0);
		vec4 tw = vec4(origin.x, origin.y, origin.z, 1.);
		mat4 transform = mat4(tx, ty, tz, tw);
		vPos = transform * vertexPosition;
	} else {
		// Float versions of ivec3's coordinates :
		float nbx = float(neighborOffset.x-1u);
		float nby = float(neighborOffset.y-1u);
		float nbz = float(neighborOffset.z-1u);

		// Each cube is one pixel wide. Which means the tex coordinates at the vertex given here are :
		float texCoordX = 1.f / fimgx;
		float texCoordY = 1.f / fimgy;
		float texCoordZ = 1.f / fimgz;
		// Base tex coordinate of the neighborhood 'cube' :
		vec3 neighborhoodBaseTexCoord = vec3(nbx * texCoordX, nby * texCoordY, nbz * texCoordZ);
		// Tex offset within the neighborhood 'cube' :
		vec3 baseTexCoord = neighborhoodBaseTexCoord + vec3(fidxx * texCoordX, fidxy * texCoordY, fidxz * texCoordZ);
		// Tex coordinate of the vertex :
		texCoord_VS = vec3(baseTexCoord.x + vertexPosition.x * texCoordX,
				baseTexCoord.y + vertexPosition.y * texCoordY,
				baseTexCoord.z + vertexPosition.z * texCoordZ);

		// The unit of displacement (size of a single cube) :
		vec4 dis = vec4(1., 1., 1., 0.);
		// The base position of the neighbor grid as a whole :
		vec4 basePos = vec4(nbx * dis.x, nby * dis.y, nbz * dis.z, 0.);
		// Position of the cube within the grid :
		vec4 posInGrid = vec4(dis.x * fidxx, dis.y * fidxy, dis.z * fidxz, 0.);
		// Final vertex position :
		vPos = (basePos + posInGrid + vertexPosition) ;
		if (voxelIndex.w == 0u) {
			vPos *= iMatrix; // make it inverse of transfo, to get the shape in initial space to reflect the 'real' space
		}
	}

	gl_Position = mvp * vPos;
	vPos_WS_VS = mMatrix * vPos;
	vPos_CS_VS = vMatrix * mMatrix * vPos;

	eyeDir_CS_VS = (vPos_CS_VS - vec4(.0,.0,.0,.0));
	lightDir_CS_VS = lightPos_CS + eyeDir_CS_VS;
}
