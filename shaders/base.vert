#version 400 core

layout(location=0) in vec4 vertexPosition;
layout(location=1) in vec4 vertexNormal;

out vec4 vPos;
out vec4 vNorm;

uniform mat4 vMatrix;
uniform mat4 pMatrix;

void main(void)
{
	mat4 mvp = vMatrix * pMatrix;
	vec3 scaledPos = vertexPosition.xyz * .5;
	vec3 finalPos = scaledPos + vec3(.25,.25,.25);
	vPos = vec4(finalPos,1.0);
	vNorm = vertexNormal;
}
