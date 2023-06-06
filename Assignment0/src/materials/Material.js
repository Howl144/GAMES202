class Material {
    //#表示私有字段，类似cpp里的private
    #flatten_uniforms;
    #flatten_attribs;
    #vsSrc;
    #fsSrc;
    // Uniforms is a map, attribs is a Array
    constructor(uniforms, attribs, vsSrc, fsSrc) {
        this.uniforms = uniforms;
        this.attribs = attribs;
        this.#vsSrc = vsSrc;
        this.#fsSrc = fsSrc;
        
        this.#flatten_uniforms = ['uModelViewMatrix', 'uProjectionMatrix', 'uCameraPos', 'uLightPos'];
        //let块级作用于，var函数作用域，块级作用域：if, for, while, switch, try/catch等。
        for (let k in uniforms) {
            //k是uniforms的key
            this.#flatten_uniforms.push(k);
        }
        this.#flatten_attribs = attribs;
    }
    //创建一个方法，用以储存mesh的额外属性
    setMeshAttribs(extraAttribs) {
        for (let i = 0; i < extraAttribs.length; i++) {
            this.#flatten_attribs.push(extraAttribs[i]);
        }
    }

    compile(gl) {
        //对象字面量（Object Literal）
        //它有两个属性 uniforms和 attribs，它们的值分别是 this.#flatten_uniforms和 this.#flatten_attribs。这个对象可以作为参数传递给 Shader类的构造函数，用来初始化着色器的 uniform 变量和 attribute 变量。
        return new Shader(gl, this.#vsSrc, this.#fsSrc,
            {
                uniforms: this.#flatten_uniforms,
                attribs: this.#flatten_attribs
            });
    }
}