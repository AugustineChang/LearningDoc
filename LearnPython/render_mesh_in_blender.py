import os
import sys
import math
import bpy

import numpy as np
from enum import Enum
from typing import Tuple, Generator
from mathutils import Vector, Matrix

def reset_scene() -> None:
    """Resets the scene to a clean state.

    Returns:
        None
    """
    # delete everything that isn't part of a camera or a light
    for obj in bpy.data.objects:
        if obj.type not in {"CAMERA"}:
            bpy.data.objects.remove(obj, do_unlink=True)

    # delete all the materials
    for material in bpy.data.materials:
        bpy.data.materials.remove(material, do_unlink=True)

    # delete all the textures
    for texture in bpy.data.textures:
        bpy.data.textures.remove(texture, do_unlink=True)

    # delete all the images
    for image in bpy.data.images:
        bpy.data.images.remove(image, do_unlink=True)

def setup_settings() -> None:
    camera.rotation_mode = 'QUATERNION'
    camera.data.sensor_width = 32
    camera.data.sensor_height = 32
    camera.data.lens_unit = 'FOV'
    camera.data.angle = math.radians(30.0)
    
    render.image_settings.file_format = "PNG"
    render.image_settings.color_mode = "RGBA"
    render.resolution_x = 1024
    render.resolution_y = 1024
    render.resolution_percentage = 100
    render.film_transparent = True
    render.threads_mode = 'FIXED'  # 使用固定线程数模式
    render.threads = 32  # 设置线程数
    
    # cycles
    scene.cycles.device = "GPU"
    scene.cycles.samples = 128   # 128
    scene.cycles.diffuse_bounces = 1
    scene.cycles.glossy_bounces = 1
    scene.cycles.transparent_max_bounces = 3  # 3
    scene.cycles.transmission_bounces = 3   # 3
    # scene.cycles.filter_width = 0.01
    bpy.context.scene.cycles.adaptive_threshold = 0
    scene.cycles.use_denoising = True
    
    # Set the device_type
    bpy.context.preferences.system.audio_device = 'None'
    bpy.context.preferences.addons["cycles"].preferences.compute_device_type = 'CUDA' # or "OPENCL"
    bpy.context.preferences.addons["cycles"].preferences.refresh_devices()    
    bpy.context.scene.cycles.tile_size = 8192
        
    # eevee
    #scene.eevee.use_soft_shadows = True
    #scene.eevee.use_ssr = True
    #scene.eevee.use_ssr_refraction = True
    #scene.eevee.taa_render_samples = 64
    #scene.eevee.use_gtao = True
    #scene.eevee.gtao_distance = 0.2
    #scene.eevee.use_motion_blur = False
    #scene.eevee.use_bloom = False
        

def setup_lights() -> None:
    #Make light just directional, disable shadows.
    light_data = bpy.data.lights.new(name=f'KeyLight', type='SUN')
    light = bpy.data.objects.new(name=f'Light', object_data=light_data)
    light.data.use_shadow = False
    light.data.energy = 3.14
    light.rotation_euler = (math.radians(-37.4), math.radians(56.7), math.radians(-24.5))
    light.location = (0.0, 0.0, 20)
    bpy.context.collection.objects.link(light)
    
    #Add another light source so stuff facing away from light is not completely dark
    light_data = bpy.data.lights.new(name=f'FillLight1', type='POINT')
    light2 = bpy.data.objects.new(name=f'Light2', object_data=light_data)
    light2.data.use_shadow = False
    light2.data.specular_factor = 0.0
    light2.location = (-1.0, 0.7, 0.2)
    bpy.context.collection.objects.link(light2)

    #Add another light source so stuff facing away from light is not completely dark
    light_data = bpy.data.lights.new(name=f'FillLight2', type='POINT')
    light3 = bpy.data.objects.new(name=f'Light3', object_data=light_data)
    light3.data.use_shadow = False
    light3.data.specular_factor = 0.0
    light3.location = (-0.7, -1.0, -0.2)
    bpy.context.collection.objects.link(light3)

def load_object(object_path: str) -> None:
    """Loads a glb model into the scene."""
    if object_path.endswith(".glb"):
        bpy.ops.import_scene.gltf(filepath=object_path)
    elif object_path.endswith(".fbx"):
        bpy.ops.import_scene.fbx(filepath=object_path)
    elif object_path.endswith(".blend"):
        bpy.ops.wm.open_mainfile(filepath=object_path)
    else:
        raise ValueError(f"Unsupported file type: {object_path}")

def get_scene_meshes() -> Generator[bpy.types.Object, None, None]:
    """Returns all meshes in the scene.

    Yields:
        Generator[bpy.types.Object, None, None]: Generator of all meshes in the scene.
    """
    for obj in bpy.context.scene.objects.values():
        if isinstance(obj.data, (bpy.types.Mesh)):
            yield obj

def scene_bbox() -> Tuple[Vector, Vector]:
    """Returns the bounding box of the scene.

    Taken from Shap-E rendering script
    (https://github.com/openai/shap-e/blob/main/shap_e/rendering/blender/blender_script.py#L68-L82)

    Raises:
        RuntimeError: If there are no objects in the scene.

    Returns:
        Tuple[Vector, Vector]: The minimum and maximum coordinates of the bounding box.
    """
    bbox_min = (math.inf,) * 3
    bbox_max = (-math.inf,) * 3
    found = False
    for obj in get_scene_meshes():
        found = True
        for coord in obj.bound_box:
            coord = Vector(coord)
            coord = obj.matrix_world @ coord
            bbox_min = tuple(min(x, y) for x, y in zip(bbox_min, coord))
            bbox_max = tuple(max(x, y) for x, y in zip(bbox_max, coord))

    if not found:
        raise RuntimeError("no objects in scene to compute bounding box for")

    return Vector(bbox_min), Vector(bbox_max)

def normalize_scene():
    """Normalizes the scene by scaling and translating it to fit in a unit cube centered
    at the origin.

    Mostly taken from the Point-E / Shap-E rendering script
    (https://github.com/openai/point-e/blob/main/point_e/evals/scripts/blender_script.py#L97-L112),
    but fix for multiple root objects: (see bug report here:
    https://github.com/openai/shap-e/pull/60).

    Returns:
        None
    """
    
    rootObjs = []
    for obj in bpy.context.scene.objects.values():
        if not obj.parent and obj.type not in {"CAMERA", "LIGHT"}:
            rootObjs.append(obj)
    
    if len(rootObjs) > 1:
        print('we have more than one root objects!!')
        # create an empty object to be used as a parent for all root objects
        parent_empty = bpy.data.objects.new("ParentEmpty", None)
        bpy.context.scene.collection.objects.link(parent_empty)

        # parent all root objects to the empty object
        for obj in rootObjs:
            obj.parent = parent_empty
        rootObjs.clear()
        rootObjs.append(parent_empty)

    bbox_min, bbox_max = scene_bbox()
    dxyz = bbox_max - bbox_min
    dist = max(dxyz[0], dxyz[1], dxyz[2])
    scale = 1.0 / dist
    offset = -(bbox_min + bbox_max) / 2 * scale
    
    offScaleMatrix = Matrix.LocRotScale(offset, None, (scale, scale, scale))
    for obj in rootObjs:
        obj.matrix_world = offScaleMatrix @ obj.matrix_world 
    
    return scale, offset

# Build intrinsic camera parameters from Blender camera data
#
# See notes on this in
# blender.stackexchange.com/questions/15102/what-is-blenders-camera-projection-matrix-model
def get_calibration_matrix_K_from_blender():
    global scene, camera
    resolution_x_in_px = scene.render.resolution_x
    resolution_y_in_px = scene.render.resolution_y
    scale = scene.render.resolution_percentage / 100
    sensor_width_in_mm = camera.data.sensor_width
    sensor_height_in_mm = camera.data.sensor_height
    pixel_aspect_ratio = scene.render.pixel_aspect_x / scene.render.pixel_aspect_y
    focal_in_mm = camera.data.lens
    
    if (camera.data.sensor_fit == 'VERTICAL'):
        # the sensor height is fixed (sensor fit is horizontal),
        # the sensor width is effectively changed with the pixel aspect ratio
        s_u = resolution_x_in_px * scale / sensor_width_in_mm / pixel_aspect_ratio
        s_v = resolution_y_in_px * scale / sensor_height_in_mm
    else: # 'HORIZONTAL' and 'AUTO'
        # the sensor width is fixed (sensor fit is horizontal),
        # the sensor height is effectively changed with the pixel aspect ratio
        s_u = resolution_x_in_px * scale / sensor_width_in_mm
        s_v = resolution_y_in_px * scale * pixel_aspect_ratio / sensor_height_in_mm

    # Parameters of intrinsic calibration matrix K
    alpha_u = focal_in_mm * s_u
    alpha_v = focal_in_mm * s_v
    u_0 = resolution_x_in_px * scale / 2
    v_0 = resolution_y_in_px * scale / 2
    skew = 0 # only use rectangular pixels

    K = Matrix(
        ((alpha_u, skew,    u_0),
        (    0  , alpha_v, v_0),
        (    0  , 0,        1 )))
    return K

def get_3x4_RT_matrix_from_blender():
    global camera
    location, rotation = camera.matrix_world.decompose()[0:2]
    R = np.asarray(rotation.to_matrix())
    t = np.asarray(location)

    cam_rec = np.asarray([[1, 0, 0], [0, -1, 0], [0, 0, -1]], np.float32)
    R = R.T
    t = -R @ t
    R_world2cv = cam_rec @ R
    t_world2cv = cam_rec @ t

    RT = np.concatenate([R_world2cv,t_world2cv[:,None]],1)
    return RT

def set_camera_transform(zenith, azimuth, distance):
    zenith = math.radians(zenith)
    azimuth = math.radians(azimuth)
    
    sinZenith = math.sin(zenith)
    point = (
        distance * math.cos(azimuth) * sinZenith,
        distance * math.sin(azimuth) * sinZenith,
        distance * math.cos(zenith)
    )
    
    global camera
    camera.location = point

    direction = -camera.location
    rot_quat = direction.to_track_quat('-Z', 'Y')
    camera.rotation_quaternion = rot_quat
    return camera

def render_and_save(view_id, suffix, camInfo = None):
    global outputImagePath, render

    render.filepath = os.path.join(outputImagePath, f"View{view_id}_{suffix}.png")
    bpy.ops.render.render(write_still=True)
    
    if camInfo is not None:
        paramPath = os.path.join(outputImagePath, f"View{view_id}.npy")
        K = get_calibration_matrix_K_from_blender()
        RT = get_3x4_RT_matrix_from_blender()
        
        paras = {}
        paras['intrinsic'] = np.array(K, np.float32)
        paras['extrinsic'] = np.array(RT, np.float32)
        paras['fov'] = camera.data.angle
        paras['azimuth'] = camInfo['azimuth']
        paras['zenith'] = camInfo['zenith']
        paras['distance'] = camInfo['distance']
        paras['focal'] = camera.data.lens
        paras['sensor_width'] = camera.data.sensor_width
        np.save(paramPath, paras)

class MaterialOutputState(Enum):
    FinalColor = 0
    BaseColor = 1
    Metallic = 2
    Roughness = 3

def matOutputState_to_string(outputState:MaterialOutputState):
    if outputState == MaterialOutputState.BaseColor:
        return "Base Color"
    elif outputState == MaterialOutputState.Metallic:
        return "Metallic"
    elif outputState == MaterialOutputState.Roughness:
        return "Roughness"
    return ""
    
def find_previous_link(nodeTree, pbrNode, socketName):
    for lk in nodeTree.links:
        if lk.to_node == pbrNode and lk.to_socket.name == socketName:
            return lk

def setup_one_material(outputState:MaterialOutputState, material):
    if material.use_nodes:        
        nodeTree = material.node_tree
        
        if "Principled BSDF" in nodeTree.nodes:
            pbrNode = nodeTree.nodes["Principled BSDF"]
            
            if outputState == MaterialOutputState.FinalColor:
                # link PBRNode -> finalOutput
                outputNode = nodeTree.nodes["Material Output"]
                outputLink = find_previous_link(nodeTree, outputNode, "Surface")
                print(outputLink.from_node, outputLink.to_node)
                nodeTree.links.remove(outputLink);
                nodeTree.links.new(pbrNode.outputs[0], outputNode.inputs["Surface"])
            else:
                paramSocketName = matOutputState_to_string(outputState)
                paramLink = find_previous_link(nodeTree, pbrNode, paramSocketName)
                
                # find/add TempEmissiveOut node
                nodeTempOut = None
                if "TempEmissiveOut" not in nodeTree.nodes:
                    nodeTempOut = nodeTree.nodes.new(type='ShaderNodeEmission')
                    nodeTempOut.name = "TempEmissiveOut"
                    nodeTempOut.label = nodeTempOut.name
                    nodeTempOut.inputs[0].default_value = (0,0,0,1)  # black RGBA
                    nodeTempOut.inputs[1].default_value = 1.0 # strength
                    
                    pbrNodeLocation = pbrNode.location
                    nodeTempOut.location = (pbrNodeLocation[0], pbrNodeLocation[1] + 200) # up 200px
                else:
                    nodeTempOut = nodeTree.nodes["TempEmissiveOut"]
                    oldTempLink = find_previous_link(nodeTree, nodeTempOut, nodeTempOut.inputs[0].name)
                    if oldTempLink is not None:
                        nodeTree.links.remove(oldTempLink);
                
                # link paramNode -> TempEmissiveOut
                if paramLink is not None:
                    nodeTree.links.new(paramLink.from_socket, nodeTempOut.inputs[0])
                else:
                    paramDefaultVal = pbrNode.inputs[paramSocketName].default_value
                    if isinstance(paramDefaultVal, float):
                        nodeTempOut.inputs[0].default_value = (paramDefaultVal, paramDefaultVal, paramDefaultVal, 1)
                    else:
                        nodeTempOut.inputs[0].default_value = paramDefaultVal
                
                # link TempEmissiveOut -> finalOutput
                outputNode = nodeTree.nodes["Material Output"]
                outputLink = find_previous_link(nodeTree, outputNode, "Surface")
                nodeTree.links.remove(outputLink);
                nodeTree.links.new(nodeTempOut.outputs[0], outputNode.inputs["Surface"])
        else:
            print(f"{material.name} is not a PBR material")
    else:
        print(f"{material.name} is not a node material")

def setup_materials(outputState:MaterialOutputState):
    for key,val in bpy.data.materials.items():
        setup_one_material(outputState, val)

if __name__ == "__main__":
    scene = bpy.context.scene
    camera = scene.camera
    render = scene.render
    
    inputGlbPath = r"D:\Temp\glb_normalization\5f26f64c8ad242719535e6daab6fb4df.glb"
    outputImagePath = inputGlbPath[:-4] # remove extension
    os.makedirs(outputImagePath, exist_ok=True)
    
    reset_scene()
    setup_settings()
    setup_lights()
    
    #reset_scene()
    load_object(inputGlbPath);
    normalize_scene();
    
    #rendering standard_cam7
    distance = 4.0
    numOfCams = 7
    zenithList = [90.0, 70.0, 100.0, 70.0, 100, 70.0, 100]
    azimuthList = [0.0, 30.0, 90.0, 150.0, 210.0, 270.0, 330.0]
    
    for view_id in range(numOfCams):
        set_camera_transform(zenithList[view_id], azimuthList[view_id], distance)
        camInfo = {"zenith":zenithList[view_id], "azimuth":azimuthList[view_id], "distance":distance};
        render_and_save(view_id, "FinalColor", camInfo)
    
    setup_materials(MaterialOutputState.BaseColor)
    for view_id in range(numOfCams):
        set_camera_transform(zenithList[view_id], azimuthList[view_id], distance)
        render_and_save(view_id, "BaseColor")
       
    setup_materials(MaterialOutputState.Metallic)
    for view_id in range(numOfCams):
        set_camera_transform(zenithList[view_id], azimuthList[view_id], distance)
        render_and_save(view_id, "Metallic")
        
    setup_materials(MaterialOutputState.Roughness)
    for view_id in range(numOfCams):
        set_camera_transform(zenithList[view_id], azimuthList[view_id], distance)
        render_and_save(view_id, "Roughness")
    
    #debug save
    #splitPath = os.path.split(inputGlbPath)
    #splitName = os.path.splitext(splitPath[1])
    #bpy.ops.wm.save_mainfile(filepath=f'{splitPath[0]}/{splitName[0]}.blend', check_existing=False)