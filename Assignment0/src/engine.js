var cameraPosition = [-20, 180, 250];

GAMES202Main();
function GAMES202Main() {
	const canvas = document.querySelector('#glcanvas');
	canvas.width = window.screen.width;
	canvas.height = window.screen.height;
	const gl = canvas.getContext('webgl');
	if (!gl) {
		alert('Unable to initialize WebGL. Your browser or machine may not support it.');
		return;
	}

	const camera = new THREE.PerspectiveCamera(75, gl.canvas.clientWidth / gl.canvas.clientHeight, 0.1, 1000);
	//轨道控制器，three.js的扩展控件
	const cameraControls = new THREE.OrbitControls(camera, canvas);
	cameraControls.enableZoom = true;//缩放
	cameraControls.enableRotate = true;//旋转
	cameraControls.enablePan = true;//平移
	cameraControls.rotateSpeed = 0.3;
	cameraControls.zoomSpeed = 1.0;
	cameraControls.panSpeed = 2.0;

	function setSize(width, height) {
		camera.aspect = width / height;
		camera.updateProjectionMatrix();
	}
	setSize(canvas.clientWidth, canvas.clientHeight);
	//resize:当窗口大小改变时，会触发resize事件，这里监听resize事件，当窗口大小改变时，重新设置相机的宽高比。
	//(parameters) => { statements }，箭头函数的特点是this指向固定，不会随着调用环境的改变而改变。
	window.addEventListener('resize', () => setSize(canvas.clientWidth, canvas.clientHeight));
	//这里position直接调用最开始定的全局变量
	camera.position.set(cameraPosition[0], cameraPosition[1], cameraPosition[2]);
	//设置 (0, 1, 0)的目的是将目标点设置为场景的中心点，这样相机就会默认看向场景的中心。
	cameraControls.target.set(0, 1, 0);
	//第一个参数是光源的强度，第二个参数是光源的颜色
	const pointLight = new PointLight(250, [1, 1, 1]);
	//Add renderer
	const renderer = new WebGLRenderer(gl, camera);
	renderer.addLight(pointLight);
	//加载模型
	loadOBJ(renderer, 'assets/mary/', 'Marry');
	//用来控制模型的位置和大小 
	var guiParams = {
		modelTransX: 0,
		modelTransY: 0,
		modelTransZ: 0,
		modelScaleX: 52,
		modelScaleY: 52,
		modelScaleZ: 52,
	}
	function createGUI() {
		// Create a new GUI instance
		const gui = new dat.gui.GUI();
		// Add a folder to the GUI for model properties
		const panelModel = gui.addFolder('Model properties');
		// Add subfolders for translation and scale
		const panelModelTrans = panelModel.addFolder('Translation');
		const panelModelScale = panelModel.addFolder('Scale');
		// Add controls for X, Y, and Z translation
		panelModelTrans.add(guiParams, 'modelTransX').name('X');
		panelModelTrans.add(guiParams, 'modelTransY').name('Y');
		panelModelTrans.add(guiParams, 'modelTransZ').name('Z');
		// Add controls for X, Y, and Z scale
		panelModelScale.add(guiParams, 'modelScaleX').name('X');
		panelModelScale.add(guiParams, 'modelScaleY').name('Y');
		panelModelScale.add(guiParams, 'modelScaleZ').name('Z');
		// Open all the folders by default
		panelModel.open();
		panelModelTrans.open();
		panelModelScale.open();
	}

	createGUI();

	function mainLoop(now) {
		cameraControls.update();

		renderer.render(guiParams);
		//它是html提供的一个处理动画的API，（）内会定义一个回调函数。这个回调函数会在浏览器下一次重绘之前执行。
		requestAnimationFrame(mainLoop);
	}
	requestAnimationFrame(mainLoop);
}
