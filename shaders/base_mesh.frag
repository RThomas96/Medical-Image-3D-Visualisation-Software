#version 150
#extension GL_ARB_explicit_attrib_location : enable

in vec4 position;
flat in vec4 normal;
in vec2 texture;

layout(location = 0) out vec4 color;
layout(location = 1) out vec4 worldPosition;

uniform vec4 objectColor;
uniform vec3 lightPosition;

void main() {
    
    float ambient = 0.2;

    vec3 norm = normalize(normal.xyz);
    vec3 lightDir = normalize(lightPosition - position.xyz); 
    float diffuse = max(dot(norm, lightDir), 0.0);

    vec3 result = (ambient + diffuse) * objectColor.xyz;
    color = vec4(result, objectColor.w);
}
