class App {
  constructor() {
	this.initScene()
	this.createTriangle()
	  
    this.render = this.render.bind(this);
    this.resize = this.resize.bind(this);
    window.addEventListener('resize', this.resize);
    window.requestAnimationFrame(this.render);
  }
  render() 
  {
	// Rotate the triangle.
	const radians = Date.now() / 1000;
	const transform = glMatrix.mat4.fromRotation(glMatrix.mat4.create(), radians, [0, 0, 1]);
	const tcm = this.engine.getTransformManager();
	const inst = tcm.getInstance(this.triangle);
	tcm.setTransform(inst, transform);
	inst.delete();
	  
	// Render the frame.
	this.renderer.render(this.swapChain, this.view);
	
    window.requestAnimationFrame(this.render);
  }
  
  resize() 
  {
    const dpr = window.devicePixelRatio;
	const width = this.canvas.width = window.innerWidth * dpr;
	const height = this.canvas.height = window.innerHeight * dpr;
	this.view.setViewport([0, 0, width, height]);
	
	//console.log(width, height)
	
	const aspect = width / height;
	//const Projection = Filament.Camera$Projection;
	//this.camera.setProjection(Projection.ORTHO, -aspect, aspect, -1, 1, 0, 1);
	
    const FOV = Filament.Camera$Fov;
    const fov = aspect < 1 ? FOV.HORIZONTAL : FOV.VERTICAL;
    this.camera.setProjectionFov(45, aspect, 1.0, 10.0, fov);
  }
  
  initScene()
  {
	this.canvas = document.getElementsByTagName('canvas')[0];
	const engine = this.engine = Filament.Engine.create(this.canvas);
	
	this.scene = engine.createScene();
	this.swapChain = engine.createSwapChain();
	this.renderer = engine.createRenderer();
	this.camera = engine.createCamera(Filament.EntityManager.get().create());
	this.view = engine.createView();
	this.view.setCamera(this.camera);
	this.view.setScene(this.scene);
	
	// Set up a blue-green background:
	this.renderer.setClearOptions({clearColor: [0.0, 0.5, 1.0, 1.0], clear: true});

	// setup camera
	const eye = [0, 0, 4], center = [0, 0, 0], up = [0, 1, 0];
    this.camera.lookAt(eye, center, up);

	// Adjust the initial viewport:
	this.resize();
  }
  
  createTriangle()
  {
	const TRIANGLE_POSITIONS = new Float32Array([
		1, 0,
		Math.cos(Math.PI * 2 / 3), Math.sin(Math.PI * 2 / 3),
		Math.cos(Math.PI * 4 / 3), Math.sin(Math.PI * 4 / 3),
	]);

	const TRIANGLE_COLORS = new Uint32Array([0xffff0000, 0xff00ff00, 0xff0000ff]);
	
	const VertexAttribute = Filament.VertexAttribute;
	const AttributeType = Filament.VertexBuffer$AttributeType;
	this.vb = Filament.VertexBuffer.Builder()
		.vertexCount(3)
		.bufferCount(2)
		.attribute(VertexAttribute.POSITION, 0, AttributeType.FLOAT2, 0, 8)
		.attribute(VertexAttribute.COLOR, 1, AttributeType.UBYTE4, 0, 4)
		.normalized(VertexAttribute.COLOR)
		.build(this.engine);

	this.vb.setBufferAt(this.engine, 0, TRIANGLE_POSITIONS);
	this.vb.setBufferAt(this.engine, 1, TRIANGLE_COLORS);
	
	this.ib = Filament.IndexBuffer.Builder()
		.indexCount(3)
		.bufferType(Filament.IndexBuffer$IndexType.USHORT)
		.build(this.engine);
	this.ib.setBuffer(this.engine, new Uint16Array([0, 1, 2]));
	
	const mat = this.engine.createMaterial('triangle.filamat');
	const matinst = mat.getDefaultInstance();
	
	this.triangle = Filament.EntityManager.get().create();
	Filament.RenderableManager.Builder(1)
		.boundingBox({ center: [-1, -1, -1], halfExtent: [1, 1, 1] })
		.material(0, matinst)
		.geometry(0, Filament.RenderableManager$PrimitiveType.TRIANGLES, this.vb, this.ib)
		.build(this.engine, this.triangle);
	this.scene.addEntity(this.triangle);
  }
}

Filament.init(['triangle.filamat'], () => { window.app = new App() } );