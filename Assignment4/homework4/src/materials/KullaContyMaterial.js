class KullaContyMaterial extends Material {

    constructor(albedo, metallic,roughness,irradianceMap,prefilterMap,pbrBrdfLut,kullaContyBrdflut,kullaContyEavglut, vertexShader, fragmentShader) {
        super({
            'uAlbedoMap':           { type: 'texture', value: albedo },
            'uMetallic':            { type: '1f', value: metallic },
            'uRoughness':           { type: '1f', value: roughness },
            'uAo':                  { type: '1f', value: 1.0 },
            
            'uIrradianceMap':       { type: 'CubeTexture', value: irradianceMap },
            'uPrefilterMap':        { type: 'CubeTexture', value: prefilterMap },
            'uPbrBrdfLUT':          { type: 'texture', value: pbrBrdfLut },

            //kulla conty
            'uKullaContyBrdflut':   { type: 'texture', value: kullaContyBrdflut },
            'uKullaContyEavglut':    { type: 'texture', value: kullaContyEavglut },

            // 'uLightPos[4]': { type: '3fv', value: null },
            // 'uLightColors[4]': { type: '3fv', value: null },
        }, [], vertexShader, fragmentShader);

    }
}

async function buildKullaContyMaterial(albedo, metallic,roughness,irradianceMap,prefilterMap,pbrBrdfLut,kullaContyBrdflut,kullaContyEavglut, vertexPath, fragmentPath) {

    let vertexShader = await getShaderString(vertexPath);
    let fragmentShader = await getShaderString(fragmentPath);

    return new KullaContyMaterial(albedo, metallic,roughness,irradianceMap,prefilterMap,pbrBrdfLut,kullaContyBrdflut,kullaContyEavglut, vertexShader, fragmentShader);
}