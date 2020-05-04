#version 400

in vec4 vertexPosition;
in vec3 vertexTexCoordinates;

uniform mat4 mMatrix;
uniform mat4 vMatrix;
uniform mat4 pMatrix;

// Offset from 0,0,0 to location normalized
uniform vec3 texOffset;
// Size of the inspector, normalized to sample size :
uniform vec3 inspectorTexSize;

// Vertex coordinates for fragment shader :
out vec4 vertexPosWorld;
out vec3 vertexTexWorld;

void main(void)
{
	mat4 mvp = pMatrix * vMatrix;
	vertexPosWorld = vertexPosition;
	vertexTexWorld = vertexTexCoordinates;
}
