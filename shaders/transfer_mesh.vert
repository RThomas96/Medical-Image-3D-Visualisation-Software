#version 150
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_explicit_attrib_location : enable

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

// --------------------------------------------------
// Uniform variables:
// --------------------------------------------------
uniform sampler2D vertices_translations;
uniform sampler2D texture_coordinates;

uniform mat4 mMat;
uniform mat4 vMat;
uniform mat4 pMat;

uniform vec3 cam;
uniform vec3 volumeEpsilon;

uniform vec4 clipPlane = vec4(.0, 1., .0, .0);

uniform vec3 cut;
uniform vec3 cutDirection;

// Scalar to displace the camera's cutting plane :
uniform float clipDistanceFromCamera;

// 'Cube' visualization (restricted to a bounding box) :
uniform vec3 visuBBMin;
uniform vec3 visuBBMax;
uniform bool shouldUseBB;

/**
* @brief function to convert 1D Index to 2D index (not normalized)
* @return The texture coordinate [0..iWrapSize]x[0..n]
*/
ivec2 Convert1DIndexTo2DIndex_Unnormed( in uint uiIndexToConvert, in int iWrapSize )
{
	int iY = int( uiIndexToConvert / uint(iWrapSize) );
	int iX = int( uiIndexToConvert - ( uint(iY) * uint(iWrapSize) ) );
	return ivec2( iX, iY );
}

bool intersectionTriangleBox(vec3 t0, vec3 t1, vec3 t2, vec3 minAABBox, vec3 maxAABBox);

float ComputeVisibility(vec3 point, vec3 p1, vec3 p2, vec3 p3) {
    vec4 point4 = vec4(point, 1.);
    vec4 cut4 = vec4(cut - cut*0.1, 1.);
    vec4 vis4 = point4 - cut4;
    vis4.xyz *= cutDirection;
    float xVis = vis4.x; // ((point.x - cut.x))*cutDirection.x;
    float yVis = vis4.y; // ((point.y - cut.y))*cutDirection.y;
    float zVis = vis4.z; // ((point.z - cut.z))*cutDirection.z;

    vec4 clippingPoint = vec4(cam, 1.);
    vec4 clippingNormal = normalize(inverse(vMat) * vec4(.0, .0, -1., .0));
    clippingPoint += clippingNormal * clipDistanceFromCamera;
    vec4 pos = point4 - clippingPoint;
    float vis = dot( clippingNormal, pos );

    vec3 minScene = cut;
    vec3 maxScene = visuBBMax;

    for(int i = 0; i < 3; ++i) {
        if(cutDirection[i] < 0) {
            maxScene[i] = minScene[i];
            minScene[i] = visuBBMin[i];
        }
    }

    if( xVis < 0.|| yVis < 0.|| zVis < 0. || vis < .0) {
        if(intersectionTriangleBox(point, p1, p2, minScene, maxScene) ||
           intersectionTriangleBox(point, p2, p3, minScene, maxScene) ||
           intersectionTriangleBox(point, p1, p3, minScene, maxScene)) {
            return 0.;
        } else {
            return 1000.;
        }
    } else {
        return 0.;
    }
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
	textCoord = Convert1DIndexTo2DIndex_Unnormed(uint(gl_InstanceID*12 ), vertWidth);
	P3_VS = vec4( texelFetch(vertices_translations, textCoord, 0).xyz, 1. );
	text3DCoordP3_VS = texelFetch(texture_coordinates, textCoord, 0).xyz;;

    visibility_VS = ComputeVisibility(P3_VS.xyz, P1_VS.xyz, P2_VS.xyz, P0_VS.xyz);

	textCoord = Convert1DIndexTo2DIndex_Unnormed(uint(gl_InstanceID*12 + 1 ), vertWidth);
	P1_VS = vec4( texelFetch(vertices_translations, textCoord, 0).xyz, 1. );
	text3DCoordP1_VS = texelFetch(texture_coordinates, textCoord, 0).xyz;;

    visibility_VS += ComputeVisibility(P1_VS.xyz, P2_VS.xyz, P0_VS.xyz, P3_VS.xyz);

	textCoord = Convert1DIndexTo2DIndex_Unnormed(uint(gl_InstanceID*12 + 2 ), vertWidth);
	P2_VS = vec4( texelFetch(vertices_translations, textCoord, 0).xyz, 1. );
	text3DCoordP2_VS = texelFetch(texture_coordinates, textCoord, 0).xyz;;

    visibility_VS += ComputeVisibility(P2_VS.xyz, P0_VS.xyz, P3_VS.xyz, P1_VS.xyz);

	textCoord = Convert1DIndexTo2DIndex_Unnormed(uint(gl_InstanceID*12 + 5 ), vertWidth);
	P0_VS = vec4( texelFetch(vertices_translations, textCoord, 0).xyz, 1. );
	text3DCoordP0_VS = texelFetch(texture_coordinates, textCoord, 0).xyz;

    visibility_VS += ComputeVisibility(P0_VS.xyz, P3_VS.xyz, P1_VS.xyz, P2_VS.xyz);

	gl_Position = pMat*vMat*mMat*P_VS;
}

void getProjMinMax(vec3 v, vec3 axis, inout float proj_min, inout float proj_max) {
    float projection = dot(axis, v);
    proj_min = min(proj_min, projection);
    proj_max = max(proj_max, projection);
}

bool intersectionTriangleAABBox(vec3 t0, vec3 t1, vec3 t2, vec3 boxCenter, vec3 halfSizeX, vec3 halfSizeY, vec3 halfSizeZ)
{
    // Taken from https://stackoverflow.com/a/17503268/9863298

    vec3 boxNormals[3] = vec3[3](normalize(halfSizeX), normalize(halfSizeY), normalize(halfSizeZ));
    vec3 triPoints[3] = vec3[3](t0, t1, t2);
    vec3 triEdges[3] = vec3[3](t1 - t0, t2 - t1, t0 - t2);
    vec3 cubeVertices[8] = vec3[8](boxCenter - halfSizeX - halfSizeY - halfSizeZ,
                                   boxCenter - halfSizeX - halfSizeY + halfSizeZ,
                                   boxCenter - halfSizeX + halfSizeY - halfSizeZ,
                                   boxCenter - halfSizeX + halfSizeY + halfSizeZ,
                                   boxCenter + halfSizeX - halfSizeY - halfSizeZ,
                                   boxCenter + halfSizeX - halfSizeY + halfSizeZ,
                                   boxCenter + halfSizeX + halfSizeY - halfSizeZ,
                                   boxCenter + halfSizeX + halfSizeY + halfSizeZ);
    // Check that the projected triangle intersect the projected box on all the axis
    for (int i = 0; i < 3; i++) {
        float proj_min =  10000000000.;
        float proj_max = -10000000000.;
        getProjMinMax(t0, boxNormals[i], proj_min, proj_max);
        getProjMinMax(t1, boxNormals[i], proj_min, proj_max);
        getProjMinMax(t2, boxNormals[i], proj_min, proj_max);

        float proj_min2 =  10000000000.;
        float proj_max2 = -10000000000.;
        for (int j = 0; j < 8; j++) {
            getProjMinMax(cubeVertices[j], boxNormals[i], proj_min2, proj_max2);
        }

        if (proj_min > proj_max2 || proj_max < proj_min2)
            return false;
    }

    vec3 normal = normalize(cross((t0 - t1), (t2 - t1)));

    float proj_min =  10000000000.;
    float proj_max = -10000000000.;
    for (int j = 0; j < 8; j++) {
        getProjMinMax(cubeVertices[j], normal, proj_min, proj_max);
    }

    float triOffset = dot(normal, t0);
    if (proj_min > triOffset || proj_max < triOffset)
        return false;

    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            vec3 axis = cross(triEdges[i], boxNormals[j]);

            float proj_min1 =  10000000000.;
            float proj_max1 = -10000000000.;
            for (int k = 0; k < 8; k++) {
                getProjMinMax(cubeVertices[k], axis, proj_min1, proj_max1);
            }

            float proj_min2 =  10000000000.;
            float proj_max2 = -10000000000.;
            for (int k = 0; k < 3; k++) {
                getProjMinMax(triPoints[k], axis, proj_min2, proj_max2);
            }

            if(proj_max1 <= proj_min2 || proj_min1 >= proj_max2)
                return false;
        }
    }

    return true;
}

bool intersectionTriangleBox(vec3 t0, vec3 t1, vec3 t2, vec3 minAABBox, vec3 maxAABBox) {
    vec3 box = maxAABBox - minAABBox;
    return intersectionTriangleAABBox(t0, t1, t2, (minAABBox + maxAABBox)/2.f, vec3(box.x * .5f, 0, 0), vec3(0, box.y * .5f, 0), vec3(0, 0, box.z * .5f));
}
