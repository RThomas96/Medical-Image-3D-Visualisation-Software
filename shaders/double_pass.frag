#version 150 core
#extension GL_ARB_separate_shader_objects : enable

#define MAIN_SHADER_UNIT

//in vec4 position;
//uniform vec4 color;

out vec4 FragColor;

void main()
{
    FragColor = vec4(1., 0., 0., 0.5);
}
