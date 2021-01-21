#version 330 core
layout (location = 0) in vec3 vertex;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 textCoord;
layout (location = 3) in vec3 vTangent;
layout (location = 4) in vec3 aBitangent;

struct L_OUT {
    vec3 lSpecular, lDilute, lColor;
};


uniform vec4 inColor0, inColor1, inColor2, inColor3;
uniform mat4 view;//world;
uniform mat4 invTranspose;//worldInvTrans;
uniform mat4 projection;//worldViewProj;
uniform mat4 model;
//uniform mat4 viewInv;
// LIGHTS
uniform bool l1enabled;
uniform int l1type;
uniform vec3 l1pos;
uniform vec3 l1color;
uniform float l1intensity;
uniform vec3 l1direction;
uniform float l1coneAngle;
uniform float l1fallOff;
uniform float l1attenuationScale;
//uniform bool l1shadowOn;
uniform mat4 l1matrix;
uniform bool l1UseSpecular;
uniform bool l2enabled;
uniform int l2type;
uniform vec3 l2pos;
uniform vec3 l2color;
uniform float l2intensity;
uniform vec3 l2direction;
uniform float l2coneAngle;
uniform float l2fallOff;
uniform float l2attenuationScale;
//uniform bool l2shadowOn;
uniform mat4 l2matrix;
uniform bool l2UseSpecular;
uniform bool l3enabled;
uniform int l3type;
uniform vec3 l3pos;
uniform vec3 l3color;
uniform float l3intensity;
uniform vec3 l3direction;
uniform float l3coneAngle;
uniform float l3fallOff;
uniform float l3attenuationScale;
//uniform bool l3shadowOn;
uniform mat4 l3matrix;
uniform bool l3UseSpecular;

// WATERCOLOR
uniform float bleedOffset;
uniform float tremorFront;
uniform float tremorSpeed;
uniform float tremorFreq;
uniform float tremor;
uniform float diffuseFactor;
uniform float diluteArea;
uniform float shaderWrap;
uniform float darkEdges;
uniform float timer;
uniform vec2 texel;
//shading
uniform float specular;
uniform float specDiffusion;
uniform float specTransparency;

out vec4 vColor0, vColor1, vColor2;
out vec4 vPreviousScreenPos;
out vec4 pos;
out vec3 posWorld;
out vec3 normalWorld;
out vec3 tangentWorld;
out vec3 binormalWorld;
out vec3 lightDir;
out vec3 viewDir;
out vec3 velocityDepth;
out vec2 texCoordF;
out float nDotV;//14
//lights
out vec3 lSpecTotal;
out vec3 lightColorTotal;
out vec3 lDiluteTotal;
out float lShadeTotal;

float getLightConeAngle(float coneAngle, float coneFallOff, vec3 lightVec, vec3 lightDir) {
    if (coneFallOff< coneAngle)
        coneFallOff = coneAngle;
    float lDotDir = dot(lightVec, lightDir);
    float edge0 = cos(coneFallOff);
    float edge1 = cos(coneAngle);
    // Hermite interpolation // smooth step function
    float cone = clamp((lDotDir - edge0) / (edge1 - edge0), 0.0f, 1.0f);
    cone = cone * cone * (3 - 2 * cone);
    return cone;
}

L_OUT calculateLight(bool lightEnable, int lightType, float lightAtten, vec3 lightPos, vec3 vertWorldPos,
    vec3 lightColor, float lightIntensity, vec3 lightDir, float lightConeAngle, float lightFallOff, mat4 lightViewPrjMatrix,
    //bool lightShadowOn,
    vec3 normalWorld, vec3 viewDir, float depth, bool lightUseSpecular) {

    L_OUT L;
    L.lSpecular = vec3(0.0);
    L.lColor = vec3(0.0);
    L.lDilute = vec3(0.0);

    if(lightEnable){
        //for Maya, flip the lightDir (weird)
        lightDir = -lightDir;
        //spot = 2, point = 3, directional = 4, ambient = 5,

        //ambient light
        //-> no diffuse, specular or shadow casting
        if (lightType == 5){
            L.lColor = lightColor * lightIntensity;
            return L;
        }

        //directional light -> no position
        bool isDirectionalLight = (lightType == 4);
        vec3 lightVec = mix(lightPos - vertWorldPos, lightDir, isDirectionalLight);
        vec3 nLightVec = normalize(lightVec); //normalized light vector

        //diffuse
        //dot product
        float nDotL = dot(normalWorld, nLightVec);

        //Wrapped Lambert
        //Derived from half lambert, presents problems with shadow mapping
        //float WL = diffuseArea + (1-diffuseArea) * nDotL;
        float dotMask = clamp(nDotL, 0.0, 1.0);
        float DF = mix(1,dotMask, diffuseFactor); //diffuse factor
        float SW = mix(0,clamp(-nDotL, 0.0, 1.0), shaderWrap); //shade wrap
        float CL = clamp(DF*(1-SW), 0.0, 1.0); //custom lambert
        vec3 diffuseColor = lightColor * lightIntensity * CL; //diffuse reflectance (lambert)

        //dilute area
        float clampVal = clamp((dotMask + (diluteArea - 1)) / diluteArea, 0.0f, 1.0f);
        vec3 diluted = vec3(clampVal, clampVal, clampVal);//todo:verify

        //specular (Phong)
        vec3 specularColor = vec3(0);
        if(lightUseSpecular){
            float rDotV = dot(reflect(nLightVec, normalWorld),-viewDir);
            float clamped = clamp(((1-specular)-rDotV)*200/5, 0.0f, 1.0f);// 5=> depth TODO find out where that value comes from
            float specularEdge = darkEdges * (clamped -1); // darkened edges mask
            float specularColorFloat = (mix(specularEdge, 0.0f, specDiffusion) + 2 * clamp(((max(1.0f - specular, rDotV) - (1 - specular)) * pow((2 - specDiffusion), 10)),0.0f, 1.0f)) * (1 - specTransparency);
            specularColor = vec3(specularColorFloat, specularColorFloat, specularColorFloat); // TODO verify
            specularColor *= clamp(dot(normalWorld, lightDir) * 2, 0.0f, 1.0f);
        }

        //attenuation
        if (!isDirectionalLight){
            bool enableAttenuation = lightAtten > 0.0001f;
            float attenuation = mix(1.0, 1 / pow(length(lightVec), lightAtten), enableAttenuation);
            //compensate diffuse and specular
            diffuseColor *= attenuation;
            specularColor *= attenuation;
        }

        // spot light Cone Angle
        if (lightType == 2) {
            float angle = getLightConeAngle(lightConeAngle, lightFallOff, nLightVec, lightDir);
            diffuseColor *= angle;
            specularColor *= angle;
        }

        //L.dilute = nDotL.xxx;
        //L.color = shadeColor;
        L.lColor = diffuseColor;
        L.lSpecular = specularColor;
        L.lDilute = diluted;
        //L.dilute = diffuseColor;
    }

    return L;
}

void main() {
    vec4 worldPos = model * vec4(vertex, 1.0);//?
    mat4 viewInv = inverse(view);
    vColor0 = inColor0;
    vColor1 = inColor1;
    vColor2 = inColor2;
    vPreviousScreenPos = inColor3;
    texCoordF = vec2(textCoord.x, 1.0 - textCoord.y);
    posWorld = (vec4(worldPos.xyz,1) * view).xyz;
    normalWorld = normalize((vec4(normal, 0.0) * view).xyz);
    vec3 mul;// = vTangent * view;
    mul.x = dot(vTangent, view[0].xyz);
    mul.y = dot(vTangent, view[1].xyz);
    mul.z = dot(vTangent, view[2].xyz);
    tangentWorld = normalize(mul.xyz);
    binormalWorld = normalize(cross(normalWorld, tangentWorld));
    //view direction
    viewDir = normalize(viewInv[3].xyz - posWorld);
    nDotV = dot(normalWorld, viewDir);
    // main light direction
    lightDir = normalize(-l1direction);
    //z-depth
    float depth = distance(posWorld, viewInv[3].xyz);
    //vec4 pos = vec4(worldPos.xyz, 1.0) * projection;

    // WATERCOLOR EFFECTS
    vec3 newPos = worldPos.xyz;
    newPos += normalWorld * clamp (inColor2.a - 0.7, 0.0, 1.0) * bleedOffset;

    float tremorAngle = min(clamp(nDotV * 1.2, 0.0, 1.0), (1 - tremorFront));
    vec4 pPos = vec4(newPos, 1) * projection;
    vec4 tremorPos = pPos + vec4((sin(timer * (tremorSpeed * 100) + worldPos.xy * (tremorFreq * 10)) * tremor * texel).xy, 0, 0);

    pos = mix(tremorPos, pPos, tremorAngle);

    vec2 velocity = ((pos.xy / pos.w)/inColor3.xy)/2;
    velocityDepth = vec3(velocity, depth);

    gl_Position = pos;

    //LIGHTS
    L_OUT l1 = calculateLight(l1enabled, l1type, l1attenuationScale, l1pos, posWorld, l1color, l1intensity, l1direction, l1coneAngle, l1fallOff, l1matrix, normalWorld, viewDir, depth, l1UseSpecular);
    L_OUT l2 = calculateLight(l2enabled, l2type, l2attenuationScale, l2pos, posWorld, l2color, l2intensity, l2direction, l2coneAngle, l3fallOff, l2matrix, normalWorld, viewDir, depth, false);
    L_OUT l3 = calculateLight(l3enabled, l3type, l3attenuationScale, l3pos, posWorld, l3color, l3intensity, l3direction, l3coneAngle, l3fallOff, l3matrix, normalWorld, viewDir, depth, false);

    lSpecTotal = l1.lSpecular + l2.lSpecular + l3.lSpecular;
    lightColorTotal = l1.lColor + l2.lColor + l3.lColor;
    lDiluteTotal = l1.lDilute + l2.lDilute + l3.lDilute;
    lShadeTotal = 1.0;
}