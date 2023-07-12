class SSRMaterial extends Material {
    constructor(diffuseMap, specularMap, light, camera, uZBufferParams,vertexShader, fragmentShader) {
        let lightIntensity = light.mat.GetIntensity();
        let lightDir = light.CalcShadingDirection();

        // Edit Start
        let uniforms = {
            'uLightRadiance': { type: '3fv', value: lightIntensity },
            'uLightDir': { type: '3fv', value: lightDir },

            'uGDiffuse': { type: 'texture', value: camera.fbo.textures[0] },
            'uGDepth': { type: 'texture', value: camera.fbo.textures[1] },
            'uGNormalWS': { type: 'texture', value: camera.fbo.textures[2] },
            'uGShadow': { type: 'texture', value: camera.fbo.textures[3] },
            'uZBufferParams': { type: '3fv', value: uZBufferParams },
        }

        for(let i = 0; i < mipMapLevel; i++){
            uniforms['uDepthTexture' + '[' + i + ']'] = { type: 'texture', value: null };
        }

        super(uniforms, [], vertexShader, fragmentShader);
        // Edit End
    }
}

async function buildSSRMaterial(diffuseMap, specularMap, light, camera, uZBufferParams, vertexPath, fragmentPath) {
    let vertexShader = await getShaderString(vertexPath);
    let fragmentShader = await getShaderString(fragmentPath);

    return new SSRMaterial(diffuseMap, specularMap, light, camera, uZBufferParams, vertexShader, fragmentShader);
}