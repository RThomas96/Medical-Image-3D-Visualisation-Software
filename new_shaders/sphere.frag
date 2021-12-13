#version 150 core
#extension GL_ARB_separate_shader_objects : enable

// Signals we're in the main shader, for any shaders inserted into this one.
#define MAIN_SHADER_UNIT

out vec4 FragColor;
  
void main()
{
    FragColor = vec4(1.0, 0.0, 0.0, 1.0);
} 
