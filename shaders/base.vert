#version 400 core

layout(location=0) in vec4 vertexPosition;
layout(location=1) in vec4 vertexNormal;

out vec4 vPos;
out vec4 vNorm;

uniform mat4 vMatrix;
uniform mat4 pMatrix;

void main(void)
{
	mat4 mvp = pMatrix * vMatrix;
	vPos = mvp * vertexPosition;
	vNorm = inverse(transpose(mvp)) * vertexNormal;
}
