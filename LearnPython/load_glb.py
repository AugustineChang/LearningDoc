import io
import os
import sys
import json
import struct
from PIL import Image 

import bpy
import bmesh
import mathutils

def bytes_to_uint32(read_bytes):
    return struct.unpack("<I", read_bytes)[0]

def bytes_to_custom(read_bytes, informat):
    return struct.unpack(f"<{informat}", read_bytes)[0]

def bytes_to_string(read_bytes):
    return read_bytes.decode("utf-8")

def floats_to_matrix(matrix_floats):
    row0 = matrix_floats[0:4]
    row1 = matrix_floats[4:8]
    row2 = matrix_floats[8:12]
    row3 = matrix_floats[12:16]
    matrix = mathutils.Matrix([row0, row1, row2, row3])
    matrix.transpose()
    return matrix

def read_bytes(numOfBytes):
    global glbBulkBytes, byteIndex
    result = glbBulkBytes[byteIndex:byteIndex+numOfBytes]
    byteIndex += numOfBytes
    return result

###################################################################################

class TreeNodeContext:
    def __init__(self):
        self.jsonData = None
        self.binData = None
        self.parentObject = None
        self.currentObject = None
        self.worldMatrix = None
        self.localMatrix = None
        self.depth = 0

class BinDataInfo:
    def __init__(self):
        self.componentType = None
        self.dataType = None
        self.dataCount = None
        self.bytesLen = None
        self.bytesOffset = None
        self.bytesStride = None
    
    def __str__(self):
        return f"{self.componentType}\n{self.dataType}\n{self.dataCount}\n{self.bytesLen}\n{self.bytesOffset}\n{self.bytesStride}"

class UVTransform:
    def __init__(self):
        self.offset = [0.0, 0.0]
        self.rotation = 0.0
        self.scale = [1.0, 1.0]
    
    def __str__(self):
        return f"offset:{self.offset};rotation:{self.rotation};scale:{self.scale};"

class TextureData:
    def __init__(self):
        self.name = None
        self.imageType = None
        self.sampleMagFilter = None
        self.sampleMinFilter = None
        self.sampleWrapU = None
        self.sampleWrapV = None
        self.sampleUVId = 0.0
        self.scale = 1.0
        self.texBinData = None
        self.uv_transform = None

    def __str__(self):
        return f"name:{self.name}\nimageType:{self.imageType}\nmagFilter:{self.sampleMagFilter}\nminFilter:{self.sampleMinFilter}\nwrapU:{self.sampleWrapU}\nwrapV:{self.sampleWrapV}\nuvId:{self.sampleUVId}\nscale:{self.scale}\nuv_transform:{self.uv_transform}"

class MaterialData:
    def __init__(self):
        self.blenderMatObj = None
        self.name = None
        self.baseColorTexture = None
        self.baseColorFactor = [1.0, 1.0, 1.0, 1.0]
        self.metallicRoughnessTexture = None
        self.metallicTexture = None
        self.roughnessTexture = None
        self.metallicFactor = 1.0
        self.roughnessFactor = 1.0
        self.normalTexture = None
    
    def __str__(self):
        if self.metallicRoughnessTexture is not None:
            return f"***************\nname:{self.name}\n\nBC:{self.baseColorFactor}\nBCTex:\n{self.baseColorTexture}\n\nNormal:\n{self.normalTexture}\n\nRoughness:{self.roughnessFactor}\nMetallic:{self.metallicFactor}\nmetallicRoughnessTex:\n{self.metallicRoughnessTexture}\n***************\n"
        else:
            return f"***************\nname:{self.name}\n\nBC:{self.baseColorFactor}\nBCTex:\n{self.baseColorTexture}\n\nNormal:\n{self.normalTexture}\n\nRoughness:{self.roughnessFactor}\nRoughnessTex:\n{self.roughnessTexture}\n\nMetallic:{self.metallicFactor}\nMetallicTexture:\n{self.metallicTexture}\n***************\n"
    
class GlobalMeshCache:
    def __init__(self):
        # mesh
        self.vertexList = [] # len = numOfVerts
        self.indexList = [] # len = 3 * numOfFaces, 
        self.normalList = [] # len == numOfVerts
        self.uvsList = [[], [], [], [], [], [], [], []] # len == numOfVerts
        self.colorList = [] # len == numOfVerts
        self.matIdList = [] # len == numOfFaces
        
        # materials
        self.materialList = [] # num Of materials
        
        #flags
        self.numOfValidUV = 0
        self.hasValidUVs = [False] * 8
        self.hasValidColor = False
        self.hasValidNormal = False

def createObject(context, objName, objData):
    global view_layer
    context.currentObject = bpy.data.objects.new(name=objName, object_data=objData)
    view_layer.active_layer_collection.collection.objects.link(context.currentObject)
    
    if context.parentObject is not None:
        context.currentObject.parent = context.parentObject
        context.currentObject.matrix_local = context.localMatrix
    else:
        context.currentObject.matrix_local = context.worldMatrix 

def processGlbFile(jsonData, binData, osgjsMatData):
    defaultScene = 0
    if "scene" in jsonData:
        defaultScene = jsonData["scene"]
    rootNodes = jsonData["scenes"][defaultScene]["nodes"]
    
    for rootNodeId in rootNodes:
        context = TreeNodeContext()
        context.jsonData = jsonData
        context.binData = binData
        context.worldMatrix = mathutils.Matrix()
        processOneNode(context, rootNodeId)
    
    global globalCache
    numOfUVs = 8
    globalCache.numOfValidUV = 0
    for uvIndex in range(numOfUVs):
        if not globalCache.hasValidUVs[uvIndex]:
            globalCache.uvsList[uvIndex] = []
        else:
            globalCache.numOfValidUV += 1
        
    inputNormalList = globalCache.normalList if globalCache.hasValidNormal else []
    inputColorList = globalCache.colorList if globalCache.hasValidColor else []
    
    inputUVsList = [[]] * numOfUVs
    for uvIndex in range(numOfUVs):
        if globalCache.hasValidUVs[uvIndex]:
            inputUVsList[uvIndex] = globalCache.uvsList[uvIndex]
    
    globalMesh = createStaticMesh("globalMesh", globalCache.vertexList, globalCache.indexList, inputNormalList, inputUVsList, inputColorList, globalCache.matIdList)
    
    matContext = TreeNodeContext()
    matContext.worldMatrix = mathutils.Matrix()
    createObject(matContext, "globalMeshObj", globalMesh)
    
    matContext.jsonData = jsonData
    matContext.binData = binData
    for oneMat in jsonData["materials"]:
        createMaterial(matContext, oneMat)
    
    #override materials
    if osgjsMatData is not None:
        loadOsgjsMaterialInfo(osgjsMatData)
    
    for matData in globalCache.materialList:
        if matData.baseColorTexture is not None:
            setupMaterialTexture(matData.baseColorTexture, matData.blenderMatObj, "Base Color", matData.baseColorFactor)
        if matData.normalTexture is not None:
            setupMaterialTexture(matData.normalTexture, matData.blenderMatObj, "Normal", 1.0)
        if matData.metallicRoughnessTexture is not None:
            setupMaterialTexture(matData.metallicRoughnessTexture, matData.blenderMatObj, "MetalRough", [matData.metallicFactor, matData.roughnessFactor])
        if matData.roughnessTexture is not None:
            setupMaterialTexture(matData.roughnessTexture, matData.blenderMatObj, "Roughness", matData.roughnessFactor)
        if matData.metallicTexture is not None:
            setupMaterialTexture(matData.metallicTexture, matData.blenderMatObj, "Metallic", matData.metallicFactor)
        
def processOneNode(context, nodeId):
    curNode = context.jsonData["nodes"][nodeId]
    
    if "matrix" in curNode:
        context.localMatrix = floats_to_matrix(curNode["matrix"])
    else:
        
        rotQuat = None
        if "rotation" in curNode:
            quat_floats = curNode["rotation"]
            rotQuat = mathutils.Quaternion((quat_floats[3], quat_floats[0], quat_floats[1], quat_floats[2]))
        else:
            rotQuat = mathutils.Quaternion()
        
        scaleVec = None
        if "scale" in curNode:
            scale_floats = curNode["scale"]
            scaleVec = mathutils.Vector((scale_floats[0], scale_floats[1], scale_floats[2]))
        else:
            scaleVec = mathutils.Vector((1.0, 1.0, 1.0))
        
        translationVec = None
        if "translation" in curNode:
            translation_floats = curNode["translation"]
            translationVec = mathutils.Vector((translation_floats[0], translation_floats[1], translation_floats[2]))
        else:
            translationVec = mathutils.Vector((0.0, 0.0, 0.0))
        
        context.localMatrix = mathutils.Matrix.LocRotScale(translationVec, rotQuat, scaleVec)
    context.worldMatrix = context.worldMatrix @ context.localMatrix 
    
    if "mesh" in curNode:
        processStaticMesh(context, curNode["mesh"])
    
    #objName = curNode["name"] if "name" in curNode else "NoNameObj"
    #createObject(context, objName, staticMesh)

    if "children" in curNode:
        childrenNodesId = curNode["children"]
        for childId in childrenNodesId:
            childContext = TreeNodeContext()
            childContext.jsonData = context.jsonData
            childContext.binData = context.binData
            childContext.worldMatrix = context.worldMatrix
            childContext.parentObject = context.currentObject
            childContext.depth = context.depth + 1
            
            processOneNode(childContext, childId)

def createStaticMesh(meshName, vertexList, indexList, normalList, uvsList, colorList, matIdsList):
    meshBuilder = bmesh.new()
    
    ################################################################################
    numOfVertices = len(vertexList)
    numOfIndices = len(indexList)
    
    numOfColors = len(colorList)
    if numOfColors > 0 and numOfColors == numOfVertices and colorList[0] is not None:
        colorLayer = meshBuilder.verts.layers.color.new("VertexColor")
    
    blenderVertexList = [None] * numOfVertices
    for index in range(numOfVertices):
        curVert = meshBuilder.verts.new(vertexList[index])
        curVert.index = index
        blenderVertexList[index] = curVert
    
    needCalcNormal = False
    numOfNormals = len(normalList)
    if numOfNormals > 0 and numOfNormals == numOfVertices and normalList[0] is not None:
        for index in range(numOfVertices):
            blenderVertexList[index].normal = normalList[index]
    else:
        needCalcNormal = True
    
    if numOfColors > 0 and numOfColors == numOfVertices and colorList[0] is not None:
        for index in range(numOfVertices): 
            blenderVertexList[index][colorLayer] = colorList[index]
    
    numOfFaces = numOfIndices // 3
    blenderFaceList = [None] * numOfFaces
    for index in range(numOfFaces):
        i0 = indexList[index*3]
        i1 = indexList[index*3+1]
        i2 = indexList[index*3+2]
        
        #if i0 >= numOfVertices or i1 >= numOfVertices or i2 >= numOfVertices:
        #    continue
        
        v0 = blenderVertexList[i0]
        v1 = blenderVertexList[i1]
        v2 = blenderVertexList[i2]
        
        try:
            curFace = meshBuilder.faces.new((v0, v1, v2))
            curFace.index = index
            curFace.material_index = matIdsList[index]
            blenderFaceList[index] = curFace
            if needCalcNormal:
                curFace.smooth = True
        except ValueError:
            pass
    
    numOfUVs = len(uvsList)
    for uvIndex in range(numOfUVs):
        uvList = uvsList[uvIndex]
        numOfCurUV = len(uvList)
        if numOfCurUV > 0 and numOfCurUV == numOfVertices and uvList[0] is not None:
            uvLayer = meshBuilder.loops.layers.uv.new(f"UV{uvIndex}")
            for index in range(numOfFaces):
                if blenderFaceList[index] is None:
                    continue
                
                for loop in blenderFaceList[index].loops:
                    cachedUV = uvList[loop.vert.index]
                    loop[uvLayer].uv = mathutils.Vector((cachedUV.x, 1.0-cachedUV.y))
    
    ################################################################################
    
    bmesh.ops.remove_doubles(meshBuilder, verts=blenderVertexList, dist=0.0001)
    if needCalcNormal:
        meshBuilder.normal_update()
    
    newMesh = bpy.data.meshes.new(name=meshName)
    newMesh.use_auto_smooth = True
    newMesh.auto_smooth_angle = 1.3089969 # 70 degree
    meshBuilder.to_mesh(newMesh)
    meshBuilder.free()
    
    return newMesh
    

def processStaticMesh(context, meshId):
    curMesh = context.jsonData["meshes"][meshId]
    meshName = curMesh["name"] if "name" in curMesh else "NoNameMesh"
    
    worldNormalMatrix = context.worldMatrix.inverted_safe()
    worldNormalMatrix.transpose()
    
    #meshVertexList = []
    #meshIndexList = []
    #meshNormalList = []
    #meshUVsList = [[], []]
    #meshColorList = []
    numOfUVs = 8
    sections = curMesh["primitives"]
    for oneSection in sections:
        vertAttr = oneSection["attributes"]
        topologyType = oneSection["mode"] if "mode" in oneSection else 4
        if topologyType < 4 or topologyType > 6:
            # skip points and lines
            continue
        
        vertIndex = vertAttr["POSITION"] if "POSITION" in vertAttr else -1
        normalIndex = vertAttr["NORMAL"] if "NORMAL" in vertAttr else -1
        tangentIndex = vertAttr["TANGENT"] if "TANGENT" in vertAttr else -1
        uvsIndices = [-1] * numOfUVs
        for uvIndex in range(numOfUVs):
            uvKey = f"TEXCOORD_{uvIndex}"
            uvsIndices[uvIndex] = vertAttr[uvKey] if uvKey in vertAttr else -1
        colorIndex = vertAttr["COLOR_0"] if "COLOR_0" in vertAttr else -1
        triangleIndex = oneSection["indices"] if "indices" in oneSection else -1
        materialIndex = oneSection["material"] if "material" in oneSection else -1
        
        vertexList = getAttributeDataArray(context, vertIndex)
        normalList = getAttributeDataArray(context, normalIndex)
        
        if triangleIndex < 0:
            indexList = []
            for index in range(len(vertexList)):
                indexList.append(index)
        else:
            indexList = getAttributeDataArray(context, triangleIndex)
        
        uvsList = [None] * numOfUVs
        for uvIndex in range(numOfUVs):
            uvsList[uvIndex] = getAttributeDataArray(context, uvsIndices[uvIndex])
        
        colorList = getAttributeDataArray(context, colorIndex)
        
        numOfVert = len(vertexList)
        numOfNormal = len(normalList)
        numOfColor = len(colorList)
        numOfFaces = len(indexList) // 3
        
        #########################################################
        #meshBaseIndex = len(meshVertexList)
        #
        #meshVertexList.extend(vertexList)
        #
        #if numOfNormal > 0:
        #    meshNormalList.extend(normalList)
        #else:
        #    meshNormalList.extend([None] * numOfVert)
        #
        #if numOfColor > 0:
        #    meshColorList.extend(colorList)
        #else:
        #    meshColorList.extend([None] * numOfVert)
        #
        #if numOfUV0 > 0:
        #    meshUVsList[0].extend(uv0List)
        #else:
        #    meshUVsList[0].extend([None] * numOfVert)
        #
        #if numOfUV1 > 0:
        #    meshUVsList[1].extend(uv1List)
        #else:
        #    meshUVsList[1].extend([None] * numOfVert)
        #
        #for index in range(numOfFaces):
        #    meshIndexList.append(meshBaseIndex + indexList[index*3])
        #    meshIndexList.append(meshBaseIndex + indexList[index*3+1])
        #    meshIndexList.append(meshBaseIndex + indexList[index*3+2])
        #    
        #########################################################
        global globalCache
        baseIndex = len(globalCache.vertexList)
        for oneVert in vertexList:
            globalCache.vertexList.append(context.worldMatrix @ oneVert)
        
        if numOfNormal > 0:
            for oneNormal in normalList:
                globalCache.normalList.append(worldNormalMatrix @ oneNormal)
            globalCache.hasValidNormal = True
        else:
            globalCache.normalList.extend([mathutils.Vector((0.0,0.0,1.0))] * numOfVert)
        
        if numOfColor > 0:
            globalCache.colorList.extend(colorList)
            globalCache.hasValidColor = True
        else:
            globalCache.colorList.extend([mathutils.Vector((1.0,1.0,1.0,1.0))] * numOfVert)
        
        for uvIndex in range(numOfUVs):
            if len(uvsList[uvIndex]) > 0:
                globalCache.uvsList[uvIndex].extend(uvsList[uvIndex])
                globalCache.hasValidUVs[uvIndex] = True
            else:
                globalCache.uvsList[uvIndex].extend([mathutils.Vector((0.0,0.0))] * numOfVert)
        
        for index in range(numOfFaces):
            globalCache.indexList.append(baseIndex + indexList[index*3])
            globalCache.indexList.append(baseIndex + indexList[index*3+1])
            globalCache.indexList.append(baseIndex + indexList[index*3+2])
            globalCache.matIdList.append(materialIndex if materialIndex >= 0 else None)

def getAttributeDataInfo(context, attrId):
    curDataAccessor = context.jsonData["accessors"][attrId]
    
    componentType = curDataAccessor["componentType"]
    dataType = curDataAccessor["type"]
    dataCount = curDataAccessor["count"]
    baseBytesOffset = curDataAccessor["byteOffset"] if "byteOffset" in curDataAccessor else 0
    
    bufferId = curDataAccessor["bufferView"]
    curBufferView = context.jsonData["bufferViews"][bufferId]
    bytesLen = curBufferView["byteLength"]
    bytesOffset = baseBytesOffset + (curBufferView["byteOffset"] if "byteOffset" in curBufferView else 0)
    bytesStride = curBufferView["byteStride"] if "byteStride" in curBufferView else 0
    
    dataInfo = BinDataInfo()
    dataInfo.componentType = componentType
    dataInfo.dataType = dataType
    dataInfo.dataCount = dataCount
    dataInfo.bytesLen = bytesLen
    dataInfo.bytesOffset = bytesOffset
    dataInfo.bytesStride = bytesStride
    
    return dataInfo

def getAttributeDataArray(context, attrId):
    if attrId < 0:
        return []
    
    dataInfo = getAttributeDataInfo(context, attrId)
    dataBytes = context.binData[dataInfo.bytesOffset: dataInfo.bytesOffset + dataInfo.bytesLen]
    
    componentSize = 0
    componentFormat = ''
    if dataInfo.componentType == 5120: # int8
        componentSize = 1
        componentFormat = 'b'
    elif dataInfo.componentType == 5121: # uint8
        componentSize = 1
        componentFormat = 'B'
    elif dataInfo.componentType == 5122: # int16
        componentSize = 2
        componentFormat = 'h'
    elif dataInfo.componentType == 5123: # uint16
        componentSize = 2
        componentFormat = 'H'
    elif dataInfo.componentType == 5125: # uint32
        componentSize = 4
        componentFormat = 'I'
    elif dataInfo.componentType == 5126: # float
        componentSize = 4
        componentFormat = 'f'
    
    numOfComponents = 0
    if dataInfo.dataType == "SCALAR":
        numOfComponents = 1
    elif dataInfo.dataType == "VEC2":
        numOfComponents = 2
    elif dataInfo.dataType == "VEC3":
        numOfComponents = 3
    elif dataInfo.dataType == "VEC4":
        numOfComponents = 4
    
    dataList = []
    elementSize = componentSize * numOfComponents
    jumpBytes = dataInfo.bytesStride - elementSize if dataInfo.bytesStride > 0 else 0
    
    byteCounter = 0
    for index in range(dataInfo.dataCount):
        for comIndex in range(numOfComponents):
            data = bytes_to_custom(dataBytes[byteCounter:byteCounter+componentSize], componentFormat)
            byteCounter += componentSize
            dataList.append(data)
        
        byteCounter += jumpBytes
    
    
    if numOfComponents > 1:
        
        divisor = 1.0
        if dataInfo.componentType == 5121: # uint8
            divisor = 255.0
        elif dataInfo.componentType == 5123: # uint16
            divisor = 65535.0
        
        vecList = []
        numOfVec = len(dataList) // numOfComponents
        for index in range(numOfVec):
            tempList = []
            baseIndex = index * numOfComponents
            for comIndex in range(numOfComponents):
                tempList.append(dataList[baseIndex + comIndex] / divisor)
            
            vecList.append(mathutils.Vector(tempList))
        
        return vecList
    else:
       return dataList

def sampler_int_to_string(intVal):
    if intVal == 9728:
        return "NEAREST"
    elif intVal == 9729:
        return "LINEAR"
    elif intVal == 9984:
        return "NEAREST_MIPMAP_NEAREST"
    elif intVal == 9985:
        return "LINEAR_MIPMAP_NEAREST"
    elif intVal == 9986:
        return "NEAREST_MIPMAP_LINEAR"
    elif intVal == 9987:
        return "LINEAR_MIPMAP_LINEAR"
    elif intVal == 33071:
        return "CLAMP_TO_EDGE"
    elif intVal == 33648:
        return "MIRRORED_REPEAT"
    elif intVal == 10497:
        return "REPEAT"
    return ""

def processOneTexture(context, textureInfo):
    textureId = textureInfo["index"]
    texNode = context.jsonData["textures"][textureId]
    imageId = texNode["source"]
    imageNode = context.jsonData["images"][imageId]
    
    if "uri" in imageNode:
        imageName = imageNode["name"] if "name" in imageNode else "unknow"
        imageUri = imageNode["uri"]
        print(f"image({imageName}) need to download({imageUri})")
        return None
    
    bufferId = imageNode["bufferView"]
    bufferNode = context.jsonData["bufferViews"][bufferId]
    
    texData = TextureData()
    if "texCoord" in textureInfo:
        texData.sampleUVId = textureInfo["texCoord"]
    
    if "scale" in textureInfo:
        texData.scale = textureInfo["scale"]
    elif "strength" in textureInfo:
        texData.scale = textureInfo["strength"]
    
    if "extensions" in textureInfo:
        if "KHR_texture_transform" in textureInfo["extensions"]:
            texTransfromNode = textureInfo["extensions"]["KHR_texture_transform"]
            
            texData.uv_transform = UVTransform()
            if "offset" in texTransfromNode:
                texData.uv_transform.offset = texTransfromNode["offset"]
            if "rotation" in texTransfromNode:
                texData.uv_transform.rotation = texTransfromNode["rotation"]
            if "scale" in texTransfromNode:
                texData.uv_transform.scale = texTransfromNode["scale"]
            
    if "name" in texNode:
        texData.name = texNode["name"]
    elif "name" in imageNode:
        texData.name = imageNode["name"]
    elif "name" in bufferNode:
        texData.name = bufferNode["name"]
    
    texData.imageType = imageNode["mimeType"]
    if "sampler" in texNode:
        samplerId = texNode["sampler"]
        texSamplerNode = context.jsonData["samplers"][samplerId]
        
        texData.sampleMagFilter = sampler_int_to_string(9729)
        texData.sampleMinFilter = sampler_int_to_string(9985)
        texData.sampleWrapU = sampler_int_to_string(33071)
        texData.sampleWrapV = sampler_int_to_string(33071)
        
        if "magFilter" in texSamplerNode:
            texData.sampleMagFilter = sampler_int_to_string(texSamplerNode["magFilter"])
        if "minFilter" in texSamplerNode:
            texData.sampleMinFilter = sampler_int_to_string(texSamplerNode["minFilter"])
        if "wrapS" in texSamplerNode:
            texData.sampleWrapU = sampler_int_to_string(texSamplerNode["wrapS"])
        if "wrapT" in texSamplerNode:
            texData.sampleWrapV = sampler_int_to_string(texSamplerNode["wrapT"])
    
    bytesLen = bufferNode["byteLength"]
    bytesOffset = bufferNode["byteOffset"] if "byteOffset" in bufferNode else 0
    texData.texBinData = context.binData[bytesOffset: bytesOffset + bytesLen]
    
    return texData

def searchTextureId(textureId, channelsNode, channelName):
    if channelName in channelsNode:
        if "texture" in channelsNode[channelName]:
            if "uid" in channelsNode[channelName]["texture"]:
                foundUid = channelsNode[channelName]["texture"]["uid"]
                return foundUid.startswith(textureId);
    return False

def searchBaseColorChannel(matChannelsNode):
    searchKeys = ["AlbedoPBR", "DiffusePBR", "DiffuseColor"]
    
    validKeys = []
    for index in range(len(searchKeys)):
        key = searchKeys[index]
        if key not in matChannelsNode:
            continue
        
        curChannel = matChannelsNode[key]
        if not curChannel["enable"]:
            continue
        
        validKeys.append(key)
        
    for index in range(len(validKeys)):
        key = validKeys[index]
        curChannel = matChannelsNode[key]
        
        if "texture" in curChannel:
            return key;
    
    return validKeys[0]

def loadOsgjsTextureInfo(textureInfoNode, texturesId2Name):
    textureNode = textureInfoNode["texture"]
    texUid = textureNode["uid"]
    if texUid not in texturesId2Name:
        print(f"Failed to find texture {texUid}")
        return None
    
    texData = TextureData()
    
    texData.name = texturesId2Name[texUid]
    texData.imageType = "external"
    texData.sampleWrapU = textureNode["wrapS"] if "wrapS" in textureNode else "CLAMP_TO_EDGE"
    texData.sampleWrapV = textureNode["wrapT"] if "wrapT" in textureNode else "CLAMP_TO_EDGE"
    texData.sampleMagFilter = textureNode["magFilter"] if "magFilter" in textureNode else "LINEAR"
    texData.sampleMinFilter = textureNode["minFilter"] if "minFilter" in textureNode else "LINEAR_MIPMAP_LINEAR"
    texData.sampleUVId = textureNode["texCoordUnit"] if "texCoordUnit" in textureNode else 0.0
    
    if "UVTransforms" in textureInfoNode:
        uvTransformNode = textureInfoNode["UVTransforms"]
        texData.uv_transform = UVTransform()
        texData.uv_transform.scale[0] = uvTransformNode["scale"][0]
        texData.uv_transform.scale[1] = uvTransformNode["scale"][1] * -1 #flip uv y
        
        texData.uv_transform.offset[0] = uvTransformNode["offset"][0]
        texData.uv_transform.offset[1] = uvTransformNode["offset"][1]
        
        texData.uv_transform.rotation = uvTransformNode["rotation"]
    else:
        texData.uv_transform.scale.y = -1 #flip uv y
    
    return texData

def loadOsgjsMaterialInfo(osgjsMatJson):
    global glbMeshPath
    if "-" in glbMeshPath:
        index = glbMeshPath.find("-")
        osgjsId = glbMeshPath[index+1:-4]
    else:
        osgjsId = os.path.basename(glbMeshPath)[:-4]
    
    modelNode = osgjsMatJson[f"/i/models/{osgjsId}"];
    materialsNode = modelNode["options"]["materials"];
    
    materialsMap = {}
    for matNode in materialsNode.values():
        if isinstance(matNode, dict):
            materialsMap[matNode["name"]] = matNode
    
    texturesId2Name = {}
    texturesList = osgjsMatJson[f"/i/models/{osgjsId}/textures?optimized=1"]["results"]
    for oneTex in texturesList:
        texturesId2Name[oneTex["uid"]] = oneTex["name"]
    
    global globalCache
    for matObj in globalCache.materialList:
        matName = matObj.name
        
        matNode = None
        if matName in materialsMap:
            matNode = materialsMap[matName]
        elif matName.startswith("tex_"):
            textureId = matName[4:]
            for val in materialsMap.values():
                channelsNode = val["channels"]
                if searchTextureId(textureId, channelsNode, "AlbedoPBR"):
                    matNode = val
                    break
                    
                if searchTextureId(textureId, channelsNode, "DiffusePBR"):
                    matNode = val
                    break
                
                if searchTextureId(textureId, channelsNode, "DiffuseColor"):
                    matNode = val
                    break
                
        else:
            for key in materialsMap.keys():
                if key.startswith(matName):
                    matNode = materialsMap[key]
                    break
        
        if matNode is None:
            print(f"Failed to find material {matName}")
            continue
        
        matChannelsNode = matNode["channels"]
        BCChannelName = searchBaseColorChannel(matChannelsNode);
        
        matObj.baseColorTexture = None
        matObj.baseColorFactor = [1.0, 1.0, 1.0, 1.0]
        bcChannelNode = matChannelsNode[BCChannelName]
        if bcChannelNode["enable"]:
            if "texture" in bcChannelNode:
                matObj.baseColorTexture = loadOsgjsTextureInfo(bcChannelNode, texturesId2Name)
            
            if "color" in bcChannelNode:
                matObj.baseColorFactor = bcChannelNode["color"]
                matObj.baseColorFactor.append(1.0)
            
            if "factor" in bcChannelNode:
                scale = bcChannelNode["factor"]
                matObj.baseColorFactor[0] *= scale
                matObj.baseColorFactor[1] *= scale
                matObj.baseColorFactor[2] *= scale
                matObj.baseColorFactor[3] *= scale
        
        matObj.metallicRoughnessTexture = None
        if BCChannelName.endswith("PBR"):
            matObj.metallicTexture = None
            matObj.metallicFactor = 1.0
            if "MetalnessPBR" in matChannelsNode:
                metalChannelNode = matChannelsNode["MetalnessPBR"]
                if metalChannelNode["enable"]:
                    if "texture" in metalChannelNode:
                        matObj.metallicTexture = loadOsgjsTextureInfo(metalChannelNode, texturesId2Name)
                    
                    if "factor" in metalChannelNode:
                        matObj.metallicFactor = metalChannelNode["factor"]
            
            matObj.roughnessTexture = None
            matObj.roughnessFactor = 1.0
            if "RoughnessPBR" in matChannelsNode:
                roughChannelNode = matChannelsNode["RoughnessPBR"]
                if roughChannelNode["enable"]:
                    if "texture" in roughChannelNode:
                        matObj.roughnessTexture = loadOsgjsTextureInfo(roughChannelNode, texturesId2Name)
                    
                    if "factor" in roughChannelNode:
                        matObj.roughnessFactor = roughChannelNode["factor"]
        else:
            matObj.metallicTexture = None
            matObj.roughnessTexture = None
            matObj.roughnessFactor = 1.0
            matObj.metallicFactor = 0.0
        
        matObj.normalTexture = None
        if "NormalMap" in matChannelsNode:
            normalChannelNode = matChannelsNode["NormalMap"]
            if normalChannelNode["enable"]:
                if "texture" in normalChannelNode:
                    matObj.normalTexture = loadOsgjsTextureInfo(normalChannelNode, texturesId2Name) 
        

def createMaterial(context, materialNode):
    matData = MaterialData()
    
    if "name" in materialNode:
        matData.name = materialNode["name"]
    else:
        matData.name = "unknow_material"
    
    matData.blenderMatObj = bpy.data.materials.new(matData.name)
    matData.blenderMatObj.use_nodes = True
    context.currentObject.data.materials.append(matData.blenderMatObj)
    
    if "normalTexture" in materialNode:
        matData.normalTexture = processOneTexture(context, materialNode["normalTexture"])
    
    if "pbrMetallicRoughness" in materialNode:
        pbrNode = materialNode["pbrMetallicRoughness"]
        if "baseColorFactor" in pbrNode:
            matData.baseColorFactor = pbrNode["baseColorFactor"]
        if "baseColorTexture" in pbrNode:
            matData.baseColorTexture = processOneTexture(context, pbrNode["baseColorTexture"])
        
        if "metallicFactor" in pbrNode:
            matData.metallicFactor = pbrNode["metallicFactor"]
        if "roughnessFactor" in pbrNode:
            matData.roughnessFactor = pbrNode["roughnessFactor"]
        if "metallicRoughnessTexture" in pbrNode:
            matData.metallicRoughnessTexture = processOneTexture(context, pbrNode["metallicRoughnessTexture"])
    
    global globalCache
    globalCache.materialList.append(matData)

def setupMaterialDefaultValue(matObj, socketName, defaultVal):
    pbrNode = matObj.node_tree.nodes.get('Principled BSDF')
    pbrNode.inputs[socketName].default_value = defaultVal

def setupMaterialTexture(textureData, matObj, socketName, socketScale):
    if textureData.imageType == "external":
        global glbMeshRoot
        imagePath = f"{glbMeshRoot}/{textureData.name}"
        if not os.path.exists(imagePath):
            return
        blenderImg = bpy.data.images.load(imagePath)
        blenderImg.pack()
    else:
        tempImagePath = f"{os.getcwd()}/{textureData.name}.png"
        pillowImg = Image.open(io.BytesIO(textureData.texBinData))
        pillowImg.save(tempImagePath)
        blenderImg = bpy.data.images.load(tempImagePath)
        blenderImg.pack()
        os.remove(tempImagePath)
    
    tex_node = matObj.node_tree.nodes.new(type='ShaderNodeTexImage')
    tex_node.image = blenderImg
    
    global globalCache
    uv_node = matObj.node_tree.nodes.new(type='ShaderNodeUVMap')
    uvIndex = int(textureData.sampleUVId)
    uvIndex = min(uvIndex, globalCache.numOfValidUV-1)
    uv_node.uv_map = f"UV{uvIndex}"
    
    uv_mapping_node = matObj.node_tree.nodes.new(type='ShaderNodeMapping')
    if textureData.uv_transform is not None:
        uvTransformNode = textureData.uv_transform
        uv_mapping_node.inputs["Scale"].default_value.x = uvTransformNode.scale[0]
        uv_mapping_node.inputs["Scale"].default_value.y = uvTransformNode.scale[1]
        
        uv_mapping_node.inputs["Location"].default_value.x = uvTransformNode.offset[0]
        uv_mapping_node.inputs["Location"].default_value.y = uvTransformNode.offset[1]
        
        uv_mapping_node.inputs["Rotation"].default_value.z = uvTransformNode.rotation
        
    if textureData.sampleWrapU == "CLAMP_TO_EDGE":
        tex_node.extension = 'EXTEND'
    elif textureData.sampleWrapU == "MIRRORED_REPEAT":
        tex_node.extension = 'REPEAT' # blender don't support mirrored repeat
    elif textureData.sampleWrapU == "REPEAT":
        tex_node.extension = 'REPEAT'
    
    if textureData.sampleMagFilter == "NEAREST":
        tex_node.interpolation = 'Closest'
    elif textureData.sampleMagFilter == "LINEAR":
        tex_node.interpolation = 'Linear'
    if textureData.sampleMinFilter == "LINEAR_MIPMAP_LINEAR":
        tex_node.interpolation = 'Cubic'
    
    pbrNode = matObj.node_tree.nodes.get('Principled BSDF')
    pbrNodeLocation = pbrNode.location
    
    lastNode = tex_node
    if socketName == "Base Color":
        uv_node.location = (pbrNodeLocation[0] - 1200, pbrNodeLocation[1] + 300)
        uv_mapping_node.location = (pbrNodeLocation[0] - 950, pbrNodeLocation[1] + 300)
        tex_node.location = (pbrNodeLocation[0] - 700, pbrNodeLocation[1] + 300)
        
        scale_node = matObj.node_tree.nodes.new(type='ShaderNodeVectorMath')
        scale_node.operation = 'MULTIPLY'
        scale_node.location = (pbrNodeLocation[0] - 350, pbrNodeLocation[1] + 300)
        scale_node.inputs[1].default_value = socketScale[:3]
        matObj.node_tree.links.new(tex_node.outputs[0], scale_node.inputs[0])
        lastNode = scale_node
        
    elif socketName == "Metallic" or socketName == "Roughness":
        bRoughness = socketName == "Roughness"
        locYOffset = -500 if bRoughness else -100
        
        blenderImg.colorspace_settings.name = 'Non-Color'
        uv_node.location = (pbrNodeLocation[0] - 1200, pbrNodeLocation[1] + locYOffset)
        uv_mapping_node.location = (pbrNodeLocation[0] - 950, pbrNodeLocation[1] + locYOffset)
        tex_node.location = (pbrNodeLocation[0] - 700, pbrNodeLocation[1] + locYOffset)
        
        separate_node = matObj.node_tree.nodes.new(type='ShaderNodeSeparateColor')
        separate_node.location = (pbrNodeLocation[0] - 420, pbrNodeLocation[1] + locYOffset)
        
        scale_node = matObj.node_tree.nodes.new(type='ShaderNodeMath')
        scale_node.operation = 'MULTIPLY'
        scale_node.location = (pbrNodeLocation[0] - 250, pbrNodeLocation[1] + locYOffset)
        scale_node.inputs[1].default_value = socketScale
        
        matObj.node_tree.links.new(tex_node.outputs[0], separate_node.inputs["Color"])
        matObj.node_tree.links.new(separate_node.outputs["Red"], scale_node.inputs[0])
        lastNode = scale_node
    
    elif socketName == "MetalRough":
        blenderImg.colorspace_settings.name = 'Non-Color'
        uv_node.location = (pbrNodeLocation[0] - 1200, pbrNodeLocation[1] - 300)
        uv_mapping_node.location = (pbrNodeLocation[0] - 950, pbrNodeLocation[1] - 300)
        tex_node.location = (pbrNodeLocation[0] - 700, pbrNodeLocation[1] - 300)
        
        scale_node = matObj.node_tree.nodes.new(type='ShaderNodeVectorMath')
        scale_node.operation = 'MULTIPLY'
        scale_node.location = (pbrNodeLocation[0] - 420, pbrNodeLocation[1] - 300)
        scale_node.inputs[1].default_value = (1.0, socketScale[1], socketScale[0])
        matObj.node_tree.links.new(tex_node.outputs[0], scale_node.inputs[0])
        
        separate_node = matObj.node_tree.nodes.new(type='ShaderNodeSeparateColor')
        separate_node.location = (pbrNodeLocation[0] - 250, pbrNodeLocation[1] - 300)
        matObj.node_tree.links.new(scale_node.outputs[0], separate_node.inputs["Color"])
        
        matObj.node_tree.links.new(separate_node.outputs["Green"], pbrNode.inputs["Roughness"])
        matObj.node_tree.links.new(separate_node.outputs["Blue"], pbrNode.inputs["Metallic"])
        
    elif socketName == "Normal":
        blenderImg.colorspace_settings.name = 'Non-Color'
        uv_node.location = (pbrNodeLocation[0] - 1200, pbrNodeLocation[1] - 900)
        uv_mapping_node.location = (pbrNodeLocation[0] - 950, pbrNodeLocation[1] - 900)
        tex_node.location = (pbrNodeLocation[0] - 700, pbrNodeLocation[1] - 900)
        
        normal_map_node = matObj.node_tree.nodes.new(type='ShaderNodeNormalMap')
        normal_map_node.uv_map = uv_node.uv_map
        normal_map_node.location = (pbrNodeLocation[0] - 350, pbrNodeLocation[1] - 900)
        normal_map_node.inputs["Strength"].default_value = socketScale
        matObj.node_tree.links.new(tex_node.outputs[0], normal_map_node.inputs["Color"])
        lastNode = normal_map_node
        
    # links
    matObj.node_tree.links.new(uv_node.outputs[0], uv_mapping_node.inputs["Vector"])
    matObj.node_tree.links.new(uv_mapping_node.outputs[0], tex_node.inputs["Vector"])
    if socketName in pbrNode.inputs:
        matObj.node_tree.links.new(lastNode.outputs[0], pbrNode.inputs[socketName])

###################################################################################

glbMeshPath = r"D:\Temp\55a55b189204298225b056beb61e4efc117014a9c63036a6be4d6db6af7c0610.glb"
glbMeshPath = r"D:\0cb0efd8bd8a405f89ce5f757ecf6e8d\0cb0efd8bd8a405f89ce5f757ecf6e8d.glb"
glbMeshRoot = os.path.dirname(glbMeshPath)
if not os.path.exists(glbMeshPath):
    print("glb does not exist!")
    sys.exit(0)

byteIndex = 0
glbFile = open(glbMeshPath,'rb')
glbFile.seek(byteIndex)
glbBulkBytes = glbFile.read()
glbFile.close()

magic = bytes_to_string(read_bytes(4))
glb_version = bytes_to_uint32(read_bytes(4))
fileLength = bytes_to_uint32(read_bytes(4))

realFileLength = len(glbBulkBytes)
if magic != "glTF" or fileLength > realFileLength:
    print("file is not glb or incomplete")
    sys.exit(0)

view_layer = bpy.context.view_layer

jsonData = None
binData = None
while byteIndex < realFileLength:
    chunkLen = bytes_to_uint32(read_bytes(4))
    chunkType = bytes_to_string(read_bytes(4))
    
    if chunkType == "JSON" and jsonData is None:
        jsonStr = bytes_to_string(read_bytes(chunkLen))
        jsonData = json.loads(jsonStr)
        
        '''
        print(jsonData)
        
        jsonFilePath = glbMeshPath.replace(".glb", ".json")
        jsonFile = open(jsonFilePath,'w', encoding="utf-8")
        jsonFile.write(jsonStr)
        jsonFile.close()
        '''
        
    elif chunkType == "BIN\0" and binData is None:
        binData = read_bytes(chunkLen)        
    else:
        print(f"unknow glb chunkType({chunkType})")
        sys.exit(0)
    
    # align to 4-byte
    remainder = chunkLen % 4
    if remainder > 0:
        read_bytes(4 - remainder)
    

if jsonData is None or binData is None:
    print("file is incomplete")
    sys.exit(0)

glbRootPath = os.path.dirname(glbMeshPath)
osgjsMaterialJsonPath = os.path.join(glbRootPath, "detail.json")
osgjsMatData = None
if os.path.exists(osgjsMaterialJsonPath):
    osgjsMaterialFile = open(osgjsMaterialJsonPath, "r", encoding='utf-8')
    matJsonStr = osgjsMaterialFile.read()
    osgjsMaterialFile.close()

    osgjsMatData = json.loads(matJsonStr)

globalCache = GlobalMeshCache()
processGlbFile(jsonData, binData, osgjsMatData)

'''

testContext = TreeNodeContext()
testContext.jsonData = jsonData
testContext.binData = binData
testContext.worldMatrix = mathutils.Matrix()
globalMesh = createStaticMesh("globalMesh", globalCache.vertexList, globalCache.indexList, globalCache.normalList, [globalCache.uv0List, globalCache.uv1List], globalCache.colorList)
createObject(testContext, "globalObj", globalMesh)

material0 = globalCache.materialList[0]
print(material0.normalTexture)

if material0.metallicRoughnessTexture is not None:
    normalImage = Image.open(io.BytesIO(material0.metallicRoughnessTexture.texBinData))
    normalImage.show()
'''

#debug save
debugBlenderPath = glbMeshPath.replace(".glb", ".blend")
bpy.ops.wm.save_mainfile(filepath=debugBlenderPath, check_existing=False)