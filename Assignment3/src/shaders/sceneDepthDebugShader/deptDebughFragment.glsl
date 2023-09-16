#version 300 es
#ifdef GL_ES
precision highp float;
#endif
uniform sampler2D uDepthTexture;
uniform vec3 uZBufferParams;
in vec2 vTextureCoord;
out vec4 FragColor;
float Linear01Depth( float z )
{
  float farDivNear = uZBufferParams.y / uZBufferParams.x;
  return 1.0 / ( ( 1.0 - farDivNear) * z + farDivNear);
}
void main() {
  vec3 depth = vec3(Linear01Depth(texture(uDepthTexture,vTextureCoord).r));
  vec3 color = pow(clamp(depth, vec3(0.0), vec3(1.0)), vec3(1.0 / 2.2));
  FragColor = vec4(vec3(color.rgb), 1.0);
}
