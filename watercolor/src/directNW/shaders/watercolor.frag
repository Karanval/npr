#version 330 core
//
//layout (location = 0) out vec3 gPosition;
//layout (location = 1) out vec3 gNormal;
//layout (location = 2) out vec4 gAlbedoSpec;

// vertex output
in vec4 vColor0, vColor1, vColor2;
in vec4 vPreviousScreenPos;
in vec4 pos;
in vec3 posWorld;
in vec3 normalWorld;
in vec3 tangentWorld;
in vec3 binormalWorld;
in vec3 lightDir;
in vec3 viewDir;
in vec3 velocityDepth;
in vec2 texture;
in float nDotV;

out vec4 FragColor;
out vec4 diffuseOut;
out vec4 specularOut;
out vec4 pigmentCtrlOut;
out vec4 substrateCtrlOut;
out vec4 edgeCtrlOut;
out vec4 abstractionCtrlOut;
out vec2 velocityOut;

uniform bool useNormalMapping;
uniform bool useSpecularMapping;
uniform bool useColorMapping;
// material textures
uniform sampler2D texture_diffuse;
uniform sampler2D texture_specular;
uniform sampler2D texture_normal;
uniform sampler2D texture_ambient;
//uniform float normalMappingMix;
uniform bool flipU;
uniform bool flipV;
uniform float bumpDepth;
uniform vec3 colorTint;
//lights
uniform vec3 l1Specular;
uniform vec3 l1Color;
uniform vec3 l1Dilute;
uniform float l1Shade;
uniform vec3 l2Specular;
uniform vec3 l2Color;
uniform vec3 l2Dilute;
uniform float l2Shade;
uniform vec3 l3Specular;
uniform vec3 l3Color;
uniform vec3 l3Dilute;
uniform float l3Shade;
//WATERcoLor
uniform float dilute;
uniform float cangiante;
uniform vec3 paperColor;
uniform float highArea;
uniform float highTransparency;
uniform float darkEdges;
uniform bool useOverrideShade;
uniform vec3 shadeColor;
uniform float diffuseFactor;
uniform vec3 atmosphereColor;
uniform float rangeStart;
uniform float rangeEnd;

void main()
{
   vec3 pixel = vec3(0);
   float transparency = 1.0f;

   vec4 control1 = vec4(vColor0.rgb, transparency);
   vec4 control2 = vec4(vColor1.rgb, transparency);
   vec4 control3 = vec4(vColor2.rgb, transparency);
   vec4 control4 = vec4(vColor0.a, vColor1.a, vColor2.a, transparency);

   vec3 nomrmalWorldFrag = normalize(normalWorld);

   // normal mapping
   if (useNormalMapping) {
      vec3 tangentWorldFrag = normalize(tangentWorld);
      vec3 binormalWorldFrag = normalize(binormalWorld);

      mat3 local2WorldTranspose = mat3(tangentWorld, binormalWorld, normalWorld);
      // retrieve texelfrom texture
      // fix normal range: rgb sampled value is in the range [0,1], but xyz normal vectors are in the range [-1,1]
      vec3 normalMap = texture(texture_normal, texture).rgb * 2.0 - 1.0;
      // mix the vertex normal and the normal map texture so we can visualize
      // the difference with normal mapping
      normalMap = normalize(mix(fs_in.Norm_tangent, N, normalMappingMix));
      if (flipU){
         normalMap.r = -normalMap.r;
      }
      if (flipV)
      {
         normalMap.g = normalMap.g;
      }
      normalMap.rg *= bumpDepth;
      vec3 normalWorldFrag = normalize(normalMap * local2WorldTranspose);
//      normalMap = normalize(TBN * normalMap);
//      gNormal = normalize(mix(normalize(Normal), normalMap, normalMappingMix));
   }

   //specular mapping
   vec4 specularMap = vec4(1.0);
   if (useSpecularMapping) {
      specularMap = texture(texture_specular, texture);
   }

   // texture mapping
   vec3 tex = colorTint;
   float grayscale = 1.0;
   if (useColorMapping) {
      vec4 sampledPixel = texture(texture_diffuse, texture);
      tex *= sampledPixel.rgb;
      transparency = sampledPixel.a;
      grayscale = 0.2989 * tex.r + 0.5870 * tex.g + 0.1140 * tex.b;
   }

   ///// LIGHTS /////
   vec3 lightTotal = l1Color + l2Color + l3Color;
   vec3 specTotal = l1Specular + l2Specular + l3Specular;
   vec3 diluteTotal = l1Dilute + l2Dilute + l3Dilute;
   float shade = l1Shade + l2Shade + l3Shade;

   diluteTotal = mix(diluteTotal, pow(diluteTotal, vec3(2.2)), clamp(-1 * dilute + cangiante, 0.0, 1.0));

   vec3 highlight = vec3(0);
   if (shade < 1.0) {
      tex.rgb = tex.rgb + clamp(diluteTotal * cangiante, 0.0, 1.0);
      tex.rgb = mix(tex.rgb, paperColor, diluteTotal * dilute);
      if (highArea > 0) {
         highlight = (max(1 - highArea.xxx, diluteTotal) - (1- highArea.xxx))*800/velocityDepth.z;
         highlight = clamp(mix(-highlight*darkEdges, highlight, trunc(highlight)), 0.0, 1.0); //highlight darkened edges
      }
   }

   vec3 watercolor = vec3(0);
   if (useOverrideShade) {
      vec3 c = mix(shadeColor, tex.rgb, clamp(lightTotal, 0.0, 1.0));
      watercolor = c + (specTotal * specularMap.rgb) + highlight * (1 - highTransparency);
   } else {
      vec3 c = mix(shadeColor * grayscale, tex.rgb, clamp(lightTotal, 0.0, 1.0));
      c = mix(vec3(1 - diffuseFactor), c, clamp(lightTotal, 0.0, 1.0)); // vec3?
      watercolor = c + (specTotal * specularMap.rgb) + highlight * (1 - highTransparency);
   }

   //FragColor = vec4(watercolor, transparency);

   vec3 wDarkenEdge = watercolor;
   if (darkEdges > 0) {
      float dEdges = clamp(nDotV * max(3, 20 / velocityDepth.z), 0.0, 1.0);
      float darkenedEdges = mix(1, dEdges, darkEdges);
      wDarkenEdge = mix(watercolor * darkenedEdges, watercolor, clamp(dilute, 0.0, 1.0) + 0.5);
   }

   vec3 controlledColor = wDarkenEdge;

   vec2 velocity = velocityDepth.xy;
   pixel = mix(controlledColor, atmosphereColor.rgb, clamp((velocityDepth.z - rangeStart)/rangeEnd, 0.0, 1.0));

   FragColor = vec4(clamp(pixel, 0.0, 1.0), transparency);
   pigmentCtrlOut = vec4(control1.xyz, 1);
   substrateCtrlOut = vec4(control2.xyz, 1);
   edgeCtrlOut = vec4(control3.xyz, 1);
   abstractionCtrlOut = vec4(control4.xyz, 1);
   velocityOut = velocity;
}