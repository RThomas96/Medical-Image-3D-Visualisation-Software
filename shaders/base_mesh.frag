#version 150
#extension GL_ARB_explicit_attrib_location : enable

in vec4 position;
flat in vec4 normal;
in vec2 texture;

layout(location = 0) out vec4 color;
layout(location = 1) out vec4 worldPosition;

uniform vec4 objectColor;
uniform vec3 lightPosition;
uniform vec3 planePosition;
uniform vec3 planeDirection;

void main() {
    //if(position.x < planePosition.x || position.y < planePosition.y || position.z < planePosition.z)
    //    discard;

    for(int i = 0; i < 3; ++i) {
        if(planeDirection[i] > 0) {
            if(position[i] < planePosition[i])
                discard;
        } else {
            if(position[i] > planePosition[i])
                discard;
        }
    }

    //if(position.x > planePosition.x + 10.)
    //    discard;

    float ambient = 0.2;

    vec3 norm = normalize(normal.xyz);
    vec3 lightDir = normalize(lightPosition - position.xyz); 
    float diffuse = max(dot(norm, lightDir), 0.2);

    vec3 result = (ambient + diffuse) * objectColor.xyz;
    color = vec4(result, objectColor.w);
}
