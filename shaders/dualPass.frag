#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D screenTexture;
uniform sampler2D depthTexture;

uniform sampler2D screenTexture2;
uniform sampler2D depthTexture2;

uniform bool alphaRendering;
uniform float alphaBlend;

//void main()
//{
//    FragColor = vec4(texture(screenTexture, TexCoords).xyz, 1.);
//    //FragColor = vec4(1., 0., 0., 1.);
//}


const float offset = 1.0 / 500.0;

void main()
{
    //vec2 offsets[9] = vec2[](
    //    vec2(-offset,  offset), // top-left
    //    vec2( 0.0f,    offset), // top-center
    //    vec2( offset,  offset), // top-right
    //    vec2(-offset,  0.0f),   // center-left
    //    vec2( 0.0f,    0.0f),   // center-center
    //    vec2( offset,  0.0f),   // center-right
    //    vec2(-offset, -offset), // bottom-left
    //    vec2( 0.0f,   -offset), // bottom-center
    //    vec2( offset, -offset)  // bottom-right
    //);

    //float kernel[9] = float[](
    //    -1, -1, -1,
    //    -1,  9, -1,
    //    -1, -1, -1
    //);

    //vec3 sampleTex[9];
    //for(int i = 0; i < 9; i++)
    //{
    //    sampleTex[i] = vec3(texture(screenTexture, TexCoords.st + offsets[i]));
    //}
    //vec3 col = vec3(0.0);
    //for(int i = 0; i < 9; i++)
    //    col += sampleTex[i] * kernel[i];
    //FragColor = vec4(col, 1.0);

    if(alphaRendering) {
        float depth1 = texture(depthTexture, TexCoords).x;
        float depth2 = texture(depthTexture2, TexCoords).x;
        if(depth1 < depth2) {
            gl_FragDepth = depth1;
        } else {
            gl_FragDepth = depth2;
        }

        vec3 color1 = texture(screenTexture, TexCoords).xyz;
        vec3 color2 = texture(screenTexture2, TexCoords).xyz;

        bool color1White = color1.r < 0.01 && color1.g < 0.01 && color1.b < 0.01;
        bool color2White = color2.r < 0.01 && color2.g < 0.01 && color2.b < 0.01;


        if(!color1White && !color2White) {
            if(abs(depth1 - depth2) < 0.0001) {
                FragColor = vec4(mix(color1.rgb, color2.rgb, alphaBlend), 1.);
            } else {
                if(depth1 < depth2) {
                    FragColor = vec4(color1, 1.);
                } else {
                    FragColor = vec4(color2, 1.);
                }
            }
        } else if(color1White && !color2White) {
            FragColor = vec4(color2.rgb, 1.0);
        } else if(color2White && !color1White) {
            FragColor = vec4(color1.rgb, 1.0);
        }
    } else {
        FragColor = vec4(texture(screenTexture, TexCoords).xyz, 1.);
        gl_FragDepth = texture(depthTexture, TexCoords).x;
    }
}
