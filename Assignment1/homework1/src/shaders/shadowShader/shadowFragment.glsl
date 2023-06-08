#ifdef GL_ES
precision mediump float;
#endif
uniform vec3 uLightPos;
uniform float uLightFarPlane;

varying highp vec3 worldPos;

//深度值float -> 颜色值RGBA ，只能存储0-1之间的float值
vec4 pack (float depth) {
    // 使用rgba 4字节共32位来存储z值,1个字节精度为1/256
    const vec4 bitShift = vec4(1.0, 256.0, 256.0 * 256.0, 256.0 * 256.0 * 256.0);
    const vec4 bitMask = vec4(1.0/256.0, 1.0/256.0, 1.0/256.0, 0.0);
    // gl_FragCoord:片元的坐标,fract():返回数值的小数部分
    vec4 rgbaDepth = fract(depth * bitShift); //32位的深度值分成4个8位存储
    rgbaDepth -= rgbaDepth.gbaa * bitMask; // 每个8位由于精度问题，无法表示后面的8位，所以减去后面的8位，否则会整体值会偏大。
    return rgbaDepth;
}

void main(){
  float lightDistance = length(worldPos - uLightPos);
  float fragDepth = lightDistance / uLightFarPlane;
  gl_FragColor = pack(fragDepth);
  //正交投影直接赋值
  // gl_FragColor = pack(gl_FragCoord.z);
}