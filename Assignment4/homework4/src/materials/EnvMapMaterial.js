class EnvMapMaterial extends Material {
    
    constructor(hdrObj,vertexShader, fragmentShader) {
        super({
            'uEquirectangularMap': { type: 'texture', value: hdrObj }
        }, [], vertexShader, fragmentShader, null);
    }
}

async function buildEnvMapMaterial(hdrObj,vertexPath, fragmentPath) {


    let vertexShader = await getShaderString(vertexPath);
    let fragmentShader = await getShaderString(fragmentPath);

    return new EnvMapMaterial(hdrObj,vertexShader, fragmentShader);

}