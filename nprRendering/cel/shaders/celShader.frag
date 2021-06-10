#version 330 core
out vec4 FragColor;

in float nDotL;
in vec2 texCoord;
in vec3 Pos_tangent;
in vec3 CamPos_tangent;
in vec3 LightDir_tangent;
in vec3 Norm_tangent;

// light uniform variables
uniform vec3 ambientLightColor;
uniform vec3 lightColor;

// material properties
uniform float ambientOcclusionMix;
uniform float normalMappingMix;
uniform float specularExponent;

// camera position
uniform vec3 viewPosition;

// material textures
uniform sampler2D texture_diffuse1;
uniform sampler2D texture_normal1;
uniform sampler2D texture_ambient1;

uniform int celAmount;
uniform bool doCelShading;
uniform bool useBPSR;

void main() {

    vec4 albedo = texture(texture_diffuse1, texCoord);

    float ambientOcclusion = texture(texture_ambient1, texCoord).r;
    ambientOcclusion = mix(1.0, ambientOcclusion, ambientOcclusionMix);

    vec3 N =  texture(texture_normal1, texCoord).rgb;
    N = normalize(N * 2.0 - 1.0);
    N = normalize(mix(Norm_tangent, N, normalMappingMix));

    // ambient light
    vec3 ambient = ambientLightColor;// * albedo.rgb;

    // parallel light
    vec3 L = normalize(LightDir_tangent);   // L: - light direction
    float diffuseModulation = max(dot(N, L), 0.0);
    vec3 diffuse = lightColor * diffuseModulation;// * albedo.rgb;

    // blinn-phong specular reflection
    vec3 V = normalize(CamPos_tangent - Pos_tangent); // V: surface to eye vector
    vec3 H = normalize(L + V); // H: half-vector between L and V
    float specModulation = max(dot(N, H), 0.0);
    specModulation = pow(specModulation, specularExponent);
    vec3 specular = lightColor * specModulation;


    //vec4 color = vec4(ambient + (diffuse + specular) * ambientOcclusion, albedo.a);
    vec4 color = albedo;

    //  Cel shading
    // calculate soft shading
    //    float shading = nDotL;
    vec3 shading;
    if (useBPSR)
    shading = ambient + (diffuse + specular) * ambientOcclusion;
    else
    shading = vec3(nDotL);

    // re-quantisize soft shading
    //    float celShading = 1.0;
    vec3 celShading = vec3(1.0);
    float step = 1.0 / celAmount;
    for (float i = 1 - (step * 2); i >= 0; i = i - step) {
        if (i == 0) i = 0.1;
        //        if (shading < i) celShading = i + step;
        if (shading.r < i) celShading.r = i + step;
        if (shading.g < i) celShading.g = i + step;
        if (shading.b < i) celShading.b = i + step;
    }

    if (doCelShading) {
        FragColor = vec4(color.rgb * celShading, color.a);
    } else {
        FragColor = vec4(color.rgb * shading, color.a);
    }

    // create texture with above result: celTexture

}