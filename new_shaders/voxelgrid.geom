#version 150
#extension GL_ARB_explicit_attrib_location : enable

#define OUTLAYOUT

layout(triangles) in;
layout(triangle_strip, max_vertices=3) out;

in vec4 vPos_WS_VS[];
in vec4 vNorm_WS_VS[];
in vec3 texCoord_VS[];

in vec4 vPos_CS_VS[];
in vec4 vNorm_CS_VS[];
in vec4 lightDir_CS_VS[];
in vec4 eyeDir_CS_VS[];

OUTLAYOUT out vec4 vPos_WS;
OUTLAYOUT out vec4 vNorm_WS;
OUTLAYOUT out vec3 texCoord;
OUTLAYOUT out vec3 barycentricCoords;
OUTLAYOUT out vec3 largestDelta;
OUTLAYOUT out vec4 vPos_CS;
OUTLAYOUT out vec4 vNorm_CS;
OUTLAYOUT out vec4 lightDir_CS;
OUTLAYOUT out vec4 eyeDir_CS;

void main() {
	vec3 p0 = vPos_CS_VS[0].xyz;
	vec3 p1 = vPos_CS_VS[1].xyz;
	vec3 p2 = vPos_CS_VS[2].xyz;
	vec3 e1 = normalize(p1 - p0);
	vec3 e2 = normalize(p2 - p1);
	vec3 e3 = normalize(p0 - p2);
	// project point 2 on e1 :
	vec3 p2e1 = p0 + dot((p2-p0), e1) / dot(e1, e1) * e1;
	// project point 0 on e2
	vec3 p0e2 = p1 + dot((p0-p1), e2) / dot(e2, e2) * e2;
	// project point 1 on e3
	vec3 p1e3 = p0 + dot((p1-p2), e3) / dot(e3, e3) * e3;
	float dist_p2e1 = length(p2 - p2e1);
	float dist_p0e2 = length(p0 - p0e2);
	float dist_p1e3 = length(p1 - p1e3);
	vec3 maxDelta = vec3(dist_p0e2, dist_p1e3, dist_p2e1);
	gl_Position = gl_in[0].gl_Position;
	barycentricCoords = vec3(dist_p0e2, .0, .0);
	vPos_WS = vPos_WS_VS[0];
	vNorm_WS = vNorm_WS_VS[0];
	texCoord = texCoord_VS[0];
	largestDelta = maxDelta;
	vPos_CS = vPos_CS_VS[0];
	vNorm_CS = vNorm_CS_VS[0];
	lightDir_CS = lightDir_CS_VS[0];
	eyeDir_CS = eyeDir_CS_VS[0];
	EmitVertex();

	gl_Position = gl_in[1].gl_Position;
	barycentricCoords = vec3(.0, dist_p1e3, .0);
	vPos_WS = vPos_WS_VS[1];
	vNorm_WS = vNorm_WS_VS[1];
	texCoord = texCoord_VS[1];
	largestDelta = maxDelta;
	vPos_CS = vPos_CS_VS[1];
	vNorm_CS = vNorm_CS_VS[1];
	lightDir_CS = lightDir_CS_VS[1];
	eyeDir_CS = eyeDir_CS_VS[1];
	EmitVertex();

	gl_Position = gl_in[2].gl_Position;
	barycentricCoords = vec3(.0, .0, dist_p2e1);
	vPos_WS = vPos_WS_VS[2];
	vNorm_WS = vNorm_WS_VS[2];
	texCoord = texCoord_VS[2];
	largestDelta = maxDelta;
	vPos_CS = vPos_CS_VS[2];
	vNorm_CS = vNorm_CS_VS[2];
	lightDir_CS = lightDir_CS_VS[2];
	eyeDir_CS = eyeDir_CS_VS[2];
	EmitVertex();

	EndPrimitive();
}
