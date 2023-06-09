class PointLight {
    /**
     * Creates an instance of PointLight.
     */
    constructor(lightIntensity, lightColor, lightPos, focalPoint, lightUp, hasShadowMap, gl) {
        //Edit Start 添加旋转参数
        this.mesh = Mesh.cube(setTransform(0, 0, 0, 0, 0, 0, 2.0, 2.0, 2.0));
        //Edit End
        this.mat = new EmissiveMaterial(lightIntensity, lightColor);
        this.lightPos = lightPos;
        this.focalPoint = focalPoint;
        this.lightUp = lightUp
        this.lightNearPlane = 0.01;
        this.lightFarPlane = 1000;//uniform需要
        this.modelMatrix;//uniform需要
        this.hasShadowMap = hasShadowMap;
        this.fbo = new FBO(gl);
        if (!this.fbo) {
            console.log("无法设置帧缓冲区对象");
            return;
        }
    }

        //Edit Start 添加旋转参数
        CalcLightMVP(translate, rotate, scale) {
        //Edit End
            let modelMatrix = mat4.create();
            let lightMVP = mat4.create();
            let viewMatrix = mat4.create();
            let projectionMatrix = mat4.create();
            let fovy = 90;
            let aspect = 1;
            //Edit Start
            // Model transform
            mat4.translate(modelMatrix, modelMatrix, translate)
            mat4.rotateX(modelMatrix, modelMatrix, rotate[0])
            mat4.rotateY(modelMatrix, modelMatrix, rotate[1])
            mat4.rotateZ(modelMatrix, modelMatrix, rotate[2])
            mat4.scale(modelMatrix, modelMatrix, scale)
            this.modelMatrix = modelMatrix;
            
            // View transform
            mat4.lookAt(viewMatrix, this.lightPos, this.focalPoint, this.lightUp)
        
            // Projection transform     
            mat4.perspective(projectionMatrix, fovy * Math.PI / 180, aspect, this.lightNearPlane, this.lightFarPlane);
    
            //Edit End
            //matrix multiplication is applied from right to left,
            //the order of transformations is opposite to the order in which they are written in code.
            //which means that the last transformation specified in code is actually applied first to the vertex。
            mat4.multiply(lightMVP, projectionMatrix, viewMatrix);
            mat4.multiply(lightMVP, lightMVP, modelMatrix);
    
            return lightMVP;
        }

}