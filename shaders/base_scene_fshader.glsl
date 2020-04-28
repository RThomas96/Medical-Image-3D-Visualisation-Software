#version 430

// output from vshader :
in vec4 vertexPosWorld;
in vec3 vertexTexWorld;

// texture sampler from medical scan :
uniform sampler3D texData;

// fragment color :
out vec4 color;

void main(void)
{
    color = vec4(abs(1.0-vertexPosWorld.z), .0, abs(vertexPosWorld.z), 1.0); // texture(texData, vertexTexWorld);
}
