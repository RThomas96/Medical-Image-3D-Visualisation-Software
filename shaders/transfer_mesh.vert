#version 130
#extension GL_EXT_gpu_shader4 : enable

// --------------------------------------------------
// VShader outputs :
// --------------------------------------------------
out vec4 P;
out vec3 text3DCoord;

out vec4 P0;
out vec3 text3DCoordP0;

out vec4 P1;
out vec3 text3DCoordP1;

out vec4 P2;
out vec3 text3DCoordP2;

out vec4 P3;
out vec3 text3DCoordP3;

out float instanceId;
out float visibility;

// --------------------------------------------------
// Uniform variables:
// --------------------------------------------------
uniform sampler2D vertices_translations;
uniform sampler2D normals_translations;
uniform sampler2D texture_coordinates;

uniform sampler2D neighbors_nb;
uniform sampler2D neighbors;
uniform sampler2D visibility_texture;

uniform mat4 mMat;
uniform mat4 vMat;
uniform mat4 pMat;

uniform vec3 cut;
uniform vec3 cutDirection;
uniform vec3 clippingPoint;
uniform vec3 clippingNormal;

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

float ComputeVisibility(vec3 point)
{
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
	int vertWidth = textureSize(vertices_translations, 0).x;

	ivec2 textCoord = Convert1DIndexTo2DIndex_Unnormed(unsigned int(gl_InstanceID*12 + gl_MultiTexCoord0.x ), vertWidth);

	vec3 textValue = texelFetch(vertices_translations, textCoord, 0).xyz;

	instanceId = gl_InstanceID;

	P = vec4(textValue.xyz, 1.);

	text3DCoord = texelFetch(texture_coordinates, textCoord, 0).xyz;

	//Storing instance vertices informations: position and texture coordinates
	textCoord = Convert1DIndexTo2DIndex_Unnormed(unsigned int(gl_InstanceID*12 ), vertWidth);
	P3 = vec4( texelFetch(vertices_translations, textCoord, 0).xyz, 1. );
	text3DCoordP3 = texelFetch(texture_coordinates, textCoord, 0).xyz;;

	visibility = ComputeVisibility(P3.xyz);

	textCoord = Convert1DIndexTo2DIndex_Unnormed(unsigned int(gl_InstanceID*12 + 1 ), vertWidth);
	P1 = vec4( texelFetch(vertices_translations, textCoord, 0).xyz, 1. );
	text3DCoordP1 = texelFetch(texture_coordinates, textCoord, 0).xyz;;

	visibility += ComputeVisibility(P1.xyz);

	textCoord = Convert1DIndexTo2DIndex_Unnormed(unsigned int(gl_InstanceID*12 + 2 ), vertWidth);
	P2 = vec4( texelFetch(vertices_translations, textCoord, 0).xyz, 1. );
	text3DCoordP2 = texelFetch(texture_coordinates, textCoord, 0).xyz;;
	visibility += ComputeVisibility(P2.xyz);

	textCoord = Convert1DIndexTo2DIndex_Unnormed(unsigned int(gl_InstanceID*12 + 5 ), vertWidth);
	P0 = vec4( texelFetch(vertices_translations, textCoord, 0).xyz, 1. );
	text3DCoordP0 = texelFetch(texture_coordinates, textCoord, 0).xyz;
	visibility += ComputeVisibility(P0.xyz);

	gl_Position = pMat*vMat*mMat*P;
}
