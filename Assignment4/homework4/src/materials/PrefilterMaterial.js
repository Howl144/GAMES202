class PrefilterMaterial extends Material {

    constructor(cubeMap,roughness,vertexShader, fragmentShader) {
        super({
            'uEnvironmentMap': { type: 'CubeTexture', value: cubeMap },
            'uRoughness': { type: '1f', value: roughness },
        }, [], vertexShader, fragmentShader, null);
    }
}

async function buildPrefilterMaterial(cubeMap,roughness,vertexPath, fragmentPath) {
    

    let vertexShader = await getShaderString(vertexPath);
    let fragmentShader = await getShaderString(fragmentPath);

    return new PrefilterMaterial(cubeMap,roughness,vertexShader, fragmentShader);

}