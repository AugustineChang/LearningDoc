import pygame
import numpy as np
from enum import IntEnum
from .common import zero_vector, cross, localToWorld, worldToLocal, makeArray
from .common import physicsVectorToPixelVector, physicsScalarToPixelScalar
from .common import screenWidth, screenHeight, getRandomDirection, getRandomFloatRange, getRandomIntRange

################################# Object Types #################################
class PhyObjType(IntEnum):
    Base = 0
    SPHERE = 1
    BOX = 2
    ALIGNEDBOX = 3
    PARTICLE = 4

class PhyObject:
    def __init__(self, pos:np.ndarray):
        self.objType = PhyObjType.Base
        self.position = pos.copy()
        self.velocity = zero_vector.copy()
        self.acceleration = zero_vector.copy()
        self.forceAccum = zero_vector.copy()
        self.mass = 0.0
        self.invMass = 0.0
        
        self.orientation = 0.0
        self.angularVelocity = 0.0
        self.angularAcceleration = 0.0
        self.torqueAccum = 0.0
        self.inertia = 0.0
        self.invInertia = 0.0

        self.status = 1
        self.boundingRadius = 0.0
        self.density = 10.0
        self.damping = 0.8
        self.restitution = 0.8
        self.fraction = 0.5

    @classmethod
    def from_xy(cls, x:float, y:float):
        return cls(makeArray([x, y]))

    def simulate(self, dt:float):
        if self.invMass > 0.0:
            newAcceleration = self.forceAccum * self.invMass
            newVelocity = self.velocity + newAcceleration * dt
            if 0.0 < self.damping and self.damping < 1.0:
                newVelocity *= np.pow(self.damping, dt)
            self.position += (self.velocity + newVelocity) * 0.5 * dt
            self.velocity = newVelocity
            self.acceleration = newAcceleration
            self.forceAccum[:] = 0.0

        if self.invInertia > 0.0:
            newAngularAcceleration = self.torqueAccum * self.invInertia
            newAngularVelocity = self.angularVelocity + newAngularAcceleration * dt
            if 0.0 < self.damping and self.damping < 1.0:
                newAngularVelocity *= np.pow(self.damping, dt)
            self.orientation += (self.angularVelocity + newAngularVelocity) * 0.5 * dt
            if self.orientation <= -np.pi:
                self.orientation += np.pi * 2.0
            elif self.orientation > np.pi:
                self.orientation -= np.pi * 2.0
            self.angularVelocity = newAngularVelocity
            self.angularAcceleration = newAngularAcceleration
            self.torqueAccum = 0.0

        #if self.isOutScreen(): 
        #    self.status = 0

    def draw(self, surface:pygame.Surface):
        pass
    
    def isOutScreen(self):
        screenPos = physicsVectorToPixelVector(self.position)

        return screenPos[0] < 0 or screenPos[0] >= screenWidth or screenPos[1] < 0 or screenPos[1] >= screenHeight

    def setMass(self, mass:float):
        self.mass = mass
        self.invMass = 1.0 / self.mass if self.mass > 0.0 else 0.0

        if self.objType == PhyObjType.SPHERE:
            self.inertia = 0.5*self.mass*self.radius*self.radius
        elif self.objType == PhyObjType.BOX:
            self.inertia = 0.08333*self.mass*np.dot(self.size, self.size)
        self.invInertia = 1.0 / self.inertia if self.inertia > 0.0 else 0.0

    def addForce(self, force:np.ndarray, pos:np.ndarray=None, source:str=None):
        self.forceAccum += force
        if pos is not None:
            toPos = pos - self.position
            self.torqueAccum += cross(toPos, force)

    def getVelocity(self, pos:np.ndarray = None):
        if pos is not None:
            toPos = pos - self.position
            if pos.ndim == 1:
                linearVelocity = makeArray([-toPos[1], toPos[0]])
            else:
                linearVelocity = toPos[:, [1, 0]]
                linearVelocity[:, 0] *= -1
            return self.velocity + self.angularVelocity * linearVelocity
        else:
            return self.velocity

    def getRotationMatrix(self):
        cos_theta = np.cos(self.orientation)
        sin_theta = np.sin(self.orientation)
        return makeArray([[cos_theta, -sin_theta], [sin_theta, cos_theta]])

    def localToWorld(self, vec:np.ndarray, isVec:bool=False):
        rotMat = self.getRotationMatrix()
        return localToWorld(vec, rotMat, None if isVec else self.position)

    def worldToLocal(self, vec:np.ndarray, isVec:bool=False):
        invRotMat = self.getRotationMatrix().T
        return worldToLocal(vec, invRotMat, None if isVec else self.position)

    def keyDown(self, keyType:int):
        pass
    def keyUp(self, keyType:int):
        pass

class PhySphere(PhyObject):
    def __init__(self, pos:np.ndarray, radius:float, color:pygame.Color):
        super().__init__(pos)
        self.objType = PhyObjType.SPHERE
        self.color = color
        self.radius = radius
        self.boundingRadius = radius
        self.setMass(np.pi * radius * radius * self.density)

        self.markPoint = makeArray([radius, 0.0])

    @classmethod
    def from_xy(cls, x:float, y:float, radius:float, color:pygame.Color):
        return cls(makeArray([x, y]), radius, color)

    def simulate(self, dt:float):
        if self.status != 1: return
        super().simulate(dt)

    def draw(self, surface:pygame.Surface):
        if self.status != 1: return
        pixelPos = physicsVectorToPixelVector(self.position)
        pixelRadius = physicsScalarToPixelScalar(self.radius)
        pygame.draw.circle(surface, self.color, pixelPos, pixelRadius)

        rotMat = self.getRotationMatrix()
        rotatedMarkPoint = self.position + rotMat @ self.markPoint
        markPixelPoint = physicsVectorToPixelVector(rotatedMarkPoint)
        pygame.draw.circle(surface, 'white', markPixelPoint, 1)

class PhyBox(PhyObject):
    def __init__(self, pos:np.ndarray, width:float, height:float, color:pygame.Color):
        super().__init__(pos)
        self.objType = PhyObjType.BOX
        self.color = color
        self.size = makeArray([width, height])
        self.boundingRadius = np.sqrt(width*width+height*height)*0.5
        self.setMass(width * height * self.density)

        extent = self.size * 0.5
        self.corners = makeArray([
            [-extent[0], -extent[1]], [-extent[0],  extent[1]], 
            [ extent[0],  extent[1]], [ extent[0], -extent[1]]
        ])

    @classmethod
    def from_xy(cls, x:float, y:float, width:float, height:float, color:pygame.Color):
        return cls(makeArray([x, y]), width, height, color)

    def simulate(self, dt:float):
        if self.status != 1: return
        super().simulate(dt)

    def draw(self, surface:pygame.Surface):
        if self.status != 1: return
        
        rotMat = self.getRotationMatrix()
        rotatedCorners = self.position + self.corners @ rotMat.T
        points = []
        for idx in range(len(rotatedCorners)):
            points.append(physicsVectorToPixelVector(rotatedCorners[idx,:]))

        pygame.draw.polygon(surface, self.color, points)

class PhyBoat(PhyBox):
    def __init__(self, pos:np.ndarray, width:float, height:float, color:pygame.Color):
        super().__init__(pos, width, height, color)
        self.moveDir:np.ndarray = None
        self.engineStrength = 4.0 * self.mass

    def keyDown(self, keyType:int):
        if keyType == pygame.K_LEFT:
            self.moveDir = makeArray([-1.0, 0.0])
        elif keyType == pygame.K_RIGHT:
            self.moveDir = makeArray([1.0, 0.0])
        elif keyType == pygame.K_UP:
            self.moveDir = makeArray([0.0, 1.0])
        elif keyType == pygame.K_DOWN:
            self.moveDir = makeArray([0.0, -1.0])
        
    def keyUp(self, keyType:int):
        if keyType == pygame.K_LEFT or keyType == pygame.K_RIGHT or keyType == pygame.K_UP or keyType == pygame.K_DOWN:
            self.moveDir = None

    def simulate(self, dt:float):
        if self.moveDir is not None:
            pos = (-self.moveDir * 0.5 - makeArray([0.0, 0.5]) * np.abs(self.moveDir[0])) * self.size
            rotMat = self. getRotationMatrix()
            pos = self.position + rotMat @ pos

            #moveDir = rotMat @ self.moveDir
            self.addForce(self.moveDir * self.engineStrength, pos)

        super().simulate(dt)

class PhySail(PhyBox):
    def __init__(self, pos:np.ndarray, len:float, color:pygame.Color):
        super().__init__(pos, 0.1*len, len, color)
        self.rotDir:float = None
        self.rotSpeed = 0.01

    @classmethod
    def from_xy(cls, x:float, y:float, len:float, color:pygame.Color):
        return cls(makeArray([x, y]), len, color)

    def keyDown(self, keyType:int):
        if keyType == pygame.K_LEFT:
            self.rotDir = 1
        elif keyType == pygame.K_RIGHT:
            self.rotDir = -1

    def keyUp(self, keyType:int):
        if keyType == pygame.K_LEFT or keyType == pygame.K_RIGHT:
            self.rotDir = None

    def simulate(self, dt:float):
        if self.rotDir is not None:
            self.orientation += self.rotDir * self.rotSpeed
        
        super().simulate(dt)

class PhyAlignedBox(PhyObject):
    def __init__(self, pos:np.ndarray, width:float, height:float, color:pygame.Color):
        super().__init__(pos)
        self.objType = PhyObjType.ALIGNEDBOX
        self.color = color
        self.size = makeArray([width, height])
        self.boundingRadius = np.sqrt(width*width+height*height)*0.5

    @classmethod
    def from_xy(cls, x:float, y:float, width:float, height:float, color:pygame.Color):
        return cls(makeArray([x, y]), width, height, color)

    def simulate(self, dt:float):
        pass

    def draw(self, surface:pygame.Surface):
        pixelPos = physicsVectorToPixelVector(self.position, True, False)
        pixelSize = physicsScalarToPixelScalar(self.size, False)
        pixelLT = pixelPos - pixelSize*0.5
        pixelLT = pixelLT.tolist()
        pixelSize = pixelSize.tolist()

        rect = pygame.Rect(pixelLT[0], pixelLT[1], pixelSize[0], pixelSize[1])
        pygame.draw.rect(surface, self.color, rect)

class PhyParticle(PhyObject):
    def __init__(self, pos:np.ndarray, radius:float, color:pygame.Color):
        super().__init__(pos)
        self.objType = PhyObjType.PARTICLE
        self.color = color
        self.velocity = getRandomDirection() * getRandomFloatRange(0.5, 2.0)
        self.radius = radius
        self.setMass(np.pi * radius * radius * 10.0)
        
        self.maxAge = 3.0
        self.age = self.maxAge

    def simulate(self, dt:float):
        if self.status != 1: return
        super().simulate(dt)
        self.age -= dt

        # 0-dead 1-alive 2-spawn
        if self.status != 0 and self.age <= 0.0: 
            self.status = 2 if getRandomIntRange(0, 2) == 0 else 0

    def draw(self, surface:pygame.Surface):
        if self.status != 1: return
        pixelPos = physicsVectorToPixelVector(self.position)
        pixelRadius = physicsScalarToPixelScalar(self.radius)

        drawCol = pygame.Color(self.color)
        drawCol.a = int(255.0 * self.age / self.maxAge + 0.5)
        pygame.draw.circle(surface, drawCol.premul_alpha(), pixelPos, pixelRadius)
