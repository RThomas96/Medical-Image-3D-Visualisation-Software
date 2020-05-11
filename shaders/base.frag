#version 400 core

in vec4 vPos;
in vec4 vNorm;

out vec4 color;

void main(void)
{
	color = vNorm * vPos.z;
	color.w = 1.0;
}
