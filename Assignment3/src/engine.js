var gl, gl_draw_buffers;

var bufferFBO;
var bumpMap;

// Edit Start
var windowWidth;//用于FBO设置宽
var windowHeight;//用于FBO设置高
var mipMapLevel;//用于SSR材质设置uniform
var depthMeshRender;//Depth Mipmap pass
var depthDebugMeshRender;//debugeDepth pass
// Edit End

GAMES202Main();

function GAMES202Main() {
	// Init canvas
	const canvas = document.querySelector('#glcanvas');
	canvas.width = window.screen.width;
	canvas.height = window.screen.height;

	// Edit Start
	windowWidth = window.screen.width;
	windowHeight = window.screen.height;

	gl = canvas.getContext('webgl2');
	if (!gl) {
		alert('Unable to initialize WebGL. Your browser or machine may not support it.');
		return;
	}

	let ext = gl.getExtension('EXT_color_buffer_float')
	if (!ext) {
		alert("Need EXT_color_buffer_float");
		return;
	}
	// Edit End

	// Add camera
	let near = 1e-3;
	let far  = 500;
	const camera = new THREE.PerspectiveCamera(75, gl.canvas.clientWidth / gl.canvas.clientHeight,near, far);
	//Add Camera
	let cameraPosition, cameraTarget , uZBufferParams;
	// Add light
	let lightPos, lightDir, lightRadiance;
	uZBufferParams = [near, far,0];



	// // Cube
	// cameraPosition = [6, 1, 0]
	// cameraTarget = [0, 0, 0]
	// // Cube
	// lightRadiance = [1, 1, 1];
	// lightPos = [-2, 4, 1];
	// lightDir = {
	// 	'x': 0.4,
	// 	'y': -0.9,
	// 	'z': -0.2,
	// };

	// Cave
	lightRadiance = [20, 20, 20];
	lightPos = [-0.45, 5.40507, 0.637043];
	lightDir = {
		'x': 0.39048811,
		'y': -0.89896828,
		'z': 0.19843153,
	};
	//Cave
	cameraPosition = [4.18927, 1.0313, 2.07331]
	cameraTarget = [2.92191, 0.98, 1.55037]

	
	
	
	camera.position.set(cameraPosition[0], cameraPosition[1], cameraPosition[2]);
	camera.fbo = new FBO(gl, 4);

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
	cameraControls.target.set(cameraTarget[0], cameraTarget[1], cameraTarget[2]);

	// Add renderer
	const renderer = new WebGLRenderer(gl, camera);

	let lightUp = [1, 0, 0];
	const directionLight = new DirectionalLight(lightRadiance, lightPos, lightDir, lightUp, renderer.gl);
	renderer.addLight(directionLight);

	// Add shapes
	let ssrFragWorldSpace = "./src/shaders/ssrShader/ssrFragment.glsl";
	let ssrFragTextrueSpace = "./src/shaders/ssrShader/ssrFragment_TextureSpace.glsl";
	let ssrFragHizTextureSpace = "./src/shaders/ssrShader/ssrFragment_HizTextureSpace.glsl";
	loadGLTF(renderer, 'assets/cube/', 'cube1', 'SSRMaterial',uZBufferParams,ssrFragHizTextureSpace);
	// loadGLTF(renderer, 'assets/cube/', 'cube2', 'SSRMaterial',uZBufferParams,ssrFragWorldSpace);
	// loadGLTF(renderer, 'assets/cave/', 'cave', 'SSRMaterial',uZBufferParams,ssrFragHizTextureSpace);
 
	// Edit Start
	// mipMapLevel = 5;
	mipMapLevel = Math.floor(Math.log2(Math.max(window.screen.width, window.screen.height)));

	let currentWidth = window.screen.width;
	let currentHeight = window.screen.height;
	let depthTexture = camera.fbo.textures[1];

	for (let i = 0; i < mipMapLevel; i++) {
		let lastWidth = currentWidth;
		let lastHeight = currentHeight;

		if(i >0){
			// calculate next viewport size
			currentWidth /= 2;
			currentHeight /= 2;

			currentWidth = Math.floor(currentWidth);
			currentHeight = Math.floor(currentHeight);

			// ensure that the viewport size is always at least 1x1
			currentWidth = currentWidth > 0 ? currentWidth : 1;
			currentHeight = currentHeight > 0 ? currentHeight : 1;
		}
		console.log("MipMap Level", i, ":", currentWidth, "x", currentHeight);
		let fb = new FBO(gl, 1, currentWidth, currentHeight);
		fb.lastWidth = lastWidth;
		fb.lastHeight = lastHeight;
		fb.width = currentWidth;
		fb.height = currentHeight;
		renderer.addDepthFBO(fb);
	}

	depthMaterial = buildSceneDepthMaterial(depthTexture, "./src/shaders/sceneDepthShader/depthVertex.glsl", "./src/shaders/sceneDepthShader/depthFragment.glsl");
	depthMaterial.then((data) => {
		depthMeshRender = new MeshRender(renderer.gl, Mesh.Quad(setTransform(0, 0, 0, 1, 1, 1)), data);
	});

	let depthDebugMaterial = buildSceneDepthDebugMaterial(uZBufferParams,"./src/shaders/sceneDepthDebugShader/depthDebugVertex.glsl", "./src/shaders/sceneDepthDebugShader/deptDebughFragment.glsl");
	depthDebugMaterial.then((data) => {
		depthDebugMeshRender = new MeshRender(renderer.gl, Mesh.Quad(setTransform(0, 0, 0, 1, 1, 1)), data);
	});
	// Edit End

	function createGUI() {
		const gui = new dat.gui.GUI();
		const lightPanel = gui.addFolder('Directional Light');
		lightPanel.add(renderer.lights[0].entity.lightDir, 'x', -10, 10, 0.1);
		lightPanel.add(renderer.lights[0].entity.lightDir, 'y', -10, 10, 0.1);
		lightPanel.add(renderer.lights[0].entity.lightDir, 'z', -10, 10, 0.1);
		lightPanel.open();
	}
	createGUI();

	//Edit Start deltaTime实现
	let prevTime = 0;
	var frames = 0;
    var updateTime = 0;
	function mainLoop(now) {
		cameraControls.update();
		frames++;
		let deltaime = (now - prevTime) / 1000;
		updateTime += deltaime;
		if (updateTime > 1) { 
            var fps = frames / updateTime;
            console.log("FPS: " + fps);
            updateTime = 0; 
            frames = 0; 
        }
		renderer.render(now, deltaime);
		requestAnimationFrame(mainLoop);
		prevTime = now;
	}
	//Edit End
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
