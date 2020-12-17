#version 130

// --------------------------------------------------
// shader definition
// --------------------------------------------------

// --------------------------------------------------
// varying variables
// --------------------------------------------------
in float instanceId;
//varying vec3 vLightDir;
//varying vec3 position;
//varying vec3 normal;

in float visibility;

in vec3 text3DCoord;

in vec4 P;
//varying vec3 N;

in vec4 P0;
in vec4 P1;
in vec4 P2;
in vec4 P3;

in vec3 text3DCoordP0;
in vec3 text3DCoordP1;
in vec3 text3DCoordP2;
in vec3 text3DCoordP3;

out vec4 colorOut;

// --------------------------------------------------
// uniform variables
// --------------------------------------------------
uniform bool solid;

uniform usampler3D Mask; // d?claration de la map mask
uniform sampler2D vertices_translations;
uniform sampler2D normals_translations;
uniform sampler2D texture_coordinates;
uniform sampler2D visibility_texture;

//uniform sampler2D neighbors_nb;
uniform sampler2D neighbors;

uniform float gridStep;

uniform float diffuseRef;
uniform float specRef;
uniform float shininess;

uniform uint visiblity_map[256];
uniform sampler1D color_texture;
uniform uint colorTexWidth;

uniform int width;
uniform int neighbor_width;
uniform int normal_width;
uniform int visibility_width;

uniform vec3 cam;

uniform float dx;
uniform float dy;
uniform float dz;

uniform vec3 cut;
uniform vec3 cutDirection;

uniform vec3 clippingPoint;
uniform vec3 clippingNormal;

bool ComputeVisibility(vec3 point){

	float xVis = (point.x - cut.x)*cutDirection.x;
	float yVis = (point.y - cut.y)*cutDirection.y;
	float zVis = (point.z - cut.z)*cutDirection.z;

	vec3 pos = point - clippingPoint;
	float vis = dot( clippingNormal, pos );
	if( xVis < 0. || yVis < 0. || zVis < 0. || vis < 0. )
		return false;
	else return true;
}

vec3 getWorldCoordinates( in ivec3 _gridCoord ){
	return vec3( (_gridCoord.x+0.5)*dx,
				 (_gridCoord.y+0.5)*dy,
				 (_gridCoord.z+0.5)*dz );
}

ivec3 getGridCoordinates( in vec4 _P ){
	return ivec3( int( _P.x/dx ) ,
				  int( _P.y/dy ) ,
				  int( _P.z/dz ) );
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

bool computeBarycentricCoordinates( in vec3 point, out float ld0 , out float ld1 , out float ld2 , out float ld3){


        ivec2 textF = Convert1DIndexTo2DIndex_Unnormed(uint(int(instanceId+0.5)*4 ), normal_width);
	vec4 texelVal = texelFetch(normals_translations, textF, 0);
	vec3 Normal_F0 = texelVal.xyz;
	float factor_0 = texelVal.w;


        textF = Convert1DIndexTo2DIndex_Unnormed(uint(int(instanceId+0.5)*4 + 1), normal_width);
	texelVal = texelFetch(normals_translations, textF, 0);
	vec3 Normal_F1 = texelVal.xyz;
	float factor_1 = texelVal.w;

        textF = Convert1DIndexTo2DIndex_Unnormed(uint(int(instanceId+0.5)*4 + 2 ), normal_width);
	texelVal = texelFetch(normals_translations, textF, 0);
	vec3 Normal_F2 = texelVal.xyz;
	float factor_2 = texelVal.w;

        textF = Convert1DIndexTo2DIndex_Unnormed(uint(int(instanceId+0.5)*4 + 3), normal_width);
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


vec3 crossProduct( vec3 a, vec3 b ){

	return vec3( a.y*b.z - a.z*b.y, a.z*b.x - a.x*b.z, a.x*b.y - a.y*b.x );
}

bool computeBarycentricCoordinates( in vec3 point, out float ld0 , out float ld1 , out float ld2 , out float ld3,
									in int id_tetra_start, out int id_tetra_end, out vec3 Current_text3DCoord){



        ivec2 textCoord3 = Convert1DIndexTo2DIndex_Unnormed(uint(id_tetra_start*12 ), width);
	vec3 N_P3 = texelFetch(vertices_translations, textCoord3, 0).xyz;

        ivec2 textCoord1 = Convert1DIndexTo2DIndex_Unnormed(uint(id_tetra_start*12 + 1 ), width);
	vec3 N_P1 = texelFetch(vertices_translations, textCoord1, 0).xyz;

        ivec2 textCoord2 = Convert1DIndexTo2DIndex_Unnormed(uint(id_tetra_start*12 + 2 ), width);
	vec3 N_P2 = texelFetch(vertices_translations, textCoord2, 0).xyz;

        ivec2 textCoord0 = Convert1DIndexTo2DIndex_Unnormed(uint(id_tetra_start*12 + 5 ), width);
	vec3 N_P0 = texelFetch(vertices_translations, textCoord0, 0).xyz;


        ivec2 textF = Convert1DIndexTo2DIndex_Unnormed(uint(id_tetra_start*4 ), normal_width);
	vec4 texelVal = texelFetch(normals_translations, textF, 0);
	vec3 Normal_F0 = texelVal.xyz;
	float factor_0 = texelVal.w;

        textF = Convert1DIndexTo2DIndex_Unnormed(uint(id_tetra_start*4 + 1), normal_width);
	texelVal = texelFetch(normals_translations, textF, 0);
	vec3 Normal_F1 = texelVal.xyz;
	float factor_1 = texelVal.w;

        textF = Convert1DIndexTo2DIndex_Unnormed(uint(id_tetra_start*4 + 2 ), normal_width);
	texelVal = texelFetch(normals_translations, textF, 0);
	vec3 Normal_F2 = texelVal.xyz;
	float factor_2 = texelVal.w;

        textF = Convert1DIndexTo2DIndex_Unnormed(uint(id_tetra_start*4 + 3), normal_width);
	texelVal = texelFetch(normals_translations, textF, 0);
	vec3 Normal_F3 = texelVal.xyz;
	float factor_3 = texelVal.w;

	float val0 = dot( point - N_P1.xyz, Normal_F0 );
	float val1 = dot( point - N_P2.xyz, Normal_F1 );
	float val2 = dot( point - N_P3.xyz, Normal_F2 );
	float val3 = dot( point - N_P0.xyz, Normal_F3 );


	ld0 = val0*factor_0;
	ld1 = val1*factor_1;
	ld2 = val2*factor_2;
	ld3 = val3*factor_3;


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

                ivec2 textCoordNeighbor = Convert1DIndexTo2DIndex_Unnormed(uint(texture_id_next_tetra), neighbor_width);
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


bool computeBarycentricCoordinatesRecursive( in vec3 point, out float ld0 , out float ld1 , out float ld2 , out float ld3,
					in int id_tetra_start, out int id_tetra_end, int max_iter, out vec3 result_text_coord ){
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


void getFirstRayVoxelIntersection( in vec3 origin, in vec3 direction, out ivec3 v0, out vec3 t_n){

	// 	vec3 origin = o;
	//
	// 	vec3 t_cut = vec3( (cut.x - o.x)/direction.x, (cut.y - o.y)/direction.y, (cut.z - o.z)/direction.z );
	//
	// 	float lambda_max = t_cut.x;
	//
	// 	if( t_cut.y > lambda_max ) lambda_max = t_cut.y;
	// 	if( t_cut.z > lambda_max ) lambda_max = t_cut.z;
	//
	// 	if( lambda_max > 0 ) origin = origin + lambda_max*direction;

	v0 = getGridCoordinates(vec4(origin.xyz, 1.));

	float xi = v0.x*dx;
	if( direction.x > 0  ) xi = xi + dx;
	float yi = v0.y*dy;
	if( direction.y > 0  ) yi = yi + dy;
	float zi = v0.z*dz;
	if( direction.z > 0  ) zi = zi + dz;
	t_n = vec3 ( ((xi - origin.x)/direction.x), ((yi - origin.y)/direction.y), ((zi - origin.z)/direction.z) );

	if( abs( direction.x ) < 0.00001 ) t_n.x = 100000000;
	if( abs( direction.y ) < 0.00001 ) t_n.y = 100000000;
	if( abs( direction.z ) < 0.00001 ) t_n.z = 100000000;

}

void main (void) {

	if( visibility > 3500. ) discard;


	/*
	if (P.x < .0f) {discard;}
	if (P.y < .0f) {discard;}
	if (P.z < .0f) {discard;}
	*/
	colorOut = P/2048.;
	return;

	vec3 V = normalize ( P.xyz - cam );

	bool in_tet = true;
	bool hit = false;


	vec3 t_next;
	ivec3 next_voxel;
	ivec3 origin_voxel;

	/**************initialization******************/

	vec3 Current_P = P.xyz;

	//Find the first intersection of the ray with the grid
	getFirstRayVoxelIntersection(Current_P, V, origin_voxel, t_next );

	vec3 dt = vec3( abs(dx/V.x), abs(dy/V.y), abs(dz/V.z) );

	ivec3 grid_step = ivec3 (-1, -1, -1);


	if( V.x > 0 ) grid_step.x = 1;
	if( V.y > 0 ) grid_step.y = 1;
	if( V.z > 0 ) grid_step.z = 1;

	float t_min = 0;

	/***********************************************/

	vec3 Current_text3DCoord;// = text3DCoord;

	vec4 Pos;

	vec4 color = vec4 (0.,0.,0.,1.);

	float v_step = dx;
	if( v_step > dy ) v_step = dy;
	if( v_step > dz ) v_step = dz;

	bool changed = false;
	//  Current_P = P.xyz;
	next_voxel = origin_voxel;


	vec3 normals[6];
	normals[0] = vec3( 1., 0., 0.); normals[1] = vec3( -1.,  0.,  0.);
	normals[2] = vec3( 0., 1., 0.); normals[3] = vec3(  0., -1.,  0.);
	normals[4] = vec3( 0., 0., 1.); normals[5] = vec3(  0.,  0., -1.);

	vec3 n;

	float val =0;
	int fragmentIteration = 0;
	while( in_tet && !hit && fragmentIteration < 10 ){
		fragmentIteration++;
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
		Current_P = Current_P + 0.0001*v_step*V;
		if( computeBarycentricCoordinates( Current_P, ld0, ld1, ld2, ld3) ){

			vec3 voxel_center_P = getWorldCoordinates( next_voxel );
			int id_tet;
			//if( computeBarycentricCoordinates( voxel_center_P, ld0, ld1, ld2, ld3) ){
			if( computeBarycentricCoordinatesRecursive( voxel_center_P, ld0, ld1, ld2, ld3, int(instanceId+0.5), id_tet, 50, Current_text3DCoord ) ){
				// Current_text3DCoord = ld0*text3DCoordP0 + ld1*text3DCoordP1 + ld2*text3DCoordP2 + ld3*text3DCoordP3;
				uint voxelIndex = texture(Mask, (Current_text3DCoord.xyz)).x;
				vec4 current_color = texelFetch(color_texture, int(voxelIndex), 0);
				if (visiblity_map[voxelIndex] > 0u) {
					if( voxelIndex > 0u ){
						ivec2 textF = Convert1DIndexTo2DIndex_Unnormed(voxelIndex, visibility_width);
						//ivec2 textF = ivec2(int(round(current_color.a*255.)), 0);
						vec3 current_visibility = texelFetch(visibility_texture, textF, 0).xyz;
					//	if(current_visibility.x>0.){
							//  val = 1.;
							//current_color = vec4(texture(Mask, Current_text3DCoord.xyz).r, 0., 0., 1.0);
							color = current_color;
							//color = vec4(1.,0.,0.,1.);
							Pos = vec4(Current_P.xyz, 1.);//vec4( (ld0*P0 + ld1*P1 + ld2*P2 + ld3*P3).xyz, 1. );
							if(ComputeVisibility(voxel_center_P.xyz) )
								hit = true;
					//	}
					}
				}

			}

		} else {
			in_tet = false;
		}


	}




	if(!in_tet || !hit) discard;
	colorOut = color;
	return;

	vec3 p = vec3 (Pos);
	vec3 v = normalize(cam-p);

	/*
	gl_FragColor = vec4(0.,0.,0.,1.);

        for (int i = 0; i < 5; i++) {

            vec3 l = normalize (gl_LightSource[i].position.xyz - p);
            l.z = l.z*-1.;

                l.y = l.y*-1.;
            //vec3 l = normalize (cam +vec3(10., 2.,0.) - p);

            if( i == 4 )
                l = normalize (cam - p);
            float ndotl = dot (l, n);
            float diffuse   = max (ndotl, 0.0);
            vec3 r = 2. * ndotl * n - l;;

            float spec = max(dot(r, v), 0.0);
            spec = pow (spec, shininess);
            spec = max (0.0, spec);

            vec4 LightContribution =
                    diffuseRef * diffuse * color + specRef * spec * gl_LightSource[i].specular*0.01;
            float factor = 1.;
            if( i == 4 || i ==3 )
                 factor = 0.5;
            gl_FragColor += factor*vec4 (LightContribution.xyz, 1);
        }
	*/

}
