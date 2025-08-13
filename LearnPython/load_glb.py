import io
import os
import sys
import json
import struct
from PIL import Image 

import bpy
import bmesh
import mathutils

###################################################################################

class TreeNodeContext:
    def __init__(self):
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
        self.labelList = [] # len == numOfVerts
        
        # object
        self.blendObject = None
        
        # materials
        self.materialList = [] # num Of materials
        
        #flags
        self.numOfValidUV = 0
        self.hasValidUVs = [False] * 8
        self.hasValidColor = False
        self.hasValidNormal = False

###################################################################################

class GlbSpliter:
    def __init__(self, glbPath):
        if not os.path.exists(glbPath):
            raise FileNotFoundError("glb does not exist!")
        
        self.globalCache = GlobalMeshCache()
        self.splitedCache = []
        
        self.glbMeshPath = glbPath
        self.glbMeshRoot = os.path.dirname(glbPath)
        self.glbExportPath = r"D:\Temp\Output"
        
        self.glbJsonData = None
        self.glbBinData = None
        self.osgjsMatData = None
        
        self.byteIndex = 0
        self.glbBulkBytes = None
        
        self.numOfUVs = 8
        
        self.extract_info_from_glb()
        self.create_global_cache_from_info()
        self.create_materials_and_textures()
        
    def extract_info_from_glb(self):
        self.byteIndex = 0
        glbFile = open(self.glbMeshPath, 'rb')
        glbFile.seek(self.byteIndex)
        self.glbBulkBytes = glbFile.read()
        glbFile.close()

        magic = self.bytes_to_string(self.read_bytes(4))
        glb_version = self.bytes_to_uint32(self.read_bytes(4))
        fileLength = self.bytes_to_uint32(self.read_bytes(4))

        realFileLength = len(self.glbBulkBytes)
        if magic != "glTF" or fileLength > realFileLength:
            raise ValueError("file is not glb or incomplete")

        self.jsonData = None
        self.binData = None
        while self.byteIndex < realFileLength:
            chunkLen = self.bytes_to_uint32(self.read_bytes(4))
            chunkType = self.bytes_to_string(self.read_bytes(4))
            
            if chunkType == "JSON" and self.jsonData is None:
                jsonStr = self.bytes_to_string(self.read_bytes(chunkLen))
                self.jsonData = json.loads(jsonStr)
                
                '''
                print(self.jsonData)
                
                jsonFilePath = self.glbMeshPath.replace(".glb", ".json")
                jsonFile = open(jsonFilePath,'w', encoding="utf-8")
                jsonFile.write(jsonStr)
                jsonFile.close()
                '''
                
            elif chunkType == "BIN\0" and self.binData is None:
                self.binData = self.read_bytes(chunkLen)        
            else:
                raise ValueError(f"unknow glb chunkType({chunkType})")
            
            # align to 4-byte
            remainder = chunkLen % 4
            if remainder > 0:
                self.read_bytes(4 - remainder)
            

        if self.jsonData is None or self.binData is None:
            raise ValueError("file is incomplete")
        
        osgjsMaterialJsonPath = os.path.join(self.glbMeshRoot, "detail.json")
        self.osgjsMatData = None
        if os.path.exists(osgjsMaterialJsonPath):
            osgjsMaterialFile = open(osgjsMaterialJsonPath, "r", encoding='utf-8')
            matJsonStr = osgjsMaterialFile.read()
            osgjsMaterialFile.close()

            self.osgjsMatData = json.loads(matJsonStr)
        
        self.byteIndex = 0
        self.glbBulkBytes = None
        
    def create_global_cache_from_info(self):
        # create globalMeshCache based on jsonData
        defaultScene = 0
        if "scene" in self.jsonData:
            defaultScene = self.jsonData["scene"]
        rootNodes = self.jsonData["scenes"][defaultScene]["nodes"]
        
        for rootNodeId in rootNodes:
            context = TreeNodeContext()
            context.worldMatrix = mathutils.Matrix()
            self.processOneNode(context, rootNodeId)
        
        self.globalCache.numOfValidUV = 0
        for uvIndex in range(self.numOfUVs):
            if not self.globalCache.hasValidUVs[uvIndex]:
                self.globalCache.uvsList[uvIndex] = []
            else:
                self.globalCache.numOfValidUV += 1
        
        if not self.globalCache.hasValidNormal:
            self.globalCache.normalList = []
        if not self.globalCache.hasValidColor:
            self.globalCache.colorList = []
    
    def processOneNode(self, context, nodeId):
        curNode = self.jsonData["nodes"][nodeId]
        
        if "matrix" in curNode:
            context.localMatrix = self.floats_to_matrix(curNode["matrix"])
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
            self.processStaticMesh(context, curNode["mesh"])

        if "children" in curNode:
            childrenNodesId = curNode["children"]
            for childId in childrenNodesId:
                childContext = TreeNodeContext()
                childContext.worldMatrix = context.worldMatrix
                childContext.parentObject = context.currentObject
                childContext.depth = context.depth + 1
                self.processOneNode(childContext, childId)
    
    def processStaticMesh(self, context, meshId):
        curMesh = self.jsonData["meshes"][meshId]
        meshName = curMesh["name"] if "name" in curMesh else "NoNameMesh"
        
        worldNormalMatrix = context.worldMatrix.inverted_safe()
        worldNormalMatrix.transpose()
        
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
            uvsIndices = [-1] * self.numOfUVs
            for uvIndex in range(self.numOfUVs):
                uvKey = f"TEXCOORD_{uvIndex}"
                uvsIndices[uvIndex] = vertAttr[uvKey] if uvKey in vertAttr else -1
            colorIndex = vertAttr["COLOR_0"] if "COLOR_0" in vertAttr else -1
            triangleIndex = oneSection["indices"] if "indices" in oneSection else -1
            materialIndex = oneSection["material"] if "material" in oneSection else -1
            
            vertexList = self.getAttributeDataArray(vertIndex)
            normalList = self.getAttributeDataArray(normalIndex)
            
            if triangleIndex < 0:
                indexList = []
                for index in range(len(vertexList)):
                    indexList.append(index)
            else:
                indexList = self.getAttributeDataArray(triangleIndex)
            
            # unfold indices by tppology
            if topologyType == 5: # TRIANGLE_STRIP
                newIndexList = []
                for index in range(len(indexList)-2):
                    newIndexList.append(indexList[index])
                    newIndexList.append(indexList[index + 1 + index % 2])
                    newIndexList.append(indexList[index + 2 - index % 2])
                
                indexList.clear()
                indexList.extend(newIndexList)
            elif topologyType == 6: # TRIANGLE_FAN
                newIndexList = []
                for index in range(len(indexList)-2):
                    newIndexList.append(indexList[index + 1])
                    newIndexList.append(indexList[index + 2])
                    newIndexList.append(indexList[0])
                
                indexList.clear()
                indexList.extend(newIndexList)
            
            uvsList = [None] * self.numOfUVs
            for uvIndex in range(self.numOfUVs):
                uvsList[uvIndex] = self.getAttributeDataArray(uvsIndices[uvIndex])
            
            colorList = self.getAttributeDataArray(colorIndex)
            
            numOfVert = len(vertexList)
            numOfNormal = len(normalList)
            numOfColor = len(colorList)
            numOfFaces = len(indexList) // 3
            
            baseIndex = len(self.globalCache.vertexList)
            for oneVert in vertexList:
                self.globalCache.vertexList.append(context.worldMatrix @ oneVert)
            
            if numOfNormal > 0:
                for oneNormal in normalList:
                    self.globalCache.normalList.append(worldNormalMatrix @ oneNormal)
                self.globalCache.hasValidNormal = True
            else:
                self.globalCache.normalList.extend([mathutils.Vector((0.0,0.0,1.0))] * numOfVert)
            
            if numOfColor > 0:
                self.globalCache.colorList.extend(colorList)
                self.globalCache.hasValidColor = True
            else:
                self.globalCache.colorList.extend([mathutils.Vector((1.0,1.0,1.0,1.0))] * numOfVert)
            
            for uvIndex in range(self.numOfUVs):
                if len(uvsList[uvIndex]) > 0:
                    self.globalCache.uvsList[uvIndex].extend(uvsList[uvIndex])
                    self.globalCache.hasValidUVs[uvIndex] = True
                else:
                    self.globalCache.uvsList[uvIndex].extend([mathutils.Vector((0.0,0.0))] * numOfVert)
            
            for index in range(numOfFaces):
                self.globalCache.indexList.append(baseIndex + indexList[index*3])
                self.globalCache.indexList.append(baseIndex + indexList[index*3+1])
                self.globalCache.indexList.append(baseIndex + indexList[index*3+2])
                self.globalCache.matIdList.append(materialIndex if materialIndex >= 0 else None)
    
    def getAttributeDataArray(self, attrId):
        if attrId < 0:
            return []
        
        dataInfo = self.getAttributeDataInfo(attrId)
        dataBytes = self.binData[dataInfo.bytesOffset: dataInfo.bytesOffset + dataInfo.bytesLen]
        
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
                data = self.bytes_to_custom(dataBytes[byteCounter:byteCounter+componentSize], componentFormat)
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
    
    def getAttributeDataInfo(self, attrId):
        curDataAccessor = self.jsonData["accessors"][attrId]
        
        componentType = curDataAccessor["componentType"]
        dataType = curDataAccessor["type"]
        dataCount = curDataAccessor["count"]
        baseBytesOffset = curDataAccessor["byteOffset"] if "byteOffset" in curDataAccessor else 0
        
        bufferId = curDataAccessor["bufferView"]
        curBufferView = self.jsonData["bufferViews"][bufferId]
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
    
    def split_mesh_by_label(self):
        self.cleanScene()
    
        defaultContext = TreeNodeContext()
        defaultContext.worldMatrix = mathutils.Matrix()
        
        if self.splitSubmeshByLabel():
            num_of_submeshes = len(self.splitedCache)
            for sub_id in range(num_of_submeshes):
                subCache = self.splitedCache[sub_id]
                
                #print("sub cache", sub_id)
                #print("  vertex:", len(subCache.vertexList))
                #print("  normal:", len(subCache.normalList))
                #for uvIndex in range(self.numOfUVs):
                #    print(f"  uv{uvIndex}:", len(subCache.uvsList[uvIndex]))
                #print("  color:", len(subCache.colorList))
                #print("  matId:", len(subCache.matIdList))
                #print("  indices:", len(subCache.indexList))
                
                subMesh = self.createStaticMesh(f"partMesh_{sub_id}", subCache.vertexList, subCache.indexList, subCache.normalList, subCache.uvsList, subCache.colorList, subCache.matIdList)
                subCache.blendObject = self.createObject(defaultContext, f"partMeshObj_{sub_id}", subMesh)
                
                #link materials to objects
                for matData in self.globalCache.materialList:
                    subCache.blendObject.data.materials.append(matData.blenderMatObj)
        else:
            globalMesh = self.createStaticMesh("globalMesh", self.globalCache.vertexList, self.globalCache.indexList, self.globalCache.normalList, self.globalCache.uvsList, self.globalCache.colorList, self.globalCache.matIdList)
            self.globalCache.blendObject = self.createObject(defaultContext, "globalMeshObj", globalMesh)
            
            #link materials to objects
            for matData in self.globalCache.materialList:
                self.globalCache.blendObject.data.materials.append(matData.blenderMatObj)
        
        for matData in self.globalCache.materialList:
            matData.blenderMatObj.use_fake_user = False
        
    def splitSubmeshByLabel(self):
        if self.globalCache.labelList is None or len(self.globalCache.labelList) <= 0:
            print("[WARNING] label list is empty!")
            return False
        
        indices_map = []
        self.splitedCache = []
        
        num_of_vertices = len(self.globalCache.vertexList)
        for vert_id in range(num_of_vertices):
            
            label = self.globalCache.labelList[vert_id]
            if label < 0:
                continue
            
            num_of_submeshes = len(self.splitedCache)
            if label >= num_of_submeshes:
                for index in range(label + 1 - num_of_submeshes):
                    newSubCache = GlobalMeshCache()
                    self.splitedCache.append(newSubCache)
                    indices_map.append({})
            
            submeshCache = self.splitedCache[label]
            
            indices_map[label][vert_id] = len(submeshCache.vertexList)
            
            submeshCache.vertexList.append(self.globalCache.vertexList[vert_id])
            if self.globalCache.hasValidNormal:
                submeshCache.normalList.append(self.globalCache.normalList[vert_id])
            for uvIndex in range(self.numOfUVs):
                if self.globalCache.hasValidUVs[uvIndex]:
                    submeshCache.uvsList[uvIndex].append(self.globalCache.uvsList[uvIndex][vert_id])
            if self.globalCache.hasValidColor:
                submeshCache.colorList.append(globalCache.colorList[vert_id])
            
        num_of_faces = len(self.globalCache.indexList) // 3
        for face_id in range(num_of_faces):
            i0 = self.globalCache.indexList[face_id*3]
            i1 = self.globalCache.indexList[face_id*3+1]
            i2 = self.globalCache.indexList[face_id*3+2]
            
            label0 = self.globalCache.labelList[i0]
            label1 = self.globalCache.labelList[i1]
            label2 = self.globalCache.labelList[i2]
            
            if label0 == label1 and label1 == label2:
                if label0 < 0:
                    continue
                
                submeshCache = self.splitedCache[label0]
                submeshCache.indexList.append(indices_map[label0][i0])
                submeshCache.indexList.append(indices_map[label0][i1])
                submeshCache.indexList.append(indices_map[label0][i2])
                submeshCache.matIdList.append(self.globalCache.matIdList[face_id])
            else:
                print("[WARNING]found face with different labels", f"face{face_id}", f"label({label0}, {label1}, {label2})")
        
        return True
    
    def debug_label_glb(self):
        num_of_vertices = len(self.globalCache.vertexList)
        half_num_of_vertices = num_of_vertices // 2
        for vert_id in range(num_of_vertices):
            if vert_id < half_num_of_vertices:
                self.globalCache.labelList.append(0)
            else:
                self.globalCache.labelList.append(1)
    
    ######################################## BLENDER ############################################
    def cleanScene(self):
        for obj in bpy.data.objects:
            bpy.data.objects.remove(obj)
        bpy.ops.outliner.orphans_purge()
    
    def createObject(self, context, objName, objData):
        view_layer = bpy.context.view_layer
        context.currentObject = bpy.data.objects.new(name=objName, object_data=objData)
        view_layer.active_layer_collection.collection.objects.link(context.currentObject)
        
        if context.parentObject is not None:
            context.currentObject.parent = context.parentObject
            context.currentObject.matrix_local = context.localMatrix
        else:
            context.currentObject.matrix_local = context.worldMatrix 
        
        context.currentObject.select_set(True)
        bpy.ops.object.shade_auto_smooth(use_auto_smooth=True, angle=1.3089969)
        context.currentObject.select_set(False)
        return context.currentObject

    def createStaticMesh(self, meshName, vertexList, indexList, normalList, uvsList, colorList, matIdsList):
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
        
        for uvIndex in range(self.numOfUVs):
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
        meshBuilder.to_mesh(newMesh)
        meshBuilder.free()
        
        return newMesh
    
    def create_materials_and_textures(self):
        for oneMat in self.jsonData["materials"]:
            self.createMaterial(oneMat)
        
        #override materials
        if self.osgjsMatData is not None:
            self.loadOsgjsMaterialInfo()
        
        for matData in self.globalCache.materialList:
            if matData.baseColorTexture is not None:
                self.setupMaterialTexture(matData.baseColorTexture, matData.blenderMatObj, "Base Color", matData.baseColorFactor)
            else:
                self.setupMaterialDefaultValue(matData.blenderMatObj, "Base Color", matData.baseColorFactor)
                
            if matData.normalTexture is not None:
                self.setupMaterialTexture(matData.normalTexture, matData.blenderMatObj, "Normal", 1.0)
            
            if matData.metallicRoughnessTexture is not None:
                self.setupMaterialTexture(matData.metallicRoughnessTexture, matData.blenderMatObj, "MetalRough", [matData.metallicFactor, matData.roughnessFactor])
            else:
                self.setupMaterialDefaultValue(matData.blenderMatObj, "Roughness", matData.roughnessFactor)
                self.setupMaterialDefaultValue(matData.blenderMatObj, "Metallic", matData.metallicFactor)
                
            if matData.roughnessTexture is not None:
                self.setupMaterialTexture(matData.roughnessTexture, matData.blenderMatObj, "Roughness", matData.roughnessFactor)
            else:
                self.setupMaterialDefaultValue(matData.blenderMatObj, "Roughness", matData.roughnessFactor)
            
            if matData.metallicTexture is not None:
                self.setupMaterialTexture(matData.metallicTexture, matData.blenderMatObj, "Metallic", matData.metallicFactor)
            else:
                self.setupMaterialDefaultValue(matData.blenderMatObj, "Metallic", matData.metallicFactor)
    
    def createMaterial(self, materialNode):
        matData = MaterialData()
        
        if "name" in materialNode:
            matData.name = materialNode["name"]
        else:
            matData.name = "unknow_material"
        
        matData.blenderMatObj = bpy.data.materials.new(matData.name)
        matData.blenderMatObj.use_nodes = True
        matData.blenderMatObj.use_fake_user = True
        
        if "normalTexture" in materialNode:
            matData.normalTexture = self.processOneTexture(materialNode["normalTexture"])
        
        if "pbrMetallicRoughness" in materialNode:
            pbrNode = materialNode["pbrMetallicRoughness"]
            if "baseColorFactor" in pbrNode:
                matData.baseColorFactor = pbrNode["baseColorFactor"]
            if "baseColorTexture" in pbrNode:
                matData.baseColorTexture = self.processOneTexture(pbrNode["baseColorTexture"])
            
            if "metallicFactor" in pbrNode:
                matData.metallicFactor = pbrNode["metallicFactor"]
            if "roughnessFactor" in pbrNode:
                matData.roughnessFactor = pbrNode["roughnessFactor"]
            if "metallicRoughnessTexture" in pbrNode:
                matData.metallicRoughnessTexture = self.processOneTexture(pbrNode["metallicRoughnessTexture"])
        
        self.globalCache.materialList.append(matData)
    
    def processOneTexture(self, textureInfo):
        textureId = textureInfo["index"]
        texNode = self.jsonData["textures"][textureId]
        imageId = texNode["source"]
        imageNode = self.jsonData["images"][imageId]
        
        if "uri" in imageNode:
            imageName = imageNode["name"] if "name" in imageNode else "unknow"
            imageUri = imageNode["uri"]
            print(f"[WARNING]image({imageName}) need to download({imageUri})")
            return None
        
        bufferId = imageNode["bufferView"]
        bufferNode = self.jsonData["bufferViews"][bufferId]
        
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
            texSamplerNode = self.jsonData["samplers"][samplerId]
            
            texData.sampleMagFilter = self.sampler_int_to_string(9729)
            texData.sampleMinFilter = self.sampler_int_to_string(9985)
            texData.sampleWrapU = self.sampler_int_to_string(10497)
            texData.sampleWrapV = self.sampler_int_to_string(10497)
            
            if "magFilter" in texSamplerNode:
                texData.sampleMagFilter = self.sampler_int_to_string(texSamplerNode["magFilter"])
            if "minFilter" in texSamplerNode:
                texData.sampleMinFilter = self.sampler_int_to_string(texSamplerNode["minFilter"])
            if "wrapS" in texSamplerNode:
                texData.sampleWrapU = self.sampler_int_to_string(texSamplerNode["wrapS"])
            if "wrapT" in texSamplerNode:
                texData.sampleWrapV = self.sampler_int_to_string(texSamplerNode["wrapT"])
        
        bytesLen = bufferNode["byteLength"]
        bytesOffset = bufferNode["byteOffset"] if "byteOffset" in bufferNode else 0
        texData.texBinData = self.binData[bytesOffset: bytesOffset + bytesLen]
        
        return texData
    
    def sampler_int_to_string(self, intVal):
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
    
    def loadOsgjsMaterialInfo(self):
        if "-" in self.glbMeshPath:
            index = self.glbMeshPath.find("-")
            osgjsId = self.glbMeshPath[index+1:-4]
        else:
            osgjsId = os.path.basename(self.glbMeshPath)[:-4]
        
        modelNode = self.osgjsMatJson[f"/i/models/{osgjsId}"];
        materialsNode = modelNode["options"]["materials"];
        
        materialsMap = {}
        for matNode in materialsNode.values():
            if isinstance(matNode, dict):
                materialsMap[matNode["name"]] = matNode
        
        texturesId2Name = {}
        texturesList = self.osgjsMatJson[f"/i/models/{osgjsId}/textures?optimized=1"]["results"]
        for oneTex in texturesList:
            texturesId2Name[oneTex["uid"]] = oneTex["name"]
        
        for matObj in self.globalCache.materialList:
            matName = matObj.name
            
            matNode = None
            if matName in materialsMap:
                matNode = materialsMap[matName]
            elif matName.startswith("tex_"):
                textureId = matName[4:]
                for val in materialsMap.values():
                    channelsNode = val["channels"]
                    if self.searchTextureId(textureId, channelsNode, "AlbedoPBR"):
                        matNode = val
                        break
                        
                    if self.searchTextureId(textureId, channelsNode, "DiffusePBR"):
                        matNode = val
                        break
                    
                    if self.searchTextureId(textureId, channelsNode, "DiffuseColor"):
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
            BCChannelName = self.searchBaseColorChannel(matChannelsNode);
            
            matObj.baseColorTexture = None
            matObj.baseColorFactor = [1.0, 1.0, 1.0, 1.0]
            bcChannelNode = matChannelsNode[BCChannelName]
            if bcChannelNode["enable"]:
                if "texture" in bcChannelNode:
                    matObj.baseColorTexture = self.getOsgjsTextureInfo(bcChannelNode, texturesId2Name)
                
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
                            matObj.metallicTexture = self.getOsgjsTextureInfo(metalChannelNode, texturesId2Name)
                        
                        if "factor" in metalChannelNode:
                            matObj.metallicFactor = metalChannelNode["factor"]
                
                matObj.roughnessTexture = None
                matObj.roughnessFactor = 1.0
                if "RoughnessPBR" in matChannelsNode:
                    roughChannelNode = matChannelsNode["RoughnessPBR"]
                    if roughChannelNode["enable"]:
                        if "texture" in roughChannelNode:
                            matObj.roughnessTexture = self.getOsgjsTextureInfo(roughChannelNode, texturesId2Name)
                        
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
                        matObj.normalTexture = self.getOsgjsTextureInfo(normalChannelNode, texturesId2Name) 
        
    def searchTextureId(self, textureId, channelsNode, channelName):
        if channelName in channelsNode:
            if "texture" in channelsNode[channelName]:
                if "uid" in channelsNode[channelName]["texture"]:
                    foundUid = channelsNode[channelName]["texture"]["uid"]
                    return foundUid.startswith(textureId);
        return False

    def searchBaseColorChannel(self, matChannelsNode):
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

    def getOsgjsTextureInfo(self, textureInfoNode, texturesId2Name):
        textureNode = textureInfoNode["texture"]
        texUid = textureNode["uid"]
        if texUid not in texturesId2Name:
            print(f"[WARNING]Failed to find texture {texUid}")
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

    def setupMaterialTexture(self, textureData, matObj, socketName, socketScale):
        if textureData.imageType == "external":
            imagePath = f"{self.glbMeshRoot}/{textureData.name}"
            if not os.path.exists(imagePath):
                print(f"[WARNING] can not load image({imagePath})!")
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
        
        uv_node = matObj.node_tree.nodes.new(type='ShaderNodeUVMap')
        uvIndex = int(textureData.sampleUVId)
        uvIndex = min(uvIndex, self.globalCache.numOfValidUV-1)
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
            tex_node.extension = 'MIRROR'
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
    
    def setupMaterialDefaultValue(self, matObj, socketName, defaultVal):
        pbrNode = matObj.node_tree.nodes.get('Principled BSDF')
        pbrNode.inputs[socketName].default_value = defaultVal
    
    def export_glbs(self):
        bpy.ops.object.select_all(action='DESELECT')
        
        base_export_path = os.path.join(self.glbExportPath, os.path.basename(self.glbMeshPath))
        
        num_of_submeshes = len(self.splitedCache)
        if num_of_submeshes > 1:
            for sub_id in range(num_of_submeshes):
                subCache = self.splitedCache[sub_id]
                
                sub_export_path = base_export_path.replace(".glb", f"_sub{sub_id}.glb")
                subCache.blendObject.select_set(True)
                bpy.ops.export_scene.gltf(filepath=sub_export_path, export_apply=True, use_selection=True, export_yup=True)
                subCache.blendObject.select_set(False)
        else:
            self.globalCache.blendObject.select_set(True)
            bpy.ops.export_scene.gltf(filepath=base_export_path, export_apply=True, use_selection=True, export_yup=False)
            self.globalCache.blendObject.select_set(False)
    
    def debug_save(self):
        #debug save
        debugBlenderPath = self.glbMeshPath.replace(".glb", ".blend")
        bpy.ops.wm.save_mainfile(filepath=debugBlenderPath, check_existing=False)
    
    ##############################################################################################
    def bytes_to_uint32(self, in_bytes):
        return struct.unpack("<I", in_bytes)[0]

    def bytes_to_custom(self, in_bytes, informat):
        return struct.unpack(f"<{informat}", in_bytes)[0]

    def bytes_to_string(self, in_bytes):
        return in_bytes.decode("utf-8")

    def floats_to_matrix(self, matrix_floats):
        row0 = matrix_floats[0:4]
        row1 = matrix_floats[4:8]
        row2 = matrix_floats[8:12]
        row3 = matrix_floats[12:16]
        matrix = mathutils.Matrix([row0, row1, row2, row3])
        matrix.transpose()
        return matrix

    def read_bytes(self, numOfBytes):
        result = self.glbBulkBytes[self.byteIndex:self.byteIndex+numOfBytes]
        self.byteIndex += numOfBytes
        return result
    
if __name__ == "__main__":
    #glbMeshPath = r"D:\Temp\55a55b189204298225b056beb61e4efc117014a9c63036a6be4d6db6af7c0610.glb"
    #glbMeshPath = r"D:\Temp\922a70675416db0b1a8eeb6c165f5a039914c6a7f568de95474cec2ecc3d0e80.glb"
    #glbMeshPath = r"D:\Temp\55a55b189204298225b056beb61e4efc117014a9c63036a6be4d6db6af7c0610.glb"
    #glbMeshPath = r"D:\0cb0efd8bd8a405f89ce5f757ecf6e8d\0cb0efd8bd8a405f89ce5f757ecf6e8d.glb"
    glbMeshPath = r"C:\Users\large\Downloads\0000627f531e4fbc8668b79be7471b8b.glb"
    
    spliter = GlbSpliter(glbMeshPath)
    #spliter.debug_label_glb()
    spliter.split_mesh_by_label()
    
    #spliter.debug_save()
    spliter.export_glbs()