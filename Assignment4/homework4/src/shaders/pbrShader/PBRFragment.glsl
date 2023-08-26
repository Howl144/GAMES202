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
    // TODO: To calculate Smith G1 here

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
    return F0 + (1.0 - F0) * pow(1.0 - max(dot(H, V), 0.0), 5.0);
}
//由于环境光来自在半球内所有围绕着法线N的方向，没有单一的半向量去决定菲涅尔因子。为了仍然能模拟菲涅尔，这里采用了法线和视线的夹角。之前的算法采用了受表面粗糙度影响的微平面半向量，作为菲涅尔方程的输入。这里我们加入粗糙度来权衡这一损失。
vec3 fresnelSchlickRoughness(vec3 F0, vec3 V, vec3 N,float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(1.0 - max(dot(N,V),0.0), 5.0);
}   

void main(void) {
    vec3 albedo = pow(texture(uAlbedoMap, vTexCoords).rgb,vec3(2.2));

    vec3 N = normalize(vNormal);
    vec3 V = normalize(uCameraPos - vWorldPos);
    vec3 R = reflect(-V, N);

    vec3 F0 = vec3(0.04); 
    F0 = mix(F0, albedo, uMetallic);
    vec3 Lo = vec3(0.0);

    for (uint i = 0u;i < 4u;++i){

        vec3 L = normalize(uLightPos[i] - vWorldPos);
        vec3 H = normalize(V + L);
        // float distance = length(uLightPos[i] - vWorldPos);
        // float attenuation = 1.0 / (distance * distance); 
        vec3 radiance = uLightColors[i] * 1.0;

        //Cook-Torrance BRDF
        float NDF = DistributionGGX(N, H, uRoughness);   
        float G   = GeometrySmith(N, V, L, uRoughness); 
        vec3 F    = fresnelSchlick(F0, V, H);
            
        float NdotL = max(dot(N,L),0.0);
        float NdotV = max(dot(N,V),0.0);

        vec3 numerator    = NDF * G * F; 
        float denominator = max((4.0 * NdotL * NdotV), 0.0000001);
        vec3 specular = numerator / denominator;
        //Reference opengl pbr
        vec3 diffuse = (vec3(1.0)-F) * (1.0 - uMetallic) * albedo / PI;
        // Lo += ( diffuse + specular) * radiance * NdotL;
        Lo += (specular) * radiance * NdotL;
    }
    //ambient lighting (we now use IBL as the ambient term)
    vec3 F = fresnelSchlickRoughness(F0, V, N,uRoughness);
    vec3 kD = (vec3(1.0) - F) * (1.0 - uMetallic);
    //漫反射光照积分项
    vec3 irradiance = texture(uIrradianceMap,N).rgb;
    vec3 diffuse = irradiance * albedo; 
    //以确保不会对一个没有数据的 mip 级别采样
    const float MAX_REFLECTION_LOD = 4.0;
    // 镜面反射光照积分项
    vec3 prefilteredColor = textureLod(uPrefilterMap,R,uRoughness*MAX_REFLECTION_LOD).rgb;
    // BRDF积分项
    vec2 brdf = texture(uPbrBrdfLUT,vec2(max(dot(N,V),0.0),uRoughness)).rg;
    //split sum
    vec3 specular = prefilteredColor * (F0 * brdf.r + brdf.g);
    vec3 ambient = (kD * diffuse + specular) * uAo;

    vec3 color = ambient + Lo;
    // HDR tonemapping
    // color = color / (color + vec3(1.0));
    // gamma correct
    color = pow(color, vec3(1.0/2.2)); 
    FragColor = vec4(color, 1.0);
}