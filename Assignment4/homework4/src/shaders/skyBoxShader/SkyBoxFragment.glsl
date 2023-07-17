#ifdef GL_ES
precision mediump float;
#endif

uniform samplerCube skybox;

varying highp vec2 vTextureCoord;
varying highp vec3 vFragPos;
varying highp vec3 vNormal;

void main() {
     gl_FragColor = textureCube(skybox, vFragPos);
}