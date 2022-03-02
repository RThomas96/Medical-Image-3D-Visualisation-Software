#version 150 core
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_explicit_attrib_location : enable

layout(location = 0)in vec4 vert;
layout(location = 1)in vec4 normal;
layout(location = 2)in vec4 tex;

uniform mat4 pMat;
uniform mat4 vMat;
uniform mat4 mMat;

uniform sampler1D positions;
uniform sampler1D visible2;
uniform sampler1D state;

out float isVisible;
out float current_state;
out vec4 position;
out vec4 o_normal;

void main()
{
    isVisible = texelFetch(visible2, gl_InstanceID, 0).x;
    current_state = texelFetch(state, gl_InstanceID, 0).x;

    vec4 pos = vec4(texelFetch(positions, gl_InstanceID, 0).xyz, 1.);
    mat4 newM = mMat;
    newM[3][0] += pos.x;
    newM[3][1] += pos.y;
    newM[3][2] += pos.z;
    position = pMat * vMat * newM * vert;
    gl_Position = vec4(position[0], position[1], position[2], position[3]);
    o_normal = normal;
}
