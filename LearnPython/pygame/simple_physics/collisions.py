import pygame
import numpy as np
from .common import epsilon, precision_type, getRandomColor, getRandomDirection, physicsVectorToPixelVector
from .objects import PhyObject, PhySphere, PhyAlignedBox, PhyParticle
from .rendering import drawDottedLine

################################# Collision Handler #################################

Velocity_Iter = 4
Position_Iter = 2

class PhyCollision:
    def __init__(self, objA:PhyObject, objB:PhyObject, position:np.ndarray, normal:np.ndarray, penetration:float):
        self.objA = objA
        self.objB = objB
        self.position = position
        self.normal = normal
        self.penetration = penetration
        self.restitution = min(self.objA.restitution, self.objB.restitution)

    def calcSeparateVelocity(self):
        rela_velo = self.objB.velocity - self.objA.velocity
        return -np.dot(rela_velo, self.normal)
    
    def calcSeparateAcceleration(self):
        rela_accel = self.objB.acceleration - self.objA.acceleration
        return -np.dot(rela_accel, self.normal)

    def resolveVelocity(self, dt:float):
        totalInvMass = self.objA.invMass + self.objB.invMass
        if totalInvMass <= 0.0:
            return
        
        separateVelo = self.calcSeparateVelocity()
        if separateVelo > 0.0:
            return

        # calc impulse
        newSeparateVelo = -separateVelo * self.restitution

        separateAccel = self.calcSeparateAcceleration()
        if separateAccel < 0.0:
            newSeparateVelo += separateAccel * dt * self.restitution
            newSeparateVelo = max(newSeparateVelo, 0.0)

        deltaVelo = newSeparateVelo - separateVelo
        impulse = deltaVelo / totalInvMass * self.normal
        
        # apply impulse
        self.objA.velocity += impulse * self.objA.invMass
        self.objB.velocity -= impulse * self.objB.invMass
    
    def resolvePosition(self, step:float=1.0):
        totalInvMass = self.objA.invMass + self.objB.invMass
        if totalInvMass <= 0.0:
            return

        if self.penetration <= 0.0:
            return

        # calc movement
        curPenetration = self.penetration * step
        movement = curPenetration / totalInvMass * self.normal
        self.penetration -= curPenetration

        # apply movement
        self.objA.position += movement * self.objA.invMass
        self.objB.position -= movement * self.objB.invMass

class CollisionGenerator:
    def __init__(self, objA:PhyObject, objB:PhyObject):
        self.objA = objA
        self.objB = objB

    def simulate(self, dt:float) -> PhyCollision:
        pass

    def draw(self, surface:pygame.Surface):
        pass

class Cable(CollisionGenerator):
    def __init__(self, objA:PhyObject, objB:PhyObject, len:float, restitution:float):
        super().__init__(objA, objB)
        self.length = len
        self.color = getRandomColor()
        self.restitution = restitution

    def simulate(self, dt:float) -> PhyCollision:
        vec_a2b = self.objB.position - self.objA.position
        dist_a2b = np.linalg.norm(vec_a2b)
        self.color.a = int(np.clip(dist_a2b/self.length, 0, 1)*255.0)

        if dist_a2b > self.length:
            position = None
            normal = vec_a2b / dist_a2b
            penetration = dist_a2b - self.length 
            pc = PhyCollision(self.objA, self.objB, position, normal, penetration)
            pc.restitution = self.restitution
            return pc
        else:
            return None

    def draw(self, surface:pygame.Surface):
        pixelPosA = physicsVectorToPixelVector(self.objA.position)
        pixelPosB = physicsVectorToPixelVector(self.objB.position)

        pygame.draw.line(surface, self.color.premul_alpha(), pixelPosA, pixelPosB, width=2)   

class Rod(CollisionGenerator):
    def __init__(self, objA:PhyObject, objB:PhyObject, len:float, restitution:float):
        super().__init__(objA, objB)
        self.length = len
        self.color = getRandomColor()
        self.restitution = restitution

    def simulate(self, dt:float) -> PhyCollision:
        vec_a2b = self.objB.position - self.objA.position
        dist_a2b = np.linalg.norm(vec_a2b)

        deltaLen = dist_a2b - self.length
        if np.abs(deltaLen) > epsilon:
            position = None
            normal = vec_a2b / dist_a2b * np.sign(deltaLen)
            penetration = np.abs(deltaLen)
            pc = PhyCollision(self.objA, self.objB, position, normal, penetration)
            pc.restitution = self.restitution
            return pc
        else:
            return None

    def draw(self, surface:pygame.Surface):
        pixelPosA = physicsVectorToPixelVector(self.objA.position)
        pixelPosB = physicsVectorToPixelVector(self.objB.position)

        pygame.draw.line(surface, self.color, pixelPosA, pixelPosB, width=4)

def sphere_vs_sphere(sphereA:PhySphere, sphereB:PhySphere):
    vec_a2b = sphereB.position - sphereA.position
    dist_a2b = np.linalg.norm(vec_a2b)
    total_r = sphereA.radius + sphereB.radius

    if dist_a2b >= total_r:
        return None
    else:
        if dist_a2b <= epsilon:
            normal = getRandomDirection()
        else:
            normal = vec_a2b / -dist_a2b
        position = sphereB.position + normal * sphereB.radius
        penetration = total_r - dist_a2b

        hit = PhyCollision(sphereA, sphereB, position, normal, penetration)
        return hit

def sphere_vs_abox(sphereA:PhySphere, aboxB:PhyAlignedBox):
    aboxMin = aboxB.position - aboxB.size*0.5
    aboxMax = aboxB.position + aboxB.size*0.5

    closest = np.clip(sphereA.position, aboxMin, aboxMax)
    vec_a2b = closest - sphereA.position
    dist_a2b = np.linalg.norm(vec_a2b)

    if dist_a2b >= sphereA.radius:
        return None
    else:
        if dist_a2b > epsilon: # 圆心在外
            normal = vec_a2b / -dist_a2b
            position = closest.copy()
            penetration = sphereA.radius - dist_a2b
        else: # 圆心在内
            vec_local = sphereA.position - aboxB.position
            dxy = aboxB.size*0.5 - np.abs(vec_local)
            if dxy[0] < dxy[1]:
                normal = np.array([np.sign(vec_local[0]), 0.0], dtype=precision_type)
            else:
                normal = np.array([0.0, np.sign(vec_local[1])], dtype=precision_type)
            position = sphereA.position + normal * dxy
            penetration = sphereA.radius + min(dxy[0], dxy[1])
        
        hit = PhyCollision(sphereA, aboxB, position, normal, penetration)
        return hit

def particle_vs_abox(particleA:PhyParticle, aboxB:PhyAlignedBox):
    aboxMin = aboxB.position - aboxB.size*0.5
    aboxMax = aboxB.position + aboxB.size*0.5

    closest = np.clip(particleA.position, aboxMin, aboxMax)
    vec_a2b = closest - particleA.position
    dist_a2b = np.linalg.norm(vec_a2b)

    if dist_a2b > epsilon:
        return None
    else:
        vec_local = particleA.position - aboxB.position
        dxy = aboxB.size*0.5 - np.abs(vec_local)
        if dxy[0] < dxy[1]:
            normal = np.array([np.sign(vec_local[0]), 0.0], dtype=precision_type)
        else:
            normal = np.array([0.0, np.sign(vec_local[1])], dtype=precision_type)
        position = particleA.position + normal * dxy
        penetration = min(dxy[0], dxy[1])
        
        hit = PhyCollision(particleA, aboxB, position, normal, penetration)
        return hit

def particle_vs_sphere(particleA:PhyParticle, sphereB:PhySphere):
    vec_a2b = sphereB.position - particleA.position
    dist_a2b = np.linalg.norm(vec_a2b)

    if dist_a2b >= sphereB.radius:
        return None
    else:
        if dist_a2b <= epsilon:
            normal = getRandomDirection()
        else:
            normal = vec_a2b / -dist_a2b
        position = sphereB.position + normal * sphereB.radius
        penetration = sphereB.radius - dist_a2b

        hit = PhyCollision(particleA, sphereB, position, normal, penetration)
        return hit

def collision_detect(objA:PhyObject, objB:PhyObject, outList:list[PhyCollision]):
    objAType = objA.objType
    objBType = objB.objType

    hit = None
    if objAType == 'sphere' and objBType == 'sphere':
        hit = sphere_vs_sphere(objA, objB)
    elif objAType == 'sphere' and objBType == 'abox':
        hit = sphere_vs_abox(objA, objB)
    elif objAType == 'abox' and objBType == 'sphere':
        hit = sphere_vs_abox(objB, objA)
    elif objAType == 'particle' and objBType == 'abox':
        hit = particle_vs_abox(objA, objB)
    elif objAType == 'abox' and objBType == 'particle':
        hit = particle_vs_abox(objB, objA)
    elif objAType == 'particle' and objBType == 'sphere':
        hit = particle_vs_sphere(objA, objB)
    elif objAType == 'sphere' and objBType == 'particle':
        hit = particle_vs_sphere(objB, objA)

    if hit is not None:
        outList.append(hit)

def collision_detect_all(dt:float, objects:list[PhyObject], particles:list[PhyParticle] = None, 
                         links:list[CollisionGenerator] = None):
    collisionList:list[PhyCollision] = []

    # check collisions    
    numOfObjs = len(objects)
    for idx in range(numOfObjs):
        for ndx in range(idx+1, numOfObjs):
            collision_detect(objects[idx], objects[ndx], collisionList)
    if particles is not None:
        for par in particles:
            for obj in objects:
                collision_detect(par, obj, collisionList)

    # gen collisions
    if links is not None:
        for lk in links:
            phyLk = lk.simulate(dt)
            if phyLk is not None:
                collisionList.append(phyLk)

    # resolve collisions
    for _ in range(Velocity_Iter):
        for collision in collisionList:
            collision.resolveVelocity(dt)
    for _ in range(Position_Iter):
        for collision in collisionList:
            collision.resolvePosition(0.4)

    # for collision in collisionList:
    #     collision.resolveVelocity(dt)
    #     collision.resolvePosition(dt)