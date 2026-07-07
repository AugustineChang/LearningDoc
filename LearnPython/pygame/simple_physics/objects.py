import pygame
import numpy as np
from .common import zero_vector, precision_type, physicsVectorToPixelVector, physicsScalarToPixelScalar
from .common import screenWidth, screenHeight, getRandomDirection, getRandomFloatRange, getRandomIntRange

################################# Object Types #################################

class PhyObject:
    def __init__(self, x:float, y:float):
        self.objType = ''
        self.position = np.array([x, y], dtype=precision_type)
        self.velocity = zero_vector.copy()
        self.acceleration = zero_vector.copy()
        self.forceAccum = zero_vector.copy()
        self.mass = 1.0
        self.invMass = None
        self.damping = 1.0#0.65
        self.restitution = 0.8

        self.status = 1

    def simulate(self, dt:float):
        newAcceleration = self.forceAccum * self.invMass
        newVelocity = self.velocity + newAcceleration * dt
        if 0.0 < self.damping and self.damping < 1.0:
            newDampVelocity = newVelocity * np.pow(self.damping, dt)
            self.position += (newVelocity + newDampVelocity) * 0.5 * dt
            self.velocity = newDampVelocity
        else:
            self.position += newVelocity * dt
            self.velocity = newVelocity
        self.acceleration = newAcceleration
        self.forceAccum[:] = 0.0

        if self.isOutScreen(): 
            self.status = 0

    def draw(self, surface:pygame.Surface):
        pass
    
    def isOutScreen(self):
        screenPos = physicsVectorToPixelVector(self.position)

        return screenPos[0] < 0 or screenPos[0] >= screenWidth or screenPos[1] < 0 or screenPos[1] >= screenHeight

    def calcInvMass(self):
        self.invMass = 1.0 / self.mass if self.mass > 0.0 else 0.0

class PhySphere(PhyObject):
    def __init__(self, x:float, y:float, radius:float, color:pygame.Color):
        super().__init__(x, y)
        self.objType = 'sphere'
        self.color = color
        self.radius = radius
        self.mass = np.pi * radius * radius * 10.0
        self.calcInvMass()

    def simulate(self, dt:float):
        if self.status != 1: return
        super().simulate(dt)

    def draw(self, surface:pygame.Surface):
        if self.status != 1: return
        pixelPos = physicsVectorToPixelVector(self.position)
        pixelRadius = physicsScalarToPixelScalar(self.radius)

        pygame.draw.circle(surface, self.color, pixelPos, pixelRadius)

class PhyAlignedBox(PhyObject):
    def __init__(self, x:float, y:float, width:float, height:float, color:pygame.Color):
        super().__init__(x, y)
        self.objType = 'abox'
        self.color = color
        self.size = np.array([width, height], dtype=precision_type)
        self.mass = 0.0
        self.calcInvMass()

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
    def __init__(self, x:float, y:float, radius:float, color:pygame.Color):
        super().__init__(x, y)
        self.objType = 'particle'
        self.color = color
        self.velocity = getRandomDirection() * getRandomFloatRange(0.5, 2.0)
        self.radius = radius
        self.mass = np.pi * radius * radius * 10.0
        self.calcInvMass()
        
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
