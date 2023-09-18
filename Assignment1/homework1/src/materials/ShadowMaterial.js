class ShadowMaterial extends Material {
// 添加rotate、lightIndex参数
    constructor(light, translate, rotate, scale, lightIndex, vertexShader, fragmentShader) {
        let lightMVP = light.CalcLightMVP(translate, rotate, scale);

        super({
            'uLightMVP': { type: 'matrix4fv', value: lightMVP },
            'uLightPos': { type: '3fv', value: light.lightPos },
            'uLightFarPlane': { type: '1f', value: light.lightFarPlane },
            'uLightModel': { type: 'matrix4fv', value: light.modelMatrix }
        // lightIndex参数
        }, [], vertexShader, fragmentShader, light.fbo, lightIndex);
        
    }
}

// 添加rotate、lightIndex参数
async function buildShadowMaterial(light, translate, rotate, scale, lightIndex, vertexPath, fragmentPath) {


    let vertexShader = await getShaderString(vertexPath);
    let fragmentShader = await getShaderString(fragmentPath);
// 添加rotate、lightIndex参数
    return new ShadowMaterial(light, translate, rotate, scale, lightIndex, vertexShader, fragmentShader);

}