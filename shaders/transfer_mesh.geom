#version 150

#define OUTLAYOUT noperspective

layout(triangles) in;
layout(triangle_strip, max_vertices=3) out;

in vec4 P_VS[];
in vec3 text3DCoord_VS[];
in vec4 P0_VS[];
in vec3 text3DCoordP0_VS[];
in vec4 P1_VS[];
in vec3 text3DCoordP1_VS[];
in vec4 P2_VS[];
in vec3 text3DCoordP2_VS[];
in vec4 P3_VS[];
in vec3 text3DCoordP3_VS[];
in float instanceId_VS[];
in float visibility_VS[];

OUTLAYOUT out vec4 P;
OUTLAYOUT out vec3 text3DCoord;
OUTLAYOUT out vec4 P0;
OUTLAYOUT out vec3 text3DCoordP0;
OUTLAYOUT out vec4 P1;
OUTLAYOUT out vec3 text3DCoordP1;
OUTLAYOUT out vec4 P2;
OUTLAYOUT out vec3 text3DCoordP2;
OUTLAYOUT out vec4 P3;
OUTLAYOUT out vec3 text3DCoordP3;
OUTLAYOUT out vec3 barycentricCoords;
OUTLAYOUT out vec3 largestDelta;
OUTLAYOUT out float instanceId;
OUTLAYOUT out float visibility;

uniform mat4 vMat;

void main(void)
{
	// Compute the triangle's barycentric coords :
	vec3 p0 = (vMat * P_VS[0]).xyz;
	vec3 p1 = (vMat * P_VS[1]).xyz;
	vec3 p2 = (vMat * P_VS[2]).xyz;
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

	// Apply and send to Rasterizer/FShader :

	int idx = 0;

	gl_Position = gl_in[idx].gl_Position;
	P = P_VS[idx];
	text3DCoord = text3DCoord_VS[idx];
	P0 = P0_VS[idx];
	text3DCoordP0 = text3DCoordP0_VS[idx];
	P1 = P1_VS[idx];
	text3DCoordP1 = text3DCoordP1_VS[idx];
	P2 = P2_VS[idx];
	text3DCoordP2 = text3DCoordP2_VS[idx];
	P3 = P3_VS[idx];
	text3DCoordP3 = text3DCoordP3_VS[idx];
	barycentricCoords = vec3(dist_p0e2, .0, .0);
	largestDelta = maxDelta;
	instanceId = instanceId_VS[idx];
	visibility = visibility_VS[idx];
	EmitVertex();

	idx++;

	gl_Position = gl_in[idx].gl_Position;
	P = P_VS[idx];
	text3DCoord = text3DCoord_VS[idx];
	P0 = P0_VS[idx];
	text3DCoordP0 = text3DCoordP0_VS[idx];
	P1 = P1_VS[idx];
	text3DCoordP1 = text3DCoordP1_VS[idx];
	P2 = P2_VS[idx];
	text3DCoordP2 = text3DCoordP2_VS[idx];
	P3 = P3_VS[idx];
	text3DCoordP3 = text3DCoordP3_VS[idx];
	barycentricCoords = vec3(.0, dist_p1e3, .0);
	largestDelta = maxDelta;
	instanceId = instanceId_VS[idx];
	visibility = visibility_VS[idx];
	EmitVertex();

	idx++;

	gl_Position = gl_in[idx].gl_Position;
	P = P_VS[idx];
	text3DCoord = text3DCoord_VS[idx];
	P0 = P0_VS[idx];
	text3DCoordP0 = text3DCoordP0_VS[idx];
	P1 = P1_VS[idx];
	text3DCoordP1 = text3DCoordP1_VS[idx];
	P2 = P2_VS[idx];
	text3DCoordP2 = text3DCoordP2_VS[idx];
	P3 = P3_VS[idx];
	text3DCoordP3 = text3DCoordP3_VS[idx];
	barycentricCoords = vec3(.0, .0, dist_p2e1);
	largestDelta = maxDelta;
	instanceId = instanceId_VS[idx];
	visibility = visibility_VS[idx];
	EmitVertex();

	EndPrimitive();
}
