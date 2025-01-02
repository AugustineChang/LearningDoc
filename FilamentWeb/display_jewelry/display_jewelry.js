const environ = 'studio_small_08_2k'
const ibl_url = `../${environ}/${environ}_ibl.ktx`;
const sky_small_url = `../${environ}/${environ}_skybox_tiny.ktx`;
const sky_large_url = `../${environ}/${environ}_skybox.ktx`;

const standardMat_url = "shaders/standard_pbr.filamat"
const diamondMat_url = "shaders/diamond.filamat"
const glassMat_url = "shaders/glass.filamat"
const subsurfaceMat_url = "shaders/subsurface.filamat"
const showNormalMat_url = "shaders/showNormal.filamat"
const debugSphereMat_url = "shaders/debugSphere.filamat"

const showTextureMat_url = "shaders/showTexture.filamat"
const show_texture_url = ""

const debugGlbMesh_url = "glbs/328264a565994e70b8e1bfdec3058eb2.glb"

const defaultNormalTex_url = "materialsLib/textures/common/T_Flat_Normal.ktx2"
const defaultAOTex_url = "materialsLib/textures/common/T_White_Linear.ktx2"
const defaultBCTex_url = "materialsLib/textures/common/T_White_Linear.ktx2"
const defaultRoughnessTex_url = "materialsLib/textures/common/T_White_Linear.ktx2"
const defaultMetallicTex_url = "materialsLib/textures/common/T_White_Linear.ktx2"

/////////////////////////////////////////////config/////////////////////////////////////////////
const jewelryName = "cdd7626b-dea2-4ebf-984b-627055caa203"//"6c84fa7b61422a516d49d259bef8f0ceba19360f9050f4f49e722d29a207c75d"//"XG3983"//"r258-1"//"RG32448A-2"//"Diamond_Brilliant"
const metalMatInfoPath = "materialsLib/showNormal.txt"
const diamondMatInfoPath = `meshesLib/${jewelryName}/facet_info.txt`
////////////////////////////////////////////////////////////////////////////////////////////////

Filament.init([sky_small_url, ibl_url,	defaultNormalTex_url, defaultBCTex_url, defaultRoughnessTex_url, defaultMetallicTex_url], () => {
	window.VertexAttribute = Filament.VertexAttribute;
	window.AttributeType = Filament.VertexBuffer$AttributeType;
	window.PrimitiveType = Filament.RenderableManager$PrimitiveType;
	window.IndexType = Filament.IndexBuffer$IndexType;
	
	// Obtain the canvas DOM object and pass it to the App.
	const canvas = document.getElementsByTagName('canvas')[0];
	window.app = new App(canvas);
} );

function Uint8ArrayToString(fileData)
{
  var dataString = "";
  for (var i = 0; i < fileData.length; i++) {
    dataString += String.fromCharCode(fileData[i]);
  }
  return dataString
}

class App {
  constructor(canvas) {
	this.canvas = canvas
	this.initScene();
	//this.createDisplayQuad();
	//this.createDebugBall();
	//this.createDebugGlbObject();
	  
    this.render = this.render.bind(this);
    this.resize = this.resize.bind(this);
    window.addEventListener('resize', this.resize);
    window.requestAnimationFrame(this.render);
	this.trackball = new Trackball(canvas, {
      homeTilt: -0.1,    // starting tilt in radians
      homeSpin: -1.57,    // starting tilt in radians
      startSpin: 0.0,   // initial spin in radians per second
      autoTick: true,    // if false, clients must call the tick method once per frame
      friction: 0.25,   // 0 means no friction (infinite spin) while 1 means no inertia
    });
	
	this.materialsMap = { 
		"standard" : standardMat_url, 
		"diamond" : diamondMat_url, 
		"glass" : glassMat_url, 
		"subsurface" : subsurfaceMat_url, 
		"showNormal" : showNormalMat_url
	};
	
	this.onLoadInfoComplete = this.onLoadInfoComplete.bind(this);
	this.onLoadAssetsCompelete = this.onLoadAssetsCompelete.bind(this);
	Filament.fetch([metalMatInfoPath, diamondMatInfoPath], this.onLoadInfoComplete);
  }
  
  onLoadInfoComplete()
  {
	this.jewelryInfos = this.getJewelryInfo()
	
	this.metalMatNumber = 0;
	this.diamondMatNumber = 0;
	for (var i = 0; i < this.jewelryInfos["MatTypes"].length; i++)
	{
		const matType = this.jewelryInfos["MatTypes"][i]
		if (matType == 0)
			this.metalMatNumber += 1
		else if (matType == 1)
			this.diamondMatNumber += 1
	}
	
	this.jewelryAssetsList = []
	
	// mesh
	this.jewelryAssetsList.push(`meshesLib/${jewelryName}/${jewelryName}.filamesh`)
	
	// diamond facet textures
	for (var i=0; i<this.jewelryInfos["MatTypes"].length; i++)
	{
		if (this.jewelryInfos["MatTypes"][i] != 1)
			continue
		
		if (i < 10)
			this.jewelryAssetsList.push(`meshesLib/${jewelryName}/FacetTexture_0${i}.ktx1`);
		else
			this.jewelryAssetsList.push(`meshesLib/${jewelryName}/FacetTexture_${i}.ktx1`);
	}
	
	// skyBox
	this.jewelryAssetsList.push(sky_large_url)
	
	// material textures
	for (var key in this.jewelryInfos["MetalPBR"])
	{
		const val = this.jewelryInfos["MetalPBR"][key]
		if (key != "Shader" && typeof(val) == "string")
		{
			this.jewelryAssetsList.push(val)
		}
	}
	
	// shaders
	if (this.diamondMatNumber > 0)
		this.jewelryAssetsList.push(this.materialsMap["diamond"])
	this.jewelryAssetsList.push(this.materialsMap[this.jewelryInfos["MetalPBR"]["Shader"]])
	
	Filament.fetch(this.jewelryAssetsList, this.onLoadAssetsCompelete);
  }
  
  onLoadAssetsCompelete()
  {
	this.createJewelryWithDiamond();
	
	// Set DiamondMat Params
	if (this.diamondMatNumber > 0 && typeof(this.jewelryMatInsts) != "undefined")
	{
		const SkyBgTexture = this.engine.createTextureFromKtx1(ibl_url);
		const SkyBgSampler = new Filament.TextureSampler(
			Filament.MinFilter.LINEAR_MIPMAP_LINEAR,
			Filament.MagFilter.LINEAR,
			Filament.WrapMode.CLAMP_TO_EDGE);
		
		const facetSampler = new Filament.TextureSampler(
			Filament.MinFilter.NEAREST,
			Filament.MagFilter.NEAREST,
			Filament.WrapMode.CLAMP_TO_EDGE);
		
		var counter = 1
		for (var i = 0; i < this.jewelryInfos["MatTypes"].length; i++)
		{
			if (this.jewelryInfos["MatTypes"][i] != 1)
				continue
			
			const matKey = `jewelMat${i}`
			const infoKey = `Section${i}`
			const diamondMat = this.jewelryMatInsts[matKey]
			
			const facetTexture = this.engine.createTextureFromKtx1(this.jewelryAssetsList[counter]);
			diamondMat.setTextureParameter('facetTexture', facetTexture, facetSampler);
			diamondMat.setTextureParameter('skyColorTexture', SkyBgTexture, SkyBgSampler);
			
			diamondMat.setFloatParameter('facetTextureWidth', this.jewelryInfos["Facet"][infoKey][0]);
			diamondMat.setFloatParameter('numOfFacets', this.jewelryInfos["Facet"][infoKey][1]);
			diamondMat.setFloatParameter('absorptionScale', 8.0);
			diamondMat.setFloatParameter('numOfBounce', 8.0);
			diamondMat.setFloatParameter('refractiveIndex', 2.42);
			diamondMat.setFloatParameter('skyColorBrightness', 0.75);
			diamondMat.setColor3Parameter('gemColor', Filament.RgbType.LINEAR, [0.37, 0.83, 0.83]);
			
			counter += 1
		}
	}
	
	if (this.metalMatNumber > 0 && typeof(this.jewelryMatInsts) != "undefined")
	{
		const metalMat = this.jewelryMatInsts["metalMat"]
		const pbrData = this.jewelryInfos["MetalPBR"]
		const shaderName = pbrData["Shader"]
		
		const TrilinearSampler = new Filament.TextureSampler(
			Filament.MinFilter.LINEAR_MIPMAP_LINEAR,
			Filament.MagFilter.LINEAR,
			Filament.WrapMode.REPEAT);
		
		// Set StandardMat Params
		if (shaderName == "standard")
		{
			const BCVal = pbrData["BC"]
			if (typeof(BCVal) == "string")
			{
				const BCTexture = this.engine.createTextureFromKtx2(BCVal, {srgb: true});
				metalMat.setTextureParameter('BCTex', BCTexture, TrilinearSampler);
				
				const BC2Val = pbrData["BC2"]
				if (typeof(BC2Val) != "undefined")
					metalMat.setColor3Parameter("BC", Filament.RgbType.LINEAR, BC2Val)
				else
					metalMat.setColor3Parameter("BC", Filament.RgbType.LINEAR, [1.0, 1.0, 1.0])
			}
			else
			{
				const defaultBCTexture = this.engine.createTextureFromKtx2(defaultBCTex_url);
				metalMat.setTextureParameter('BCTex', defaultBCTexture, TrilinearSampler);
				metalMat.setColor3Parameter("BC", Filament.RgbType.LINEAR, BCVal)
			}
			
			const BrightnessVal = pbrData["Brightness"]
			if (typeof(BrightnessVal) == "object")
			{
				metalMat.setFloat3Parameter("Brightness", BrightnessVal)
			}
			else
			{
				metalMat.setFloat3Parameter("Brightness", [1,1,1])
			}
			
			const RoughnessVal = pbrData["Roughness"]
			if (typeof(RoughnessVal) == "string")
			{
				const RoughnessTexture = this.engine.createTextureFromKtx2(RoughnessVal);
				metalMat.setTextureParameter('RoughnessTex', RoughnessTexture, TrilinearSampler);
				metalMat.setFloatParameter("Roughness", 1.0)
			}
			else
			{
				const RoughnessTexture = this.engine.createTextureFromKtx2(defaultRoughnessTex_url);
				metalMat.setTextureParameter('RoughnessTex', RoughnessTexture, TrilinearSampler);
				metalMat.setFloatParameter("Roughness", RoughnessVal)
			}
			
			const MetallicVal = pbrData["Metallic"]
			if (typeof(MetallicVal) == "string")
			{
				const MetallicTexture = this.engine.createTextureFromKtx2(MetallicVal);
				metalMat.setTextureParameter('MetallicTex', MetallicTexture, TrilinearSampler);
				metalMat.setFloatParameter("Metallic", 1.0)
			}
			else
			{
				const MetallicTexture = this.engine.createTextureFromKtx2(defaultMetallicTex_url);
				metalMat.setTextureParameter('MetallicTex', MetallicTexture, TrilinearSampler);
				metalMat.setFloatParameter("Metallic", MetallicVal)
			}
			
			const NormalVal = pbrData["Normal"]
			if (typeof(NormalVal) == "string")
			{
				const NormalTexture = this.engine.createTextureFromKtx2(NormalVal);
				metalMat.setTextureParameter('NormalTex', NormalTexture, TrilinearSampler);
			}
			else
			{
				const NormalTexture = this.engine.createTextureFromKtx2(defaultNormalTex_url);
				metalMat.setTextureParameter('NormalTex', NormalTexture, TrilinearSampler);
			}
			
			const AOVal = pbrData["AO"]
			if (typeof(AOVal) == "string")
			{
				const AOTexture = this.engine.createTextureFromKtx2(AOVal);
				metalMat.setTextureParameter('AOTex', AOTexture, TrilinearSampler);
			}
			else
			{
				const AOTexture = this.engine.createTextureFromKtx2(defaultAOTex_url);
				metalMat.setTextureParameter('AOTex', AOTexture, TrilinearSampler);
			}
			
			const TilingVal = pbrData["Tiling"]
			if (typeof(TilingVal) == "number")
			{
				metalMat.setFloat2Parameter('TexTiling', [TilingVal, TilingVal]);
			}
			else
			{
				metalMat.setFloat2Parameter('TexTiling', [1.0, 1.0]);
			}
			
			/*const meshOffset = this.jewelryInfos["BoundingBox"][0]
			meshOffset[0] = Math.min(meshOffset[0], 0.0)
			meshOffset[1] = Math.min(meshOffset[1], 0.0)
			meshOffset[2] = Math.min(meshOffset[2], 0.0)
			meshOffset[0] = Math.abs(meshOffset[0])
			meshOffset[1] = Math.abs(meshOffset[1])
			meshOffset[2] = Math.abs(meshOffset[2])
			const offsetScale = [
				meshOffset[0],
				meshOffset[1],
				meshOffset[2],
				this.jewelryInfos["Scaling"][0],
			]
			metalMat.setFloat4Parameter('MeshOffsetScale', offsetScale);*/
			
			const NormalStrengthVal = pbrData["NormalStrength"]
			if (typeof(NormalStrengthVal) == "number")
			{
				metalMat.setFloatParameter('NormalStrength', NormalStrengthVal);
			}
			else
			{
				metalMat.setFloatParameter('NormalStrength', 1.0);
			}
			
			const ClearCoatVal = pbrData["ClearCoat"]
			if (typeof(ClearCoatVal) == "number")
			{
				metalMat.setFloatParameter('ClearCoat', ClearCoatVal);
			}
			else
			{
				metalMat.setFloatParameter('ClearCoat', 0.0);
			}
			
			const ClearCoatHeightVal = pbrData["ClearCoatHeight"]
			if (typeof(ClearCoatHeightVal) == "number")
			{
				metalMat.setFloatParameter('ClearCoatHeight', ClearCoatHeightVal);
			}
			else
			{
				metalMat.setFloatParameter('ClearCoatHeight', 0.0);
			}
			
			const ClearCoatRoughnessVal = pbrData["ClearCoatRoughness"]
			if (typeof(ClearCoatRoughnessVal) == "number")
			{
				const CCRoughnessTexture = this.engine.createTextureFromKtx2(defaultRoughnessTex_url);
				metalMat.setTextureParameter('ClearCoatRoughnessTex', CCRoughnessTexture, TrilinearSampler);
				metalMat.setFloatParameter('ClearCoatRoughness', ClearCoatRoughnessVal);
			}
			else if (typeof(ClearCoatRoughnessVal) == "string")
			{
				const CCRoughnessTexture = this.engine.createTextureFromKtx2(ClearCoatRoughnessVal);
				metalMat.setTextureParameter('ClearCoatRoughnessTex', CCRoughnessTexture, TrilinearSampler);
				metalMat.setFloatParameter('ClearCoatRoughness', 1.0);
			}
			else
			{
				const CCRoughnessTexture = this.engine.createTextureFromKtx2(defaultRoughnessTex_url);
				metalMat.setTextureParameter('ClearCoatRoughnessTex', CCRoughnessTexture, TrilinearSampler);
				metalMat.setFloatParameter('ClearCoatRoughness', 1.0);
			}
			
			const ClearCoatNormalVal = pbrData["ClearCoatNormal"]
			if (typeof(ClearCoatNormalVal) == "string")
			{
				const ClearCoatNormalTexture = this.engine.createTextureFromKtx2(ClearCoatNormalVal);
				metalMat.setTextureParameter('ClearCoatNormalTex', ClearCoatNormalTexture, TrilinearSampler);
			}
			else
			{
				const ClearCoatNormalTexture = this.engine.createTextureFromKtx2(defaultNormalTex_url);
				metalMat.setTextureParameter('ClearCoatNormalTex', ClearCoatNormalTexture, TrilinearSampler);
			}
			
			const ClearCoatNormalStrengthVal = pbrData["ClearCoatNormalStrength"]
			if (typeof(ClearCoatNormalStrengthVal) == "number")
			{
				metalMat.setFloatParameter('ClearCoatNormalStrength', ClearCoatNormalStrengthVal);
			}
			else
			{
				metalMat.setFloatParameter('ClearCoatNormalStrength', 1.0);
			}
		}
		
		// Set GlassMat Params
		else if (shaderName == "glass")
		{
			const BCVal = pbrData["BC"]
			if (typeof(BCVal) == "object")
			{
				metalMat.setFloat3Parameter("GlassColor", BCVal)
			}
			else
			{
				metalMat.setFloat3Parameter("GlassColor", [1.0, 1.0, 1.0])
			}
			
			const RoughnessVal = pbrData["Roughness"]
			if (typeof(RoughnessVal) == "number")
			{
				metalMat.setFloatParameter('Roughness', RoughnessVal);
			}
			else
			{
				metalMat.setFloatParameter('Roughness', 0.05);
			}
			
			const MetallicVal = pbrData["Metallic"]
			if (typeof(MetallicVal) == "number")
			{
				metalMat.setFloatParameter("Metallic", MetallicVal)
			}
			else
			{
				metalMat.setFloatParameter("Metallic", 0.0)
			}
			
			const NormalVal = pbrData["Normal"]
			if (typeof(NormalVal) == "string")
			{
				const NormalTexture = this.engine.createTextureFromKtx2(NormalVal);
				metalMat.setTextureParameter('NormalTex', NormalTexture, TrilinearSampler);
			}
			else
			{
				const NormalTexture = this.engine.createTextureFromKtx2(defaultNormalTex_url);
				metalMat.setTextureParameter('NormalTex', NormalTexture, TrilinearSampler);
			}
			
			const TilingVal = pbrData["Tiling"]
			if (typeof(TilingVal) == "number")
			{
				metalMat.setFloat2Parameter('TexTiling', [TilingVal, TilingVal]);
			}
			else
			{
				metalMat.setFloat2Parameter('TexTiling', [1.0, 1.0]);
			}
			
			const NormalStrengthVal = pbrData["NormalStrength"]
			if (typeof(NormalStrengthVal) == "number")
			{
				metalMat.setFloatParameter('NormalStrength', NormalStrengthVal);
			}
			else
			{
				metalMat.setFloatParameter('NormalStrength', 1.0);
			}
			
			const IORVal = pbrData["IOR"]
			if (typeof(IORVal) == "number")
			{
				metalMat.setFloatParameter('IOR', IORVal);
			}
			else
			{
				metalMat.setFloatParameter('IOR', 1.0);
			}
		}
		
		// Set SubsurfaceMat Params
		else if (shaderName == "subsurface")
		{
			const BCVal = pbrData["BC"]
			if (typeof(BCVal) == "object")
			{
				metalMat.setFloat3Parameter("BC", BCVal)
			}
			else
			{
				metalMat.setFloat3Parameter("BC", [1.0, 1.0, 1.0])
			}
			
			const RoughnessVal = pbrData["Roughness"]
			if (typeof(RoughnessVal) == "number")
			{
				metalMat.setFloatParameter('Roughness', RoughnessVal);
			}
			else
			{
				metalMat.setFloatParameter('Roughness', 0.05);
			}
			
			const MetallicVal = pbrData["Metallic"]
			if (typeof(MetallicVal) == "number")
			{
				metalMat.setFloatParameter("Metallic", MetallicVal)
			}
			else
			{
				metalMat.setFloatParameter("Metallic", 0.0)
			}
			
			const NormalVal = pbrData["Normal"]
			if (typeof(NormalVal) == "string")
			{
				const NormalTexture = this.engine.createTextureFromKtx2(NormalVal);
				metalMat.setTextureParameter('NormalTex', NormalTexture, TrilinearSampler);
			}
			else
			{
				const NormalTexture = this.engine.createTextureFromKtx2(defaultNormalTex_url);
				metalMat.setTextureParameter('NormalTex', NormalTexture, TrilinearSampler);
			}
			
			const TilingVal = pbrData["Tiling"]
			if (typeof(TilingVal) == "number")
			{
				metalMat.setFloat2Parameter('TexTiling', [TilingVal, TilingVal]);
			}
			else
			{
				metalMat.setFloat2Parameter('TexTiling', [1.0, 1.0]);
			}
			
			const NormalStrengthVal = pbrData["NormalStrength"]
			if (typeof(NormalStrengthVal) == "number")
			{
				metalMat.setFloatParameter('NormalStrength', NormalStrengthVal);
			}
			else
			{
				metalMat.setFloatParameter('NormalStrength', 1.0);
			}
			
			const subsurfacePowerVal = pbrData["SubsurfacePower"]
			if (typeof(subsurfacePowerVal) == "number")
			{
				metalMat.setFloatParameter("SubsurfacePower", subsurfacePowerVal)
			}
			else
			{
				metalMat.setFloatParameter("SubsurfacePower", 12.234)
			}
			
			const subsurfaceColorVal = pbrData["SubsurfaceColor"]
			if (typeof(subsurfaceColorVal) == "object")
			{
				metalMat.setFloat3Parameter("SubsurfaceColor", subsurfaceColorVal)
			}
			else
			{
				metalMat.setFloat3Parameter("SubsurfaceColor", [1.0, 1.0, 1.0])
			}
		}
	}
	
	// Set show texture
	if (typeof(this.showMatInst) != "undefined")
	{
		const showTexture = this.engine.createTextureFromKtx1(show_texture_url);
		const sampler = new Filament.TextureSampler(
			Filament.MinFilter.NEAREST,
			Filament.MagFilter.NEAREST,
			Filament.WrapMode.CLAMP_TO_EDGE);
		this.showMatInst.setTextureParameter('displayTexture', showTexture, sampler);
	}

	// Replace low-res skybox with high-res skybox.
	if (typeof(this.skybox) != "undefined")
	{
		this.engine.destroySkybox(this.skybox);
		this.skybox = this.engine.createSkyFromKtx1(sky_large_url);
		this.scene.setSkybox(this.skybox);
	}
	
	if (typeof(this.jewelryObject) != "undefined")
	{
		this.scene.addEntity(this.jewelryObject);
	}
  }
  
  render() 
  {
	// interactive with mesh
	if (typeof(this.jewelryObject) != "undefined")
	{
		const tcm = this.engine.getTransformManager();
		const inst = tcm.getInstance(this.jewelryObject);
		
		var objMatrix = glMatrix.mat4.clone(this.trackball.getMatrixX())
		glMatrix.mat4.scale(objMatrix, objMatrix, this.jewelryInfos["Scaling"])
		glMatrix.mat4.translate(objMatrix, objMatrix, this.jewelryInfos["Offset"])
		
		tcm.setTransform(inst, objMatrix);
		inst.delete();
		
		//const eye = [0, 0, 3], center = [0, 0, 0], up = [0, 1, 0];
		//glMatrix.vec3.transformMat4(eye, eye, this.trackball.getMatrix())
		//this.camera.lookAt(eye, center, up);
	}
	else if (typeof(this.glbMeshObject) != "undefined")
	{
		const tcm = this.engine.getTransformManager();
		const inst = tcm.getInstance(this.glbMeshObject.getRoot());
		tcm.setTransform(inst, this.trackball.getMatrixX());
		inst.delete();
	}
	
	if (typeof(this.debugBall) != "undefined")
	{
		const vertPos = [1.313, 32.665, -55.38]//[0.0, -0.42554, 0.0]//[0.08984, 0.27832, -0.21692]
		const faceNormal = [0.22454, 0.94026, -0.25593]//[0.20032, 0.85205, -0.48364]
		
		const ballPos = glMatrix.vec3.scale(glMatrix.vec3.create(), faceNormal, 0.1)
		glMatrix.vec3.add(ballPos, ballPos, vertPos)
		const lookTarget = glMatrix.vec3.add(glMatrix.vec3.create(), vertPos, faceNormal)
		const up = [0, 1, 0]
		
		const transform = glMatrix.mat4.create()
		glMatrix.mat4.targetTo(transform, ballPos, lookTarget, up)
		glMatrix.mat4.scale(transform, transform, [0.02, 0.02, 0.1])
		const tcm = this.engine.getTransformManager();
		const inst = tcm.getInstance(this.debugBall);
		tcm.setTransform(inst, transform);
		inst.delete();
	}
	
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
	
	// post-process
	this.view.setBloomOptions({ 
		strength: 0.2,
		enabled: true,
		
		levels: 3,
		resolution: 32
	});
	
	// Set up a blue-green background:
	this.renderer.setClearOptions({clearColor: [0.0, 0.0, 0.0, 1.0], clear: true});
	
	// setup camera
	const eye = [-3, 0, 0], center = [0, 0, 0], up = [0, 1, 0];
    this.camera.lookAt(eye, center, up);
	
	// create lights
	this.indirectLight = this.engine.createIblFromKtx1(ibl_url);
	this.indirectLight.setIntensity(31400);
	this.scene.setIndirectLight(this.indirectLight);
	
	// create skybox
	this.skybox = this.engine.createSkyFromKtx1(sky_small_url);
	this.scene.setSkybox(this.skybox);
	
	// Adjust the initial viewport:
	this.resize();
  }
  
  createJewelryWithDiamond()
  {
	// create material
	const shaderName = this.jewelryInfos["MetalPBR"]["Shader"]
	const jewelryMaterial = this.metalMatNumber > 0 ? this.engine.createMaterial(this.materialsMap[shaderName]) : null;
	const diamondMaterial = this.diamondMatNumber > 0 ? this.engine.createMaterial(diamondMat_url) : null;
	
	this.jewelryMatInsts = {}
	
	const metalMatInst = this.metalMatNumber > 0 ? jewelryMaterial.createInstance() : null
	this.jewelryMatInsts["metalMat"] = metalMatInst;
	
	for (var m = 0; m < this.jewelryInfos["MatTypes"].length; m++)
	{
		if (this.jewelryInfos["MatTypes"][m] == 1)
			this.jewelryMatInsts[`jewelMat${m}`] = diamondMaterial.createInstance();
	}
	
	// create mesh
	const filamesh = this.engine.loadFilamesh(this.jewelryAssetsList[0], metalMatInst, this.jewelryMatInsts);
	this.jewelryObject = filamesh.renderable;
  }
  
  getJewelryInfo()
  {
	var infoMap = {}
	
	const pbrString = Uint8ArrayToString(Filament.assets[metalMatInfoPath])
	var pbrLines = pbrString.split("\r\n");
	
	function getPBRValue(key, value)
	{
		const numberVal = Number(value)
		if (!isNaN(numberVal))
		{
			return numberVal
		}
		else if (value.startsWith("["))
		{
			value = value.replace("[", "");
			value = value.replace("]", "");
			const valList = value.split(",")
			return [Number(valList[0]), Number(valList[1]), Number(valList[2])]
		}
		else if (key.includes("Shader"))
			return value;
		else
			return `materialsLib/textures/${value}`;
	}
	
	infoMap["MetalPBR"] = { "Shader" : "standard", "BC" : [1.0, 1.0, 1.0], "Roughness" : 1.0, "Metallic" : 0.0 };
	pbrLines.forEach(function(line) {
		const midIndex = line.indexOf(": ")
		const key = line.slice(0, midIndex)
		const value = line.slice(midIndex+2)
		
		infoMap["MetalPBR"][key] = getPBRValue(key, value)
	});
	
	///////////////////////////////////////////////////////////////////
	
	const infoString = Uint8ArrayToString(Filament.assets[diamondMatInfoPath])	
	var infoLines = infoString.split("\r\n");
	
	infoMap["Facet"] = {}
	infoLines.forEach(function(line) {
		const midIndex = line.indexOf(": ")
		const key = line.slice(0, midIndex)
		const value = line.slice(midIndex+2)
		
		if (key.includes("Section"))
		{
			var valList = value.split(" ")
			valList[0] = parseInt(valList[0], 10);
			valList[1] = parseInt(valList[1], 10);
			
			infoMap["Facet"][key] = valList;
		}
		else if (key.includes("BoundingBox"))
		{
			var valList = value.split(";")
			var minVec = valList[0].split(",")
			var maxVec = valList[1].split(",")
			
			for (var i = 0; i < 3; i++)
			{
				minVec[i] = parseFloat(minVec[i]);
				maxVec[i] = parseFloat(maxVec[i]);
			}
			
			infoMap[key] = [minVec, maxVec]
		}
		else if (key.includes("MatTypes"))
		{
			var typesStr = value.replace("[", "")
			typesStr = typesStr.replace("]", "")
			
			var valList = typesStr.split(",")
			var typesList = []
			for (var i = 0; i < valList.length; i++)
			{
				const oneType = parseInt(valList[i], 10);
				typesList[i] = oneType;
			}
			infoMap[key] = typesList
		}
	});
	
	const objSize = glMatrix.vec3.create()
	glMatrix.vec3.subtract(objSize, infoMap["BoundingBox"][1], infoMap["BoundingBox"][0])
	const objMaxSize = Math.max(objSize[0], Math.max(objSize[1], objSize[2]))
	const uniformScale = 2.0 / objMaxSize
	infoMap["Scaling"] = [uniformScale, uniformScale, uniformScale]
	
	const objCenter = glMatrix.vec3.create()
	glMatrix.vec3.add(objCenter, infoMap["BoundingBox"][0], infoMap["BoundingBox"][1])
	glMatrix.vec3.scale(objCenter, objCenter, -0.5)
	infoMap["Offset"] = objCenter
	
	///////////////////////////////////////////////////////////////////
	
	console.log(infoMap)
	
	return infoMap
  }
  
  createDebugBall()
  {
	// create material
	const material = this.engine.createMaterial(debugSphereMat_url);
	const matInstance = material.createInstance();
	
	// create sphere
	this.debugBall = Filament.EntityManager.get().create();
	this.scene.addEntity(this.debugBall);
	
	const icosphere = new Filament.IcoSphere(5);

	const vb = Filament.VertexBuffer.Builder()
	  .vertexCount(icosphere.vertices.length / 3)
	  .bufferCount(2)
	  .attribute(VertexAttribute.POSITION, 0, AttributeType.FLOAT3, 0, 0)
	  .attribute(VertexAttribute.TANGENTS, 1, AttributeType.SHORT4, 0, 0)
	  .normalized(VertexAttribute.TANGENTS)
	  .build(this.engine);

	const ib = Filament.IndexBuffer.Builder()
	  .indexCount(icosphere.triangles.length)
	  .bufferType(IndexType.USHORT)
	  .build(this.engine);

	vb.setBufferAt(this.engine, 0, icosphere.vertices);
	vb.setBufferAt(this.engine, 1, icosphere.tangents);
	ib.setBuffer(this.engine, icosphere.triangles);

	Filament.RenderableManager.Builder(1)
	  .boundingBox({ center: [0, 0, 0], halfExtent: [1, 1, 1] })
	  .material(0, matInstance)
	  .geometry(0, PrimitiveType.TRIANGLES, vb, ib)
	  .build(this.engine, this.debugBall);
  }
  
  createDebugGlbObject()
  {
	const loader = this.engine.createAssetLoader();
    const glbMeshObject = this.glbMeshObject = loader.createAsset(debugGlbMesh_url);
	
	const onDone = () => {
		loader.delete();
		this.scene.addEntities(glbMeshObject.getEntities());
	};
	glbMeshObject.loadResources(onDone);
  }
  
  createDisplayQuad()
  {
	const QUAD_POSITIONS = new Float32Array([
		-1, -1, 0,
		 1, -1, 0,
		-1,  1, 0,
		 1,  1, 0
	]);
	const QUAD_UV0S = new Float32Array([
		0, 0,
		1, 0,
		0, 1,
		1, 1
	]);
	const QUAD_COLORS = new Uint32Array([0xff000000, 0xff0000ff, 0xff00ff00, 0xff00ffff]);
	
	this.vb = Filament.VertexBuffer.Builder()
		.vertexCount(4)
		.bufferCount(3)
		.attribute(VertexAttribute.POSITION, 0, AttributeType.FLOAT3, 0, 12)
		.attribute(VertexAttribute.UV0, 1, AttributeType.FLOAT2, 0, 8)
		.attribute(VertexAttribute.COLOR, 2, AttributeType.UBYTE4, 0, 4)
		.normalized(VertexAttribute.COLOR)
		.build(this.engine);

	this.vb.setBufferAt(this.engine, 0, QUAD_POSITIONS);
	this.vb.setBufferAt(this.engine, 1, QUAD_UV0S);
	this.vb.setBufferAt(this.engine, 2, QUAD_COLORS);
	
	this.ib = Filament.IndexBuffer.Builder()
		.indexCount(6)
		.bufferType(IndexType.USHORT)
		.build(this.engine);
	this.ib.setBuffer(this.engine, new Uint16Array([0, 3, 2, 0, 1, 3]));
	
	const showMat = this.engine.createMaterial(showTextureMat_url);
	this.showMatInst = showMat.getDefaultInstance();
	
	this.quad = Filament.EntityManager.get().create();
	Filament.RenderableManager.Builder(1)
		.boundingBox({ center: [0, 0, 0], halfExtent: [1, 1, 1] })
		.material(0, this.showMatInst)
		.geometry(0, PrimitiveType.TRIANGLES, this.vb, this.ib)
		.build(this.engine, this.quad);
	this.scene.addEntity(this.quad);
  }
}