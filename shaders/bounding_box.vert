#version 400 core

/****************************************/
/**************** Inputs ****************/
/****************************************/
layout(location=0) in vec4 vertexPosition;	// Vertex position, in world space

/****************************************/
/*************** Outputs ****************/
/****************************************/
out vec4 vPos;

/****************************************/
/*************** Uniforms ***************/
/****************************************/
uniform mat4 pMat;	// Projection matrix
uniform mat4 vMat;	// View matrix

/****************************************/
/*********** Function headers ***********/
/****************************************/

/****************************************/
/***************** Main *****************/
/****************************************/
void main(void) {
	gl_Position = pMat * vMat * vertexPosition;
}

/****************************************/
/************** Functions ***************/
/****************************************/

