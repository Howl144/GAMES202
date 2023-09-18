var cameraPosition = [30, 30, 30]

//生成的纹理的分辨率，纹理必须是标准的尺寸 256*256 1024*1024  2048*2048
var resolution = 2048;

GAMES202Main();

function GAMES202Main() {
	// Init canvas and gl
	const canvas = document.querySelector('#glcanvas');
	canvas.width = window.screen.width;
	canvas.height = window.screen.height;
	const gl = canvas.getContext('webgl2');
	if (!gl) {
		alert('Unable to initialize WebGL. Your browser or machine may not support it.');
		return;
	}
	console.log('WebGL version: ' + gl.getParameter(gl.VERSION));
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
	let focalPoint = [0, 0, 0];
	let lightUp = [0, 1, 0]
	//第一个光源的亮度
	let lightPos1 = [0, 0, 40];
	const pointLight = new PointLight(2, [1, 1, 1], lightPos1, focalPoint, lightUp, true, renderer.gl);
	renderer.addLight(pointLight);
	//添加第二个光源
	let lightPos2 = [0, 0, -40];
	const pointLight2 = new PointLight(2, [1, 1, 1], lightPos2, focalPoint, lightUp, true, renderer.gl);
	renderer.addLight(pointLight2);
	//添加第三个光源
	let lightPos3 = [40, 0, 0];
	const pointLight3 = new PointLight(2, [1, 1, 1], lightPos3, focalPoint, lightUp, true, renderer.gl);
	renderer.addLight(pointLight3);
	//添加第四个光源
	let lightPos4 = [-40, 0, 0];
	const pointLight4 = new PointLight(2, [1, 1, 1], lightPos4, focalPoint, lightUp, true, renderer.gl);
	renderer.addLight(pointLight4);


	// Add shapes
	// 添加缩放参数
	let floorTransform = setTransform(0, 0, 0, 0, 0, 0, 6, 6, 6);
	let obj1Transform = setTransform(0, 0, 0, 0, 0, 0, 10, 10, 10);

	loadOBJ(renderer, 'assets/mary/', 'Marry', 'PhongMaterial', obj1Transform);
	loadOBJ(renderer, 'assets/floor/', 'floor', 'PhongMaterial', floorTransform);

	// function createGUI() {
	// 	const gui = new dat.gui.GUI();
	// 	// const panelModel = gui.addFolder('Model properties');
	// 	// panelModelTrans.add(GUIParams, 'x').name('X');
	// 	// panelModel.open();
	// }
	// createGUI();

	// deltaTime实现
	let prevTime = 0;

	function mainLoop(now) {
		cameraControls.update();
		let deltaime = (now - prevTime) / 1000;
		renderer.render(now, deltaime);
		requestAnimationFrame(mainLoop);
		prevTime = now;
	}
	
	requestAnimationFrame(mainLoop);
}

// 添加rotate参数
function setTransform(t_x, t_y, t_z, r_x, r_y, r_z, s_x, s_y, s_z) {

	return {
		modelTransX: t_x,
		modelTransY: t_y,
		modelTransZ: t_z,
		modelRotateX: r_x,
		modelRotateY: r_y,
		modelRotateZ: r_z,
		
		modelScaleX: s_x,
		modelScaleY: s_y,
		modelScaleZ: s_z,
	};
}


//角度转弧度
function degrees2Radians(degrees){
	return 3.1415927 / 180 * degrees;
}
