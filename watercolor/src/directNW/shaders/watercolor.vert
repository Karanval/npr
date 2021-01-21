#version 330 core
layout (location = 0) in vec3 vertex;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 textCoord;
layout (location = 3) in vec3 vTangent;
layout (location = 4) in vec3 aBitangent;

uniform vec4 inColor0, inColor1, inColor2, inColor3;
uniform mat4 view;//world;
uniform mat4 invTranspose;//worldInvTrans;
uniform mat4 projection;//worldViewProj;
uniform mat4 model;
//uniform mat4 viewInv;
uniform vec3 light1Dir;
uniform float bleedOffset;
uniform float tremorFront;
uniform float tremorSpeed;
uniform float tremorFreq;
uniform float tremor;
uniform float timer;
uniform vec2 texel;

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
out vec2 texture;
out float nDotV;

void main() {
    vec4 worldPos = model * vec4(vertex, 1.0);//?
    mat4 viewInv = inverse(view);
    vColor0 = inColor0;
    vColor1 = inColor1;
    vColor2 = inColor2;
    vPreviousScreenPos = inColor3;
    texture = vec2(textCoord.x, 1.0 - textCoord.y);
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
    lightDir = normalize(-light1Dir);
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
}