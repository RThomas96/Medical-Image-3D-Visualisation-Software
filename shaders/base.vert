#version 400 core

layout(location=0) in vec4 vertexPosition;
layout(location=1) in vec4 vertexNormal;
layout(location=2) in vec3 vertexTexCoord;

out vec4 vPos_WS;
out vec4 vNorm_WS;
out vec3 texCoord;

out vec4 vPos_CS;
out vec4 vNorm_CS;
out vec4 lightDir_CS;
out vec4 eyeDir_CS;

uniform mat4 mMatrix;
uniform mat4 vMatrix;
uniform mat4 pMatrix;
uniform vec4 lightPos; // will always be worldspace here !

void main(void) {
	//mat4 mMatrix = mat4(1.0);
	mat4 minverse = inverse(transpose(mMatrix));
	mat4 mvp = pMatrix * vMatrix * mMatrix;
	gl_Position = mvp * vertexPosition;
	vPos_WS = mMatrix * vertexPosition;
	vNorm_WS = normalize(vertexNormal);

	vPos_CS = vMatrix * vertexPosition;
	vNorm_CS = vMatrix * minverse * vertexNormal;
}
