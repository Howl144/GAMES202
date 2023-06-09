attribute vec3 aVertexPosition;
attribute vec3 aNormalPosition;
attribute vec2 aTextureCoord;


uniform mat4 uLightMVP;
uniform mat4 uLightModel;

varying highp vec3 worldPos;

void main(void) {
  worldPos = (uLightModel * vec4(aVertexPosition, 1.0)).xyz;
  gl_Position = uLightMVP * vec4(aVertexPosition, 1.0);
}