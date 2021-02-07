#version 330 core
out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D edgeTexture;
uniform sampler2D noiseTexture;
uniform float lineDistortion; // noiseAmp
uniform bool doLineTremor;
uniform bool normalizeDistortion;
uniform bool randomize;

float rand(vec2 co){
    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

void main() {
    vec4 finalColor;
    if (doLineTremor) {
        // Image distortion
        // noiseAmp determines how much the picture is distorted
        float noiseAmp = lineDistortion;
        vec2 noise;
        vec2 rCoords = vec2 (rand(TexCoords), rand(TexCoords));
        if (randomize) noise = texture(noiseTexture, rCoords).xy;
        else noise = texture(noiseTexture, TexCoords).xy;
        if (normalizeDistortion)
            noise = normalize(noise * 2.0 - vec2(1.0));
        noise *= noiseAmp;
        // distortion of image / texture coord

        finalColor = texture(edgeTexture, TexCoords + noise);
    } else {
        finalColor = texture(edgeTexture, TexCoords);
    }
    FragColor = finalColor;
}
