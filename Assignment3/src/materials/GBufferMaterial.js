class GBufferMaterial extends Material {
    constructor(diffuseMap, normalMap, light, camera, uZBufferParams,vertexShader, fragmentShader) {
        let lightVP = light.CalcLightVP();

        super({
            'uKd': { type: 'texture', value: diffuseMap.texture },
            'uNt': { type: 'texture', value: normalMap.texture },

            'uLightVP': { type: 'matrix4fv', value: lightVP },
            'uShadowMap': { type: 'texture', value: light.fbo.textures[0] },
            'uZBufferParams': { type: '3fv', value: uZBufferParams },
        }, [], vertexShader, fragmentShader, camera.fbo);
    }
}

async function buildGbufferMaterial (diffuseMap, normalMap, light, camera, uZBufferParams,vertexPath, fragmentPath) {
    let vertexShader = await getShaderString(vertexPath);
    let fragmentShader = await getShaderString(fragmentPath);

    return new GBufferMaterial(diffuseMap, normalMap, light, camera, uZBufferParams, vertexShader, fragmentShader);
}