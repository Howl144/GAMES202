class SceneDepthDebugMaterial extends Material {
    constructor(ZBufferParams,vertexShader, fragmentShader) {
        let uniforms = {
            'uDepthTexture' : { type: 'texture', value: 0 },
            'uZBufferParams' : { type: '3fv', value: ZBufferParams }

        }
        super(uniforms, [], vertexShader, fragmentShader);
    }
}

async function buildSceneDepthDebugMaterial(ZBufferParams,vertexPath, fragmentPath) {
    let vertexShader = await getShaderString(vertexPath);
    let fragmentShader = await getShaderString(fragmentPath);

    return new SceneDepthDebugMaterial(ZBufferParams,vertexShader, fragmentShader);
}