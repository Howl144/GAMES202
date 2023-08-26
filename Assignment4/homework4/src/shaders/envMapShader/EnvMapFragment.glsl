#version 300 es
#ifdef GL_ES
precision mediump float;
#endif

out vec4 fragColor;

in highp vec3 WorldPos;

uniform sampler2D uEquirectangularMap;

const vec2 invAtan = vec2(0.1591, 0.3183);

// hdr文件存储每个浮点值的方式
// 每个通道存储 8 位，再以 alpha 通道存放指数
// 因此利用这种方式解码
vec3 hdrDecode(vec4 encoded){
    float exponent = encoded.a * 256.0 - 128.0;
    vec3 mantissa = encoded.rgb;
    return exp2(exponent) * mantissa;
}


vec2 SampleSphericalMap(vec3 v)
{
    vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
    uv *= invAtan;
    uv += 0.5;
    return uv;
}

void main()
{		
    vec2 uv = SampleSphericalMap(normalize(WorldPos));

    vec4 enCodeColor = texture(uEquirectangularMap,uv).rgba;
    
    vec3 deCodeColor = hdrDecode(enCodeColor);

    //debug Lut
    // vec3 deCodeColor = enCodeColor.rgb;

    fragColor = vec4(vec3(deCodeColor), 1.0);
}
