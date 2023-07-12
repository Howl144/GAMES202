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

uniform mat4 uViewMatrix;
uniform mat4 uProjectionMatrix;
in vec4 vPosWorld;
float windowWidth = 1920.0;
float windowHeight = 1080.0;
float maxDistance = 100.0;

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

vec4 Project(vec4 a) {
  return a / a.w;
}

/*
 * Transform point from world space to screen space([0, 1] x [0, 1])
 *
 */
vec2 GetScreenCoordinate(vec3 posWorld) {
  vec2 uv = Project(uProjectionMatrix * uViewMatrix * vec4(posWorld, 1.0)).xy * 0.5 + 0.5;
  return uv;
}

vec3 VS2SS(vec3 posView) {
  vec3 uvz = Project(uProjectionMatrix * vec4(posView, 1.0)).xyz * 0.5 + 0.5;
  return uvz;
}


float GetGBufferDepth(vec2 uv) {
  return texture(uGDepth, uv).x;
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

float LinearizeDepth(float vDepth){
  float u_Near = uZBufferParams.x;
  float u_Far = uZBufferParams.y;
  float z = vDepth * 2.0 - 1.0; 
  return (2.0 * u_Near * u_Far) / (u_Far + u_Near - z * (u_Far - u_Near)); 
}

struct Ray
{
  vec3 Origin;
  vec3 Direction;
};

struct Result
{
  bool IsHit;

  vec2 UV;
  vec3 Position;

  int IterationCount;
};

vec4 projectToScreenSpace(vec3 point)
{
	return uProjectionMatrix * vec4(point,1.0);
}

vec3 projectToViewSpace(vec3 point)
{
	return vec3(uViewMatrix * vec4(point,1.0));
}

float distanceSquared(vec2 A, vec2 B) 
{
  A -= B;
  return dot(A, A);
}
bool Query(float depth, vec2 uv)
{
  float depth1 = -LinearizeDepth(texelFetch(uGDepth,ivec2(uv),0).r);
  return depth < depth1;
}

//DDA,数值方法求解微分方程的算法
Result RayMarching(Ray ray)
{
	Result result;

  //endPos 不能超出近平面，否则反射出的颜色是错误的。超过的话，origin到近平面的最短距离 = origin沿dir到近平面的距离 * cosθ，cosθ = dir.z / 1.0;
  float rayLength = ((ray.Origin.z + ray.Direction.z * maxDistance) > -uZBufferParams.x) ?
  (-uZBufferParams.x - ray.Origin.z) / ray.Direction.z : maxDistance;

	vec3 V0 = ray.Origin;
	vec3 V1 = ray.Origin + ray.Direction * rayLength;
  //将viewSpace下的origin和endPos转到screenSpace
	vec4 H0 = projectToScreenSpace(V0);
	vec4 H1 = projectToScreenSpace(V1);

	float k0 = 1.0 / H0.w;
  float k1 = 1.0 / H1.w;
  //Rasterizing a 3D line segment is very similar to rasterizing a 2D one. Rasterization can trivially interpolate any property linearly along the iteration direction
  //. If the property is in homogeneous space, then the result will be linear interpolation of the corresponding value in 3D. Specifically, let point Q be a 3D point on the ray and H = M ·(Q,1)be its homogeneous perspective projection by matrix M. Properties k =1/(H ·(0,0,0,1))and Q·k interpolate linearly in 2D. So, treating the point and reciprocal of the homogeneous w as functions along the 2D line, ∂(Q·k) / ∂x , ∂k / ∂x , ∂(Q·k) / ∂y , and ∂k / ∂y are constant in screen space. 
	vec3 Q0 = V0 * k0; 
  vec3 Q1 = V1 * k1;

	// NDC-space
  vec2 P0 = H0.xy * k0;
  vec2 P1 = H1.xy * k1;
	vec2 Size = vec2(windowWidth,windowHeight);
	//Screen Space
	P0 = (P0 + 1.0) / 2.0 * Size;
	P1 = (P1 + 1.0) / 2.0 * Size;

  //opt
	// P1 += vec2((distanceSquared(P0, P1) < 0.0001) ? 0.01 : 0.0);

	vec2 Delta = P1 - P0;

  //是否重新排序，我们只处理Delta较大情况
	bool Permute = false;
  if (abs(Delta.x) < abs(Delta.y)) { 
      Permute = true;
      Delta = Delta.yx; P0 = P0.yx; P1 = P1.yx; 
  }
	float StepDir = sign(Delta.x);
  float Invdx = StepDir / Delta.x;
	
  
  //偏导函数的数值表示法
  vec3  dQ = (Q1 - Q0) * Invdx;
  float dk = (k1 - k0) * Invdx;
  vec2  dP = vec2(StepDir, Delta.y * Invdx);
	float stride = 2.0;
  dP *= stride; dQ *= stride; dk *= stride;
  float jitter = 1.0;
  P0 += dP * jitter; Q0 += dQ * jitter; k0 += dk * jitter;
	
	float Step = 0.0;
	float MaxStep = 1000.0;
	float EndX = P1.x * StepDir;

	float k = k0;
	vec3 Q = Q0;
  vec2 P = P0;
  //x + δx,y + δy,Q.z + δQ.z ...，δx = StepDir。
  //δy = δx * Delta.y * Invdx，同理Q.z 和 k。
	for(;((P.x * StepDir) <= EndX) && 
      Step < MaxStep;
      Step+=1.0,P += dP, Q.z += dQ.z, k += dk)
	{
		result.UV = Permute ? P.yx : P;
		float depth;
    //At any 2D point (x,y), the corresponding 3D point is Q0(x,y)=(Q·k)(x,y) / k(x,y)，Q0(z)=(Q·k)(z) / k(z)
		depth = Q.z / k;
		if(result.UV.x > windowWidth || result.UV.x < 0.0 || result.UV.y > windowHeight || result.UV.y < 0.0)
			break;
		result.IsHit = Query(depth, result.UV);
		if (result.IsHit && Step > 1.0)
			break;
	}
	return result;
}

vec3 EvalReflect(vec3 wo,vec2 screenUV) {
  vec3 normalWS = GetGBufferNormalWorld(screenUV);
  vec3 reflectDir = normalize(reflect(-wo,normalWS));
  vec3 rayOriginWS = vPosWorld.xyz;
  vec3 rayEndWS = rayOriginWS + reflectDir * 1.0;
  vec3 rayOriginVS = projectToViewSpace(rayOriginWS);
  vec3 rayEndVS    = projectToViewSpace(rayEndWS);
  vec3 reflectDirVS = normalize(rayEndVS - rayOriginVS);

  // vec3 rayOriginSS = VS2SS(rayOriginVS);
  // vec3 rayEndSS    = VS2SS(rayEndVS);

  Ray ray;
  ray.Origin = rayOriginVS;
  ray.Direction = reflectDirVS;

  Result result = RayMarching(ray);
  if(result.IsHit){
    return GetGBufferDiffuse(result.UV / vec2(windowWidth,windowHeight)).xyz;
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
  // L = EvalReflect(wo,screenUV);


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

    vec3 rayOriginWS = vPosWorld.xyz;
    vec3 rayEndWS    = rayOriginWS + dir * 1.0;
    vec3 rayOriginVS = projectToViewSpace(rayOriginWS);
    vec3 rayEndVS    = projectToViewSpace(rayEndWS);
    vec3 reflectDirVS = normalize(rayEndVS - rayOriginVS);
    Ray ray;
    ray.Origin = rayOriginVS;
    ray.Direction = reflectDirVS;

    Result result = RayMarching(ray);
    if(result.IsHit){
      vec2 hitScreenUV = result.UV / vec2(windowWidth,windowHeight);
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