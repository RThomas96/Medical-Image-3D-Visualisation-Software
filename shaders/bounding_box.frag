#version 150 core

/****************************************/
/**************** Inputs ****************/
/****************************************/
in vec4 vPos;	// Vertex position, in world space

/****************************************/
/*************** Outputs ****************/
/****************************************/
out vec4 color;

/****************************************/
/*************** Uniforms ***************/
/****************************************/
uniform mat4 pMat;	// Projection matrix
uniform mat4 vMat;	// View matrix
uniform vec3 bbColor;	// Asked color for the lines
uniform vec3 bbSize;	// Bounding box dimensions
uniform vec3 bbPos;	// Bounding box original position (min point)

/****************************************/
/*********** Function headers ***********/
/****************************************/

/****************************************/
/***************** Main *****************/
/****************************************/
void main(void) {
	color.xyz = bbColor;
	color.w = 1.f;
	return;
}

/****************************************/
/************** Functions ***************/
/****************************************/
