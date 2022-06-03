#version 150
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_explicit_attrib_location : enable

#define INLAYOUT

// Signals we're in the main shader, for any shaders inserted into this one.
#define MAIN_SHADER_UNIT

/****************************************/
/**************** Inputs ****************/
/****************************************/
INLAYOUT in vec4 P;
INLAYOUT in vec3 text3DCoord;
INLAYOUT in vec4 P0;
INLAYOUT in vec3 text3DCoordP0;
INLAYOUT in vec4 P1;
INLAYOUT in vec3 text3DCoordP1;
INLAYOUT in vec4 P2;
INLAYOUT in vec3 text3DCoordP2;
INLAYOUT in vec4 P3;
INLAYOUT in vec3 text3DCoordP3;

INLAYOUT in vec3 barycentricCoords;
INLAYOUT in vec3 largestDelta;

INLAYOUT in float instanceId;
INLAYOUT in float visibility;

/****************************************/
/*************** Outputs ****************/
/****************************************/
layout(location = 0) out vec4 colorOut;
layout(location = 1) out vec4 sceneSpaceFragmentPos;

/****************************************/
/*************** Uniforms ***************/
/****************************************/
uniform usampler3D texData; // déclaration de la map mask : texture 3D à raycaster
uniform sampler2D vertices_translations;
uniform sampler2D normals_translations;
uniform sampler2D texture_coordinates;
uniform sampler2D visibility_texture;
uniform sampler2D neighbors;

// Phong :
uniform float diffuseRef;
uniform float specRef;
// Light positions
uniform vec3 lightPositions[8];

// Grid voxel dimensions :
uniform vec3 voxelSize;
uniform ivec3 gridSize;

// Camera position world-space :
uniform vec3 cam;
// Variables for visibility :
uniform vec3 cut;
uniform vec3 cutDirection;
uniform float clipDistanceFromCamera;

uniform mat4 mMat;
uniform mat4 vMat;
uniform mat4 pMat;

uniform vec3 visuBBMin;
uniform vec3 visuBBMax;
uniform bool shouldUseBB;

uniform bool displayWireframe;

// The structure which defines every attributes for the color channels.
struct colorChannelAttributes {
	uint isVisible;			// /*align : ui64*/ Is this color channel enabled/visible ?
	uint colorScaleIndex;	// /*align : ui64*/ The color channel to choose
	uvec2 visibleBounds;	// /*align : vec4*/ The bounds of the visible values
	uvec2 colorScaleBounds;	// /*align : vec4*/ The value bounds for the color scale
};

uniform uint mainChannelIndex;						// The index of the main channel in the voxel data
uniform sampler1D colorScales[4];					// All the color scales available (all encoded as 1D textures)
layout(std140) uniform ColorBlock {
	colorChannelAttributes attributes[4];	// Color attributes laid out in this way : [ main, R, G, B ]
} colorChannels;

// Evaluate the fragment with per-channel color scales :
vec4 fragmentEvaluationSingleChannel(in uvec3 color);

// General utility functions :
vec3 crossProduct( vec3 a, vec3 b );
vec3 getWorldCoordinates(in ivec3 _gridCoord);
ivec3 getGridCoordinates( in vec4 _P );

// Coordinate changes for textures :
ivec2 Convert1DIndexTo2DIndex_Unnormed( in uint uiIndexToConvert, in int iWrapSize );
ivec2 Convert1DIndexTo2DIndex_Unnormed_Flipped( in uint uiIndexToConvert, in int iWrapSize );

// Barycentric search :
bool computeBarycentricCoordinates(in vec3 point, out float ld0, out float ld1, out float ld2, out float ld3);
bool computeBarycentricCoordinates(in vec3 point, out float ld0, out float ld1, out float ld2, out float ld3, in int id_tetra_start, out int id_tetra_end, out vec3 Current_text3DCoord);
bool computeBarycentricCoordinatesRecursive(in vec3 point, out float ld0, out float ld1, out float ld2, out float ld3, in int id_tetra_start, out int id_tetra_end, int max_iter, out vec3 result_text_coord);
void getFirstRayVoxelIntersection( in vec3 origin, in vec3 direction, out ivec3 v0, out vec3 t_n);

// Compute a fragment's visibility :
bool ComputeVisibility(vec3 point);

// Color and shading :
bool checkAndColorizeVoxel(in uvec3 voxel, out vec4 color) ;
vec3 phongComputation(vec4 position, vec3 normal, vec4 color, vec3 lightPos, vec3 phongDetails, mat3 lightDetails);

#pragma include_color_shader;

#line 2111

void main (void) {
	sceneSpaceFragmentPos = vec4(.0,.0,.0,.0);
	gl_FragDepth = gl_FragCoord.z;

    if( visibility > 3500. ) discard;


    // Shows the wireframe of the mesh :
    if(displayWireframe == true) {
        float epsilon = 0.008;
        float distMin = min(barycentricCoords.x/largestDelta.x, min(barycentricCoords.y/largestDelta.y, barycentricCoords.z/largestDelta.z));

        // Enables a 1-pass wireframe mode :
        if (distMin < epsilon) {// && visibility > 0.) {
            float factor = (visibility/3500.);
            colorOut = vec4(1.-factor, factor, 1.-factor, 1.);
            sceneSpaceFragmentPos = vec4(P.xyz, 2.f);
            return;
        }
    }
	

	// Default color of the fragment
	colorOut = vec4(.0, .0, .0, .0);

	vec3 V = normalize(P.xyz - cam);

	bool hit = false; // have we hit a voxle that is supposed to be shown yet ?
	bool in_tet = true; // are we in a tetrahedron ? (prevents analyzing ray when not needed)

	vec3 t_next; // The delta-t in all axes for the DDA analysis
	ivec3 next_voxel; // The next voxel index in the grid
	ivec3 origin_voxel; // The ray's origin voxel for the grid

	// imitates the normals of a cube of size 1.
	vec3 normals[6];
	normals[0] = vec3( 1., 0., 0.); normals[1] = vec3( -1.,  0.,  0.);
	normals[2] = vec3( 0., 1., 0.); normals[3] = vec3(  0., -1.,  0.);
	normals[4] = vec3( 0., 0., 1.); normals[5] = vec3(  0.,  0., -1.);

	// Stepping increments to traverse the grid :
	ivec3 grid_step = ivec3 (-1, -1, -1);
	if( V.x > 0 ) grid_step.x = 1;
	if( V.y > 0 ) grid_step.y = 1;
	if( V.z > 0 ) grid_step.z = 1;

	// keeps track of the smallest coordinate of the 't' vector (scalar of V for raycasting)
	float t_min = 0;

	/**************initialization******************/

	vec3 Current_P = P.xyz;

	// Start the raytracing path right before the actual hit, caused interferences before
	Current_P = Current_P - .1*V;

	//Find the first intersection of the ray with the grid
	getFirstRayVoxelIntersection(Current_P, V, origin_voxel, t_next );


	//vec3 dt = vec3( abs(voxelSize.xyz/V.xyz) );
	vec3 dt = vec3( abs(voxelSize.x/V.x), abs(voxelSize.y/V.y), abs(voxelSize.z/V.z) );


	/***********************************************/

	vec3 Current_text3DCoord;

	vec4 Pos = vec4(0.,0.,0.,1.);

	vec4 color = vec4 (0.6,0.,0.6,1.);

	float v_step = voxelSize.x;
	if( v_step > voxelSize.y ) v_step = voxelSize.y;
	if( v_step > voxelSize.z ) v_step = voxelSize.z;

	next_voxel = origin_voxel;

	int maxFragIter = 200;
	int maxTetrIter =  50;

	vec3 n = vec3(0.,0.,0.);

	int fragmentIteration = 0;
	while( in_tet && !hit && fragmentIteration < maxFragIter ){
		fragmentIteration++;
		// step in the smallest direction : x, y, or z
		if( t_next.x < t_next.y && t_next.x < t_next.z ){
			Current_P = P.xyz + t_next.x*V;
			t_min = t_next.x;
			t_next.x = t_next.x + dt.x;
			next_voxel.x = next_voxel.x + grid_step.x;
			if( V.x > 0 )
				n = normals[1];
			else
				n = normals[0];
		} else if( t_next.y < t_next.x && t_next.y < t_next.z ){
			Current_P = P.xyz + t_next.y*V;
			t_min = t_next.y;
			t_next.y = t_next.y + dt.y;
			next_voxel.y = next_voxel.y + grid_step.y;
			if( V.y > 0 )
				n = normals[3];
			else
				n = normals[2];
		} else{
			Current_P = P.xyz + t_next.z*V;
			t_min = t_next.z;
			t_next.z = t_next.z + dt.z;
			next_voxel.z = next_voxel.z + grid_step.z;
			if( V.z > 0 )
				n = normals[5];
			else
				n = normals[4];
		}

		float ld0, ld1, ld2, ld3;
		Current_P = Current_P + 0.0001*v_step*V; // guess it's for not self-intersecting ?
		// If the barycentric coordinates are valid, then :
		if(computeBarycentricCoordinates(Current_P, ld0, ld1, ld2, ld3)) {
			int id_tet;
			vec3 voxel_center_P = getWorldCoordinates(next_voxel);
			if( ComputeVisibility(voxel_center_P.xyz) ) {
				// Traverse the texture, using barycentric coords to 'jump' to another tetrahedra if needed :
				if(computeBarycentricCoordinatesRecursive(voxel_center_P, ld0, ld1, ld2, ld3, int(instanceId+0.5), id_tet, maxTetrIter, Current_text3DCoord)){
					// Get this voxel's value :
					uvec3 voxelIndex = texture(texData, Current_text3DCoord).xyz;
					vec4 finalValue = fragmentEvaluationSingleChannel(voxelIndex);
					// If the color was valid (alpha is not null) :
					if (finalValue.w > 0.005f) {
						color = finalValue;
						// Current position will change :
						Pos = vec4(Current_P.xyz, 1.);
						hit = true;
					}

				}
			}

		} else {
			in_tet = false;
		}

	}

	if(!in_tet || !hit) discard;

	// Phong details :
	float phongAmbient = .75;	// How much of the original color to keep [0-1]
	float factor = (1. - phongAmbient) / 3.;
	mat3 lightDetails = mat3(
		vec3(1., 1., 1.),	// light diffuse color
		vec3(1., 1., 1.),	// light specular color
		vec3(.0, .0, .0)	// nothing
	);
	vec3 phongDetails = vec3(
		factor*.9,	// kd = diffuse coefficient
		factor*.1,	// ks = specular coefficient
		5.	// Shininess
	);
	colorOut.a = 1.;
	colorOut.xyz += phongAmbient * color.xyz;
	// Phong computation :
	colorOut.xyz += phongComputation(Pos, n, color, lightPositions[0], phongDetails, lightDetails);
	colorOut.xyz += phongComputation(Pos, n, color, lightPositions[4], phongDetails, lightDetails);
	// Phong for camera light :
	colorOut.xyz += phongComputation(Pos, n, color, cam, phongDetails, lightDetails);

	sceneSpaceFragmentPos = Pos;

	// Update fragment depth to prevent sorting issues !!!
	vec4 compute_depth = pMat * vMat * sceneSpaceFragmentPos;
    gl_FragDepth = 0.5f * ((compute_depth.z / compute_depth.w) + 1.f);

	return;
}

/****************************************/
/************** Functions ***************/
/****************************************/

bool ComputeVisibility(vec3 point)
{
	if (shouldUseBB == false) {
		vec4 point4 = vec4(point, 1.);
		vec4 cut4 = vec4(cut, 1.);
		vec4 vis4 = point4 - cut4;
		vis4.xyz *= cutDirection;
		float xVis = vis4.x; // (point.x - cut.x)*cutDirection.x;
		float yVis = vis4.y; // (point.y - cut.y)*cutDirection.y;
		float zVis = vis4.z; // (point.z - cut.z)*cutDirection.z;

		vec4 clippingPoint = vec4(cam, 1.);
		vec4 clippingNormal = normalize(inverse(vMat) * vec4(.0, .0, -1., .0));
		clippingPoint += clippingNormal * clipDistanceFromCamera ;
		vec4 pos = point4 - clippingPoint;
		float vis = dot( clippingNormal, pos );

		if( xVis < 0.|| yVis < 0.|| zVis < 0. || vis < .0)
			return false;
		else return true;
	} else {
		if (point.x < visuBBMin.x) { return false; }
		if (point.y < visuBBMin.y) { return false; }
		if (point.z < visuBBMin.z) { return false; }
		if (point.x > visuBBMax.x) { return false; }
		if (point.y > visuBBMax.y) { return false; }
		if (point.z > visuBBMax.z) { return false; }
		return true;
	}
}

vec3 getWorldCoordinates( in ivec3 _gridCoord )
{
	vec4 p = vec4( (_gridCoord.x+0.5)*voxelSize.x,
			(_gridCoord.y+0.5)*voxelSize.y,
			(_gridCoord.z+0.5)*voxelSize.z, 1. );
	return p.xyz;
}

ivec3 getGridCoordinates( in vec4 _P )
{
	vec4 pw = _P;
	return ivec3( int( pw.x/voxelSize.x ) ,
                  int( pw.y/voxelSize.y ) ,
                  int( pw.z/voxelSize.z ) );
}

ivec2 Convert1DIndexTo2DIndex_Unnormed( in uint uiIndexToConvert, in int iWrapSize )
{
	int iY = int( uiIndexToConvert / uint( iWrapSize ) );
	int iX = int( uiIndexToConvert - ( uint( iY ) * uint( iWrapSize ) ) );
	return ivec2( iX, iY );
}

ivec2 Convert1DIndexTo2DIndex_Unnormed_Flipped( in uint uiIndexToConvert, in int iWrapSize )
{
	int iY = int( uiIndexToConvert / uint( iWrapSize ) );
	int iX = int( uiIndexToConvert - ( uint( iY ) * uint( iWrapSize ) ) );
	return ivec2( iY, iX );
}

bool computeBarycentricCoordinates( in vec3 point, out float ld0 , out float ld1 , out float ld2 , out float ld3)
{
	// normal texture width :
	int nWidth = textureSize(normals_translations, 0).x;

	ivec2 textF = Convert1DIndexTo2DIndex_Unnormed(uint(int(instanceId+0.5)*4 ), nWidth);
	vec4 texelVal = texelFetch(normals_translations, textF, 0);
	vec3 Normal_F0 = texelVal.xyz;
	float factor_0 = texelVal.w;


	textF = Convert1DIndexTo2DIndex_Unnormed(uint(int(instanceId+0.5)*4 + 1), nWidth);
	texelVal = texelFetch(normals_translations, textF, 0);
	vec3 Normal_F1 = texelVal.xyz;
	float factor_1 = texelVal.w;

	textF = Convert1DIndexTo2DIndex_Unnormed(uint(int(instanceId+0.5)*4 + 2 ), nWidth);
	texelVal = texelFetch(normals_translations, textF, 0);
	vec3 Normal_F2 = texelVal.xyz;
	float factor_2 = texelVal.w;

	textF = Convert1DIndexTo2DIndex_Unnormed(uint(int(instanceId+0.5)*4 + 3), nWidth);
	texelVal = texelFetch(normals_translations, textF, 0);
	vec3 Normal_F3 = texelVal.xyz;
	float factor_3 = texelVal.w;


	float val0 = dot( point - P1.xyz, Normal_F0 );
	float val1 = dot( point - P2.xyz, Normal_F1 );
	float val2 = dot( point - P3.xyz, Normal_F2 );
	float val3 = dot( point - P0.xyz, Normal_F3 );


	ld0 = val0*factor_0;
	ld1 = val1*factor_1;
	ld2 = val2*factor_2;
	ld3 = val3*factor_3;

	if(ld0 < 0. || ld0 > 1. || ld1 < 0. || ld1 > 1. || ld2 < 0. || ld2 >1. || ld3 < 0. || ld3 > 1. ){
		return false;
	}

	return true;
}

vec3 crossProduct( vec3 a, vec3 b )
{
	return vec3( a.y*b.z - a.z*b.y, a.z*b.x - a.x*b.z, a.x*b.y - a.y*b.x );
}

bool computeBarycentricCoordinates(in vec3 point, out float ld0 , out float ld1 , out float ld2 , out float ld3,
				   in int id_tetra_start, out int id_tetra_end, out vec3 Current_text3DCoord)
{

	int vWidth = textureSize(vertices_translations, 0).x;
	int nWidth = textureSize(normals_translations, 0).x;
	int neighborWidth = textureSize(neighbors, 0).x;

	ivec2 textCoord3 = Convert1DIndexTo2DIndex_Unnormed(uint(id_tetra_start*12 ), vWidth);
	vec3 N_P3 = texelFetch(vertices_translations, textCoord3, 0).xyz;

	ivec2 textCoord1 = Convert1DIndexTo2DIndex_Unnormed(uint(id_tetra_start*12 + 1 ), vWidth);
	vec3 N_P1 = texelFetch(vertices_translations, textCoord1, 0).xyz;

	ivec2 textCoord2 = Convert1DIndexTo2DIndex_Unnormed(uint(id_tetra_start*12 + 2 ), vWidth);
	vec3 N_P2 = texelFetch(vertices_translations, textCoord2, 0).xyz;

	ivec2 textCoord0 = Convert1DIndexTo2DIndex_Unnormed(uint(id_tetra_start*12 + 5 ), vWidth);
	vec3 N_P0 = texelFetch(vertices_translations, textCoord0, 0).xyz;


	ivec2 textF = Convert1DIndexTo2DIndex_Unnormed(uint(id_tetra_start*4 ), nWidth);
	vec4 texelVal = texelFetch(normals_translations, textF, 0);
	vec3 Normal_F0 = texelVal.xyz;
	float factor_0 = texelVal.w;

	textF = Convert1DIndexTo2DIndex_Unnormed(uint(id_tetra_start*4 + 1), nWidth);
	texelVal = texelFetch(normals_translations, textF, 0);
	vec3 Normal_F1 = texelVal.xyz;
	float factor_1 = texelVal.w;

	textF = Convert1DIndexTo2DIndex_Unnormed(uint(id_tetra_start*4 + 2 ), nWidth);
	texelVal = texelFetch(normals_translations, textF, 0);
	vec3 Normal_F2 = texelVal.xyz;
	float factor_2 = texelVal.w;

	textF = Convert1DIndexTo2DIndex_Unnormed(uint(id_tetra_start*4 + 3), nWidth);
	texelVal = texelFetch(normals_translations, textF, 0);
	vec3 Normal_F3 = texelVal.xyz;
	float factor_3 = texelVal.w;

	float val0 = dot( point - N_P1.xyz, Normal_F0 );
	float val1 = dot( point - N_P2.xyz, Normal_F1 );
	float val2 = dot( point - N_P3.xyz, Normal_F2 );
	float val3 = dot( point - N_P0.xyz, Normal_F3 );

	// compute the actual barycentric coords :
	ld0 = val0*factor_0;
	ld1 = val1*factor_1;
	ld2 = val2*factor_2;
	ld3 = val3*factor_3;

	// if we're out of the barycentric coordinates for this tetrahedron :
	if(ld0 < 0. || ld0 > 1. || ld1 < 0. || ld1 > 1. || ld2 < 0. || ld2 >1. || ld3 < 0. || ld3 > 1. ){
		int texture_id_next_tetra = id_tetra_start*4;

		if( val1 >= val0 && val1 >= val2 && val1 >= val3 ){
			texture_id_next_tetra = texture_id_next_tetra + 1;
		} else if( val2 >= val0 && val2 >= val1 && val2 >= val3 ){
			texture_id_next_tetra = texture_id_next_tetra + 2;
		} else if( val3 >= val0 && val3 >= val1 && val3 >= val2 ){
			texture_id_next_tetra = texture_id_next_tetra + 3;
		}

		/*
		if( ld1 <= ld0 && ld1 <= ld2 && ld1 <= ld3 ){
			  texture_id_next_tetra = texture_id_next_tetra + 1;
		} else if( ld2 <= ld0 && ld2 <= ld1 && ld2 <= ld3 ){
			  texture_id_next_tetra = texture_id_next_tetra + 2;
		} else if( ld3 <= ld0 && ld3 <= ld1 && ld3 <= ld2 ){
			  texture_id_next_tetra = texture_id_next_tetra + 3;
		}
		*/

		ivec2 textCoordNeighbor = Convert1DIndexTo2DIndex_Unnormed(uint(texture_id_next_tetra), neighborWidth);
		vec4 texV = vec4( texelFetch(neighbors, textCoordNeighbor, 0).xyz, 1. );

		if( texV.x < 0 )
			id_tetra_end = -1;
		else
			id_tetra_end = int ( texV.x );

		return false;
	}


	vec3 text3DCoordNP0 = texelFetch(texture_coordinates, textCoord0, 0).xyz;
	vec3 text3DCoordNP1 = texelFetch(texture_coordinates, textCoord1, 0).xyz;
	vec3 text3DCoordNP2 = texelFetch(texture_coordinates, textCoord2, 0).xyz;
	vec3 text3DCoordNP3 = texelFetch(texture_coordinates, textCoord3, 0).xyz;

	Current_text3DCoord = ld0*text3DCoordNP0 + ld1*text3DCoordNP1 + ld2*text3DCoordNP2 + ld3*text3DCoordNP3;

	return true;
}

bool computeBarycentricCoordinatesRecursive(in vec3 point, out float ld0 , out float ld1 , out float ld2 , out float ld3,
					in int id_tetra_start, out int id_tetra_end, int max_iter, out vec3 result_text_coord )
{
	int current_iter = 0;
	int current_tet_visited = id_tetra_start;
	int next_tet_to_visit;
	while(  current_iter  <  max_iter  ){
		++current_iter;
		bool foundTet = computeBarycentricCoordinates( point, ld0 , ld1 , ld2 , ld3,
							current_tet_visited, next_tet_to_visit, result_text_coord );

		current_tet_visited = next_tet_to_visit;
		if( foundTet ){
			return true;
		}

		if( next_tet_to_visit < 0 )
			return false;
	}
	return false;
}


void getFirstRayVoxelIntersection( in vec3 ray_origin, in vec3 ray_direction, out ivec3 v0, out vec3 t_n)
{
    vec4 pw = vec4(ray_origin.xyz, 1.);
    v0 = ivec3(floor(pw.x/voxelSize.x), floor(pw.y/voxelSize.y), floor(pw.z/voxelSize.z));
    //v0 = ivec3(int(pw.x/voxelSize.x), int(pw.y/voxelSize.y), int(pw.z/voxelSize.z));

    float xi = v0.x*voxelSize.x;
    if( ray_direction.x > 0 ) xi += voxelSize.x;
    float yi = v0.y*voxelSize.y;
    if( ray_direction.y > 0 ) yi += voxelSize.y;
    float zi = v0.z*voxelSize.z;
    if( ray_direction.z > 0 ) zi += voxelSize.z;
    t_n = vec3 ( ((xi - ray_origin.x)/ray_direction.x), ((yi - ray_origin.y)/ray_direction.y), ((zi - ray_origin.z)/ray_direction.z) );

    if( abs( ray_direction.x ) < 0.00001 ) t_n.x = 100000000;
    if( abs( ray_direction.y ) < 0.00001 ) t_n.y = 100000000;
    if( abs( ray_direction.z ) < 0.00001 ) t_n.z = 100000000;
}

vec3 phongComputation(vec4 position, vec3 normal, vec4 color, vec3 lightPos, vec3 phongDetails, mat3 lightDetails) {
	vec3 lightDiffuse = lightDetails[0];
	vec3 lightSpecular = lightDetails[1];

	float phong_Diffuse = phongDetails.x;
	float phong_Specular = phongDetails.y;
	float phong_Shininess = phongDetails.z;

	vec3 lm = normalize(lightPos - position.xyz);
	vec3 n = normalize(normal);
	vec3 r = 2.f * max(.0, dot(lm, n)) * n - lm;
	vec3 v = normalize(cam - position.xyz);

	float dotlmn = dot(lm, n);

	return phong_Diffuse * max(.0, dotlmn) * lightDiffuse + phong_Specular * pow(max(.0, dot(r, v)), phong_Shininess) * lightSpecular;
}

bool checkAndColorizeVoxel(in uvec3 voxel, out vec4 return_color) {
	// data to show :
	float voxVal;
	vec2 cB, tB;
	return true;
}
