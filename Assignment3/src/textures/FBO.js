class FBO{
    constructor(gl, GBufferNum, width, height){
        //定义错误函数
        function error() {
            if(framebuffer) gl.deleteFramebuffer(framebuffer);
            if(texture) gl.deleteFramebuffer(texture);
            if(depthBuffer) gl.deleteFramebuffer(depthBuffer);
            return null;
        }

        function CreateAndBindColorTargetTexture(fbo, attachment, width, height) {
            //创建纹理对象并设置其尺寸和参数
            var texture = gl.createTexture();
            if(!texture){
                console.log("无法创建纹理对象");
                return error();
            }
            gl.bindTexture(gl.TEXTURE_2D, texture);
            gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA32F, width, height, 0, gl.RGBA, gl.FLOAT, null);
            gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.NEAREST);
            gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.NEAREST);
            gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE);
            gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE);

            gl.framebufferTexture2D(gl.FRAMEBUFFER, attachment, gl.TEXTURE_2D, texture, 0);

            return texture;
        };

        //创建帧缓冲区对象
        var framebuffer = gl.createFramebuffer();
        if(!framebuffer){
            console.log("无法创建帧缓冲区对象");
            return error();
        }
        gl.bindFramebuffer(gl.FRAMEBUFFER, framebuffer);
        
        //在JavaScript中，对象是动态的，可以随时添加属性和方法
	    framebuffer.attachments = [];
	    framebuffer.textures = []

        if(width == null){
            width = windowWidth;
        }
        if(height == null){
            height = windowHeight;
        }

        framebuffer.width = width;
        framebuffer.height = height;

	    for (var i = 0; i < GBufferNum; i++) {
            var attachment = gl.COLOR_ATTACHMENT0 + i;
	    	// var texture = CreateAndBindColorTargetTexture(framebuffer, attachment);
            var texture = CreateAndBindColorTargetTexture(framebuffer, attachment, width, height, 0);
	    	framebuffer.attachments.push(attachment);
	    	framebuffer.textures.push(texture);

            if(gl.checkFramebufferStatus(gl.FRAMEBUFFER) != gl.FRAMEBUFFER_COMPLETE)
                console.log(gl.checkFramebufferStatus(gl.FRAMEBUFFER));
	    }
	    // * Tell the WEBGL_draw_buffers extension which FBO attachments are
	    //   being used. (This extension allows for Multiple Render Targets.)
        gl.drawBuffers(framebuffer.attachments);

        // Create depth buffer
        var depthBuffer = gl.createRenderbuffer(); // Create a renderbuffer object
        gl.bindRenderbuffer(gl.RENDERBUFFER, depthBuffer); // Bind the object to target
        gl.renderbufferStorage(gl.RENDERBUFFER, gl.DEPTH_COMPONENT16, width, height);
        gl.framebufferRenderbuffer(gl.FRAMEBUFFER, gl.DEPTH_ATTACHMENT, gl.RENDERBUFFER, depthBuffer);

        gl.bindFramebuffer(gl.FRAMEBUFFER, null);
        gl.bindTexture(gl.TEXTURE_2D, null);
        gl.bindRenderbuffer(gl.RENDERBUFFER, null);

        return framebuffer;
    }
}