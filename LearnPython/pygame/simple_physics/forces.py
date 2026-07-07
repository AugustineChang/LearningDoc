import pygame
import numpy as np
from .common import epsilon, zero_vector, precision_type, physicsVectorToPixelVector, getRandomColor, getRandomDirection
from .objects import PhyObject

################################# Force Types #################################

class ForceGenerator:
    def __init__(self):
        self.status = 1 # 0-dead 1-alive

    def simulate(self, dt:float):
        pass

    def draw(self, surface:pygame.Surface):
        pass

    def addForce(self, obj:PhyObject):
        pass

class GravityForce(ForceGenerator):
    def __init__(self):
        super().__init__()
        self.gAccleration = np.array([0.0, -9.8], dtype=precision_type) * 0.1

    def addForce(self, obj:PhyObject):
        obj.forceAccum += self.gAccleration * obj.mass

class DragForce(ForceGenerator):
    def __init__(self):
        super().__init__()
        self.k1 = 0.1
        self.k2 = 0.05

    def addForce(self, obj:PhyObject):
        force = obj.velocity.copy()
        veloLen = np.linalg.norm(force)
        force = force / max(veloLen, epsilon)

        obj.forceAccum += -force * (veloLen*self.k1 + veloLen*veloLen*self.k2)

class RoundWindForce(ForceGenerator):
    def __init__(self):
        super().__init__()
        self.time = 10.0
        self.forceIntensity = 0.015

    def simulate(self, dt:float):
        self.time -= dt

    def addForce(self, obj:PhyObject):
        if self.time > 0.0:
            posLen = np.linalg.norm(obj.position)
            posDir = obj.position / max(posLen, epsilon)
            forceDir = np.array([-posDir[1], posDir[0]], dtype=precision_type)
            
            obj.forceAccum += forceDir * self.forceIntensity

class TestUpForce(ForceGenerator):
    def __init__(self):
        super().__init__()
        self.force = np.array([0.0, 10.0], dtype=precision_type)
        self.posY = -4

    def draw(self, surface):
        startPos = physicsVectorToPixelVector(np.array([-4, self.posY], dtype=precision_type))
        endPos = physicsVectorToPixelVector(np.array([4, self.posY], dtype=precision_type))

        pygame.draw.line(surface, 'white', startPos, endPos, width=2)

    def addForce(self, obj:PhyObject):
        if obj.position[1] <= self.posY:
            obj.forceAccum += self.force

class AttractionForce(ForceGenerator):
    def __init__(self):
        super().__init__()
        self.forceIntensity = 0.02
        self.point = np.array([0.0, 1.0], dtype=precision_type)

    def addForce(self, obj:PhyObject):
        force = self.point - obj.position
        dist = np.linalg.norm(force)
        force = force / max(dist, epsilon)
        obj.forceAccum += force * self.forceIntensity

class SpringForce(ForceGenerator):
    def __init__(self, endA:PhyObject|np.ndarray, endB:PhyObject|np.ndarray, restLen:float, springConst:float, 
                 dampingRatio:float=1.0, isRope:bool=False):
        super().__init__()
        self.isRope = isRope
        self.endA:PhyObject|np.ndarray = endA
        self.endB:PhyObject|np.ndarray = endB
        self.color  = getRandomColor()
        self.restLength = restLen
        self.maxLength = restLen * 3.0
        self.springConstant = springConst
        self.dampingRatio = dampingRatio

    @staticmethod
    def getEndPosition(end:PhyObject|np.ndarray):
        return end.position if isinstance(end, PhyObject) else end

    @staticmethod
    def getEndInvMass(end:PhyObject|np.ndarray):
        return end.invMass if isinstance(end, PhyObject) else 0.0

    @staticmethod
    def getEndVelocity(end:PhyObject|np.ndarray):
        return end.velocity if isinstance(end, PhyObject) else zero_vector.copy()

    def simulate(self, dt:float):
        if self.status != 1: return
        isEndAMovable = isinstance(self.endA, PhyObject)
        isEndBMovable = isinstance(self.endB, PhyObject)
        if not isEndAMovable and not isEndBMovable:
            return

        forceDir = SpringForce.getEndPosition(self.endB) - SpringForce.getEndPosition(self.endA)
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
        m_reduced_inv = SpringForce.getEndInvMass(self.endB) + SpringForce.getEndInvMass(self.endA) 
        if m_reduced_inv > 0.0:
            m_reduced = 1.0 / m_reduced_inv
            c = self.dampingRatio * 2.0 * np.sqrt(self.springConstant * m_reduced) # 临界阻尼
            v_rela = SpringForce.getEndVelocity(self.endB) - SpringForce.getEndVelocity(self.endA)
            dampF = -c * np.dot(v_rela, forceDir)
        else:
            dampF = 0.0

        # apply force
        force = (springF + dampF) * forceDir
        if isEndAMovable:
            self.endA.forceAccum -= force
        if isEndBMovable:
            self.endB.forceAccum += force

    def draw(self, surface):
        if self.status != 1: return
        pixelPosA = physicsVectorToPixelVector(SpringForce.getEndPosition(self.endA))
        pixelPosB = physicsVectorToPixelVector(SpringForce.getEndPosition(self.endB))

        pygame.draw.line(surface, self.color, pixelPosA, pixelPosB, width=2)   