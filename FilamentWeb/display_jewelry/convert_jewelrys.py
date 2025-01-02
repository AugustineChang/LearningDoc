import os
import sys

meshes_fila = []
meshes_blender = []
facetTextures = []
pbrTextures = []
shaders = []

filameshPath = r"D:\Projects\Filament\FilamentBin\bin\filamesh.exe"
mipgenPath = r"D:\Projects\Filament\FilamentBin\bin\mipgen.exe"
matcPath = r"D:\Projects\Filament\FilamentBin\bin\matc.exe"
blenderPath = r"D:\Projects\Blender3.3\build_windows_x64_vc17_Release\bin\Release\blender.exe"
blenderScriptPath = r"D:\Projects\Filament\FilamentWeb\display_jewelry\autogen_uv0_normal.py"

rootPath = sys.argv[1] if len(sys.argv) >= 2 else os.getcwd()
for root, dirs, files in os.walk(rootPath):
    for oneFile in files:
        lowerFileName = oneFile.lower()
        if lowerFileName.endswith(".obj"):
            rawMeshPath = os.path.join(root, oneFile)
            
            mtlPath = rawMeshPath.replace(".obj", ".mtl")
            filaMeshPath = rawMeshPath.replace(".obj", ".filamesh")
            
            if not os.path.exists(mtlPath): # need process by blender&filament
                meshes_fila.append((rawMeshPath, filaMeshPath))
                meshes_blender.append(rawMeshPath)
            elif not os.path.exists(filaMeshPath): # need process by filament
                meshes_fila.append((rawMeshPath, filaMeshPath))
            
        elif lowerFileName.endswith(".exr"):
            rawTexturePath = os.path.join(root, oneFile)
            processedTexturePath = rawTexturePath.replace(".exr", ".ktx1")
            if os.path.exists(processedTexturePath):
                continue
            facetTextures.append((rawTexturePath, processedTexturePath))
            
        elif lowerFileName.endswith(".png"):
            rawTexturePath = os.path.join(root, oneFile)
            processedTexturePath = rawTexturePath.replace(".png", ".ktx2")
            if os.path.exists(processedTexturePath):
                continue
            pbrTextures.append((rawTexturePath, processedTexturePath))
        
        elif lowerFileName.endswith(".mat"):
            rawShaderPath = os.path.join(root, oneFile)
            processedShaderPath = rawShaderPath.replace(".mat", ".filamat")
            if os.path.exists(processedShaderPath):
                continue
            shaders.append((rawShaderPath, processedShaderPath))

numOfFilaMeshes = len(meshes_fila)
numOfBlenderMeshes = len(meshes_blender)
numOfFacetTextures = len(facetTextures)
numOfPBRTextures = len(pbrTextures)
numOfShaders = len(shaders)
numOfTotal = numOfFilaMeshes + numOfBlenderMeshes + numOfFacetTextures + numOfPBRTextures + numOfShaders

print(f"NumOfMeshes(fila):{numOfFilaMeshes}\n\
NumOfMeshes(blender):{numOfBlenderMeshes}\n\
NumOfFacetTextures:{numOfFacetTextures}\n\
NumOfPBRTextures:{numOfPBRTextures}\n\
NumOfShaders:{numOfShaders}")

if numOfBlenderMeshes > 0:
    print("////////////////////////////Blender////////////////////////////")
for oneMesh in meshes_blender:
    commandStr = f"{blenderPath} -b -P {blenderScriptPath} \"{oneMesh}\""
    os.system(commandStr)

if numOfFilaMeshes > 0:
    print("////////////////////////////Filamesh////////////////////////////")
for oneMesh in meshes_fila:
    print(oneMesh[0], "->", oneMesh[1])
    commandStr = f"{filameshPath} --compress \"{oneMesh[0]}\" \"{oneMesh[1]}\""
    os.system(commandStr)

if numOfFacetTextures > 0:
    print("////////////////////////////FacetTextures////////////////////////////")
for oneMesh in facetTextures:
    print(oneMesh[0], "->", oneMesh[1])
    commandStr = f"{mipgenPath} --linear --float --format=ktx --mip-levels=1 \"{oneMesh[0]}\" \"{oneMesh[1]}\""
    os.system(commandStr)

if numOfPBRTextures > 0:
    print("////////////////////////////PBRTextures////////////////////////////")
for oneMesh in pbrTextures:
    print(oneMesh[0], "->", oneMesh[1])
    
    if "Normal" in oneMesh[0]:
        commandStr = f"{mipgenPath} --linear --compression=uastc_normals --strip-alpha --kernel=NORMALS \"{oneMesh[0]}\" \"{oneMesh[1]}\""
    elif "Roughness" in oneMesh[0] or "Metal" in oneMesh[0] or "Occlusion" in oneMesh[0]:
        commandStr = f"{mipgenPath} --grayscale --linear --compression=uastc \"{oneMesh[0]}\" \"{oneMesh[1]}\""
    else:
        commandStr = f"{mipgenPath} --compression=uastc \"{oneMesh[0]}\" \"{oneMesh[1]}\""
    os.system(commandStr)
    
if numOfShaders > 0:
    print("////////////////////////////Shaders////////////////////////////")
for oneShader in shaders:
    print(oneShader[0], "->", oneShader[1])
    commandStr = f"{matcPath} -a opengl -o {oneShader[1]} {oneShader[0]}"
    os.system(commandStr)   

if numOfTotal > 0:
    print("////////////////////////////End////////////////////////////")