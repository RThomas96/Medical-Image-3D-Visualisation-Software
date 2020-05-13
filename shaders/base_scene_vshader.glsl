#version 400 core

layout (location=0) in vec4 vertexPosition;
layout (location=1) in vec3 vertexTexCoordinates;

// HELLOOOOOO

out vec4 vPos_WS;
out vec3 vertexTexWorld;

out vec4 vPos_CS;
out vec4 lightDir_CS;
out vec4 eyeDir_CS;

uniform mat4 mMatrix;
uniform mat4 vMatrix;
uniform mat4 pMatrix;
uniform vec4 lightPos; // will always be worldspace here !

// Offset from 0,0,0 to location normalized
uniform vec3 texOffset;
// Size of the inspector, normalized to sample size :
uniform vec3 inspectorTexSize;

void main(void)
{
	mat4 mvp = pMatrix * vMatrix * mMatrix;
	gl_Position = mvp * vertexPosition;
	vPos_WS = mMatrix * vertexPosition;

	vPos_CS = vMatrix * vertexPosition;

	vertexTexWorld = vertexTexCoordinates;
	vec3 p = texOffset + inspectorTexSize;
}
