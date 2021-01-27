#version 330 core

out vec4 FragColor;

in VS_OUT {
    vec3 Pos_eye;
    vec3 N_eye;
    vec3 Light_eye; // light pos?
    vec2 textCoord;
    float nDotV;
} vs_in;

in vec2 fsInUV;
// light uniform variables
uniform vec3 ambientLightColor;
uniform vec3 lightColor;
uniform bool useLightModel;

// attenuation/dilutes
uniform float attenuationC0;
uniform float attenuationC1;
uniform float attenuationC2;

// material properties
uniform float ambientOcclusionMix;
uniform float specularExponent;

// material textures
uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;
uniform sampler2D texture_normal1;
uniform sampler2D texture_ambient1;

uniform float blinn;

// Textures
uniform bool useNormalTexture;
uniform bool useSpecularTexture;

//WATERCOLOR
uniform vec3 paperColor;
// Reflectance (cangiante illumination)
uniform float dilution; // dilution variable d
uniform float cangiante; // cangiante variable c
uniform float diluteArea; //dilute area dA
uniform bool applyReflectance;
// Turbulence
uniform bool applyTurbulence;
uniform float turbulenceControl;
uniform vec2 texelSize;

void main() {
//    vec3 pixel =vec3(0);
//    float transparency = 1.0f;
//    vec4 albedo = texture(texture_diffuse1, vs_in.textCoord);
//    vec3 color = albedo.rgb;
////    vec4 normal = texture(texture_normal1, vs_in.textCoord);
////    color = normal.rgb;
//
//    if (useLightModel) {
//        float ambientOcclusion = texture(texture_ambient1, vs_in.textCoord).r;
//        ambientOcclusion = mix(1.0, ambientOcclusion, ambientOcclusionMix);
//
//        // ambient component
//        vec3 ambient = ambientLightColor * color;
//
//        // diffuse component
//        // L: vertex to light vector
//        vec3 L_eye = normalize(vs_in.Light_eye - vs_in.Pos_eye).xyz;
//        float diffuseModulation = max(dot(vs_in.N_eye, L_eye), 0.0);
//        vec3 diffuse = lightColor * diffuseModulation * color;
//
//        // specular component
//        // R: incident light (-L) reflection vector, you can also use the reflect() function
//        vec3 R_eye =  -L_eye - 2 * dot(-L_eye, vs_in.N_eye) * vs_in.N_eye;
//        float specModulation = pow(max(dot(R_eye, normalize(-vs_in.Pos_eye)), 0.0), specularExponent);
//        vec3 specular = lightColor * specModulation;
//
//        // attenuation
//        float dist = length(vs_in.Pos_eye - vs_in.Light_eye);
//        float attenuation =  1.0 / (attenuationC0 + attenuationC1 * dist + attenuationC2 * dist * dist);
//
//        color = mix(color, (ambient + (diffuse + specular) * attenuation) * ambientOcclusion, 0.7);
//    }
//    // REFLECTANCE
//    // R.1 Calculate area of effect: DA = (L dot N + (dA - 1))/dA
//    float areaOfEffect = (vs_in.nDotV + (diluteArea- 1))/diluteArea;
//    // R.2 Calculate cangiante color: Cc = C + (DA x c)
//    vec3 cangianteColor = color + (areaOfEffect * cangiante);
//    // R.3 lerp cangiante color towars paper color by dilution: Cd = d x DA(Cp - Cc) +Cc
//    // vec3 reflectanceColor = mix(cangianteColor, paperColor, dilution);
//    vec3 reflectanceColor = dilution * (areaOfEffect * (paperColor - cangianteColor)) + cangianteColor;
//    if (applyReflectance) {
//        color = reflectanceColor;
//    }
//    // TURBULENCE
//    vec3 turbulenceColor;
//    if (turbulenceControl < 0.5) {
//        turbulenceColor = pow(color, vec3(3 - (turbulenceControl * 4)));
//    } else {
//        turbulenceColor = ((turbulenceControl - 0.5) *(2*(paperColor - color))) + color;
//    }
//    if (applyTurbulence) {
//        color = turbulenceColor;
//    }
////
//    FragColor = vec4(color, transparency);
    float dx = texelSize.x;
    float dy = texelSize.y;

    vec3 center = sampleNrm( texture_normal1, vec2(0.0, 0.0) );

    // sampling just these 3 neighboring fragments keeps the outline thin.
    vec3 top = sampleNrm( texture_normal1, vec2(0.0, dy) );
    vec3 topRight = sampleNrm( texture_normal1, vec2(dx, dy) );
    vec3 right = sampleNrm( texture_normal1, vec2(dx, 0.0) );

    // the rest is pretty arbitrary, but seemed to give me the
    // best-looking results for whatever reason.

    vec3 t = center - top;
    vec3 r = center - right;
    vec3 tr = center - topRight;

    t = abs( t );
    r = abs( r );
    tr = abs( tr );

    float n;
    n = max( n, t.x );
    n = max( n, t.y );
    n = max( n, t.z );
    n = max( n, r.x );
    n = max( n, r.y );
    n = max( n, r.z );
    n = max( n, tr.x );
    n = max( n, tr.y );
    n = max( n, tr.z );

    // threshold and scale.
    n = 1.0 - clamp( clamp((n * 2.0) - 0.8, 0.0, 1.0) * 1.5, 0.0, 1.0 );

    FragColor = vec4(texture(texture_diffuse1, fsInUV).rgb * (0.1 + 0.9*n), 1.0);

}
// colorImgae, depth image, control imgae mask, one dim gaussian normalized kernel weights(size = 21, o = 20) depth threshold T
vec3 jointBilateralColorBleeding(vec3 I, vec3 Z) {
    return vec3(0);
}