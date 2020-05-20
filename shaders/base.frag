#version 400 core

in vec4 vPos_WS;
in vec4 vNorm_WS;
in vec3 texCoord;

in vec4 vPos_CS;
in vec4 vNorm_CS;
in vec4 lightDir_CS;
in vec4 eyeDir_CS;

out vec4 color;

uniform vec4 lightPos;
uniform usampler3D texData;

/// Takes a uvec3 of an R8UI-based texture and spits out an RGB color by converting
// from R(uchar)G(void)B(void) to HSV first, then to RGB
vec4 R8UIToRGB(in uvec3 ucolor) {
	float a = 0.f / 255.f;
	float b = 255.f / 255.f;
	float c = 50.f / 255.f;
	float d = 200.f / 255.f;
	// Get the red component in floating point :
	float r = 1.f - ((b - a) / (d - c)) * ((float(ucolor.r)/255.f)-c)+a;
	// Convert to HSV space (from glsl-hsv2rgb on github) :
	vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
	vec3 p = abs(fract(vec3(r,r,r) + K.xyz) * 6.0 - K.www);
	vec3 rgb = mix(K.xxx, clamp(p - K.xxx, .1, .7), r); // change min/max vals of clamp() to change saturation
	return vec4(rgb.r, rgb.g, rgb.b, 1.0);
}

void main(void)
{
	vec4 lightColor = vec4(255./255., 214./255., 170./255.,1.0);
	float lightPower = 1.0;
	// colors :
	vec4 matDifColor = vec4(0.8,0.8,0.8,1.0);
	vec4 matAmbColor =vec4(0.27,0.27,0.27,1.0) * matDifColor; // mid grey and diffuse
	vec4 matSpeColor = vec4(255./255., 214./255., 170./255.,1.0);

	// distance between lights and vt :
	float dist = length(lightPos - vPos_WS);

	vec4 n = normalize(vNorm_CS);
	vec4 l = normalize(lightDir_CS);
	float cosTheta = clamp(dot(n,l), 0.0, 1.0);
	vec4 e = normalize(eyeDir_CS);
	vec4 r = reflect(-l,n);
	float cosAlpha = clamp(dot(e,r), 0.0, 1.0);

	vec4 basecolor = vec4(0.27, 0.27, 0.27, 1.0);
	vec4 fragDif = matDifColor * lightColor * lightPower * cosTheta / pow(dist,2.0); fragDif.w = .0;
	vec4 fragSpe = (matSpeColor * lightColor * lightPower * pow(cosAlpha,5.0)) / pow(dist,2.0); fragSpe.w = .0;
	uvec3 ui = texture(texData, texCoord).rgb;
	//color = vec4(float(ui.x)/255., float(ui.y)/255., float(ui.z)/255., 1.);
	//color = vNorm_WS;
	color = R8UIToRGB(ui);
}
