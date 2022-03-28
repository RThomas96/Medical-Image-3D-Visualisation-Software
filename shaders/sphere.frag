#version 150 core
#extension GL_ARB_separate_shader_objects : enable

// Signals we're in the main shader, for any shaders inserted into this one.
#define MAIN_SHADER_UNIT

in float current_state;
in float isVisible;
in vec4 position;
in vec4 o_normal;

uniform vec3 lightPosition;
  
out vec4 FragColor;

void main()
{
    //if(isVisible == 0.)
    //    discard;

    bool isNONE      = abs(current_state - 0) < 0.001;
    bool isAT_RANGE  = abs(current_state - 1) < 0.001;
    bool isSELECTED  = abs(current_state - 2) < 0.001;
    bool isLOCK      = abs(current_state - 3) < 0.001;
    bool isMOVE      = abs(current_state - 4) < 0.001;
    bool isWAITING   = abs(current_state - 5) < 0.001;
    bool isHIGHLIGHT = abs(current_state - 6) < 0.001;

    vec4 color = vec4(0., 0., 0., 1.);

    if(isNONE) {
        color = vec4(1., 0., 0., 1.);
    }

    if(isAT_RANGE) {
        color = vec4(0., 0.9, 0., 1.);
    }

    if(isMOVE) {
        color = vec4(0., 1., 0., 1.);
    }

    if(isLOCK) {
        color = vec4(0.9, 0.9, 0.9, 1.);
    }

    if(isWAITING) {
        color = vec4(218./255., 112./255., 214./255., 1.);
    }

    if(isHIGHLIGHT) {
        color = vec4(255./255., 215./255., 0./255., 1.);
    }

    vec3 norm = normalize(o_normal.xyz);
    vec3 lightDir = normalize(lightPosition - position.xyz); 
    float diffuse = max(dot(norm, lightDir), 0.0);

    //FragColor = diffuse * color;
    FragColor = color;
} 
