#version 330 core
out vec4 FragColor;

in float nDotL;
in vec2 texCoord;

// material textures
uniform sampler2D texture_diffuse1;

uniform int celAmount;
uniform bool doCelShading;

void main() {
    vec4 albedo = texture(texture_diffuse1, texCoord);
    vec4 color = vec4(albedo.rgb, 1.0);
    //  Cel shading
    // calculate soft shading
    float shading = nDotL;

    // re-quantisize soft shading
    float celShading = 1.0;
    float step = 1.0 / celAmount;
    for (float i = 1 - (step * 2); i >= 0; i = i - step) {
        if (i == 0) i = 0.1;
        if (shading < i) celShading = i + step;
    }

    if (doCelShading) {
        FragColor = color * celShading;
    } else {
        FragColor = color * shading;
    }

    // create texture with above result: celTexture

}