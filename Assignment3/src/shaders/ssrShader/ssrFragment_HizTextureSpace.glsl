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

uniform sampler2D uDepthTexture[10];

in mat4 vWorldToScreen;
in vec4 vPosWorld;
uniform mat4 uViewMatrix;
uniform mat4 uProjectionMatrix;
uniform mat4 uInvProjMat;
uniform mat4 uTransInvViewMat;
float windowWidth = 1920.0;
float windowHeight = 1080.0;


#define M_PI 3.1415926535897932384626433832795
#define TWO_PI 6.283185307
#define INV_PI 0.31830988618
#define INV_TWO_PI 0.15915494309

#define MAX_MIPMAP_LEVEL 9
#define MAX_THICKNESS 0.0017

out vec4 FragColor;

struct RayStartInfo {
  vec4 samplePosInCS;
  vec4 samplePosInVS;
  vec3 samplePosInTS;
  vec3 sampleNormalInVS;
  vec3 sampleNormalInWS;
  vec3 debugRay;
};

struct SSRay {
  vec3 rayPosInTS;
  vec3 rayDirInTS;
  vec3 rayPosInVS;
  vec3 rayDirInVS;
  float maxDistance;
  float minDistance;
  vec3 debugRay;
};

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
/*
 * Transform point from world space to screen space([0, 1] x [0, 1])
 *
 */
vec2 GetScreenCoordinate(vec3 posWorld) {
  vec2 uv = Project(vWorldToScreen * vec4(posWorld, 1.0)).xy * 0.5 + 0.5;
  return uv;
}

vec3 GetScreenCoordinate3(vec3 posWorld) {
  vec3 screenUVZ = Project(vWorldToScreen * vec4(posWorld, 1.0)).xyz * 0.5 + 0.5;
  return screenUVZ;
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

ivec2 getCellCount(int level){
  if(level == 0){
    return textureSize(uDepthTexture[0], 0);
  }
  else if(level == 1){
    return textureSize(uDepthTexture[1], 0);
  }
  else if(level == 2){
    return textureSize(uDepthTexture[2], 0);
  }
    else if(level == 3){
    return textureSize(uDepthTexture[3], 0);
  }
    else if(level == 4){
    return textureSize(uDepthTexture[4], 0);
  }
    else if(level == 5){
    return textureSize(uDepthTexture[5], 0);
  }
    else if(level == 6){
    return textureSize(uDepthTexture[6], 0);
  }
    else if(level == 7){
    return textureSize(uDepthTexture[7], 0);
  }
    else if(level == 8){
    return textureSize(uDepthTexture[8], 0);
  }
    else if(level == 9){
    return textureSize(uDepthTexture[9], 0);
  }
  return textureSize(uDepthTexture[0], 0);
}


float getMinimumDepthPlane(vec2 pos, int level){
  vec2 cellCount = vec2(getCellCount(level));
  ivec2 cell = ivec2(floor(pos * cellCount));

  if(level == 0){
    return texelFetch(uDepthTexture[0], cell, 0).x;
  }
  else if(level == 1){
    return texelFetch(uDepthTexture[1], cell, 0).x;
  }
  else if(level == 2){
    return texelFetch(uDepthTexture[2], cell, 0).x;
  }
    else if(level == 3){
    return texelFetch(uDepthTexture[3], cell, 0).x;
  }
    else if(level == 4){
    return texelFetch(uDepthTexture[4], cell, 0).x;
  }
    else if(level == 5){
    return texelFetch(uDepthTexture[5], cell, 0).x;
  }
    else if(level == 6){
    return texelFetch(uDepthTexture[6], cell, 0).x;
  }
    else if(level == 7){
    return texelFetch(uDepthTexture[7], cell, 0).x;
  }
    else if(level == 8){
    return texelFetch(uDepthTexture[8], cell, 0).x;
  }
    else if(level == 9){
    return texelFetch(uDepthTexture[9], cell, 0).x;
  }
  return texelFetch(uDepthTexture[0], cell, 0).x;
}

bool crossedCellBoundary(vec2 oldCellIdx,vec2 newCellIdx){
    return (floor(oldCellIdx.x) != floor(newCellIdx.x) )
        || (floor(oldCellIdx.y) != floor(newCellIdx.y) );
}

float DepthBack2Ndc(float screenSpaceDepth){
  return screenSpaceDepth * 2.0 - 1.0;
}
bool unpackVSInfo(
  in vec2 tid,
  out RayStartInfo rayStartInfo) {
  // Unpack the depth from the texture.
  float sampleDepth = DepthBack2Ndc(texelFetch(uGDepth, ivec2(tid), 0).x);
  if(sampleDepth == 1.0) return false;

  // Unpack the normal from the texture.
  rayStartInfo.sampleNormalInWS = texelFetch(uGNormalWS, ivec2(tid), 0).xyz;
  rayStartInfo.sampleNormalInVS = normalize((uTransInvViewMat * vec4(rayStartInfo.sampleNormalInWS, 0)).xyz);
  
  // From the depth, compute the position in clip space.
  rayStartInfo.samplePosInCS =  vec4((vec2(tid) / vec2(windowWidth,windowHeight) )*2.0-1.0, sampleDepth, 1.0);
  // From the depth, compute the position in view space.
  rayStartInfo.samplePosInVS = uInvProjMat * rayStartInfo.samplePosInCS;
  rayStartInfo.samplePosInVS /= rayStartInfo.samplePosInVS.w;

  // Texture space
  rayStartInfo.samplePosInTS = (rayStartInfo.samplePosInCS.xyz) * 0.5 + 0.5;
  return true;
}

SSRay PrepareSSRT(
  in RayStartInfo startInfo,
  in vec3 reflDirInVS
) {
  vec4 reflectionInVS = vec4(reflDirInVS,0);
  // Compute the reflection direction in clip space.
  vec4 reflectionEndPosInVS = startInfo.samplePosInVS + reflectionInVS * 1.0;
  
  vec4 reflectionEndPosInCS = uProjectionMatrix * vec4(reflectionEndPosInVS.xyz, 1);
  reflectionEndPosInCS /= reflectionEndPosInCS.w;
  // Transform to texture space  
  vec3 outReflDirInTS = normalize(((reflectionEndPosInCS * 0.5) - (startInfo.samplePosInCS * 0.5)).xyz );
  // Compute the maximum distance to trace before the ray goes outside of the visible area.
  float outMaxDistance = outReflDirInTS.x >= 0.0 ? (1.0 - startInfo.samplePosInTS.x) / outReflDirInTS.x  : -startInfo.samplePosInTS.x / outReflDirInTS.x;
  outMaxDistance = min(outMaxDistance, outReflDirInTS.y < 0.0 ? (-startInfo.samplePosInTS.y / outReflDirInTS.y) : ( (1.0-startInfo.samplePosInTS.y) / outReflDirInTS.y));
  outMaxDistance = min(outMaxDistance, outReflDirInTS.z < 0.0 ? (-startInfo.samplePosInTS.z / outReflDirInTS.z) : ((1.0-startInfo.samplePosInTS.z)/outReflDirInTS.z));
  SSRay ray;
  //ray.rayPosInVS =  startInfo.samplePosInVS.xyz;
  //ray.rayDirInVS = normalize(reflectionEndPosInVS.xyz - startInfo.samplePosInVS.xyz);
  ray.rayPosInTS = startInfo.samplePosInTS;
  ray.rayDirInTS = outReflDirInTS;
  ray.maxDistance = outMaxDistance;
  //ray.minDistance = 0.0;
  return ray;
}

vec2 getCell(vec2 pos, vec2 startCellCount){
  return floor( pos * startCellCount );
}

vec3 intersectDepthPlane(in vec3 o, in vec3 d, float t){
  return o + d * t;
}

vec3 intersectCellBoundary(
  vec3 o, 
  vec3 d, 
  vec2 rayCell, 
  vec2 cell_count, 
  vec2 crossStep, 
  vec2 crossOffset
){
  vec2 index = rayCell + crossStep;
  vec2 boundary = index / cell_count;
  boundary += crossOffset;
  vec2 delta = boundary - o.xy;
  delta /= d.xy;
  float t = min(delta.x, delta.y);
  return intersectDepthPlane(o, d, t);
}

vec2 saturate(vec2 v) { return clamp(v, 0.0, 1.0); }

bool FindIntersection_HiZ(
  in SSRay ss_ray,
  out vec3 intersection
) {
  vec3 start = ss_ray.rayPosInTS;
  vec3 rayDir = ss_ray.rayDirInTS;
  float maxTraceDistance = ss_ray.maxDistance;

  vec2 crossStep = vec2(rayDir.x >= 0.0 ? 1.0 : -1.0, rayDir.y >= 0.0 ? 1.0 : -1.0);
  vec2 crossOffset = crossStep / vec2(windowWidth,windowHeight) / 128.;
  crossStep = saturate(crossStep);

  vec3 ray = start;
  float minZ = ray.z;
  float maxZ = ray.z + rayDir.z * maxTraceDistance;
  float deltaZ = (maxZ - minZ);

  vec3 o = ray;
  vec3 d = rayDir * maxTraceDistance;

  int startLevel = 2;
  int stopLevel = 0;

  vec2 startCellCount = vec2(getCellCount(startLevel));
  vec2 rayCell = getCell(ray.xy, startCellCount);
  ray = intersectCellBoundary(o, d, rayCell, startCellCount, crossStep, crossOffset * 128. * 2. );

  int level = startLevel;
  int iter = 0;
  bool isBackwardRay = rayDir.z < 0.;
  float Dir = isBackwardRay ? -1. : 1.;
  while( level >= stopLevel && ray.z * Dir <= maxZ * Dir && ++iter < 1000){
    vec2 cellCount = vec2(getCellCount(level));
    vec2 oldCellIdx = getCell(ray.xy, cellCount);
    float cell_minZ = getMinimumDepthPlane((oldCellIdx + 0.5) / cellCount, level);
    // if(cell_minZ == 1.0) return false;//解决SSR测试的问题
    vec3 tmpRay = ((cell_minZ > ray.z) && !isBackwardRay) ? intersectDepthPlane(o, d, (cell_minZ - minZ) / deltaZ) : ray;
    vec2 newCellIdx = getCell(tmpRay.xy, cellCount);
    float thickness = level == 0 ? (ray.z - cell_minZ) : 0.;
    bool crossed  = (isBackwardRay && (cell_minZ > ray.z))||(thickness > MAX_THICKNESS)|| crossedCellBoundary(oldCellIdx, newCellIdx);
    ray = crossed ? intersectCellBoundary(o, d, oldCellIdx, cellCount, crossStep, crossOffset) : tmpRay;
    level = crossed ? min(MAX_MIPMAP_LEVEL, level + 1): level - 1;
  }
  bool intersected = (level < stopLevel);
  intersection = intersected ? ray : vec3(0.0);
  return intersected;
}

vec3 projectToViewSpace(vec3 point)
{
	return vec3(uViewMatrix * vec4(point,1.0));
}

vec4 projectToClipSpace(vec3 point)
{
	return uProjectionMatrix * vec4(point,1.0);
}
// test Screen Space Ray Tracing 
vec3 EvalReflect(vec3 wo,vec2 screenUV) {
  // //从Gbufer中恢复TS中的原点和反射方向
  // vec2 tid = gl_FragCoord.xy;
  // RayStartInfo startInfo;
  // bool hasIsect = unpackVSInfo(tid, startInfo);
  // if(!hasIsect) {
  //     return vec3(0.);
  // }
  // vec3 camToSampleInVS = normalize(startInfo.samplePosInVS.xyz);
  // vec3 refDirInVS = normalize(reflect(camToSampleInVS, startInfo.sampleNormalInVS));
  // SSRay ray = PrepareSSRT(startInfo, refDirInVS);

  //从世界坐标中恢复Ts中的原点和反射方向 速度稍微快点点
  vec3 normalWS = GetGBufferNormalWorld(screenUV);
  vec3 reflectDir = normalize(reflect(-wo,normalWS));
  vec3 rayOriginWS = vPosWorld.xyz;
  vec3 rayEndWS = rayOriginWS + reflectDir * 1.0;
  vec3 rayOriginVS = projectToViewSpace(rayOriginWS);
  vec3 rayEndVS    = projectToViewSpace(rayEndWS);
  vec4 rayOriginCS = projectToClipSpace(rayOriginVS);
  vec4 rayEndCS    = projectToClipSpace(rayEndVS);
  rayOriginCS = rayOriginCS.xyzw / rayOriginCS.w;
  rayEndCS    = rayEndCS.xyzw / rayEndCS.w;
  vec3 rayOriginTS = (vec3(rayOriginCS) + 1.0) * 0.5;
  vec3 rayEndTS = (vec3(rayEndCS) + 1.0) * 0.5;
  vec3 reflectDirTS = normalize(rayEndTS - rayOriginTS);
  float outMaxDistance = reflectDirTS.x >= 0.0 ? (1.0 - rayOriginTS.x) / reflectDirTS.x  : -rayOriginTS.x / reflectDirTS.x;
  outMaxDistance = min(outMaxDistance, reflectDirTS.y < 0.0 ? (-rayOriginTS.y / reflectDirTS.y) : ( (1.0-rayOriginTS.y) / reflectDirTS.y));
  outMaxDistance = min(outMaxDistance, reflectDirTS.z < 0.0 ? (-rayOriginTS.z / reflectDirTS.z) : ((1.0-rayOriginTS.z)/reflectDirTS.z));
  SSRay ray;
  ray.rayPosInTS = rayOriginTS;
  ray.rayDirInTS = reflectDirTS;
  ray.maxDistance = outMaxDistance;

  vec3 hitPos;
  vec3 reflectedColor = vec3(0.);
  if(FindIntersection_HiZ(ray, hitPos)) {
    reflectedColor = GetGBufferDiffuse(hitPos.xy);
  }
  return reflectedColor;
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
  // L = EvalReflect(wo,screenUV);

  //直接光照
  //L = V * Le * brdf * cos / pdf,pdf==1.
  vec3 normal = GetGBufferNormalWorld(screenUV);
  L = EvalDiffuse(screenUV) * EvalDirectionalLight(screenUV) * max(0., dot(normal, wi));

  //间接光
  vec3 L_ind = vec3(0.0);
  for(int i = 0; i < SAMPLE_NUM; i++){
    float pdf;
    vec3 localDir = SampleHemisphereCos(s, pdf);
    vec3 b1, b2;
    LocalBasis(normal, b1, b2);
    vec3 refDirInWS = normalize(mat3(b1, b2, normal) * localDir);//ssgi
    vec3 refDirInVS = normalize((uTransInvViewMat * vec4(refDirInWS, 0)).xyz);

    vec2 tid = gl_FragCoord.xy;
    RayStartInfo startInfo;
    bool hasIsect = unpackVSInfo(tid, startInfo);
    if(!hasIsect) {
      break;
    }

    SSRay ray = PrepareSSRT(startInfo, refDirInVS);

    vec3 hitPos;
    if(FindIntersection_HiZ(ray, hitPos)){
      //castRay =  V * Le * brdf * cos / pdf,pdf == 1.
      vec3 hitNormal = GetGBufferNormalWorld(hitPos.xy);
      vec3 castRay = EvalDiffuse(hitPos.xy) * EvalDirectionalLight(hitPos.xy) * max(0., dot(hitNormal, wi));
      //L_ind += castRay * brdf * cos / pdf
      L_ind += castRay * EvalDiffuse(screenUV) * max(0., dot(normal, refDirInWS)) / pdf;
    }
  }
  L_ind /= float(SAMPLE_NUM);
  L = L + L_ind;

  vec3 color = pow(L, vec3(1.0 / 2.2));
  FragColor = vec4(vec3(color.rgb), 1.0);
}