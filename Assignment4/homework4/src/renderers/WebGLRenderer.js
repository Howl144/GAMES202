class WebGLRenderer {
    meshes = [];
    shadowMeshes = [];
    lights = [];

    constructor(gl, camera) {
        this.gl = gl;
        this.camera = camera;
    }

    addLight(light) {
        this.lights.push({
            entity: light,
            meshRender: new MeshRender(this.gl, light.mesh, light.mat)
        });
    }
    addMeshRender(mesh) { this.meshes.push(mesh); }
    addShadowMeshRender(mesh) { this.shadowMeshes.push(mesh); }


    render() {
        const gl = this.gl;

        gl.clearColor(0.0, 0.0, 0.0, 1.0); // Clear to black, fully opaque
        gl.clearDepth(1.0); // Clear everything
        gl.enable(gl.DEPTH_TEST); // Enable depth testing
        gl.depthFunc(gl.LEQUAL); // Near things obscure far things
        gl.viewport(0,0,globalWidth,globalHeight);

        console.assert(this.lights.length != 0, "No light");
        // console.assert(this.lights.length == 1, "Multiple lights");

        for (let l = 0; l < this.lights.length; l++) {
            // Draw light
            // TODO: Support all kinds of transform
            this.lights[l].meshRender.mesh.transform.translate = this.lights[l].entity.lightPos;
            this.lights[l].meshRender.draw(this.camera);

            // Camera pass
            for (let i = 0; i < this.meshes.length; i++) {
                
                if(this.meshes[i].materialName == "KullaContyMaterial" || this.meshes[i].materialName == "PBRMaterial"){
                    let ID = this.meshes[i].shader.program.glShaderProgram;
                    this.gl.useProgram(ID);
                    gl.uniform3fv(gl.getUniformLocation(
                        ID, "uLightPos" + "[" + l + "]"), 
                        this.lights[l].entity.lightPos);
                    gl.uniform3fv(gl.getUniformLocation(
                        ID, "uLightColors" + "[" + l + "]"), 
                        this.lights[l].entity.lightRadiance);
                }
                this.meshes[i].draw(this.camera);
            }
        }
    }
}