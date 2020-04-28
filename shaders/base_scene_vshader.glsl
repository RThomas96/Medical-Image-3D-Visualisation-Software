#version 400

layout(location=0) in vec4 vertexPosition;
layout(location=1) in vec3 vertexTexCoordinates;

uniform mat4 mMatrix;
uniform mat4 vMatrix;
uniform mat4 pMatrix;

// Transformation matrix for if we draw real/initial space :
uniform mat4 transformationMatrix;

// Offset from 0,0,0 to location normalized
uniform vec3 texOffset;
// Size of the inspector, normalized to sample size :
uniform vec3 inspectorTexSize;

// Vertex coordinates for fragment shader :
out vec4 vertexPosWorld;
out vec3 vertexTexWorld;

void main(void)
{
	// todo : check if the order for matrix multiplication is correct, don't remember.
	mat4 mvp = mvMatrix * pMatrix;
	mat4 mmvp = mvp * transformationMatrix;
	vertexPosWorld = mmvp * vertexPosition;
	vertexTexWorld = vertexTexCoordinates;
}
