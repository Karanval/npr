#version 330 core

out vec4 FragColor;

// TEXTURES
uniform sampler2D texture_diffuse;
uniform sampler2D texture_specular;
uniform sampler2D texture_normal;
uniform sampler2D texture_ambient;

void main() {
    vec3 pixel =vec3(0);
    float transparency = 1.0f;
    vec4 albedo = texture(texture_diffuse, texCoordF);
    vec3 color = albedo.rgb;


    FragColor = vec4(clamp(pixel, 0.0, 1.0), transparency);
}