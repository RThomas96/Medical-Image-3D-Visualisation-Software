#version 150 core
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_explicit_attrib_location : enable

layout(location = 0)in vec4 vert;
layout(location = 1)in vec4 normal;

uniform mat4 pMat;
uniform mat4 vMat;
uniform mat4 mMat;

out vec4 position;

void main()
{
    position = pMat * vMat * mMat * vert;
    gl_Position = vec4(position[0], position[1], position[2], position[3]);
}
