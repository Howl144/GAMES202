class PointLight {
    constructor(lightRadiance, lightPos, lightUp,gl,hasShadowMap=false) {
        this.mesh = Mesh.cube(setTransform(0, 0, 0, 2, 2, 2, 0));
        this.mat = new EmissiveMaterial(lightRadiance);
        this.lightRadiance = lightRadiance;
        this.lightPos = lightPos;
        this.lightUp = lightUp;

        this.hasShadowMap = hasShadowMap;
        this.fbo = new FBO(gl);
        if (!this.fbo) {
            console.log("无法设置帧缓冲区对象");
            return;
        }
    }
}