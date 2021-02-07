#version 330 core
out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D celTexture;
uniform vec2 texelSize;
uniform bool doEdgeDetection;
uniform bool doEdgeOnly;
uniform float strokeSize;

//void main()
//{
//    vec3 col = texture(celTexture, TexCoords).rgb;
//    FragColor = vec4(col, 1.0);
//}

void main() {
    // Edge detection
    float fSizeX =  texelSize.x * strokeSize;
    float fSizeY =  texelSize.y * strokeSize;
    vec4 samples[9];
    vec2 offset[9];
    offset[0] = TexCoords;
    //texture coordinate for neighboring pixels
    offset[1] = TexCoords + vec2(fSizeX, fSizeY);
    offset[2] = TexCoords + vec2(-fSizeX, -fSizeY);
    offset[3] = TexCoords + vec2(fSizeX, 0.0);
    offset[4] = TexCoords + vec2(-fSizeX, 0.0);
    offset[5] = TexCoords + vec2(0.0, fSizeY);
    offset[6] = TexCoords + vec2(0.0, -fSizeY);
    offset[7] = TexCoords + vec2(fSizeX, -fSizeY);
    offset[8] = TexCoords + vec2(-fSizeX, fSizeY);
    vec4 edge = vec4(0.0);
    // apply filter (Laplace)
    int i;
    for ( i = 1; i < 9; i++) {
        edge += texture(celTexture, offset[i]);
    }
    edge += texture(celTexture, offset[0]) * (-8.0);
    // invert and output edges
    if (doEdgeDetection) {
        if (doEdgeOnly) {
            FragColor = vec4(1.0) - edge;
        } else {
            FragColor = vec4(texture(celTexture, TexCoords).rgb, 1.0) - edge;
        }
    } else {
        FragColor = vec4(texture(celTexture, TexCoords).rgb, 1.0);
    }
}