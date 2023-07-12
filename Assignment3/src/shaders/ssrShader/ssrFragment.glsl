#version 300 es

#ifdef GL_ES
precision highp float;
#endif

uniform vec3 uLightDir;
uniform vec3 uCameraPos;
uniform vec3 uLightRadiance;
uniform sampler2D uGDiffuse;
uniform sampler2D uGDepth;
uniform sampler2D uGNormalWS;
uniform sampler2D uGShadow;
uniform vec3 uZBufferParams;

uniform sampler2D uDepthTexture[12];

in mat4 vWorldToScreen;
in vec4 vPosWorld;

#define M_PI 3.1415926535897932384626433832795
#define TWO_PI 6.283185307
#define INV_PI 0.31830988618
#define INV_TWO_PI 0.15915494309

#define MAX_MIPMAP_LEVEL 9

out vec4 FragColor;

float Rand1(inout float p) {
  p = fract(p * .1031);
  p *= p + 33.33;
  p *= p + p;
  return fract(p);
}

vec2 Rand2(inout float p) {
  return vec2(Rand1(p), Rand1(p));
}

float InitRand(vec2 uv) {
	vec3 p3  = fract(vec3(uv.xyx) * .1031);
  p3 += dot(p3, p3.yzx + 33.33);
  return fract((p3.x + p3.y) * p3.z);
}

vec3 SampleHemisphereUniform(inout float s, out float pdf) {
  vec2 uv = Rand2(s);
  float z = uv.x;
  float phi = uv.y * TWO_PI;
  float sinTheta = sqrt(1.0 - z*z);
  vec3 dir = vec3(sinTheta * cos(phi), sinTheta * sin(phi), z);
  pdf = INV_TWO_PI;
  return dir;
}

vec3 SampleHemisphereCos(inout float s, out float pdf) {
  vec2 uv = Rand2(s);
  float z = sqrt(1.0 - uv.x);
  float phi = uv.y * TWO_PI;
  float sinTheta = sqrt(uv.x);
  vec3 dir = vec3(sinTheta * cos(phi), sinTheta * sin(phi), z);
  pdf = z * INV_PI;
  return dir;
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

float Linear01Depth( float z )
{
  float farDivNear = uZBufferParams.y / uZBufferParams.x;
  return 1.0 / ( ( 1.0 - farDivNear) * z + farDivNear);
}

vec4 Project(vec4 a) {
  return a / a.w;
}

float GetDepth(vec3 posWorld) {
  //screen space linearDepth
  float screenZ = Project(vWorldToScreen * vec4(posWorld, 1.0)).z * 0.5 + 0.5;
  screenZ = Linear01Depth(screenZ);
  return screenZ;
}

/*
 * Transform point from world space to screen space([0, 1] x [0, 1])
 *
 */
vec2 GetScreenCoordinate(vec3 posWorld) {
  vec2 uv = Project(vWorldToScreen * vec4(posWorld, 1.0)).xy * 0.5 + 0.5;
  return uv;
}

float GetGBufferDepth(vec2 uv) {
  return Linear01Depth(texture(uGDepth, uv).x);
}

vec3 GetGBufferNormalWorld(vec2 uv) {
  vec3 normal = texture(uGNormalWS, uv).xyz;
  return normal;
}

float GetGBufferuShadow(vec2 uv) {
  float visibility = texture(uGShadow, uv).x;
  return visibility;
}

vec3 GetGBufferDiffuse(vec2 uv) {
  vec3 diffuse = texture(uGDiffuse, uv).xyz;
  diffuse = pow(diffuse, vec3(2.2));
  return diffuse;
}

/*
 * Evaluate diffuse bsdf value.
 *
 * wi, wo are all in world space.
 * uv is in screen space, [0, 1] x [0, 1].
 *
 */
vec3 EvalDiffuse(vec2 uv) {
  vec3 albedo  = GetGBufferDiffuse(uv);
  return albedo * INV_PI;
}

/*
 * Evaluate directional light with shadow map
 * uv is in screen space, [0, 1] x [0, 1].
 *
 */
vec3 EvalDirectionalLight(vec2 uv) {
  vec3 Le = GetGBufferuShadow(uv) * uLightRadiance;
  return Le;
}


bool RayMarch(vec3 ori, vec3 dir, out vec3 hitPos) {
  float step = 0.02;
  const int totalStepTimes = 1000; 
  int curStepTimes = 0;

  vec3 stepDir = normalize(dir) * step;
  vec3 curPos = ori;
  for(int curStepTimes = 0; curStepTimes < totalStepTimes; curStepTimes++)
  {
    float curDepth = GetDepth(curPos);
    vec2 curScreenUV = GetScreenCoordinate(curPos);
    float gBufferDepth = GetGBufferDepth(curScreenUV);

    if(curDepth - gBufferDepth > 0.0001){
      hitPos = curPos;
      return true;
    }
    //o + t * d
    curPos += stepDir;
  }

  return false;
}

// test Screen Space Ray Tracing 
vec3 EvalReflect(vec3 wi, vec3 wo, vec2 uv) {
  vec3 worldNormal = GetGBufferNormalWorld(uv);
  vec3 relfectDir = normalize(reflect(-wo, worldNormal));
  vec3 hitPos;
  if(RayMarch(vPosWorld.xyz, relfectDir, hitPos)){
      vec2 screenUV = GetScreenCoordinate(hitPos);
      return GetGBufferDiffuse(screenUV);
  }
  else{
    return vec3(0.0);
  }
}

#define SAMPLE_NUM 1

void main() {
  float s = InitRand(gl_FragCoord.xy);

  vec3 L = vec3(0.0);

  vec3 worldPos = vPosWorld.xyz;
  vec2 screenUV = GetScreenCoordinate(vPosWorld.xyz);
  vec3 wi = normalize(uLightDir);
  vec3 wo = normalize(uCameraPos - worldPos);

  // //test
  // L = EvalReflect(wi,wo,screenUV);

  //直接光照
  //L = V * Le * brdf * cos / pdf,pdf==1.
  vec3 L_Normal = GetGBufferNormalWorld(screenUV);
  L = EvalDiffuse(screenUV) * EvalDirectionalLight(screenUV) * max(0., dot(L_Normal, wi));

  //间接光
  vec3 L_ind = vec3(0.0);
  for(int i = 0; i < SAMPLE_NUM; i++){
    float pdf;
    vec3 localDir = SampleHemisphereCos(s, pdf);
    vec3 L_ind_Normal = GetGBufferNormalWorld(screenUV);
    vec3 b1, b2;
    LocalBasis(L_ind_Normal, b1, b2);
    vec3 dir = normalize(mat3(b1, b2, L_ind_Normal) * localDir);

    //world space pos
    vec3 hitPos;
    if(RayMarch(worldPos, dir, hitPos)){
      vec2 hitScreenUV = GetScreenCoordinate(hitPos);
      //castRay =  V * Le * brdf * cos / pdf,pdf == 1.
      vec3 hitNormal = GetGBufferNormalWorld(hitScreenUV);
      vec3 castRay = EvalDiffuse(hitScreenUV) * EvalDirectionalLight(hitScreenUV) * max(0., dot(hitNormal, wi));
      //L_ind += castRay * brdf * cos / pdf
      L_ind += castRay * EvalDiffuse(screenUV) * max(0., dot(L_ind_Normal, dir)) / pdf;
    }
  }

  L_ind /= float(SAMPLE_NUM);

  L = L + L_ind;

  vec3 color = pow(clamp(L, vec3(0.0), vec3(1.0)), vec3(1.0 / 2.2));
  FragColor = vec4(vec3(color.rgb), 1.0);
}