class BrdfMaterial extends Material {

    constructor(vertexShader, fragmentShader) {
        super({
        }, [], vertexShader, fragmentShader);

    }
}

async function buildBrdfMaterial(vertexPath, fragmentPath) {

    let vertexShader = await getShaderString(vertexPath);
    let fragmentShader = await getShaderString(fragmentPath);

    return new BrdfMaterial(vertexShader, fragmentShader);
}