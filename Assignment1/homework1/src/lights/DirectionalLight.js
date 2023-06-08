class DirectionalLight {

    constructor(lightIntensity, lightColor, lightPos, focalPoint, lightUp, hasShadowMap, gl) {
        //Edit Start 添加旋转参数
        this.mesh = Mesh.cube(setTransform(0, 0, 0, 0, 0, 0, 1, 1, 1));
        //Edit End
        this.mat = new EmissiveMaterial(lightIntensity, lightColor);
        this.lightPos = lightPos;
        this.focalPoint = focalPoint;
        this.lightUp = lightUp

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
        let lightMVP = mat4.create();
        let modelMatrix = mat4.create();
        let viewMatrix = mat4.create();
        let projectionMatrix = mat4.create();

        //https://glmatrix.net/docs/module-mat4.html


        //Edit Start
        // Model transform
        mat4.translate(modelMatrix, modelMatrix, translate)
        mat4.rotateX(modelMatrix, modelMatrix, rotate[0])
        mat4.rotateY(modelMatrix, modelMatrix, rotate[1])
        mat4.rotateZ(modelMatrix, modelMatrix, rotate[2])
        mat4.scale(modelMatrix, modelMatrix, scale)
        
        // View transform
        mat4.lookAt(viewMatrix, this.lightPos, this.focalPoint, this.lightUp)
    
        // Projection transform
        var r = 110;  
	    var l = -r;  
	    var t = 110;  
	    var b = -t;  
	
	    var n = 0.01;  
	    var f = 400; 
        //平行光阴影，投影矩阵可用正交矩阵，变换后仍然保持线性深度
        mat4.ortho(projectionMatrix, l, r, b, t, n, f);

        //Edit End
        //matrix multiplication is applied from right to left,
        //the order of transformations is opposite to the order in which they are written in code.
        //which means that the last transformation specified in code is actually applied first to the vertex。
        mat4.multiply(lightMVP, projectionMatrix, viewMatrix);
        mat4.multiply(lightMVP, lightMVP, modelMatrix);

        return lightMVP;
    }
}
