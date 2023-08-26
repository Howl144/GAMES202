class CubeTexture {
    constructor(gl, urls = null) {
        this.urls = urls;
        this.gl = gl;
    }

    async init() {
        var img = new Array(6);
        const gl = this.gl;
        for (let i = 0; i < 6; i++) {
            img[i] = new Image();
            img[i].src = this.urls[i];
            var loadImage = async img => {
                return new Promise((resolve, reject) => {
                    img.onload = async () => {
                        console.log("Image Loaded");
                        resolve(true);
                    };
                });
            };
            await loadImage(img[i]);
        }

        this.texture = gl.createTexture();
        gl.bindTexture(gl.TEXTURE_CUBE_MAP, this.texture);

        for (var j = 0; j < 6; j++) {
            gl.texImage2D(gl.TEXTURE_CUBE_MAP_POSITIVE_X + j, 0, gl.RGB, gl.RGB, gl.UNSIGNED_BYTE, img[j]);
        }
        gl.texParameteri(gl.TEXTURE_CUBE_MAP, gl.TEXTURE_MAG_FILTER, gl.LINEAR);
        gl.texParameteri(gl.TEXTURE_CUBE_MAP, gl.TEXTURE_MIN_FILTER, gl.LINEAR);
        gl.generateMipmap(gl.TEXTURE_CUBE_MAP);
        gl.bindTexture(gl.TEXTURE_CUBE_MAP, null);
    }
}