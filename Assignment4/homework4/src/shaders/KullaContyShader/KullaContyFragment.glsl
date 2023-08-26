#version 300 es
#ifdef GL_ES
precision mediump float;
#endif

out vec4 FragColor;

in vec2 vTexCoords;
in vec3 vWorldPos;
in vec3 vNormal;

// IBL
uniform samplerCube uIrradianceMap;
uniform samplerCube uPrefilterMap;
uniform sampler2D uPbrBrdfLUT;

// material parameters
uniform sampler2D uAlbedoMap;
uniform float uMetallic;
uniform float uRoughness;
uniform float uAo;

//light
uniform vec3 uLightPos[4];
uniform vec3 uLightColors[4];
uniform vec3 uCameraPos;

//kulla conty
uniform sampler2D uKullaContyBrdflut;
uniform sampler2D uKullaContyEavglut;

const float PI = 3.14159265359;
const float Inv2PI = 0.1591549431;

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
   // TODO: To calculate GGX NDF here

    float a = roughness*roughness;
    float a2 = a*a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;

    float nom   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / max(denom, 0.0000001);
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    // TODO: To calculate Schlick G1 here
    
    float a = roughness;
    float k = (a + 1.0) * (a + 1.0) / 8.0;

    float nom = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    // TODO: To calculate Smith G here

    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

vec3 fresnelSchlick(vec3 F0, vec3 V, vec3 H)
{
    // TODO: To calculate Schlick F here
    
    return F0 + (1.0 - F0) * pow(clamp(1.0 - max(dot(H, V), 0.0), 0.0, 1.0), 5.0);
}

vec3 fresnelSchlickRoughness(vec3 F0, vec3 V, vec3 N,float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(1.0 - max(dot(N,V),0.0), 5.0);
} 

//https://blog.selfshadow.com/publications/s2017-shading-course/imageworks/s2017_pbs_imageworks_slides_v2.pdf
vec3 AverageFresnel(vec3 r, vec3 g)
{
    return vec3(0.087237) + 0.0230685*g - 0.0864902*g*g + 0.0774594*g*g*g
           + 0.782654*r - 0.136432*r*r + 0.278708*r*r*r
           + 0.19744*g*r + 0.0360605*g*g*r - 0.2586*g*r*r;
}

vec3 MultiScatterBRDF(float NdotL, float NdotV)
{
  vec3 albedo = pow(texture(uAlbedoMap, vTexCoords).rgb,vec3(2.2));

  vec3 E_o = texture(uKullaContyBrdflut, vec2(NdotL, uRoughness)).xyz;
  vec3 E_i = texture(uKullaContyBrdflut, vec2(NdotV, uRoughness)).xyz;

  vec3 E_avg = texture(uKullaContyEavglut, vec2(0, uRoughness)).xyz;
  //gold
  vec3 edgetint = vec3(0.94806,0.86104,0.60760);
  vec3 F_avg = AverageFresnel(albedo, edgetint);
  
  // TODO: To calculate fms and missing energy here
  vec3 F_ms = (1.0 - E_o) * (1.0 - E_i) / (PI * (1.0 - E_avg));
  vec3 F_add = F_avg * E_avg / (1.0 - F_avg * (1.0 - E_avg));

  return F_add * F_ms;
  
}
//split sum
vec3 MultiScatterBRDF(float NdotL, float NdotV, vec3 F)
{
  vec3 albedo = pow(texture(uAlbedoMap, vTexCoords).rgb,vec3(2.2));

  // A split-sum result in which R-channel repesent F interger term
  vec3 E_o = texture(uKullaContyBrdflut, vec2(NdotL, uRoughness)).xyz;
  vec3 E_i = texture(uKullaContyBrdflut, vec2(NdotV, uRoughness)).xyz;

  // Split sum result add here.
  vec3 Emu_o = F * E_o.x + vec3(1.0) * E_o.y;
  vec3 Emu_i = F * E_i.x + vec3(1.0) * E_i.y;

  vec3 E_avg = texture(uKullaContyEavglut, vec2(0, uRoughness)).xyz;
  vec3 E_avgss = F * E_avg.x + vec3(1.0) * E_avg.y;
  
  vec3 edgetint = vec3(1.0,1.0,1.0);
  vec3 F_avg = AverageFresnel(albedo, edgetint);
  
  // TODO: To calculate fms and missing energy here
  vec3 F_ms = (1.0 - Emu_o) * (1.0 - Emu_i) / (PI * (1.0 - E_avgss));
  vec3 F_add = F_avg * E_avgss / (1.0 - F_avg * (1.0 - E_avgss));
  return F_ms ;
}

void main(void) {
  vec3 albedo = pow(texture(uAlbedoMap, vTexCoords).rgb,vec3(2.2));;

  vec3 N = normalize(vNormal);
  vec3 V = normalize(uCameraPos - vWorldPos);
  vec3 R = reflect(-V,N);

  vec3 F0 = vec3(0.04); 
  F0 = mix(F0, albedo, uMetallic);
  vec3 Lo = vec3(0.0);

  for(uint i = 0u;i < 4u;++i){
    vec3 L = normalize(uLightPos[i] - vWorldPos);
    vec3 H = normalize(L + V);
    // float distance = length(uLightPos[i] - vWorldPos);
    // float attenuation = 1.0 / (distance * distance);
    vec3 radiance = uLightColors[i];

    //Cook-Torrance BRDF
    float NDF = DistributionGGX(N,H,uRoughness);
    float G   = GeometrySmith(N,V,L,uRoughness);
    vec3 F    = fresnelSchlick(F0,V,H);

    float NdotL = max(dot(N,L),0.0);
    float NdotV = max(dot(N,V),0.0);

    vec3 numerator = NDF * G * F;
    float denominator = max((4.0 * NdotL * NdotV),0.0000001);
    vec3 specular = numerator / denominator;
    // //not split sum
    // vec3 Fms = MultiScatterBRDF(NdotL, NdotV);
    // //split sum
    vec3 Fms = MultiScatterBRDF(NdotL, NdotV);
    vec3 BRDF = specular + Fms;
    Lo += BRDF * radiance * NdotL; 
  }
  //IBL
  vec3 F = fresnelSchlickRoughness(F0,V,N,uRoughness);
  vec3 kD = (vec3(1.0) - F) * (1.0 - uMetallic);
  //漫反射光照积分项
  vec3 irradiance = texture(uIrradianceMap,N).rgb;
  vec3 diffuse = irradiance * albedo;

  const float MAX_REFLECTION_LOD = 4.0;
  //镜面反射光照项
  vec3 prefilteredColor = textureLod(uPrefilterMap,R,uRoughness * MAX_REFLECTION_LOD).rgb;
  //BRDF积分项
  vec2 brdf = texture(uPbrBrdfLUT,vec2(max(dot(N,V),0.0),uRoughness)).rg;
  //split sum
  vec3 specular = prefilteredColor * (F0 * brdf.r + brdf.g);
  vec3 ambient = (kD * diffuse + specular) * uAo;

  vec3 color = ambient + Lo;
  //HDR tonemapping
  // color = color / (color + vec3(1.0));
  //gamma correct
  color = pow(color,vec3(1.0/2.2));
  FragColor = vec4(color, 1.0);
}