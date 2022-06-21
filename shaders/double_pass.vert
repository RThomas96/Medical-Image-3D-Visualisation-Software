#version 330 core

layout(location = 0)in vec3 vert;

uniform mat4 pMat;
uniform mat4 vMat;
uniform mat4 mMat;

void main()
{
    //gl_Position = pMat * vMat * mMat * vec4(vert.xyz, 1.);
    //gl_Position = pMat * vec4(vert.xyz, 1.);
    gl_Position = vec4(vert.xyz, 1.);
}
