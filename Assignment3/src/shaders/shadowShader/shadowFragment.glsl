#version 300 es

#ifdef GL_ES
// #extension GL_EXT_draw_buffers: enable
precision highp float;
#endif

layout(location = 0) out vec4 Frag0;

uniform vec3 uCameraPos;

in vec3 vNormal;
in vec2 vTextureCoord;
in float vDepth;

vec4 EncodeFloatRGBA(float v) {
  vec4 enc = vec4(1.0, 255.0, 65025.0, 160581375.0) * v;
  enc = fract(enc);
  enc -= enc.yzww * vec4(1.0/255.0,1.0/255.0,1.0/255.0,0.0);
  return enc;
}

void main(){
  //vDepth  = gl_FragCoord.z * 2.0 - 1.0;
  //you can also use gl_FragCoord that contains the window-relative coordinates of the current fragment
  Frag0 = vec4(vec3(vDepth), 1.0);
}