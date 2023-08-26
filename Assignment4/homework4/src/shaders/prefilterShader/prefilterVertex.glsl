#version 300 es
layout (location = 0) in vec3 aVertexPosition;

out highp vec3 WorldPos;

uniform mat4 uProjectionMatrix;
uniform mat4 uViewMatrix;

void main()
{
    WorldPos = aVertexPosition;  
    gl_Position =  uProjectionMatrix * uViewMatrix * vec4(WorldPos, 1.0);
}