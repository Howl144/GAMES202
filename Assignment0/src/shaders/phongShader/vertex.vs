attribute vec3 aVertexPosition;
attribute vec3 aNormalPosition;
attribute vec2 aTextureCoord;

uniform mat4 uModelViewMatrix;
uniform mat4 uProjectionMatrix;

//varying highp是一个修饰符，用于声明变量的精度，varying是用于在顶点着色器和片段着色器之间传递数据的修饰符，highp是最高精度的修饰符，用于声明变量的精度，精度越高，精度损失越小，但是性能消耗越大，所以一般情况下，我们会根据实际情况来选择变量的精度，比如，对于顶点坐标，我们一般会使用highp，因为顶点坐标的精度对于渲染结果来说至关重要，而对于纹理坐标，我们一般会使用mediump，因为纹理坐标的精度对于渲染结果来说不是那么重要，而且纹理坐标的精度越高，性能消耗越大，所以我们会根据实际情况来选择变量的精度。
varying highp vec2 vTextureCoord;
varying highp vec3 vFragPos;
varying highp vec3 vNormal;


void main(void) {

  vFragPos = aVertexPosition;
  vNormal = mat3(transpose(inverse(uModelViewMatrix))) * aNormalPosition;

  gl_Position = uProjectionMatrix * uModelViewMatrix * vec4(aVertexPosition, 1.0);

  vTextureCoord = aTextureCoord;

}