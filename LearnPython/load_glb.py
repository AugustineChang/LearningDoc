import os
import sys
import json
import struct

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

def createObject(context, objName, objData):
    global view_layer
    context.currentObject = bpy.data.objects.new(name=objName, object_data=objData)
    view_layer.active_layer_collection.collection.objects.link(context.currentObject)
    
    if context.parentObject is not None:
        context.currentObject.parent = context.parentObject
        context.currentObject.matrix_local = context.localMatrix
    else:
        context.currentObject.matrix_local = context.worldMatrix 


def processGlbFile(jsonData, binData):
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
    
    staticMesh = None
    if "mesh" in curNode:
        staticMesh = createStaticMesh(context, curNode["mesh"])
    
    objName = curNode["name"] if "name" in curNode else "NoNameObj"
    createObject(context, objName, staticMesh)

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


def createStaticMesh(context, meshId):
    curMesh = context.jsonData["meshes"][meshId]
    
    meshBuilder = bmesh.new()
    
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
        uv0Index = vertAttr["TEXCOORD_0"] if "TEXCOORD_0" in vertAttr else -1
        uv1Index = vertAttr["TEXCOORD_1"] if "TEXCOORD_1" in vertAttr else -1
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
        
        uv0List = getAttributeDataArray(context, uv0Index)
        uv1List = getAttributeDataArray(context, uv1Index)
        uvsList = [uv0List, uv1List]
        
        colorList = getAttributeDataArray(context, colorIndex)
        
        ################################################################################
        numOfVertices = len(vertexList)
        numOfIndices = len(indexList)
        
        blenderVertexList = [None] * numOfVertices
        for index in range(numOfVertices):
            curVert = meshBuilder.verts.new(vertexList[index])
            curVert.index = index
            blenderVertexList[index] = curVert
        
        needCalcNormal = False
        numOfNormals = len(normalList)
        if numOfNormals > 0 and numOfNormals == numOfVertices:
            for index in range(numOfVertices):
                blenderVertexList[index].normal = normalList[index]
        else:
            needCalcNormal = True
        
        numOfColors = len(colorList)
        if numOfColors > 0 and numOfColors == numOfVertices:
            colorLayer = meshBuilder.verts.layers.color.new("VertexColor")
            for index in range(numOfVertices):
                colorLen = len(colorList[index])
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
                blenderFaceList[index] = curFace
                if needCalcNormal:
                    curFace.smooth = True
            except ValueError:
                pass
        
        numOfUVs = len(uvsList)
        for uvIndex in range(numOfUVs):
            uvList = uvsList[uvIndex]
            numOfCurUV = len(uvList)
            if numOfCurUV > 0 and numOfCurUV == numOfVertices:
                uvLayer = meshBuilder.loops.layers.uv.new(f"UV{uvIndex}")
                for index in range(numOfFaces):
                    if blenderFaceList[index] is None:
                        continue
                    
                    for loop in blenderFaceList[index].loops:
                        loop[uvLayer].uv = uvList[loop.vert.index]
        
        ################################################################################
        #createMaterial(context, materialIndex)
    
    bmesh.ops.remove_doubles(meshBuilder, verts=blenderVertexList, dist=0.0001)
    if needCalcNormal:
        meshBuilder.normal_update()
    
    meshName = curMesh["name"] if "name" in curMesh else "NoNameMesh"
    newMesh = bpy.data.meshes.new(name=meshName)
    newMesh.use_auto_smooth = True
    newMesh.auto_smooth_angle = 1.3089969 # 70 degree
    meshBuilder.to_mesh(newMesh)
    meshBuilder.free()
    
    return newMesh

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


def createMaterial(context, materialId):
    # TODO..
    pass

###################################################################################

glbMeshPath = r"D:\Temp\0031b7a50ac14fc483c3ee36605eb51f.glb"
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

processGlbFile(jsonData, binData)

#debug save
debugBlenderPath = glbMeshPath.replace(".glb", ".blend")
bpy.ops.wm.save_mainfile(filepath=debugBlenderPath, check_existing=False)