/*
		This shader is used for rendering the input/coarse-mesh

		Matthias Holländer 2008/2009
*/

#version 130
const float c_E                     = 2.718281828459045235360287471352662497757247093699959574966967627724076630353547594571382178525166427; ///< Euler's number
const float c_Pi                    = 3.1415926535897932384626433832795028841971693993751;
const float c_2_Pi                  = 6.233185307179586476925286766559005768394338798750211641949889184615632812572417997256069650684234135;
const float c_Pi_Over_180           = 0.017453292519943295769236907684886;
const float c_Pi_Over_2             = 1.570796326794896619231321691639751442098584699687552910487472296153908203143104499314017412671058533;
const float c_Pi_Over_4             = 0.7853981633974483096156608458198757210492923498437764552437361480769541015715522496570087063355292669;
const float c_Pi_Over_360           = 0.00872664625997164788461845384244;
const float c_180_Over_Pi           = 57.295779513082320876798154814105;
const float c_Log_E                 = 0.4342944819032518276511289189166050822943970058036665661144537831658646492088707747292249493384317483;
const float c_Ln_10                 = 2.302585092994045684017991454684364207601101488628772976033327900967572609677352480235997205089598298;
const float c_Sqrt_2                = 1.4142135623730950488016887242096980785696718753769;
const float c_Sqrt_3                = 1.7320508075688772935274463415058723669428052538104;
const float c_Log_E_2               = 0.693147180559945309417232121458176568075500134360255;                                                 ///< log of 2 to basis e
const float c_Golden                = 1.618034;
const float c_Tiny                  = 1.e-8;            ///< Smallest float possible
const float c_Small                 = 1.e-6;            ///< Small enough float to use as delta value

/**
 * @brief For calculating the angle between two vectors
 * @param v1 NORMALIZED vector1
 * @param v2 NORMALIZED vector2
 * @return Angle in radians (!!!)
 */
float GetDiffAngle( in vec3 v1, in vec3 v2 )
{
	return acos( dot( v1, v2 ) );
}

/**
 * @brief For squared value
 * @param fX Value to square
 */
float Sqr( in float fX )
{
	return fX * fX;
}

// --------------------------------------------------
// shader definition
// --------------------------------------------------

uniform bool solid;

in vec3 vLightDir;
in vec3 my_position;
in vec3 normal;

in float visibility;
in vec3 nb_neighbor;

uniform uint visiblity_map[256];
uniform vec4 color_map[256];

/**
 * @brief Calculates the light attenuation
 */
float CalcAttenuation( in gl_LightSourceParameters vLightDef,
		       in vec3 vVertex,
		       in vec3 vLightVec )
{
	float fDistance = length( vVertex - vLightVec );
	return ( 1.0 / ( vLightDef.constantAttenuation +
					 vLightDef.linearAttenuation * fDistance +
					 vLightDef.quadraticAttenuation * Sqr( fDistance ) ) );
}

/**
 * @brief Calculates the ambient term
 */
vec4 CalcAmbient( in gl_LightModelParameters vLightDef,
		  in gl_MaterialParameters vMatDef )
{
	return vLightDef.ambient * vMatDef.ambient;
}

/**
 * @brief Calculates the diffuse term
 */
vec4 CalcDiffuse( in int iLightIdx,
		  in vec3 vLightVec,
		  in vec3 vNormal )
{
	float fLambert = max( dot( vLightVec, vNormal ), 0.0 );
	//return fLambert * ( vLightDef.diffuse * vMatDef.diffuse );
	return fLambert * ( gl_LightSource[ iLightIdx ].diffuse * gl_FrontMaterial.diffuse );

}

/**
 * @brief Calculates the Phong specular term
 */
vec4 CalcSpecularPhong( in gl_LightSourceParameters vLightDef,
			in gl_MaterialParameters vMatDef,
			in vec3 vLightVec,
			in vec3 vNormal,
			in vec3 vVertex )
{
	vec3 vReflected = reflect( vLightVec, vNormal );
	float fSpecularIntensity = pow( max( dot( vReflected, vVertex ), 0.0 ), vMatDef.shininess );
	return fSpecularIntensity * ( vLightDef.specular * vMatDef.specular );
}

/**
 * @brief Calculates the Blinn specular term
 */
vec4 CalcSpecularBlinn( in gl_LightSourceParameters vLightDef,
			in gl_MaterialParameters vMatDef,
			in vec3 vLightVec,
			in vec3 vNormal,
			in vec3 vVertex )
{
	vec3 vHalf = normalize( ( vLightVec + -vVertex ) * 0.5 );
	float fSpec = pow( max( dot( vNormal, vHalf ), 0.0 ), vMatDef.shininess );
	return fSpec * ( vLightDef.specular * vMatDef.specular );
}

/**
 * @brief Calculates the Gaussian specular term
 */
vec4 CalcSpecularGaussian( in gl_LightSourceParameters vLightDef,
			   in gl_MaterialParameters vMatDef,
			   in vec3 vLightVec,
			   in vec3 vNormal,
			   in vec3 vVertex )
{
	vec3 vHalf = normalize( ( vLightVec + -vVertex ) * 0.5 );
	//float fFraction = (GetDiffAngle(vNormal,vHalf)) / fSurfaceSmoothness;
	float fFraction = ( GetDiffAngle( vNormal, vHalf ) ) / ( vMatDef.shininess * 0.001f );
	float fSpec = max( pow( c_E, - Sqr( fFraction ) ), 0.0 );
	return fSpec * ( vLightDef.specular * vMatDef.specular );
}

/**
 * @brief Calculates the Beckmann specular term
 */
vec4 CalcSpecularBeckmann( in gl_LightSourceParameters vLightDef,
			   in gl_MaterialParameters vMatDef,
			   in vec3 vLightVec,
			   in vec3 vNormal,
			   in vec3 vVertex )
{
	vec3 vHalf = normalize( ( vLightVec + -vVertex ) *  0.5 );
	float fCosTerm = dot( vNormal, vHalf );
	float fSurfaceSmoothness = ( vMatDef.shininess * 0.001f );
	float fDivisor = 4.0 * Sqr( fSurfaceSmoothness ) * pow( fCosTerm, 4.0 );
	float fTan = tan( GetDiffAngle( vNormal, vHalf ) );
	float fExponent = Sqr( fTan / fSurfaceSmoothness );
	float fSpec = max( ( ( pow( c_E, -fExponent ) ) / fDivisor ), 0.0 );
	return fSpec * ( vLightDef.specular * vMatDef.specular );
}

/**
 * @brief Calculates the CookTorrance specular term
 */
vec4 CalcSpecularCookTorrance( in gl_LightSourceParameters vLightDef,
			       in gl_MaterialParameters vMatDef,
							   in vec3 vLightVec,
							   in vec3 vNormal,
							   in vec3 vVertex )
{
	// Lambda for the Fresnel-Term
	float fLambda = 0.8;		// Kind of the offset for the size

	vec3 E = -vVertex;
	float fEDotN = dot( E, vNormal );
	vec3 vHalf = normalize( vLightVec - vVertex );
	float fNDotH = dot( vNormal, vHalf );
	float fSurfaceSmoothness = ( vMatDef.shininess * 0.0005 );

	// calculate D:
	float fDivisor = 4.0 * Sqr( fSurfaceSmoothness ) * Sqr( Sqr( fNDotH ) );
	float fExponent = tan( GetDiffAngle( vNormal, vHalf ) ) / fSurfaceSmoothness;
	fExponent *= fExponent;
	float fD = ( pow( c_E, -fExponent ) ) / fDivisor;

	// calculate F (using the Schlick-term), several differnt ones:
	float fF = fLambda + ( ( 1.0 - fLambda ) * max( pow( 1.0 - dot( E, vHalf ), 5.0 ), 0.0 ) );

	//fF = pow((1.0 + fEDotN), fLambda);
	//fF = fLambda;							// Simplest

	// calculate G:
	float fEDotH_Rec = 1.0 / dot( E, vHalf );
	float fG1 = 2.0 * fNDotH * fEDotN * fEDotH_Rec;
	float fG2 = 2.0 * fNDotH * ( max( dot( vLightVec, vNormal ), 0.0 ) ) * fEDotH_Rec;
	float fG = min( max( min( fG1, fG2 ), 0.0 ), 1.0);

	// all in all:
	float fSpec = ( fD * fF * fG ) / ( fEDotN );
	fSpec = max( fSpec, 0.f );
	return fSpec * ( vLightDef.specular * vMatDef.specular );
}

/**
 * @brief Calculates the Schlick specular term
 */
vec4 CalcSpecularSchlick( in gl_LightSourceParameters vLightDef,
			  in gl_MaterialParameters vMatDef,
			  in vec3 vLightVec,
			  in vec3 vNormal,
			  in vec3 vVertex )
{
	vec3 vRefl = reflect( vLightVec, vNormal );
	float fT = max( dot( vRefl, vVertex ), 0.0 );
	float fSpec = max( fT / ( vMatDef.shininess - vMatDef.shininess * fT + fT ), 0.0 );
	return fSpec * ( vLightDef.specular * vMatDef.specular );
}


void main (void) {


    if(visibility > 0.) discard;

    vec4 color = vec4 (0.0, 0.0, 0.0, 1.);
    if(solid) {
	color = gl_FrontMaterial.diffuse;
    } else {

	vec4 vSpecular  = vec4(0.0, 0.0, 0.0, 0.0);

	vec3 N_Normed = normalize(normal); // Normal

	// Calculate Attenuation before the LightVec is normalized
	float fAttenuation = CalcAttenuation(gl_LightSource[0], my_position, vLightDir);

	vec3 vLightDir_Normed = normalize(vLightDir);
	vec3 V_Normed = normalize(my_position);

	vec4 vDiffuse = CalcDiffuse(0, /*gl_FrontMaterial,*/	vLightDir_Normed, N_Normed);
	vec4 vAmbient = CalcAmbient(gl_LightModel, gl_FrontMaterial);

	if (gl_FrontMaterial.shininess <= 0.00001) {
		vSpecular = vec4(0.0, 0.0, 0.0, 1.0);
	}
	else {
		//vSpecular = CalcSpecular( gl_LightSource[0], gl_FrontMaterial, vLightDir_Normed, N_Normed, V_Normed );
		//vSpecular = CalcSpecularCookTorrance( gl_LightSource[0], gl_FrontMaterial, vLightDir_Normed, N_Normed, V_Normed );

		//vSpecular = CalcSpecularBlinn( gl_LightSource[0], gl_FrontMaterial, vLightDir_Normed, N_Normed, V_Normed );
		//vSpecular = CalcSpecularWarren( gl_LightSource[0], gl_FrontMaterial, vLightDir_Normed, N_Normed, V_Normed );
		vSpecular = CalcSpecularSchlick( gl_LightSource[0], gl_FrontMaterial, vLightDir_Normed, N_Normed, V_Normed );

	}
	color = (fAttenuation * (vAmbient + vDiffuse + vSpecular));

    }
    gl_FragColor = color;

}
