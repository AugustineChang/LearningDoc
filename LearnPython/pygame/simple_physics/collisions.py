import numpy as np
from .common import epsilon, getRandomDirection, precision_type
from .objects import PhyObject, PhySphere, PhyAlignedBox, PhyParticle

################################# Collision Handler #################################

class PhyCollision:
    def __init__(self, objA:PhyObject, objB:PhyObject, position:np.ndarray, normal:np.ndarray, penetration:float):
        self.objA = objA
        self.objB = objB
        self.position = position
        self.normal = normal
        self.penetration = penetration

    def calcSeparateVelocity(self):
        rela_velo = self.objB.velocity - self.objA.velocity
        return -np.dot(rela_velo, self.normal)
    
    def resolveCollision(self):
        separateVelo = self.calcSeparateVelocity()
        if separateVelo > 0.0:
            return

        totalInvMass = self.objA.invMass + self.objB.invMass
        if totalInvMass <= 0.0:
            return

        # calc movement
        movement = self.penetration / totalInvMass * self.normal

        # apply movement
        self.objA.position += movement * self.objA.invMass
        self.objB.position -= movement * self.objB.invMass

        # calc impulse
        restitution = min(self.objA.restitution, self.objB.restitution)
        newSeparateVelo = -separateVelo * restitution
        deltaVelo = newSeparateVelo - separateVelo
        impulse = deltaVelo / totalInvMass * self.normal
        
        # apply impulse
        self.objA.velocity += impulse * self.objA.invMass
        self.objB.velocity -= impulse * self.objB.invMass

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

def collision_detect(objA:PhyObject, objB:PhyObject):
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
        hit.resolveCollision()

def collision_detect_all(objects:list[PhyObject], particles:list[PhyParticle] = None):
    numOfObjs = len(objects)
    for idx in range(numOfObjs):
        for ndx in range(idx+1, numOfObjs):
            collision_detect(objects[idx], objects[ndx])
    if particles is not None:
        for par in particles:
            for obj in objects:
                collision_detect(par, obj)