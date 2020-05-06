#version 400 core

layout (location=0) in vec4 vertexPosition;
layout (location=1) in vec3 vertexTexCoordinates;

// HELLOOOOOO

// Vertex coordinates for fragment shader :
out vec4 vertexPosWorld;
out vec3 vertexTexWorld;

uniform mat4 mMatrix;
uniform mat4 vMatrix;
uniform mat4 pMatrix;

// Offset from 0,0,0 to location normalized
uniform vec3 texOffset;
// Size of the inspector, normalized to sample size :
uniform vec3 inspectorTexSize;

void main(void)
{
	mat4 mvp = pMatrix * vMatrix;
	vertexPosWorld = mvp * vertexPosition;
	vertexTexWorld = vertexTexCoordinates;
	vec3 p = texOffset + inspectorTexSize;
}
