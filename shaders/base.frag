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
uniform sampler3D texData;

void main(void)
{
	vec4 lightColor = vec4(255./255., 214./255., 170./255.,1.0);
	float lightPower = 1.0;
	// colors :
	vec4 matDifColor = vec4(0.8,0.8,0.8,1.0) * vNorm_CS.z;
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
	color = texture(texData, texCoord);
	//color = vNorm_WS;
}
