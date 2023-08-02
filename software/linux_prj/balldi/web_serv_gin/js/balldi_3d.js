	import * as THREE from 'three'
	import { STLLoader } from 'stlloader';

	function create_three(dom)
	{
		var ret = {};
		/**
		 * 创建场景对象Scene
		 */
		var scene = new THREE.Scene();
		/**
		 * 创建网格模型
		 */
		// var geometry = new THREE.SphereGeometry(60, 40, 40); //创建一个球体几何对象
		//var geometry = new THREE.BoxGeometry(100, 100, 100); //创建一个立方体几何对象Geometry
		var loader = new STLLoader();
		loader.load("/balldi.stl", geometry => {
			var material = new THREE.MeshLambertMaterial({color: 0xCCCCCC}); //材质对象Material
			var mesh = new THREE.Mesh(geometry, material); //网格模型对象Mesh
			mesh.scale.set(1,1,1)
			geometry.center();
			scene.add(mesh); //网格模型添加到场景中
			ret.mesh = mesh;
		});
		/**
		 * 光源设置
		 */
		//点光源
		var point = new THREE.PointLight(0xffffff);
		point.position.set(200, -400, 500); //点光源位置
        point.intensity = 0.8;
		scene.add(point); //点光源添加到场景中
		//环境光
		var ambient = new THREE.AmbientLight(0x444444);
		scene.add(ambient);
		// console.log(scene)
		// console.log(scene.children)
		/**
		* 相机设置
		*/
		var width = dom.offsetWidth; //窗口宽度
		var height = dom.offsetHeight; //窗口高度
		var k = width / height; //窗口宽高比
		var s = 100; //三维场景显示范围控制系数，系数越大，显示的范围越大
		//创建相机对象
		var camera = new THREE.OrthographicCamera(-s * k, s * k, s, -s, 1, 1000);
		camera.position.set(0, -200, 20); //设置相机位置
		camera.lookAt(scene.position); //设置相机方向(指向的场景对象)
		/**
		* 创建渲染器对象
		*/
		var renderer = new THREE.WebGLRenderer();
		renderer.setSize(width, height);//设置渲染区域尺寸
		renderer.setClearColor(0xb9d3ff, 1); //设置背景颜色
		dom.appendChild(renderer.domElement); //body元素中插入canvas对象
		//执行渲染操作   指定场景、相机作为参数
		renderer.render(scene, camera);

		ret.scene    = scene;
		ret.camera   = camera;
		ret.renderer = renderer;

		return ret;
		//{
		//	scene:scene,
		//	mesh:mesh,
		//	camera:camera,
		//	renderer:renderer
		//};
	}

	function obj_set_roll(mesh, roll, pitch, yaw)
	{
		if(mesh == undefined) return;

		mesh.rotation.set(
			Math.PI * roll  / 180.0,
			Math.PI * pitch / 180.0,
			Math.PI * yaw   / 180.0
		);
	}

export { create_three, obj_set_roll }
