/*
		This shader is used for rendering the input/coarse-mesh

		Matthias Holländer 2008/2009
*/

// --------------------------------------------------
// shader definition
// --------------------------------------------------
#version 130
#extension GL_EXT_gpu_shader4 : enable

// --------------------------------------------------
// Uniform variables:
// --------------------------------------------------
	uniform int offset;

	uniform int normalDirection;
	uniform int width;

	uniform int neighbor_width;
	uniform int neighbor_nb_width;

	uniform bool visibility_check;

	uniform vec3 clippingPoint;
	uniform vec3 clippingNormal;
	uniform vec3 cut;
	uniform vec3 cutDirection;

	uniform uint visiblity_map[256];
	uniform vec4 color_map[256];


// --------------------------------------------------
// varying variables
// --------------------------------------------------
	out vec3 vLightDir;

	out vec3 my_position;
	out vec3 normal;

	//varying vec3 text3DCoord;
	out float visibility;

	out vec3 nb_neighbor;
// --------------------------------------------------
// Vertex-Shader
// --------------------------------------------------

	uniform sampler2D vertices_translations;
	uniform sampler2D normals_translations;
	uniform sampler2D texture_coordinates;
	uniform sampler2D neighbors_nb;
	uniform sampler2D neighbors;


/**
* @brief function to convert 1D Index to 2D index (not normalized)
* @return The texture coordinate [0..iWrapSize]x[0..n]
*/
ivec2 Convert1DIndexTo2DIndex_Unnormed( in unsigned int uiIndexToConvert, in int iWrapSize )
{
	int iY = int( uiIndexToConvert / unsigned int( iWrapSize ) );
	int iX = int( uiIndexToConvert - ( unsigned int( iY ) * unsigned int( iWrapSize ) ) );
	return ivec2( iX, iY );
}

float ComputeVisibility(vec3 point){

	float xVis = (point.x - cut.x)*cutDirection.x;
	float yVis = (point.y - cut.y)*cutDirection.y;
	float zVis = (point.z - cut.z)*cutDirection.z;

	vec3 pos = point - clippingPoint;
	//float vis = dot( clippingNormal, pos );
	if( xVis < 0.|| yVis < 0.|| zVis < 0. )
		return 1000.;
	else return 0.;
}

void main()
{

	ivec2 n_textCoord = Convert1DIndexTo2DIndex_Unnormed(unsigned int(gl_InstanceID), neighbor_nb_width);
	nb_neighbor = texelFetch(neighbors_nb, n_textCoord, 0).xyz;

	ivec2 textCoord = Convert1DIndexTo2DIndex_Unnormed(unsigned int(gl_InstanceID*12 + gl_MultiTexCoord0.x ), width);

	vec3 textValue = texelFetch(vertices_translations, textCoord, 0).xyz;

	vec4 v_position = vec4(textValue, 1.);

	my_position = (gl_ModelViewMatrix * v_position).xyz;

	textValue = texelFetch(normals_translations, textCoord, 0).xyz;

	normal = gl_NormalMatrix * textValue ;//* normalDirection;

	vLightDir = gl_LightSource[0].position.xyz - my_position;

	visibility = 0.;
	if(visibility_check){
		visibility = ComputeVisibility(v_position.xyz);
	}

	gl_Position = gl_ModelViewProjectionMatrix*v_position;

}
