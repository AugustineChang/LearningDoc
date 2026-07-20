import pygame
import numpy as np
from .common import epsilon, makeArray, lerp, arcAreaAndL, polygonAreaAndCenter
from .common import physicsVectorToPixelVector, getSceneSize, getRandomColor, getRandomDirection
from .objects import PhyObject
from .rendering import drawDottedLine

################################# Force Types #################################

class ForceGenerator:
    def __init__(self):
        self.status = 1 # 0-dead 1-alive

    def simulate(self, dt:float):
        pass

    def draw(self, surface:pygame.Surface):
        pass

    def applyForce(self, obj:PhyObject):
        pass

class GravityForce(ForceGenerator):
    def __init__(self):
        super().__init__()
        self.gAccleration = makeArray([0.0, -9.8]) * 0.5

    def applyForce(self, obj:PhyObject):
        obj.addForce(self.gAccleration * obj.mass)

class AeroForce(ForceGenerator):
    def __init__(self, density:float, windVelo:np.ndarray):
        super().__init__()
        self.density = density
        self.windVelocity = windVelo
        self.k1 = 0.05

    def applyForce(self, obj:PhyObject):
        relativeVelo = obj.getVelocity() - self.windVelocity
        relativeSpeed = np.linalg.norm(relativeVelo)
        if relativeSpeed > epsilon:
            if obj.objType == "sphere":
                frontal_area = 2.0 * obj.radius
                k2 = 0.5 * self.density * frontal_area * 0.47

            elif obj.objType == "box":
                normal = makeArray([-relativeVelo[1], relativeVelo[0]])
                normal /= relativeSpeed

                rotMat = obj.getRotationMatrix()
                points = obj.position + obj.corners @ rotMat.T
                projections = np.dot(points, normal)
                frontal_area = np.max(projections) - np.min(projections)

                k2 = 0.5 * self.density * frontal_area * 1.05

            elif obj.objType == "particle":
                frontal_area = 1.0
                k2 = 0.5 * self.density * frontal_area * 0.47
            else:
                return

            obj.addForce(-k2 * relativeSpeed * relativeVelo - self.k1 * relativeVelo)

class RoundWindForce(ForceGenerator):
    def __init__(self):
        super().__init__()
        self.forceIntensity = 0.015

    def applyForce(self, obj:PhyObject):
        posLen = np.linalg.norm(obj.position)
        posDir = obj.position / max(posLen, epsilon)
        forceDir = makeArray([-posDir[1], posDir[0]])
        
        obj.addForce(forceDir * self.forceIntensity)

class AttractionForce(ForceGenerator):
    def __init__(self):
        super().__init__()
        self.forceIntensity = 0.02
        self.point = makeArray([0.0, 1.0])

    def applyForce(self, obj:PhyObject):
        force = self.point - obj.position
        dist = np.linalg.norm(force)
        force = force / max(dist, epsilon)
        obj.addForce(force * self.forceIntensity)

class BuoyancyForce(ForceGenerator):
    def __init__(self, density:float, level:float):
        super().__init__()
        self.density = density
        self.liquidLevel = level
        self.force = makeArray([0.0, 9.8]) * 0.5 * density
        self.color = pygame.Color(45, 164, 255)

    def draw(self, surface):
        sceneW, sceneHeight = getSceneSize()
        startPos = physicsVectorToPixelVector(makeArray([sceneW*-0.5, self.liquidLevel]))
        endPos = physicsVectorToPixelVector(makeArray([sceneW*0.5, self.liquidLevel]))

        pygame.draw.line(surface, self.color, startPos, endPos, width=2)

    def applyForce(self, obj:PhyObject):
        if obj.objType == "sphere":
            toLevel = self.liquidLevel - obj.position[1]
            dist = np.abs(toLevel)

            area = 0.0
            arcMaxW = 0.0
            if toLevel <= -obj.radius:
                return
            elif toLevel <= 0.0:
                area, l = arcAreaAndL(dist, obj.radius)
                arcMaxW = l
            elif toLevel < obj.radius:
                arcArea, _ = arcAreaAndL(dist, obj.radius)
                area = np.pi * obj.radius * obj.radius - arcArea
                arcMaxW = 2.0 * obj.radius
            else:
                area = np.pi * obj.radius * obj.radius

            obj.addForce(self.force * area)

            # water drag
            speed = np.linalg.norm(obj.getVelocity())
            if speed > epsilon:
                absNormalizedVelo = np.abs(obj.getVelocity() / speed)

                if toLevel <= 0.0:
                    spanVec = makeArray([arcMaxW, obj.radius - dist])
                    frontal_area = min(np.dot(spanVec, absNormalizedVelo), obj.radius*2.0)
                elif toLevel < obj.radius:
                    spanVec = makeArray([arcMaxW, obj.radius + dist])
                    frontal_area = min(np.dot(spanVec, absNormalizedVelo), obj.radius*2.0)
                else:
                    area = np.pi * obj.radius * obj.radius
                    frontal_area = 2.0 * obj.radius

                dragforce = -0.5 * self.density * frontal_area * 0.47 * speed * obj.getVelocity()
                obj.addForce(dragforce)

        elif obj.objType == "box":
            
            rotMat = obj.getRotationMatrix()
            points = obj.position + obj.corners @ rotMat.T

            isSubmerged = points[:, 1] < self.liquidLevel 
            if not any(isSubmerged):
                return

            submergedPoints = []
            numCorners = len(obj.corners)
            for idx in range(numCorners):
                n_idx = (idx + 1) % numCorners
                if isSubmerged[idx]:
                    submergedPoints.append(points[idx])
                
                if isSubmerged[idx] != isSubmerged[n_idx]:
                    alpha = (self.liquidLevel - points[idx][1]) / (points[n_idx][1] - points[idx][1])
                    submergedPoints.append(lerp(points[idx], points[n_idx], alpha))
            submergedPoints = makeArray(submergedPoints)

            area, center = polygonAreaAndCenter(submergedPoints)
            obj.addForce(self.force * area, center)

            # water drag
            rolledPoints = np.roll(submergedPoints, -1, axis=0)
            edgeVecs = rolledPoints - submergedPoints
            
            ## edge mask
            edgeLens = np.linalg.norm(edgeVecs, axis=1)
            mask = edgeLens > epsilon
            edgeLens = edgeLens[mask]
            edgeVecs = edgeVecs[mask]
            submergedPoints = submergedPoints[mask]
            
            ## calc sample times
            sampleTimes = np.maximum(1, np.round(edgeLens / 0.25)).astype(np.int32)

            ## calc sample ratios
            groupIndices = np.arange(sampleTimes.sum())
            starts = np.zeros_like(sampleTimes)
            starts[1:] = np.cumsum(sampleTimes)[:-1]
            groupStarts = np.repeat(starts, sampleTimes)
            numerators = groupIndices - groupStarts + 1
            denominators = np.repeat(sampleTimes + 1, sampleTimes)
            sampleRatios = numerators / denominators
            sampleRatios = sampleRatios[:, np.newaxis]

            ## calc sample indices 
            sampleIndices = np.arange(len(submergedPoints))
            sampleIndices = np.repeat(sampleIndices, sampleTimes)

            ## calc sample points
            samplePoints = submergedPoints[sampleIndices, :] + edgeVecs[sampleIndices, :] * sampleRatios

            ## velocity mask
            sampleVelos = obj.getVelocity(samplePoints)
            speed = np.linalg.norm(sampleVelos, axis=1)
            mask = speed > epsilon
            samplePoints = samplePoints[mask]
            sampleVelos = sampleVelos[mask]
            speed = speed[mask]
            sampleIndices = sampleIndices[mask]

            edgeNormals = edgeVecs[:, [1, 0]] / sampleTimes[:, np.newaxis]
            edgeNormals[:, 0] *= -1

            projections = np.sum(edgeNormals[sampleIndices, :]*sampleVelos, axis=1)
            frontal_area = np.maximum(projections, 0.0) / speed
            frontal_area = frontal_area[:, np.newaxis]
            speed = speed[:, np.newaxis]

            dragforce = -0.5 * self.density * frontal_area * 1.05 * speed * sampleVelos

            for idx in range(len(dragforce)):
                obj.addForce(dragforce[idx,:], samplePoints[idx,:])

class SpringForce(ForceGenerator):
    def __init__(self, endA:PhyObject, endB:PhyObject, restLen:float, springConst:float, 
                 offsetA:np.ndarray = None, offsetB:np.ndarray = None,
                 dampingRatio:float=1.0, isRope:bool=False):
        super().__init__()
        self.isRope = isRope
        self.endA:PhyObject = endA
        self.endB:PhyObject = endB
        self.offsetA = offsetA
        self.offsetB = offsetB
        self.color = getRandomColor()
        self.restLength = restLen
        self.maxLength = restLen * springConst * 0.3
        self.springConstant = springConst
        self.dampingRatio = dampingRatio

    def getEndPosition(self, isA:bool):
        offset = self.offsetA if isA else self.offsetB
        obj = self.endA if isA else self.endB

        if offset is not None:
            return obj.localToWorld(offset)
        else:
            return obj.position

    def simulate(self, dt:float):
        if self.status != 1: return
        isEndAMovable = isinstance(self.endA, PhyObject)
        isEndBMovable = isinstance(self.endB, PhyObject)
        if not isEndAMovable and not isEndBMovable:
            return

        positionA = self.getEndPosition(True)
        positionB = self.getEndPosition(False)
        forceDir = positionB - positionA
        springLen = np.linalg.norm(forceDir)
        if springLen <= epsilon:
            forceDir = getRandomDirection()
        else:
            forceDir = forceDir / springLen

        is_skip = self.isRope and (springLen < self.restLength)
        if is_skip:
            return
        
        if springLen > self.maxLength:
            self.status = 0 # break
            return

        # springF
        deltaDist = (springLen - self.restLength)
        springF = deltaDist * -self.springConstant

        # dampF
        m_reduced_inv = self.endB.invMass + self.endA.invMass
        if m_reduced_inv > 0.0:
            m_reduced = 1.0 / m_reduced_inv
            c = self.dampingRatio * 2.0 * np.sqrt(self.springConstant * m_reduced) # 临界阻尼
            v_rela = self.endB.getVelocity(positionB) - self.endA.getVelocity(positionA)
            dampF = -c * np.dot(v_rela, forceDir)
        else:
            dampF = 0.0

        # apply force
        force = (springF + dampF) * forceDir
        if isEndAMovable:
            self.endA.addForce(-force, positionA)
        if isEndBMovable:
            self.endB.addForce(force, positionB)

    def draw(self, surface):
        if self.status != 1: return

        pixelPosA = physicsVectorToPixelVector(self.getEndPosition(True), True, False)
        pixelPosB = physicsVectorToPixelVector(self.getEndPosition(False), True, False)

        drawDottedLine(surface, self.color, pixelPosA, pixelPosB, width=2)   