#version 400

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoord;

// --------------------------------------------------
// VShader outputs :
// --------------------------------------------------
out vec4 P_VS;
out vec3 text3DCoord_VS;
out vec4 P0_VS;
out vec3 text3DCoordP0_VS;
out vec4 P1_VS;
out vec3 text3DCoordP1_VS;
out vec4 P2_VS;
out vec3 text3DCoordP2_VS;
out vec4 P3_VS;
out vec3 text3DCoordP3_VS;
out float instanceId_VS;
out float visibility_VS;
//out float gl_ClipDistance[1];

// --------------------------------------------------
// Uniform variables:
// --------------------------------------------------
uniform sampler2D vertices_translations;
uniform sampler2D texture_coordinates;

uniform mat4 mMat;
uniform mat4 vMat;
uniform mat4 pMat;

uniform vec4 clipPlane = vec4(.0, 1., .0, .0);

uniform vec3 cut;
uniform vec3 cutDirection;
uniform vec3 clippingPoint;
uniform vec3 clippingNormal;

/**
* @brief function to convert 1D Index to 2D index (not normalized)
* @return The texture coordinate [0..iWrapSize]x[0..n]
*/
ivec2 Convert1DIndexTo2DIndex_Unnormed( in uint uiIndexToConvert, in int iWrapSize )
{
	int iY = int( uiIndexToConvert / unsigned int(iWrapSize) );
	int iX = int( uiIndexToConvert - ( unsigned int(iY) * unsigned int(iWrapSize) ) );
	return ivec2( iX, iY );
}

float ComputeVisibility(vec3 point)
{
	mat4 iGrid = mat4(1.); //mMat;
	vec4 point4 = vec4(point, 1.);
	vec4 cut4 = vec4(cut, 1.);
	vec4 vis4 = (/* iGrid */ point4) - cut4;
	vis4.xyz *= cutDirection;
	float xVis = vis4.x; // ((point.x - cut.x))*cutDirection.x;
	float yVis = vis4.y; // ((point.y - cut.y))*cutDirection.y;
	float zVis = vis4.z; // ((point.z - cut.z))*cutDirection.z;

	// vec3 pos = point - clippingPoint;
	// float vis = dot( clippingNormal, pos );
	if( xVis < 0.|| yVis < 0.|| zVis < 0. )
		return 1000.;
	else return 0.;
}

void main()
{
	int vertWidth = textureSize(vertices_translations, 0).x;

	ivec2 textCoord = Convert1DIndexTo2DIndex_Unnormed(uint(gl_InstanceID*12 + texCoord.x), vertWidth);

	vec3 textValue = texelFetch(vertices_translations, textCoord, 0).xyz;

	instanceId_VS = float(gl_InstanceID);

	P_VS = vec4(textValue.xyz, 1.);

	text3DCoord_VS = texelFetch(texture_coordinates, textCoord, 0).xyz;

	//Storing instance vertices informations: position and texture coordinates
	textCoord = Convert1DIndexTo2DIndex_Unnormed(unsigned int(gl_InstanceID*12 ), vertWidth);
	P3_VS = vec4( texelFetch(vertices_translations, textCoord, 0).xyz, 1. );
	text3DCoordP3_VS = texelFetch(texture_coordinates, textCoord, 0).xyz;;

	visibility_VS = ComputeVisibility(P3_VS.xyz);

	textCoord = Convert1DIndexTo2DIndex_Unnormed(unsigned int(gl_InstanceID*12 + 1 ), vertWidth);
	P1_VS = vec4( texelFetch(vertices_translations, textCoord, 0).xyz, 1. );
	text3DCoordP1_VS = texelFetch(texture_coordinates, textCoord, 0).xyz;;

	visibility_VS += ComputeVisibility(P1_VS.xyz);

	textCoord = Convert1DIndexTo2DIndex_Unnormed(unsigned int(gl_InstanceID*12 + 2 ), vertWidth);
	P2_VS = vec4( texelFetch(vertices_translations, textCoord, 0).xyz, 1. );
	text3DCoordP2_VS = texelFetch(texture_coordinates, textCoord, 0).xyz;;

	visibility_VS += ComputeVisibility(P2_VS.xyz);

	textCoord = Convert1DIndexTo2DIndex_Unnormed(unsigned int(gl_InstanceID*12 + 5 ), vertWidth);
	P0_VS = vec4( texelFetch(vertices_translations, textCoord, 0).xyz, 1. );
	text3DCoordP0_VS = texelFetch(texture_coordinates, textCoord, 0).xyz;

	visibility_VS += ComputeVisibility(P0_VS.xyz);

	// variable to toggle the use of the grid transformation matrix :
	mat4 viewTransfo = mat4(1.f);
/*
	viewTransfo = mat4(1., .0, .0, .0,
				.0, 1., .0, .0,
				.0, .0, 30., .0,
				.0, .0, .0, 1.);
*/
	gl_Position = pMat*vMat*viewTransfo*P_VS;
	//gl_ClipDistance[0] = dot(P_VS, vMat * clipPlane);
}
