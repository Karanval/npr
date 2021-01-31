#version 330 core
out vec4 FragColor;
in vec2 texCoord;

uniform sampler2D celTexture;
uniform vec2 texelSize;
uniform bool doEdgeDetection;

void main() {
    // Edge detection
    float fSizeX =  texelSize.x;
    float fSizeY =  texelSize.y;
    vec4 samples[9];
    vec2 offset[9];
    offset[0] = texCoord;
    //texture coordinate for neighboring pixels
    offset[1] = texCoord + vec2(fSizeX, fSizeY);
    offset[2] = texCoord + vec2(-fSizeX, -fSizeY);
    offset[3] = texCoord + vec2(fSizeX, 0.0);
    offset[4] = texCoord + vec2(-fSizeX, 0.0);
    offset[5] = texCoord + vec2(0.0, fSizeY);
    offset[6] = texCoord + vec2(0.0, -fSizeY);
    offset[7] = texCoord + vec2(fSizeX, -fSizeY);
    offset[8] = texCoord + vec2(-fSizeX, fSizeY);
    vec4 edge = vec4(0.0);
    // apply filter (Laplace)
    int i;
    for ( i = 1; i < 9; i++) {
        edge += texture(celTexture, offset[i]);
    }
    edge += texture(celTexture, offset[0]) * (-8.0);
    // invert and output edges
    if (doEdgeDetection) {
        FragColor = vec4(1.0) - edge;
    } else {
        FragColor = vec4(texture(celTexture, texCoord).rgb, 1.0);
    }
}