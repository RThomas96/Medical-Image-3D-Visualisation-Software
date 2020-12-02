#version 400

// Geometry shader input layout :
layout(triangles) in;
// Geometry shader output layout :
layout(triangle_strip, max_vertices=3) out;

// GS inputs :
in vec4 vPos_WS_VS[];
in vec4 vNorm_WS_VS[];
in vec3 texCoord_VS[];

in vec4 vPos_CS_VS[];
in vec4 vNorm_CS_VS[];
in vec4 lightDir_CS_VS[];
in vec4 eyeDir_CS_VS[];

// GS outputs :

void main() {
	EndPrimitive();
}
