class SkyBoxMaterial extends Material {

    constructor(cubeMap,vertexShader, fragmentShader) {
        super({
            'uSkybox': { type: 'CubeTexture', value: cubeMap }
        }, [], vertexShader, fragmentShader, null);
    }
}

async function buildSkyBoxMaterial(cubeMap,vertexPath, fragmentPath) {
    

    let vertexShader = await getShaderString(vertexPath);
    let fragmentShader = await getShaderString(fragmentPath);

    return new SkyBoxMaterial(cubeMap,vertexShader, fragmentShader);

}