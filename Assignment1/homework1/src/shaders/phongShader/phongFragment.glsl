#ifdef GL_ES
precision mediump float;
#endif
// Phong related variables
uniform sampler2D uSampler;
uniform sampler2D uShadowMap;
uniform vec3 uKd;
uniform vec3 uKs;
uniform vec3 uLightPos;
uniform vec3 uCameraPos;
uniform vec3 uLightIntensity;
uniform float uLightFarPlane;
uniform float uLightWorldSize;

varying highp vec2 vTextureCoord;
varying highp vec3 vFragPos;
varying highp vec3 vNormal;

// Shadow map related variables
#define NUM_SAMPLES 32
vec2 staticPoissonDisk[64];


#define SHADOW_MAP_SIZE 2048.
#define TEXEL_SIZE 1.0 / SHADOW_MAP_SIZE


#define EPS 1e-5
#define PI 3.141592653589793
#define PI2 6.283185307179586


varying vec4 vPositionFromLight;
//已经有人过研究眼睛中视锥细胞的分布，与人眼相似，光感受器在猴子眼睛的中央窝外部的分布称之为泊松圆盘分布
//这是一种抗锯齿的采样方法。
void initStaticPoissonDisk() {
    //将staticPoissonDisk进行初始化
    staticPoissonDisk[0] = vec2(-0.5119625, -0.4827938);
    staticPoissonDisk[1] = vec2(-0.2171264, -0.4768726);
    staticPoissonDisk[2] = vec2(-0.7552931, -0.2426507);
    staticPoissonDisk[3] = vec2(-0.7136765, -0.4496614);
    staticPoissonDisk[4] = vec2(-0.5938849, -0.6895654);
    staticPoissonDisk[5] = vec2(-0.3148003, -0.7047654);
    staticPoissonDisk[6] = vec2(-0.42215, -0.2024607);
    staticPoissonDisk[7] = vec2(-0.9466816, -0.2014508);
    staticPoissonDisk[8] = vec2(-0.8409063, -0.03465778);
    staticPoissonDisk[9] = vec2(-0.6517572, -0.07476326);
    staticPoissonDisk[10] = vec2(-0.1041822, -0.02521214);
    staticPoissonDisk[11] = vec2(-0.3042712, -0.02195431);
    staticPoissonDisk[12] = vec2(-0.5082307, 0.1079806);
    staticPoissonDisk[13] = vec2(-0.08429877, -0.2316298);
    staticPoissonDisk[14] = vec2(-0.9879128, 0.1113683);
    staticPoissonDisk[15] = vec2(-0.3859636, 0.3363545);
    staticPoissonDisk[16] = vec2(-0.1925334, 0.1787288);
    staticPoissonDisk[17] = vec2(0.003256182, 0.138135);
    staticPoissonDisk[18] = vec2(-0.8706837, 0.3010679);
    staticPoissonDisk[19] = vec2(-0.6982038, 0.1904326);
    staticPoissonDisk[20] = vec2(0.1975043, 0.2221317);
    staticPoissonDisk[21] = vec2(0.1507788, 0.4204168);
    staticPoissonDisk[22] = vec2(0.3514056, 0.09865579);
    staticPoissonDisk[23] = vec2(0.1558783, -0.08460935);
    staticPoissonDisk[24] = vec2(-0.0684978, 0.4461993);
    staticPoissonDisk[25] = vec2(0.3780522, 0.3478679);
    staticPoissonDisk[26] = vec2(0.3956799, -0.1469177);
    staticPoissonDisk[27] = vec2(0.5838975, 0.1054943);
    staticPoissonDisk[28] = vec2(0.6155105, 0.3245716);
    staticPoissonDisk[29] = vec2(0.3928624, -0.4417621);
    staticPoissonDisk[30] = vec2(0.1749884, -0.4202175);
    staticPoissonDisk[31] = vec2(0.6813727, -0.2424808);
    staticPoissonDisk[32] = vec2(-0.6707711, 0.4912741);
    staticPoissonDisk[33] = vec2(0.0005130528, -0.8058334);
    staticPoissonDisk[34] = vec2(0.02703013, -0.6010728);
    staticPoissonDisk[35] = vec2(-0.1658188, -0.9695674);
    staticPoissonDisk[36] = vec2(0.4060591, -0.7100726);
    staticPoissonDisk[37] = vec2(0.7713396, -0.4713659);
    staticPoissonDisk[38] = vec2(0.573212, -0.51544);
    staticPoissonDisk[39] = vec2(-0.3448896, -0.9046497);
    staticPoissonDisk[40] = vec2(0.1268544, -0.9874692);
    staticPoissonDisk[41] = vec2(0.7418533, -0.6667366);
    staticPoissonDisk[42] = vec2(0.3492522, 0.5924662);
    staticPoissonDisk[43] = vec2(0.5679897, 0.5343465);
    staticPoissonDisk[44] = vec2(0.5663417, 0.7708698);
    staticPoissonDisk[45] = vec2(0.7375497, 0.6691415);
    staticPoissonDisk[46] = vec2(0.2271994, -0.6163502);
    staticPoissonDisk[47] = vec2(0.2312844, 0.8725659);
    staticPoissonDisk[48] = vec2(0.4216993, 0.9002838);
    staticPoissonDisk[49] = vec2(0.4262091, -0.9013284);
    staticPoissonDisk[50] = vec2(0.2001408, -0.808381);
    staticPoissonDisk[51] = vec2(0.149394, 0.6650763);
    staticPoissonDisk[52] = vec2(-0.09640376, 0.9843736);
    staticPoissonDisk[53] = vec2(0.7682328, -0.07273844);
    staticPoissonDisk[54] = vec2(0.04146584, 0.8313184);
    staticPoissonDisk[55] = vec2(0.9705266, -0.1143304);
    staticPoissonDisk[56] = vec2(0.9670017, 0.1293385);
    staticPoissonDisk[57] = vec2(0.9015037, -0.3306949);
    staticPoissonDisk[58] = vec2(-0.5085648, 0.7534177);
    staticPoissonDisk[59] = vec2(0.9055501, 0.3758393);
    staticPoissonDisk[60] = vec2(0.7599946, 0.1809109);
    staticPoissonDisk[61] = vec2(-0.2483695, 0.7942952);
    staticPoissonDisk[62] = vec2(-0.4241052, 0.5581087);
    staticPoissonDisk[63] = vec2(-0.1020106, 0.6724468);
}

float rand(vec2 co){
    return fract(sin(dot(co, vec2(12.9898,78.233))) * 43758.5453);
}

vec2 randomRotation(vec2 center, vec2 poissonVector) {
    float angle = rand(center) * 2.0 * 3.14159265; //生成一个在[0, 2*PI]范围内的随机角度
    mat2 rotationMatrix = mat2(cos(angle), -sin(angle), sin(angle), cos(angle));
    return rotationMatrix * poissonVector;
}

//颜色值RGBA -> 深度值float
float unpack(vec4 rgbaDepth) {
  const vec4 bitShift = vec4(1.0, 1.0/256.0, 1.0/(256.0*256.0), 1.0/(256.0*256.0*256.0));
  return dot(rgbaDepth, bitShift);
}

float getShadowBias(float biasC){
  vec3 normal = normalize(vNormal);
  vec3 lightDir = normalize(uLightPos - vFragPos);
  return max(biasC * (1.0 - dot(normal, lightDir)), 1e-5);
}



float useShadowMap(sampler2D shadowMap, vec4 shadowCoord, float biasC){
  float depth = unpack(texture2D(shadowMap, shadowCoord.xy));
  float cur_depth = shadowCoord.z;
  float bias = getShadowBias(biasC);
  if(cur_depth > depth + bias){
    //不可见
    return 0.0;
  }
  else{
    //可见
    return 1.0;
  }
}



float PCF(sampler2D shadowMap, vec4 coords, float biasC, float filterRadius) {
  float visibility = 0.0;
  for(int i = 0; i < NUM_SAMPLES; i++){
    vec2 poissonDisk = staticPoissonDisk[i];
    // 应用随机旋转
    vec2 rotatedPoissonDisk = randomRotation(coords.xy, poissonDisk) * filterRadius * TEXEL_SIZE; 
    visibility += useShadowMap(shadowMap, coords + vec4(rotatedPoissonDisk, 0., 0.) , biasC);
  }
  return visibility / float(NUM_SAMPLES);
}


float findBlocker(sampler2D shadowMap, vec4 coords, float zReceiver, float calibratedLightSize) {
  int blockerNum = 0;
  float avgBlockDepth = 0.;
  float blockerDepth = unpack(texture2D(shadowMap, coords.xy));
  float searchRadius = 0.0;
  //blockerDepth和zReceiver大致相等时，固定search范围
  if(abs(blockerDepth - zReceiver) < 3.0 * TEXEL_SIZE){
    searchRadius = calibratedLightSize;
    // return searchRadius = 1.0;//debug
  }else{
    searchRadius = calibratedLightSize * (zReceiver - blockerDepth) / zReceiver;
    // return searchRadius = 0.0;//debug
  }

  for(int i = 0; i < NUM_SAMPLES; i++){
    vec2 poissonDisk = staticPoissonDisk[i];
    vec2 rotatedPoissonDisk = randomRotation(coords.xy, poissonDisk); // 应用随机旋转
    float shadowDepth = unpack(texture2D(shadowMap, coords.xy + rotatedPoissonDisk * searchRadius * TEXEL_SIZE));
    if(zReceiver > shadowDepth){
      blockerNum++;
      avgBlockDepth += shadowDepth;
    }
  }

  if(blockerNum == 0)
    return -1.0;
  else
    //avgBlockerDepth，是有blocker的地方才算它的depth，所有这里不是除以NUM_SAMPLES
    return avgBlockDepth / float(blockerNum);
}


float PCSS(sampler2D shadowMap, vec4 coords, float biasC,float calibratedLightSize){
  float zReceiver = coords.z;

  // STEP 1: avgblocker depth 
  float avgBlockerDepth = findBlocker(shadowMap, coords, zReceiver,calibratedLightSize);

  if(avgBlockerDepth == -1.0)
    return 1.0;

  // STEP 2: penumbra size
  float penumbra = (zReceiver - avgBlockerDepth) * calibratedLightSize / avgBlockerDepth;
  float filterRadius = penumbra;

  // STEP 3: filtering
  return PCF(shadowMap, coords, biasC, filterRadius);
}


vec3 blinnPhong() {
  
  vec3 color = texture2D(uSampler, vTextureCoord).rgb;
  color = pow(color, vec3(2.2));

  float distance = length(uLightPos - vFragPos);
  vec3 attenuation = uLightIntensity / (1.0 + 0.045 * distance + 0.0075 * (distance * distance)); 

  vec3 ambient = 0.01 * color;

  vec3 lightDir = normalize(uLightPos);
  vec3 normal = normalize(vNormal);
  float diff = max(dot(lightDir, normal), 0.0);
  vec3 diffuse = diff * color * attenuation;

  vec3 viewDir = normalize(uCameraPos - vFragPos);
  vec3 halfDir = normalize((lightDir + viewDir));
  float spec = pow(max(dot(halfDir, normal), 0.0), 32.0);
  vec3 specular = uKs  * spec * attenuation;

  vec3 radiance = (ambient + diffuse + specular);
  vec3 phongColor = pow(radiance, vec3(1.0 / 2.2));
  return phongColor;
}
//gl_FragCoord.z
float LinearizeDepth(float depth) 
{
    float near = 0.01;
    float far = uLightFarPlane;
    float z = depth * 2.0 - 1.0; // back to NDC 
    return (2.0 * near * far) / (far + near - z * (far - near));    
}

void main(void) {
  initStaticPoissonDisk();
  //vPositionFromLight为光源空间下的裁剪空间坐标，除以w结果为NDC坐标
  vec3 shadowCoord = vPositionFromLight.xyz / vPositionFromLight.w;
  //把[-1,1]的NDC坐标转换为[0,1]的坐标
  shadowCoord.xyz = (shadowCoord.xyz + 1.0) / 2.0;
  //手动计算深度值
  float fragDepth = length(vFragPos-uLightPos) / uLightFarPlane;

  float visibility = 0.0;

  // Shadow Bias
  float biasC = 0.002;
  // Light Size
  float calibratedLightSize = 100.0;//clamp(uLightWorldSize, 10.0, 100.0)
  //点光源没有使用cubemap，将就限制一下范围
  if(shadowCoord.x < 0.0 || shadowCoord.x > 1.0  || shadowCoord.y < 0.0 || shadowCoord.y > 1.0 || shadowCoord.z < 0.0 || shadowCoord.z > 1.0){
    //超出平截头体范围的值直接返回1.0
    visibility = 1.0;
  }
  else{
    // 硬阴影
    // visibility = useShadowMap(uShadowMap, vec4(shadowCoord.xy,fragDepth, 1.0), biasC);
    // PCF
    // visibility = PCF(uShadowMap, vec4(shadowCoord.xy,fragDepth, 1.0), biasC, calibratedLightSize);
    // PCSS
    visibility = PCSS(uShadowMap, vec4(shadowCoord.xy,fragDepth, 1.0), biasC,calibratedLightSize);
  }

  vec3 phongColor = blinnPhong();

  gl_FragColor = vec4(phongColor * visibility, 1.0);

  // float debug1 = findBlocker(uShadowMap, vec4(shadowCoord.xy,fragDepth, 1.0), fragDepth, calibratedLightSize);
  // gl_FragColor = vec4(vec3(debug1),1.0);
}