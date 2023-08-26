class PBRMaterial extends Material {

    constructor(albedo, metallic,roughness,irradianceMap,prefilterMap,pbrBrdfLut, vertexShader, fragmentShader) {
        super({
            'uAlbedoMap':          { type: 'texture', value: albedo },
            'uMetallic':        { type: '1f', value: metallic },
            'uRoughness':       { type: '1f', value: roughness },
            'uAo':              { type: '1f', value: 1.0 },
            
            'uIrradianceMap':   { type: 'CubeTexture', value: irradianceMap },
            'uPrefilterMap':    { type: 'CubeTexture', value: prefilterMap },
            'uPbrBrdfLUT':         { type: 'texture', value: pbrBrdfLut },

            // 'uLightPos[4]': { type: '3fv', value: null },
            // 'uLightColors[4]': { type: '3fv', value: null },
        }, [], vertexShader, fragmentShader);

    }
}

async function buildPBRMaterial(albedo, metallic,roughness,irradianceMap,prefilterMap,pbrBrdfLut, vertexPath, fragmentPath) {

    let vertexShader = await getShaderString(vertexPath);
    let fragmentShader = await getShaderString(fragmentPath);

    return new PBRMaterial(albedo, metallic,roughness,irradianceMap,prefilterMap,pbrBrdfLut, vertexShader, fragmentShader);
}