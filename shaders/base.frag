#version 400 core

in vec4 vPos;
in vec4 vNorm;

void main(void)
{
	gl_FragColor = vNorm * vPos.z;
}
