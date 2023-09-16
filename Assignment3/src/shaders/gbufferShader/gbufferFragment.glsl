#version 300 es
#ifdef GL_ES
precision highp float;
#endif
//MRT
layout(location = 0) out vec4 Frag0;  
layout(location = 1) out vec4 Frag1;  
layout(location = 2) out vec4 Frag2;
layout(location = 3) out vec4 Frag3;

//diffuse map
uniform sampler2D uKd;
//normal map
uniform sampler2D uNt;
uniform sampler2D uShadowMap;
uniform vec3 uZBufferParams;

in mat4 vWorldToLight;
in vec2 vTextureCoord;
in vec4 vPosWorld;
in vec3 vNormalWS;


float SimpleShadowMap(vec3 posWorld,float bias){
  //perspective of light
  vec4 posLight = vWorldToLight * vec4(posWorld, 1.0);
  //to screen space
  vec2 shadowCoord = clamp(posLight.xy * 0.5 + 0.5, vec2(0.0), vec2(1.0));
  float depthSM = texture(uShadowMap, shadowCoord).x;
  float depth = posLight.z;
  //阶跃函数，0.0为阈值，小于它返回0.0，大于它返回1.0
  return step(0.0, depthSM - depth + bias);
}

void LocalBasis(vec3 n, out vec3 b1, out vec3 b2) {
  float sign_ = sign(n.z);
  if (n.z == 0.0) {
    sign_ = 1.0;
  }
  float a = -1.0 / (sign_ + n.z);
  float b = n.x * n.y * a;
  b1 = vec3(1.0 + sign_ * n.x * n.x * a, sign_ * b, -sign_ * n.x);
  b2 = vec3(b, sign_ + n.y * n.y * a, -n.y);
}

vec3 ApplyTangentNormalMap() {
  vec3 t, b;
  vec3 normal = normalize(vNormalWS);
  // t b n
  LocalBasis(normal, t, b);
  //法线向量的范围在-1到1之间
  vec3 nt = texture(uNt, vTextureCoord).xyz * 2.0 - 1.0;
  //法线从切线空间转到世界空间
  nt = normalize(nt.x * t + nt.y * b + nt.z * normal);
  return nt;
}

float Linear01Depth( float z )
{
  float farDivNear = uZBufferParams.y / uZBufferParams.x;
  return 1.0 / ( ( 1.0 - farDivNear) * z + farDivNear);
}

void main(void) {
  vec3 kd = texture(uKd, vTextureCoord).rgb;
  //albedo 
  Frag0 = vec4(kd, 1.0);
  //depth not linear
  Frag1 = vec4(vec3(gl_FragCoord.z), 1.0);
  //world space normal(uNt)
  Frag2 = vec4(ApplyTangentNormalMap(), 1.0);
  //shadow value 0 or 1
  Frag3 = vec4(vec3(SimpleShadowMap(vPosWorld.xyz, 1e-4)), 1.0);
}
