#version 300 es

layout (location = 0) in vec3 aVertexPosition;
layout (location = 1) in vec3 aNormalPosition;
layout (location = 2) in vec2 aTextureCoord;

uniform mat4 uLightVP;
uniform mat4 uModelMatrix;
uniform mat4 uViewMatrix;
uniform mat4 uProjectionMatrix;

out mat4 vWorldToLight;

out vec2 vTextureCoord;
out vec3 vNormalWS;
out vec4 vPosWorld;

void main(void) {
  vPosWorld = uModelMatrix * vec4(aVertexPosition, 1.0);
  vNormalWS = mat3(uModelMatrix) * aNormalPosition;
  vTextureCoord = aTextureCoord;
  vWorldToLight = uLightVP;

  gl_Position = uProjectionMatrix * uViewMatrix * uModelMatrix * vec4(aVertexPosition, 1.0);
}