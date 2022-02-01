#version 150
#extension GL_ARB_explicit_attrib_location : enable

// Geometry shader input layout :
layout(triangles) in;
// Geometry shader output layout :
layout(triangle_strip, max_vertices=3) out;

// GS inputs :
in vec4 vPos[];
in vec4 vNorm[];
in vec3 texCoord[];

noperspective out vec4 vPos_GS;
noperspective out vec4 vNorm_GS;
noperspective out vec3 texCoord_GS;

// GS outputs :

void main() {
	vPos_GS = vPos[0];
	vNorm_GS = vNorm[0];
	texCoord_GS = texCoord[0];
	EmitVertex();
	vPos_GS = vPos[1];
	vNorm_GS = vNorm[1];
	texCoord_GS = texCoord[1];
	EmitVertex();
	vPos_GS = vPos[2];
	vNorm_GS = vNorm[2];
	texCoord_GS = texCoord[2];
	EmitVertex();
	EndPrimitive();
}
