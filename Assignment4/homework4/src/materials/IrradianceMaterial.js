class IrradianceMaterial extends Material {

    constructor(cubeMap,vertexShader, fragmentShader) {
        super({
            'uEnvironmentMap': { type: 'CubeTexture', value: cubeMap }
        }, [], vertexShader, fragmentShader, null);
    }
}

async function buildIrradianceMaterial(cubeMap,vertexPath, fragmentPath) {
    

    let vertexShader = await getShaderString(vertexPath);
    let fragmentShader = await getShaderString(fragmentPath);

    return new IrradianceMaterial(cubeMap,vertexShader, fragmentShader);

}