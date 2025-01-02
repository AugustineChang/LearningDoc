const environ = 'venetian_crossroads_2k'
const ibl_url = `../${environ}/${environ}_ibl.ktx`;
const sky_small_url = `../${environ}/${environ}_skybox_tiny.ktx`;
const sky_large_url = `../${environ}/${environ}_skybox.ktx`;
const albedo_url = `albedo.ktx2`;
const ao_url = `ao.ktx2`;
const metallic_url = `metallic.ktx2`;
const normal_url = `normal.ktx2`;
const roughness_url = `roughness.ktx2`;
const filamesh_url  = `monkey.filamesh`;
const filamat_url = `monkey.filamat`

Filament.init([filamat_url, filamesh_url, sky_small_url, ibl_url], () => {
	// Obtain the canvas DOM object and pass it to the App.
	const canvas = document.getElementsByTagName('canvas')[0];
	window.app = new App(canvas);
} );

class App {
  constructor(canvas) {
	this.canvas = canvas
	this.initScene()
	this.createMonkey()
	  
    this.render = this.render.bind(this);
    this.resize = this.resize.bind(this);
    window.addEventListener('resize', this.resize);
    window.requestAnimationFrame(this.render);
	this.trackball = new Trackball(canvas, {
      homeTilt: 0.25,    // starting tilt in radians
      startSpin: 0.0,   // initial spin in radians per second
      autoTick: true,    // if false, clients must call the tick method once per frame
      friction: 0.25,   // 0 means no friction (infinite spin) while 1 means no inertia
    });
	
	Filament.fetch([sky_large_url, albedo_url, roughness_url, metallic_url, normal_url, ao_url], () => {
		const albedo = this.engine.createTextureFromKtx2(albedo_url, {srgb: true});
		const roughness = this.engine.createTextureFromKtx2(roughness_url);
		const metallic = this.engine.createTextureFromKtx2(metallic_url);
		const normal = this.engine.createTextureFromKtx2(normal_url);
		const ao = this.engine.createTextureFromKtx2(ao_url);

		const sampler = new Filament.TextureSampler(
			Filament.MinFilter.LINEAR_MIPMAP_LINEAR,
			Filament.MagFilter.LINEAR,
			Filament.WrapMode.CLAMP_TO_EDGE);

		this.matInstance.setTextureParameter('albedo', albedo, sampler);
		this.matInstance.setTextureParameter('roughness', roughness, sampler);
		this.matInstance.setTextureParameter('metallic', metallic, sampler);
		this.matInstance.setTextureParameter('normal', normal, sampler);
		this.matInstance.setTextureParameter('ao', ao, sampler);

		// Replace low-res skybox with high-res skybox.
		this.engine.destroySkybox(this.skybox);
		this.skybox = this.engine.createSkyFromKtx1(sky_large_url);
		this.scene.setSkybox(this.skybox);

		this.scene.addEntity(this.meshObject);
	});
	
  }
  render() 
  {
	// interactive with mesh
	const tcm = this.engine.getTransformManager();
	const inst = tcm.getInstance(this.meshObject);
	tcm.setTransform(inst, this.trackball.getMatrix());
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
    const aspect = width / height;
    const FOV = Filament.Camera$Fov;
    const fov = aspect < 1 ? FOV.HORIZONTAL : FOV.VERTICAL;
    this.camera.setProjectionFov(45, aspect, 1.0, 10.0, fov);
  }
  
  initScene()
  {
	const engine = this.engine = Filament.Engine.create(this.canvas);
	
	this.scene = engine.createScene();
	this.swapChain = engine.createSwapChain();
	this.renderer = engine.createRenderer();
	this.camera = engine.createCamera(Filament.EntityManager.get().create());
	this.view = engine.createView();
	this.view.setCamera(this.camera);
	this.view.setScene(this.scene);
	
	// Set up a blue-green background:
	this.renderer.setClearOptions({clearColor: [0.0, 0.0, 0.0, 1.0], clear: true});
	
	// Adjust the initial viewport:
	this.resize();
  }
  
  createMonkey()
  {
	// create material
	const material = this.engine.createMaterial(filamat_url);
	this.matInstance = material.createInstance();

	// create mesh
	const filamesh = this.engine.loadFilamesh(filamesh_url, this.matInstance);
	this.meshObject = filamesh.renderable;
	
	// setup camera
	const eye = [0, 0, 4], center = [0, 0, 0], up = [0, 1, 0];
    this.camera.lookAt(eye, center, up);
	
	// create lights
	this.indirectLight = this.engine.createIblFromKtx1(ibl_url);
	this.indirectLight.setIntensity(100000);
	this.scene.setIndirectLight(this.indirectLight);
	
	// create skybox
	this.skybox = this.engine.createSkyFromKtx1(sky_small_url);
	this.scene.setSkybox(this.skybox);
  }
}