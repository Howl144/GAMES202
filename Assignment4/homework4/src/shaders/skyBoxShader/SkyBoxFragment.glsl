#ifdef GL_ES
precision mediump float;
#endif

uniform samplerCube uSkybox;

varying highp vec3 vFragPos;

void main() {
     gl_FragColor = textureCube(uSkybox, vFragPos);
}