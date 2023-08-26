class IBL{
    constructor(gl,hdrTexture){
        this.gl = gl;
        this.hdrTexture = hdrTexture;
        //color attachment
        this.envCubemap = this.createCubeMapBuffer(512,512,true);
        this.irradianceMap = this.createCubeMapBuffer(32,32,false);
        this.prefilterMap = this.createCubeMapBuffer(128,128,true);
        this.brdfLUTObj = new Texture(gl); 
        //buffer
        this.captureFBO =  gl.createFramebuffer();
        this.captureRBO = gl.createRenderbuffer();
        
        this.captureProjection = mat4.create();
        this.captureViews = [];
        //render cube
        this.cubeVBO = 0;
        this.cubeVAO = 0;
        //render quad
        this.quadVAO = 0;
        this.quadVBO = 0;
        //shader
        this.equirectangularToCubemapShader;
        this.irradianceShader;
        this.prefilterShader;
        this.brdfShader;
        getErrorMessage(gl,"IBL.js");
    }
    async init(){
        const gl = this.gl;
        mat4.perspective(this.captureProjection, 90 * PI / 180 , 1.0, 0.1, 10.0);
        let looAtMatrices = mat4.create();
        mat4.lookAt(looAtMatrices,vec3.fromValues(0.0, 0.0, 0.0),vec3.fromValues(1.0,  0.0,  0.0),vec3.fromValues(0.0, -1.0,  0.0));
        this.captureViews.push(looAtMatrices);
        looAtMatrices = mat4.create();
        mat4.lookAt(looAtMatrices,vec3.fromValues(0.0, 0.0, 0.0),vec3.fromValues(-1.0,  0.0,  0.0),vec3.fromValues(0.0, -1.0,  0.0));
        this.captureViews.push(looAtMatrices);
        looAtMatrices = mat4.create();
        mat4.lookAt(looAtMatrices,vec3.fromValues(0.0, 0.0, 0.0),vec3.fromValues(0.0,  1.0,  0.0),vec3.fromValues(0.0, 0.0, 1.0));
        this.captureViews.push(looAtMatrices);
        looAtMatrices = mat4.create();
        mat4.lookAt(looAtMatrices,vec3.fromValues(0.0, 0.0, 0.0),vec3.fromValues(0.0,  -1.0,  0.0),vec3.fromValues(0.0, 0.0, -1.0));
        this.captureViews.push(looAtMatrices);
        looAtMatrices = mat4.create();
        mat4.lookAt(looAtMatrices,vec3.fromValues(0.0, 0.0, 0.0),vec3.fromValues(0.0,  0.0,  1.0),vec3.fromValues(0.0, -1.0,  0.0));
        this.captureViews.push(looAtMatrices);
        looAtMatrices = mat4.create();
        mat4.lookAt(looAtMatrices,vec3.fromValues(0.0, 0.0, 0.0),vec3.fromValues(0.0,  0.0,  -1.0),vec3.fromValues(0.0, -1.0,  0.0));
        this.captureViews.push(looAtMatrices);

        let envMap = buildEnvMapMaterial(null,"./src/shaders/envMapShader/EnvMapVertex.glsl", "./src/shaders/envMapShader/EnvMapFragment.glsl");
        let envMapData = await envMap;
        this.equirectangularToCubemapShader = envMapData.compile(gl);

        let irradiance = buildIrradianceMaterial(this.envCubemap,"./src/shaders/irradianceShader/irradianceVertex.glsl", "./src/shaders/irradianceShader/irradianceFragment.glsl");
        let irradianceData = await irradiance;
        this.irradianceShader = irradianceData.compile(gl);

        let prefilter = buildPrefilterMaterial(this.envCubemap,null,"./src/shaders/prefilterShader/prefilterVertex.glsl", "./src/shaders/prefilterShader/prefilterFragment.glsl");
        let prefilterData = await prefilter;
        this.prefilterShader = prefilterData.compile(gl);

        let brdf = buildBrdfMaterial("./src/shaders/brdfShader/brdfVertex.glsl", "./src/shaders/brdfShader/brdfFragment.glsl");
        let brdfData = await brdf;
        this.brdfShader = brdfData.compile(gl);

        gl.bindFramebuffer(gl.FRAMEBUFFER, this.captureFBO);
        gl.bindRenderbuffer(gl.RENDERBUFFER, this.captureRBO);
        gl.renderbufferStorage(gl.RENDERBUFFER, gl.DEPTH_COMPONENT16, 512, 512);
        gl.framebufferRenderbuffer(gl.FRAMEBUFFER, gl.DEPTH_ATTACHMENT, gl.RENDERBUFFER, this.captureRBO);

        getErrorMessage(gl,"IBL.js");
    }

    caculateEnvCubemap(){
        const gl = this.gl;
        const captureFBO = this.captureFBO;
        const captureRBO = this.captureRBO;

        gl.useProgram(this.equirectangularToCubemapShader.program.glShaderProgram);
        gl.uniform1i(
            this.equirectangularToCubemapShader.program.uniforms["uEquirectangularMap"],
            0);
        gl.uniformMatrix4fv(
            this.equirectangularToCubemapShader.program.uniforms["uProjectionMatrix"],
            false,
            this.captureProjection);
        gl.activeTexture(gl.TEXTURE0);
        gl.bindTexture(gl.TEXTURE_2D, this.hdrTexture);

        gl.viewport(0, 0, 512, 512); // don't forget to configure the viewport to the capture dimensions.
        gl.bindFramebuffer(gl.FRAMEBUFFER, captureFBO);
        for (let i = 0; i < 6; ++i){
            gl.uniformMatrix4fv(
                this.equirectangularToCubemapShader.program.uniforms["uViewMatrix"],
                false,
                this.captureViews[i]);
            
            gl.framebufferTexture2D(gl.FRAMEBUFFER, gl.COLOR_ATTACHMENT0, gl.TEXTURE_CUBE_MAP_POSITIVE_X + i, this.envCubemap, 0);

            let bufferStatu = gl.checkFramebufferStatus(gl.FRAMEBUFFER);
            if(bufferStatu != gl.FRAMEBUFFER_COMPLETE)
                console.log("FramebufferErr:" + bufferStatu);//36054

            gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);
    
            this.renderCube();
        }
        gl.bindFramebuffer(gl.FRAMEBUFFER, null);
        this.genMipmap(gl.TEXTURE_CUBE_MAP,this.envCubemap);
        getErrorMessage(gl,"IBL.js");
        return this.envCubemap;
    }

    caculateIrradianceMap(){
        const gl = this.gl;
        const captureFBO = this.captureFBO;
        const captureRBO = this.captureRBO;

        gl.bindFramebuffer(gl.FRAMEBUFFER, captureFBO);
        gl.bindRenderbuffer(gl.RENDERBUFFER, captureRBO);
        gl.renderbufferStorage(gl.RENDERBUFFER, gl.DEPTH_COMPONENT24, 32, 32);

        gl.useProgram(this.irradianceShader.program.glShaderProgram);
        gl.uniform1i(
            this.irradianceShader.program.uniforms["uEnvironmentMap"],
            0);
        gl.uniformMatrix4fv(
            this.irradianceShader.program.uniforms["uProjectionMatrix"],
            false,
            this.captureProjection);
        gl.activeTexture(gl.TEXTURE0);
        gl.bindTexture(gl.TEXTURE_CUBE_MAP, this.envCubemap);
        gl.viewport(0, 0, 32, 32); // don't forget to configure the viewport to the capture dimensions.

        gl.bindFramebuffer(gl.FRAMEBUFFER, captureFBO);
        for (let i = 0; i < 6; ++i)
        {
            gl.uniformMatrix4fv(
                this.irradianceShader.program.uniforms["uViewMatrix"],
                false,
                this.captureViews[i]);
            gl.framebufferTexture2D(gl.FRAMEBUFFER, gl.COLOR_ATTACHMENT0, gl.TEXTURE_CUBE_MAP_POSITIVE_X + i, this.irradianceMap, 0);
            //debug frameBuffer
            let bufferStatu = gl.checkFramebufferStatus(gl.FRAMEBUFFER);
            if(bufferStatu != gl.FRAMEBUFFER_COMPLETE)
                console.log("FramebufferErr:" + bufferStatu);//36057 Not all attached images have the same width and height.

            gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);
            this.renderCube();
        }
        gl.bindFramebuffer(gl.FRAMEBUFFER, null);
        getErrorMessage(gl,"IBL.js");
        return this.irradianceMap;
    }

    caculatePrefilterMap(){
        const gl = this.gl;
        const captureFBO = this.captureFBO;
        const captureRBO = this.captureRBO;
        

        gl.useProgram(this.prefilterShader.program.glShaderProgram);
        gl.uniform1i(this.prefilterShader.program.uniforms["uEnvironmentMap"],0);
        gl.uniformMatrix4fv(
            this.prefilterShader.program.uniforms["uProjectionMatrix"],
            false,
            this.captureProjection);

        this.genMipmap(gl.TEXTURE_CUBE_MAP,this.prefilterMap);
        gl.activeTexture(gl.TEXTURE0);
        gl.bindTexture(gl.TEXTURE_CUBE_MAP, this.envCubemap);

        gl.bindFramebuffer(gl.FRAMEBUFFER, captureFBO);
        const maxMipLevels = 5;
        for (let mip = 0; mip < maxMipLevels; ++mip)
        {
            // reisze framebuffer according to mip-level size.
            let mipWidth  = 128 * Math.pow(0.5, mip);
            let mipHeight = 128 * Math.pow(0.5, mip);

            gl.bindRenderbuffer(gl.RENDERBUFFER, captureRBO);
            gl.renderbufferStorage(gl.RENDERBUFFER, gl.DEPTH_COMPONENT24, mipWidth, mipHeight);
            gl.viewport(0, 0, mipWidth, mipHeight);
            
            let roughness = mip / (maxMipLevels - 1);
            gl.uniform1f(this.prefilterShader.program.uniforms["uRoughness"],roughness);
            for (let i = 0; i < 6; ++i)
            {
                
                gl.uniformMatrix4fv(
                    this.prefilterShader.program.uniforms["uViewMatrix"],
                    false,
                    this.captureViews[i]);
                gl.framebufferTexture2D(gl.FRAMEBUFFER, gl.COLOR_ATTACHMENT0, gl.TEXTURE_CUBE_MAP_POSITIVE_X + i, this.prefilterMap, mip);
                //debug frameBuffer
                let bufferStatu = gl.checkFramebufferStatus(gl.FRAMEBUFFER);
                if(bufferStatu != gl.FRAMEBUFFER_COMPLETE)
                    console.log("FramebufferErr:" + bufferStatu);//36057 Not all attached images have the same width and height.
    
                gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);
                this.renderCube();
            }
        }
        gl.bindFramebuffer(gl.FRAMEBUFFER, null);
        getErrorMessage(gl,"IBL.js");
        return this.prefilterMap;
    }
    
    caculateLut(){
        const gl = this.gl;
        const captureFBO = this.captureFBO;
        const captureRBO = this.captureRBO;

        // generate a 2D LUT from the BRDF equations used.
        let brdfLUTTexture = this.brdfLUTObj.texture;
        // pre-allocate enough memory for the LUT texture.
        gl.bindTexture(gl.TEXTURE_2D, brdfLUTTexture);
        gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGB, 512, 512, 0, gl.RGB, gl.UNSIGNED_BYTE,null);
        // be sure to set wrapping mode to GL_CLAMP_TO_EDGE
        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE);
        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE);
        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR);
        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.LINEAR);
        // then re-configure capture framebuffer object and render screen-space quad with BRDF shader.
        gl.bindFramebuffer(gl.FRAMEBUFFER, captureFBO);
        gl.bindRenderbuffer(gl.RENDERBUFFER, captureRBO);
        gl.renderbufferStorage(gl.RENDERBUFFER, gl.DEPTH_COMPONENT16, 512, 512);
        gl.framebufferTexture2D(gl.FRAMEBUFFER, gl.COLOR_ATTACHMENT0, gl.TEXTURE_2D, brdfLUTTexture, 0);

        let bufferStatu = gl.checkFramebufferStatus(gl.FRAMEBUFFER);
        if(bufferStatu != gl.FRAMEBUFFER_COMPLETE)
            console.log("FramebufferErr:" + bufferStatu);//36057 Not all attached images have the same width and height.

        getErrorMessage(gl,"IBL.js");

        gl.viewport(0, 0, 512, 512);
        gl.useProgram(this.brdfShader.program.glShaderProgram);

        gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);
        this.renderQuad();

        gl.bindFramebuffer(gl.FRAMEBUFFER, null);
        getErrorMessage(gl,"IBL.js");
        return this.brdfLUTObj;
    }

    renderCube(){
        const gl = this.gl;
        if(this.cubeVAO == 0){
            let vertices = new Float32Array([
                // back face
                -1.0, -1.0, -1.0,  0.0,  0.0, -1.0, 0.0, 0.0, // bottom-left
                1.0,  1.0, -1.0,  0.0,  0.0, -1.0, 1.0, 1.0, // top-right
                1.0, -1.0, -1.0,  0.0,  0.0, -1.0, 1.0, 0.0, // bottom-right         
                1.0,  1.0, -1.0,  0.0,  0.0, -1.0, 1.0, 1.0, // top-right
                -1.0, -1.0, -1.0,  0.0,  0.0, -1.0, 0.0, 0.0, // bottom-left
                -1.0,  1.0, -1.0,  0.0,  0.0, -1.0, 0.0, 1.0, // top-left
                // front face
                -1.0, -1.0,  1.0,  0.0,  0.0,  1.0, 0.0, 0.0, // bottom-left
                1.0, -1.0,  1.0,  0.0,  0.0,  1.0, 1.0, 0.0, // bottom-right
                1.0,  1.0,  1.0,  0.0,  0.0,  1.0, 1.0, 1.0, // top-right
                1.0,  1.0,  1.0,  0.0,  0.0,  1.0, 1.0, 1.0, // top-right
                -1.0,  1.0,  1.0,  0.0,  0.0,  1.0, 0.0, 1.0, // top-left
                -1.0, -1.0,  1.0,  0.0,  0.0,  1.0, 0.0, 0.0, // bottom-left
                // left face
                -1.0,  1.0,  1.0, -1.0,  0.0,  0.0, 1.0, 0.0, // top-right
                -1.0,  1.0, -1.0, -1.0,  0.0,  0.0, 1.0, 1.0, // top-left
                -1.0, -1.0, -1.0, -1.0,  0.0,  0.0, 0.0, 1.0, // bottom-left
                -1.0, -1.0, -1.0, -1.0,  0.0,  0.0, 0.0, 1.0, // bottom-left
                -1.0, -1.0,  1.0, -1.0,  0.0,  0.0, 0.0, 0.0, // bottom-right
                -1.0,  1.0,  1.0, -1.0,  0.0,  0.0, 1.0, 0.0, // top-right
                // right face
                1.0,  1.0,  1.0,  1.0,  0.0,  0.0, 1.0, 0.0, // top-left
                1.0, -1.0, -1.0,  1.0,  0.0,  0.0, 0.0, 1.0, // bottom-right
                1.0,  1.0, -1.0,  1.0,  0.0,  0.0, 1.0, 1.0, // top-right         
                1.0, -1.0, -1.0,  1.0,  0.0,  0.0, 0.0, 1.0, // bottom-right
                1.0,  1.0,  1.0,  1.0,  0.0,  0.0, 1.0, 0.0, // top-left
                1.0, -1.0,  1.0,  1.0,  0.0,  0.0, 0.0, 0.0, // bottom-left     
                // bottom face
                -1.0, -1.0, -1.0,  0.0, -1.0,  0.0, 0.0, 1.0, // top-right
                1.0, -1.0, -1.0,  0.0, -1.0,  0.0, 1.0, 1.0, // top-left
                1.0, -1.0,  1.0,  0.0, -1.0,  0.0, 1.0, 0.0, // bottom-left
                1.0, -1.0,  1.0,  0.0, -1.0,  0.0, 1.0, 0.0, // bottom-left
                -1.0, -1.0,  1.0,  0.0, -1.0,  0.0, 0.0, 0.0, // bottom-right
                -1.0, -1.0, -1.0,  0.0, -1.0,  0.0, 0.0, 1.0, // top-right
                // top face
                -1.0,  1.0, -1.0,  0.0,  1.0,  0.0, 0.0, 1.0, // top-left
                1.0,  1.0 , 1.0,  0.0,  1.0,  0.0, 1.0, 0.0, // bottom-right
                1.0,  1.0, -1.0,  0.0,  1.0,  0.0, 1.0, 1.0, // top-right     
                1.0,  1.0,  1.0,  0.0,  1.0,  0.0, 1.0, 0.0, // bottom-right
                -1.0,  1.0, -1.0,  0.0,  1.0,  0.0, 0.0, 1.0, // top-left
                -1.0,  1.0,  1.0,  0.0,  1.0,  0.0, 0.0, 0.0  // bottom-left 
            ]);
            this.cubeVBO = gl.createBuffer();
            gl.bindBuffer(gl.ARRAY_BUFFER, this.cubeVBO);
            gl.bufferData(gl.ARRAY_BUFFER, vertices, gl.STATIC_DRAW);
    
            this.cubeVAO = gl.createVertexArray();
            gl.bindVertexArray(this.cubeVAO);
    
            let position = 0;
            gl.vertexAttribPointer(position, 3, gl.FLOAT, false, 8 * Float32Array.BYTES_PER_ELEMENT, 0);
            gl.enableVertexAttribArray(position);
    
            let normal = 1;
            gl.vertexAttribPointer(normal, 3, gl.FLOAT, false, 8 * Float32Array.BYTES_PER_ELEMENT, 3 * Float32Array.BYTES_PER_ELEMENT);
            gl.enableVertexAttribArray(normal);
    
            let texcoord = 2;
            gl.vertexAttribPointer(texcoord, 2, gl.FLOAT, false, 8 * Float32Array.BYTES_PER_ELEMENT, 6 * Float32Array.BYTES_PER_ELEMENT);
            gl.enableVertexAttribArray(texcoord);
    
            gl.bindBuffer(gl.ARRAY_BUFFER, null);
            gl.bindVertexArray(null);
        }
        gl.bindVertexArray(this.cubeVAO);
        gl.drawArrays(gl.TRIANGLES, 0, 36);
        gl.bindVertexArray(null);
    }

    renderQuad(){
        const gl = this.gl;

        if (this.quadVAO == 0) {
            const quadVertices = [
            // positions        // texture Coords
            -1.0, 1.0, 0.0, 0.0, 1.0,
            -1.0, -1.0, 0.0, 0.0, 0.0,
            1.0, 1.0, 0.0, 1.0, 1.0,
            1.0, -1.0, 0.0, 1.0, 0.0,
            ];
            
            // setup plane VAO
            this.quadVAO = gl.createVertexArray();
            gl.bindVertexArray(this.quadVAO);

            this.quadVBO = gl.createBuffer();
            gl.bindBuffer(gl.ARRAY_BUFFER, this.quadVBO);
            gl.bufferData(gl.ARRAY_BUFFER, new Float32Array(quadVertices), gl.STATIC_DRAW);
            gl.enableVertexAttribArray(0);
            gl.vertexAttribPointer(0, 3, gl.FLOAT, false, 5 * Float32Array.BYTES_PER_ELEMENT, 0);
            gl.enableVertexAttribArray(1);
            gl.vertexAttribPointer(1, 2, gl.FLOAT, false, 5 * Float32Array.BYTES_PER_ELEMENT, 3 * Float32Array.BYTES_PER_ELEMENT);
            gl.bindVertexArray(null);
        }

        gl.bindVertexArray(this.quadVAO);
        gl.drawArrays(gl.TRIANGLE_STRIP, 0, 4);
        gl.bindVertexArray(null);

    }

    createCubeMapBuffer(width,height,LinMipmapLin=false){
        const gl = this.gl;
        let ret = gl.createTexture();
        getErrorMessage(gl,"CubeTexture.js");
        gl.bindTexture(gl.TEXTURE_CUBE_MAP, ret);

        for (var j = 0; j < 6; j++) {
            gl.texImage2D(gl.TEXTURE_CUBE_MAP_POSITIVE_X + j, 0, gl.RGB, width, height, 0, gl.RGB, gl.UNSIGNED_BYTE, null);
        }
        gl.texParameteri(gl.TEXTURE_CUBE_MAP, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE);
        gl.texParameteri(gl.TEXTURE_CUBE_MAP, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE);
        gl.texParameteri(gl.TEXTURE_CUBE_MAP, gl.TEXTURE_WRAP_R, gl.CLAMP_TO_EDGE);
        gl.texParameteri(gl.TEXTURE_CUBE_MAP, gl.TEXTURE_MAG_FILTER, gl.LINEAR);
        if(LinMipmapLin == true){
            gl.texParameteri(gl.TEXTURE_CUBE_MAP, gl.TEXTURE_MIN_FILTER, gl.LINEAR_MIPMAP_LINEAR);
            // gl.generateMipmap(gl.TEXTURE_CUBE_MAP);//Set it later
        }else{
            gl.texParameteri(gl.TEXTURE_CUBE_MAP, gl.TEXTURE_MIN_FILTER, gl.LINEAR);
        }

        gl.bindTexture(gl.TEXTURE_CUBE_MAP, null);
        getErrorMessage(gl,"CubeTexture.js");
        return ret;
    }
    genMipmap(type,texture){
        const gl = this.gl;
        gl.bindTexture(type,texture);
        gl.generateMipmap(type);
        gl.bindTexture(type,null);
    }
}