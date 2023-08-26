attribute vec3 aVertexPosition;
attribute vec3 aNormalPosition;
attribute vec2 aTextureCoord;

uniform mat4 uModelMatrix;
uniform mat4 uViewMatrix;
uniform mat4 uProjectionMatrix;


varying highp vec3 vFragPos;

void main() {
    vec3 fragPos = aVertexPosition;
    //Picture ambient light needs to flip y
    // fragPos.y = -fragPos.y;
    vFragPos = normalize(fragPos);
    mat4 viewMatrix = uViewMatrix;
    viewMatrix = mat4(mat3(viewMatrix));
    vec4 clipPos = uProjectionMatrix * viewMatrix * uModelMatrix * vec4(aVertexPosition, 1.0);
    gl_Position = clipPos.xyww; 
}