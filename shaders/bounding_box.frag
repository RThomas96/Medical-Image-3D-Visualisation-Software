#version 400 core

#extension GL_ARB_separate_shader_objects : enable

/****************************************/
/**************** Inputs ****************/
/****************************************/
layout(location=0) in vec4 vPos;	// Vertex position, in world space

/****************************************/
/*************** Outputs ****************/
/****************************************/
out vec4 color;

/****************************************/
/*************** Uniforms ***************/
/****************************************/
uniform mat4 pMat;	// Projection matrix
uniform mat4 vMat;	// View matrix
uniform vec4 bbColor = vec4(.257, .257, .257, 1.);	// Default BB color = mid-gray
uniform vec3 bbSize;	// Bounding box dimensions
uniform vec3 bbPos;	// Bounding box original position (min point)

/****************************************/
/*********** Function headers ***********/
/****************************************/

/****************************************/
/***************** Main *****************/
/****************************************/
void main(void) {
	color = bbColor;
	return;
}

/****************************************/
/************** Functions ***************/
/****************************************/
