var cameraPosition = [0, 80, 300];

var envmap = [
	'assets/cubemap/Indoor'
];

var guiParams = {
	envmapId: 0
}

var cubeMaps = [];

//生成的纹理的分辨率，纹理必须是标准的尺寸 256*256 1024*1024  2048*2048
var resolution = 1024;
let globalWidth;
let globalHeight;
let kullaContyBrdflut, kullaContyEavglut;
let envMapPass = null;
const PI = 3.1415926536;



GAMES202Main();


function getErrorMessage(gl,file) {
    let error = gl.getError();
    while (error != gl.NO_ERROR) {
        let errorString;
        switch (error) {
            case gl.INVALID_ENUM:                   errorString = "INVALID_ENUM"; break;
            case gl.INVALID_VALUE:                  errorString = "INVALID_VALUE"; break;
            case gl.INVALID_OPERATION:              errorString = "INVALID_OPERATION"; break;
            case gl.STACK_OVERFLOW:                 errorString = "STACK_OVERFLOW"; break;
            case gl.STACK_UNDERFLOW:                errorString = "STACK_UNDERFLOW"; break;
            case gl.OUT_OF_MEMORY:                  errorString = "OUT_OF_MEMORY"; break;
            case gl.INVALID_FRAMEBUFFER_OPERATION:  errorString = "INVALID_FRAMEBUFFER_OPERATION"; break;
        }
        console.log(`${errorString} | ${file}`);
        error = gl.getError();
    }
}

async function GAMES202Main() {
	// Init canvas and gl
	const canvas = document.querySelector('#glcanvas');
	globalWidth = canvas.width = window.screen.width;
	globalHeight = canvas.height = window.screen.height;
	const gl = canvas.getContext('webgl2');
	if (!gl) {
		alert('Unable to initialize WebGL. Your browser or machine may not support it.');
		return;
	}
	// Add camera
	const camera = new THREE.PerspectiveCamera(75, gl.canvas.clientWidth / gl.canvas.clientHeight, 1e-2, 1000);
	camera.position.set(cameraPosition[0], cameraPosition[1], cameraPosition[2]);

	// Add resize listener
	function setSize(width, height) {
		camera.aspect = width / height;
		camera.updateProjectionMatrix();
	}
	setSize(canvas.clientWidth, canvas.clientHeight);
	window.addEventListener('resize', () => setSize(canvas.clientWidth, canvas.clientHeight));

	// Add camera control
	const cameraControls = new THREE.OrbitControls(camera, canvas);
	cameraControls.enableZoom = true;
	cameraControls.enableRotate = true;
	cameraControls.enablePan = true;
	cameraControls.rotateSpeed = 0.3;
	cameraControls.zoomSpeed = 1.0;
	cameraControls.panSpeed = 0.8;
	cameraControls.target.set(0, 0, 0);

	// Add renderer
	const renderer = new WebGLRenderer(gl, camera);

	// Add lights
	let lightRadiance = [1, 1, 1];
	let lightUp = [1, 0, 0];//not use here

	let lightPos1 = [180, 0, 100];
	const pointLight1 = new PointLight(lightRadiance, lightPos1, lightUp, renderer.gl);
	renderer.addLight(pointLight1);
	let lightPos2 = [180, 100, 100];
	const pointLight2 = new PointLight(lightRadiance, lightPos2, lightUp, renderer.gl);
	renderer.addLight(pointLight2);
	let lightPos3 = [-140, 0, 100];
	const pointLight3 = new PointLight(lightRadiance, lightPos3, lightUp, renderer.gl);
	renderer.addLight(pointLight3);
	let lightPos4 = [-140, 100, 100];
	const pointLight4 = new PointLight(lightRadiance, lightPos4, lightUp, renderer.gl);
	renderer.addLight(pointLight4);
	

	//image path
	// let Path_Emu = 'assets/ball/split_sum/Emu_IS_LUT.png';
	// let Path_Eavg = 'assets/ball/split_sum/Eavg_IS_LUT.png';
	let Path_Emu = 'assets/ball/no_split_sum/Emu_IS_LUT.png';
	let Path_Eavg = 'assets/ball/no_split_sum/Eavg_IS_LUT.png';


	// Add Sphere
	let img = new Image(); // brdfLUT
	img.src = Path_Emu;
	var loadImage = async img => {
		return new Promise((resolve, reject) => {
			img.onload = async () => {
				console.log("Image Loaded");
				resolve(true);
			};
		});
	};
	await loadImage(img);
	kullaContyBrdflut = new Texture(gl);
	kullaContyBrdflut.CreateImageTexture(gl, img);

	let img1 = new Image(); // eavgLUT
	img1.src = Path_Eavg;
	var loadImage = async img => {
		return new Promise((resolve, reject) => {
			img.onload = async () => {
				console.log("Image Loaded");
				resolve(true);
			};
		});
	};
	await loadImage(img1);
	kullaContyEavglut = new Texture(gl);
	kullaContyEavglut.CreateImageTexture(gl, img1);

	// Add SkyBox
	for (let i = 0; i < envmap.length; i++) {
		let urls = [
			envmap[i] + '/posx.jpg',
			envmap[i] + '/negx.jpg',
			envmap[i] + '/negy.jpg',
			envmap[i] + '/posy.jpg',
			envmap[i] + '/posz.jpg',
			envmap[i] + '/negz.jpg',
		];
		cubeMaps.push(new CubeTexture(gl, urls))
		await cubeMaps[i].init();
	}
	getErrorMessage(gl,"engine.js");
	
	//ibl
	let envCubemap,irradianceMap,prefilterMap;
	let pbrBrdfLutObj;
	let hdrObj = new Texture(gl);
	//加载HDR文件
	async function loadTexture() {
		return new Promise((resolve) => {
			new THREE.RGBELoader().load("assets/winter_sky_1k.hdr", function(texture) {
				console.log("HDR Loaded");
				resolve(texture);
			});
		});
	}

	async function integral() {
		let hdrData = await loadTexture();
		let data = hdrData.image.data;
		let width = hdrData.image.width;
		let height = hdrData.image.height;
		gl.bindTexture(gl.TEXTURE_2D, hdrObj.texture);
		gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, width, height, 0, gl.RGBA, gl.UNSIGNED_BYTE,new Uint8Array(data));
		////debug texture
		// gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, 1, 1, 0, gl.RGBA, gl.UNSIGNED_BYTE,
		// 	new Uint8Array([0, 0, 255, 255]));
		gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE);
		gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE);
		gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR);
		gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.LINEAR);
		gl.bindTexture(gl.TEXTURE_2D, null);
		getErrorMessage(gl,"engine.js");
		let ibl = new IBL(gl,hdrObj.texture);
		await ibl.init();
		envCubemap = ibl.caculateEnvCubemap();
		irradianceMap = ibl.caculateIrradianceMap();
		prefilterMap = ibl.caculatePrefilterMap();
		pbrBrdfLutObj = ibl.caculateLut();
	}
	await integral();
	//Display the cubeMap and debug it
	let transform = setTransform(0, 50, 50, 150, 150, 150);
	loadOBJ(renderer, 'assets/testObj/', 'testObj', 'SkyBoxMaterial', transform, envCubemap);
	// //debug Texture
	// loadOBJ(renderer, 'assets/testObj/', 'testObj', 'EnvMapMaterial', transform,null,pbrBrdfLutObj);

	//PBR
	let metallic = 1.0;
	let pbrSphere1Transform = setTransform(360, 0, 0, 180, 180, 180, 0, Math.PI, 0);
	loadGLTF(renderer, 'assets/ball/', 'ball', 'PBRMaterial', pbrSphere1Transform,irradianceMap,prefilterMap,pbrBrdfLutObj,metallic,0.95);
	let pbrSphere2Transform = setTransform(280, 0, 0, 180, 180, 180, 0, Math.PI, 0);
	loadGLTF(renderer, 'assets/ball/', 'ball', 'PBRMaterial', pbrSphere2Transform,irradianceMap,prefilterMap,pbrBrdfLutObj,metallic,0.85);
	let pbrSphere3Transform = setTransform(200, 0, 0, 180, 180, 180, 0, Math.PI, 0);
	loadGLTF(renderer, 'assets/ball/', 'ball', 'PBRMaterial', pbrSphere3Transform,irradianceMap,prefilterMap,pbrBrdfLutObj,metallic,0.75);
	let pbrSphere4Transform = setTransform(120, 0, 0, 180, 180, 180, 0, Math.PI, 0);
	loadGLTF(renderer, 'assets/ball/', 'ball', 'PBRMaterial', pbrSphere4Transform,irradianceMap,prefilterMap,pbrBrdfLutObj,metallic,0.65);
	let pbrSphere5Transform = setTransform(40, 0, 0, 180, 180, 180, 0, Math.PI, 0);
	loadGLTF(renderer, 'assets/ball/', 'ball', 'PBRMaterial', pbrSphere5Transform,irradianceMap,prefilterMap,pbrBrdfLutObj,metallic,0.55);
	let pbrSphere6Transform = setTransform(-40, 0, 0, 180, 180, 180, 0, Math.PI, 0);
	loadGLTF(renderer, 'assets/ball/', 'ball', 'PBRMaterial', pbrSphere6Transform,irradianceMap,prefilterMap,pbrBrdfLutObj,metallic,0.45);
	let pbrSphere7Transform = setTransform(-120, 0, 0, 180, 180, 180, 0, Math.PI, 0);
	loadGLTF(renderer, 'assets/ball/', 'ball', 'PBRMaterial', pbrSphere7Transform,irradianceMap,prefilterMap,pbrBrdfLutObj,metallic,0.35);
	let pbrSphere8Transform = setTransform(-200, 0, 0, 180, 180, 180, 0, Math.PI, 0);
	loadGLTF(renderer, 'assets/ball/', 'ball', 'PBRMaterial', pbrSphere8Transform,irradianceMap,prefilterMap,pbrBrdfLutObj,metallic,0.25);
	let pbrSphere9Transform = setTransform(-280, 0, 0, 180, 180, 180, 0, Math.PI, 0);
	loadGLTF(renderer, 'assets/ball/', 'ball', 'PBRMaterial', pbrSphere9Transform,irradianceMap,prefilterMap,pbrBrdfLutObj,metallic,0.15);
	let pbrSphere10Transform = setTransform(-360, 0, 0, 180, 180, 180, 0, Math.PI, 0);
	loadGLTF(renderer, 'assets/ball/', 'ball', 'PBRMaterial', pbrSphere10Transform,irradianceMap,prefilterMap,pbrBrdfLutObj,metallic,0.05);

	//Kulla Conty PBR
	let kullaContySphere1Transform = setTransform(360, 60, 0, 180, 180, 180, 0, Math.PI, 0);
	loadGLTF(renderer, 'assets/ball/', 'ball', 'KullaContyMaterial', kullaContySphere1Transform,irradianceMap,prefilterMap,pbrBrdfLutObj,metallic,0.95);
	let kullaContySphere2Transform = setTransform(280, 60, 0, 180, 180, 180, 0, Math.PI, 0);
	loadGLTF(renderer, 'assets/ball/', 'ball', 'KullaContyMaterial', kullaContySphere2Transform,irradianceMap,prefilterMap,pbrBrdfLutObj,metallic,0.85);
	let kullaContySphere3Transform = setTransform(200, 60, 0, 180, 180, 180, 0, Math.PI, 0);
	loadGLTF(renderer, 'assets/ball/', 'ball', 'KullaContyMaterial', kullaContySphere3Transform,irradianceMap,prefilterMap,pbrBrdfLutObj,metallic,0.75);
	let kullaContySphere4Transform = setTransform(120, 60, 0, 180, 180, 180, 0, Math.PI, 0);
	loadGLTF(renderer, 'assets/ball/', 'ball', 'KullaContyMaterial', kullaContySphere4Transform,irradianceMap,prefilterMap,pbrBrdfLutObj,metallic,0.65);
	let kullaContySphere5Transform = setTransform(40, 60, 0, 180, 180, 180, 0, Math.PI, 0);
	loadGLTF(renderer, 'assets/ball/', 'ball', 'KullaContyMaterial', kullaContySphere5Transform,irradianceMap,prefilterMap,pbrBrdfLutObj,metallic,0.55);
	let kullaContySphere6Transform = setTransform(-40, 60, 0, 180, 180, 180, 0, Math.PI, 0);
	loadGLTF(renderer, 'assets/ball/', 'ball', 'KullaContyMaterial', kullaContySphere6Transform,irradianceMap,prefilterMap,pbrBrdfLutObj,metallic,0.45);
	let kullaContySphere7Transform = setTransform(-120, 60, 0, 180, 180, 180, 0, Math.PI, 0);
	loadGLTF(renderer, 'assets/ball/', 'ball', 'KullaContyMaterial', kullaContySphere7Transform,irradianceMap,prefilterMap,pbrBrdfLutObj,metallic,0.35);
	let kullaContySphere8Transform = setTransform(-200, 60, 0, 180, 180, 180, 0, Math.PI, 0);
	loadGLTF(renderer, 'assets/ball/', 'ball', 'KullaContyMaterial', kullaContySphere8Transform,irradianceMap,prefilterMap,pbrBrdfLutObj,metallic,0.25);
	let kullaContySphere9Transform = setTransform(-280, 60, 0, 180, 180, 180, 0, Math.PI, 0);
	loadGLTF(renderer, 'assets/ball/', 'ball', 'KullaContyMaterial', kullaContySphere9Transform,irradianceMap,prefilterMap,pbrBrdfLutObj,metallic,0.15);
	let kullaContySphere10Transform = setTransform(-360, 60, 0, 180, 180, 180, 0, Math.PI, 0);
	loadGLTF(renderer, 'assets/ball/', 'ball', 'KullaContyMaterial', kullaContySphere10Transform,irradianceMap,prefilterMap,pbrBrdfLutObj,metallic,0.05);



	function mainLoop(now) {
		cameraControls.update();

		renderer.render();

		requestAnimationFrame(mainLoop);
	}
	requestAnimationFrame(mainLoop);
}

function setTransform(t_x, t_y, t_z, s_x, s_y, s_z, r_x = 0, r_y = 0, r_z = 0) {
	return {
		modelTransX: t_x,
		modelTransY: t_y,
		modelTransZ: t_z,
		modelScaleX: s_x,
		modelScaleY: s_y,
		modelScaleZ: s_z,
		modelRotateX: r_x,
		modelRotateY: r_y,
		modelRotateZ: r_z,
	};
}
