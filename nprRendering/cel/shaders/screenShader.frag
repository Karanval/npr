#version 330 core
out vec4 FragColor;
in vec2 texCoord;

uniform sampler2D edgeTexture;
uniform sampler2D noiseTexture;
uniform float tremorAmount; // noiseAmp
uniform bool doLineTremor;

void main() {
    vec4 finalColor;
    if (doLineTremor) {
        // Image distortion
        // noiseAmp determines how much the picture is distorted
        float noiseAmp = tremorAmount;
        vec2 noise;
        noise = texture(noiseTexture, texCoord).xy;
        noise = normalize(noise * 2.0 - vec2(1.0));
        noise *= noiseAmp;
        // distortion of image / texture coord

        //vec4 finalColor = texture(nprTexture, texCoord + noise);
        finalColor = texture(edgeTexture, texCoord + noise);
    } else {
        finalColor = texture(edgeTexture, texCoord);
    }
    FragColor = finalColor;
}