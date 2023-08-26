#version  300 es

layout (location = 0) in vec3 aVertexPosition;
layout (location = 1) in vec3 aNormalPosition;
layout (location = 2) in vec2 aTextureCoord;

out vec2 vTexCoords;
out vec3 vWorldPos;
out vec3 vNormal;

uniform mat4 uProjectionMatrix;
uniform mat4 uViewMatrix;
uniform mat4 uModelMatrix;

void main()
{
    vTexCoords = aTextureCoord;
    vWorldPos = vec3(uModelMatrix * vec4(aVertexPosition, 1.0));
    vNormal = transpose(inverse(mat3(uModelMatrix))) * aNormalPosition;   

    gl_Position =  uProjectionMatrix * uViewMatrix * vec4(vWorldPos, 1.0);
}