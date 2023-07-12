#version 300 es

#ifdef GL_ES
precision mediump float;
#endif

layout(location = 0) out vec4 FragColor;

uniform sampler2D uSampler;
uniform sampler2D uDepthMipMap;
uniform vec3 uLastMipSize;
uniform int uCurLevel;

in vec2 vTextureCoord;

void main(){
    if(uCurLevel == 0){
        vec3 color = texture(uSampler, vTextureCoord).rgb;
        FragColor = vec4(color, 1.0);
    }else{

    //gl_FragCoord.xy由视口gl.viewport决定。
    ivec2 thisLevelTexelCoord = ivec2(gl_FragCoord.xy);//向下取整
    ivec2 previousLevelBaseTexelCoord = thisLevelTexelCoord * 2;

    vec4 depthTexelValues;
    //texelFetch 使用的是未归一化的坐标直接访问纹理中的纹素，不执行任何形式的过滤和插值操作，坐标范围为实际载入纹理图像的宽和高，且为整数。
    depthTexelValues.x = texelFetch(uDepthMipMap,
                                      previousLevelBaseTexelCoord,
                                      0).r;
    depthTexelValues.y = texelFetch(uDepthMipMap,
                                      previousLevelBaseTexelCoord + ivec2(1, 0),
                                      0).r;
    depthTexelValues.z = texelFetch(uDepthMipMap,
                                      previousLevelBaseTexelCoord + ivec2(1, 1),
                                      0).r;
    depthTexelValues.w = texelFetch(uDepthMipMap,
                                      previousLevelBaseTexelCoord + ivec2(0, 1),
                                      0).r;

    float minDepth = min(min(depthTexelValues.x, depthTexelValues.y),
                          min(depthTexelValues.z, depthTexelValues.w));

    // Incorporate additional texels if the previous level's width or height (or both)
    // are odd.
    ivec2 u_previousLevelDimensions = ivec2(uLastMipSize.x, uLastMipSize.y);
    //是否为奇数
    bool shouldIncludeExtraColumnFromPreviousLevel = ((u_previousLevelDimensions.x & 1) != 0);
    bool shouldIncludeExtraRowFromPreviousLevel = ((u_previousLevelDimensions.y & 1) != 0);
    if (shouldIncludeExtraColumnFromPreviousLevel) {
      vec2 extraColumnTexelValues;
      extraColumnTexelValues.x = texelFetch(uDepthMipMap,
                                                previousLevelBaseTexelCoord + ivec2(2, 0),
                                                0).r;
      extraColumnTexelValues.y = texelFetch(uDepthMipMap,
                                                previousLevelBaseTexelCoord + ivec2(2, 1),
                                                0).r;

      // In the case where the width and height are both odd, need to include the
      // 'corner' value as well.
      if (shouldIncludeExtraRowFromPreviousLevel) {
        float cornerTexelValue = texelFetch(uDepthMipMap,
                                                  previousLevelBaseTexelCoord + ivec2(2, 2),
                                                  0).r;
        minDepth = min(minDepth, cornerTexelValue);
      }
      minDepth = min(minDepth, min(extraColumnTexelValues.x, extraColumnTexelValues.y));
    }
    if (shouldIncludeExtraRowFromPreviousLevel) {
      vec2 extraRowTexelValues;
      extraRowTexelValues.x = texelFetch(uDepthMipMap,
                                            previousLevelBaseTexelCoord + ivec2(0, 2),
                                            0).r;
      extraRowTexelValues.y = texelFetch(uDepthMipMap,
                                            previousLevelBaseTexelCoord + ivec2(1, 2),
                                            0).r;
      minDepth = min(minDepth, min(extraRowTexelValues.x, extraRowTexelValues.y));
    }

    FragColor = vec4(vec3(minDepth), 1.0);
    }
}