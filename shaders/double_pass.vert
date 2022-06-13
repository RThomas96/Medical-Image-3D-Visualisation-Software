#version 330 core

layout(location = 0)in vec3 vert;
//layout(location = 1)in vec4 normal;

//uniform mat4 pMat;
//uniform mat4 vMat;
//uniform mat4 mMat;

//out vec4 position;

void main()
{
    //position = pMat * vMat * mMat * vert;
    gl_Position = vec4(vert.x, vert.y, vert.z, 1.0);
}
