import bpy
import os
import sys

if len(sys.argv) < 5:
    print("Error: need a file path!")
    sys.exit(0)

for obj in bpy.data.objects:
    bpy.data.objects.remove(obj)
view_layer = bpy.context.view_layer

meshPath = sys.argv[4]

#import obj
bpy.ops.import_scene.obj(filepath=meshPath, use_image_search=False, use_split_groups=True)

for oneObj in bpy.data.objects:
    #start edit
    view_layer.objects.active = oneObj
    bpy.ops.object.mode_set(mode='EDIT')

    #project uv & recalc normals
    bpy.ops.mesh.select_mode(type='FACE')
    bpy.ops.mesh.select_all(action='SELECT')
    bpy.ops.uv.smart_project()
    bpy.ops.mesh.normals_make_consistent(inside=False)
    oneObj.data.use_auto_smooth=False
    
    #end edit
    bpy.ops.object.mode_set(mode='OBJECT')
    view_layer.objects.active = None

#process materials
for oneObj in bpy.data.objects:
    if "_jewel" not in oneObj.name:
        continue
       
    numOfMats = len(oneObj.data.materials)
    if numOfMats != 1:
        continue
    
    index = oneObj.name.rfind("jewel")
    matIndex = int(oneObj.name[index+5:])
    matPair = oneObj.data.materials.items()[0]
    newMatName = f"{matPair[0]}{matIndex}"
    
    newMatObj = bpy.data.materials.new(newMatName)
    oneObj.data.materials[0] = newMatObj
    
    #print(oneObj.name)
    #print(matPair[0], "->", newMatName)

#debug save
splitPath = os.path.split(meshPath)
splitName = os.path.splitext(splitPath[1])
#bpy.ops.wm.save_mainfile(filepath=f'{splitPath[0]}/{splitName[0]}.blend', check_existing=False)

# export obj
os.rename(meshPath, meshPath+".objback")
bpy.ops.export_scene.obj(filepath=f'{splitPath[0]}/{splitName[0]}.obj', group_by_object=True)