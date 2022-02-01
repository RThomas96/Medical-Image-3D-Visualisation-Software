#version 150 core
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_explicit_attrib_location : enable

/****************************************/
/**************** Inputs ****************/
/****************************************/
layout(location=0) in vec3 vertexPosition;	// Vertex position, in world space

/****************************************/
/*************** Outputs ****************/
/****************************************/
out vec4 vPos;

/****************************************/
/*************** Uniforms ***************/
/****************************************/
uniform mat4 pMat;	// Projection matrix
uniform mat4 vMat;	// View matrix
uniform vec3 bbSize;	// Bounding box dimensions
uniform vec3 bbPos;	// Bounding box original position (min point)

/****************************************/
/*********** Function headers ***********/
/****************************************/

/****************************************/
/***************** Main *****************/
/****************************************/
void main(void) {
	vPos = vec4(.0, .0, .0, 1.);
	vPos.xyz = bbPos + (vertexPosition.xyz * bbSize);
	gl_Position = pMat * vMat * vPos;
}

/****************************************/
/************** Functions ***************/
/****************************************/

