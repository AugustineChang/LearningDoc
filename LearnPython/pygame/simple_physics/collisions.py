import pygame
import numpy as np
from .common import epsilon, cross, clip, makeArray, localToWorld, worldToLocal
from .common import getRandomDirection, physicsVectorToPixelVector
from .objects import PhyObjType, PhyObject, PhySphere, PhyBox, PhyAlignedBox, PhyParticle

################################# Collision Handler #################################

Velocity_Iter = 4
Position_Iter = 2

class PhyCollision:
    def __init__(self, objA:PhyObject, objB:PhyObject, position:np.ndarray, normal:np.ndarray, penetration:float):
        self.objA = objA
        self.objB = objB
        self.position = position.copy() if position is not None else position
        self.normal = normal.copy()
        self.tangent = makeArray([-normal[1], normal[0]])
        self.penetration = penetration # collision depth
        self.restitution = min(self.objA.restitution, self.objB.restitution) # collision loss
        self.fraction = min(self.objA.fraction, self.objB.fraction) # friction factor

    def calcSeparateVelocity(self):
        rela_velo = self.objB.getVelocity(self.position) - self.objA.getVelocity(self.position)
        return -np.dot(rela_velo, self.normal)
    
    def calcSeparateAcceleration(self):
        rela_accel = self.objB.acceleration - self.objA.acceleration
        return -np.dot(rela_accel, self.normal)

    def calcFrictionVelocity(self):
        rela_velo = self.objB.getVelocity(self.position) - self.objA.getVelocity(self.position)
        return -np.dot(rela_velo, self.tangent)

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

        hasCollisionPos = self.position is not None
        if hasCollisionPos:
            rA = self.position - self.objA.position
            rB = self.position - self.objB.position
            rA_x_n = cross(rA, self.normal)
            rB_x_n = cross(rB, self.normal)

            denom = totalInvMass + rA_x_n*rA_x_n*self.objA.invInertia + rB_x_n*rB_x_n*self.objB.invInertia
        else:
            denom = totalInvMass

        impulseIntensity = deltaVelo / denom
        impulse = impulseIntensity * self.normal
        
        # apply impulse
        self.objA.velocity += impulse * self.objA.invMass
        self.objB.velocity -= impulse * self.objB.invMass

        if hasCollisionPos:
            # angular impulse
            self.objA.angularVelocity += cross(rA, impulse) * self.objA.invInertia
            self.objB.angularVelocity -= cross(rB, impulse) * self.objB.invInertia

            # friction
            frictionVelo = self.calcFrictionVelocity()

            rA_x_t = cross(rA, self.tangent)
            rB_x_t = cross(rB, self.tangent)
            tanDenom = totalInvMass + rA_x_t*rA_x_t*self.objA.invInertia + rB_x_t*rB_x_t*self.objB.invInertia
            
            tanImpulseIntensity = -frictionVelo / tanDenom
            if np.abs(frictionVelo) > epsilon:
                clipSize = self.fraction*impulseIntensity
                tanImpulseIntensity = np.clip(tanImpulseIntensity, -clipSize, clipSize)
            tanImpulse = tanImpulseIntensity * self.tangent

            self.objA.velocity += tanImpulse * self.objA.invMass
            self.objB.velocity -= tanImpulse * self.objB.invMass
            self.objA.angularVelocity += cross(rA, tanImpulse) * self.objA.invInertia
            self.objB.angularVelocity -= cross(rB, tanImpulse) * self.objB.invInertia

            # rolling
            # todo.....

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
    def __init__(self, objA:PhyObject, objB:PhyObject, len:float, restitution:float, color:pygame.Color):
        super().__init__(objA, objB)
        self.length = len
        self.color = color
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
    def __init__(self, objA:PhyObject, objB:PhyObject, len:float, restitution:float, color:pygame.Color):
        super().__init__(objA, objB)
        self.length = len if len is not None else np.linalg.norm(objA.position - objB.position)
        self.color = color
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

##################################################################################

def sphere_vs_sphere(sphereA:PhySphere, sphereB:PhySphere, outList:list[PhyCollision]):
    vec_a2b = sphereB.position - sphereA.position
    dist_a2b = np.linalg.norm(vec_a2b)
    total_r = sphereA.radius + sphereB.radius

    if dist_a2b >= total_r:
        return
    else:
        if dist_a2b <= epsilon:
            normal = getRandomDirection()
        else:
            normal = vec_a2b / -dist_a2b
        position = sphereB.position + normal * sphereB.radius
        penetration = total_r - dist_a2b

        hit = PhyCollision(sphereA, sphereB, position, normal, penetration)
        outList.append(hit)

def sphere_vs_abox(sphereA:PhySphere, aboxB:PhyAlignedBox, outList:list[PhyCollision]):
    aboxMin = aboxB.position - aboxB.size*0.5
    aboxMax = aboxB.position + aboxB.size*0.5

    closest = np.clip(sphereA.position, aboxMin, aboxMax)
    vec_a2b = closest - sphereA.position
    dist_a2b = np.linalg.norm(vec_a2b)

    if dist_a2b >= sphereA.radius:
        return
    else:
        if dist_a2b > epsilon: # 圆心在外
            normal = vec_a2b / -dist_a2b
            position = closest.copy()
            penetration = sphereA.radius - dist_a2b
        else: # 圆心在内
            vec_local = sphereA.position - aboxB.position
            dxy = aboxB.size*0.5 - np.abs(vec_local)
            if dxy[0] < dxy[1]:
                normal = makeArray([np.sign(vec_local[0]), 0.0])
            else:
                normal = makeArray([0.0, np.sign(vec_local[1])])
            position = sphereA.position + normal * dxy
            penetration = sphereA.radius + min(dxy[0], dxy[1])
        
        hit = PhyCollision(sphereA, aboxB, position, normal, penetration)
        outList.append(hit)

def particle_vs_abox(particleA:PhyParticle, aboxB:PhyAlignedBox, outList:list[PhyCollision]):
    aboxMin = aboxB.position - aboxB.size*0.5
    aboxMax = aboxB.position + aboxB.size*0.5

    closest = np.clip(particleA.position, aboxMin, aboxMax)
    vec_a2b = closest - particleA.position
    dist_a2b = np.linalg.norm(vec_a2b)

    if dist_a2b > epsilon:
        return
    else:
        vec_local = particleA.position - aboxB.position
        dxy = aboxB.size*0.5 - np.abs(vec_local)
        if dxy[0] < dxy[1]:
            normal = makeArray([np.sign(vec_local[0]), 0.0])
        else:
            normal = makeArray([0.0, np.sign(vec_local[1])])
        position = particleA.position + normal * dxy
        penetration = min(dxy[0], dxy[1])
        
        hit = PhyCollision(particleA, aboxB, position, normal, penetration)
        outList.append(hit)

def particle_vs_sphere(particleA:PhyParticle, sphereB:PhySphere, outList:list[PhyCollision]):
    vec_a2b = sphereB.position - particleA.position
    dist_a2b = np.linalg.norm(vec_a2b)

    if dist_a2b >= sphereB.radius:
        return
    else:
        if dist_a2b <= epsilon:
            normal = getRandomDirection()
        else:
            normal = vec_a2b / -dist_a2b
        position = sphereB.position + normal * sphereB.radius
        penetration = sphereB.radius - dist_a2b

        hit = PhyCollision(particleA, sphereB, position, normal, penetration)
        outList.append(hit)

def convex_collision_detail(referencePoints:np.ndarray, incidentPoints:np.ndarray, 
                            referenceEdge:int, referenceNormal:np.ndarray) -> list[np.ndarray]:
    # incident edge
    incidentNPoints = np.roll(incidentPoints, -1, axis=0)
    incidentEdgeVecs = incidentNPoints - incidentPoints
    incidentNormals = incidentEdgeVecs[:, [1, 0]]
    incidentNormals[:, 0] *= -1
    incidentDots = np.sum(incidentNormals * referenceNormal, axis=1)
    incidentEdege = np.argmin(incidentDots)
    inciEndA = incidentPoints[incidentEdege, :]
    inciEndB = incidentNPoints[incidentEdege, :]

    # clip
    numRefPoints = len(referencePoints)
    refEndA = referencePoints[referenceEdge, :]
    refEndB = referencePoints[(referenceEdge+1)%numRefPoints, :]
    refEdgeVec = refEndB - refEndA
    refEdgeLen = np.linalg.norm(refEdgeVec)
    refEdgeVec /= max(refEdgeLen, epsilon)
    
    inciEnds = clip(inciEndA, inciEndB, refEdgeVec, refEndA)
    if len(inciEnds) >= 2:
        inciEnds = clip(inciEnds[0], inciEnds[1], -refEdgeVec, refEndB)
    inciEnds = makeArray(inciEnds)

    # calc depth
    if len(inciEnds) <= 0:
        return []

    depths = np.sum(-referenceNormal*(inciEnds - refEndA), axis=1)
    contact_indices = np.where(depths > 0)

    return list(inciEnds[contact_indices])

def box_vs_box(boxA:PhyBox, boxB:PhyBox|PhyAlignedBox, outList:list[PhyCollision]):
    separationAxis = []
    rotMatA = boxA.getRotationMatrix()
    rotMatB = boxB.getRotationMatrix()
    separationAxis.append(rotMatA[:,0])
    separationAxis.append(rotMatA[:,1])
    separationAxis.append(rotMatB[:,0])
    separationAxis.append(rotMatB[:,1])
    separationAxis = makeArray(separationAxis)

    pointsA = boxA.position + boxA.corners @ rotMatA.T
    if isinstance(boxB, PhyAlignedBox):
        extent = boxB.size*0.5
        pointsB = makeArray([
            [-extent[0], -extent[1]], [-extent[0],  extent[1]], 
            [ extent[0],  extent[1]], [ extent[0], -extent[1]]
        ]) + boxB.position
    else:
        pointsB = boxB.position + boxB.corners @ rotMatB.T

    projA = pointsA @ separationAxis.T
    projB = pointsB @ separationAxis.T

    minA = np.min(projA, axis=0)
    minB = np.min(projB, axis=0)
    maxA = np.max(projA, axis=0)
    maxB = np.max(projB, axis=0)

    overlap = np.minimum(maxA, maxB) - np.maximum(minA, minB)
    if np.any(overlap <= 0.0):
        return 
    else:
        minIdx = np.argmin(overlap)
        minOverlap = overlap[minIdx]
        
        vec_b2a = boxA.position - boxB.position
        normal = separationAxis[minIdx,:]
        normalOut = normal * np.sign(np.dot(vec_b2a, normal))

        isRefA = (minIdx < 2)
        referenceObj = boxA if isRefA else boxB
        incidentObj = boxB if isRefA else boxA

        vecR2I = incidentObj.position - referenceObj.position
        normalSign = np.sign(np.dot(vecR2I, normal)).astype(np.int32)
        refEdge = (2 - (minIdx % 2) + 2*min(normalSign, 0)*-1) % 4
        refNormal = normal*normalSign

        referencePoints = pointsA if isRefA else pointsB
        incidentPoints = pointsB if isRefA else pointsA
        hitPositions = convex_collision_detail(referencePoints, incidentPoints, refEdge, refNormal)
        for hitPos in hitPositions:
            hit = PhyCollision(boxA, boxB, hitPos, normalOut, minOverlap)
            outList.append(hit)

def sphere_vs_box(sphereA:PhySphere, boxB:PhyBox, outList:list[PhyCollision]):
    rotMat = boxB.getRotationMatrix()
    localAPos = worldToLocal(sphereA.position, rotMat.T, boxB.position)

    aboxMin = -boxB.size*0.5
    aboxMax = boxB.size*0.5

    closest = np.clip(localAPos, aboxMin, aboxMax)
    vec_a2b = closest - localAPos
    dist_a2b = np.linalg.norm(vec_a2b)

    if dist_a2b >= sphereA.radius:
        return
    else:
        if dist_a2b > epsilon: # 圆心在外
            normal = vec_a2b / -dist_a2b
            position = closest.copy()
            penetration = sphereA.radius - dist_a2b
        else: # 圆心在内
            dxy = boxB.size*0.5 - np.abs(localAPos)
            if dxy[0] < dxy[1]:
                normal = makeArray([np.sign(localAPos[0]), 0.0])
            else:
                normal = makeArray([0.0, np.sign(localAPos[1])])
            position = localAPos + normal * dxy
            penetration = sphereA.radius + min(dxy[0], dxy[1])

        normal = localToWorld(normal, rotMat)
        position = localToWorld(position, rotMat, boxB.position)
        hit = PhyCollision(sphereA, boxB, position, normal, penetration)
        outList.append(hit)

def particle_vs_box(particleA:PhyParticle, boxB:PhyBox, outList:list[PhyCollision]):
    rotMat = boxB.getRotationMatrix()
    localAPos = worldToLocal(particleA.position, rotMat.T, boxB.position)

    aboxMin = -boxB.size*0.5
    aboxMax = boxB.size*0.5

    closest = np.clip(localAPos, aboxMin, aboxMax)
    vec_a2b = closest - localAPos
    dist_a2b = np.linalg.norm(vec_a2b)

    if dist_a2b > epsilon:
        return
    else:
        dxy = boxB.size*0.5 - np.abs(localAPos)
        if dxy[0] < dxy[1]:
            normal = makeArray([np.sign(localAPos[0]), 0.0])
        else:
            normal = makeArray([0.0, np.sign(localAPos[1])])
        position = localAPos + normal * dxy
        penetration = min(dxy[0], dxy[1])

        normal = localToWorld(normal, rotMat)
        position = localToWorld(position, rotMat, boxB.position)
        hit = PhyCollision(particleA, boxB, position, normal, penetration)
        outList.append(hit)

def collision_detect(objA:PhyObject, objB:PhyObject, outList:list[PhyCollision]):
    objAType = objA.objType
    objBType = objB.objType
    if (objAType == PhyObjType.ALIGNEDBOX and objBType == PhyObjType.ALIGNEDBOX) or \
        (objAType == PhyObjType.PARTICLE and objBType == PhyObjType.PARTICLE):
        return

    # broad phase: 包围球
    rA = objA.boundingRadius
    rB = objB.boundingRadius
    delta = objB.position - objA.position
    if np.dot(delta, delta) > (rA + rB)**2:
        return

    if objAType == PhyObjType.SPHERE and objBType == PhyObjType.SPHERE:
        sphere_vs_sphere(objA, objB, outList)
    elif objAType == PhyObjType.SPHERE and objBType == PhyObjType.ALIGNEDBOX:
        sphere_vs_abox(objA, objB, outList)
    elif objAType == PhyObjType.ALIGNEDBOX and objBType == PhyObjType.SPHERE:
        sphere_vs_abox(objB, objA, outList)
    elif objAType == PhyObjType.PARTICLE and objBType == PhyObjType.ALIGNEDBOX:
        particle_vs_abox(objA, objB, outList)
    elif objAType == PhyObjType.ALIGNEDBOX and objBType == PhyObjType.PARTICLE:
        particle_vs_abox(objB, objA, outList)
    elif objAType == PhyObjType.PARTICLE and objBType == PhyObjType.SPHERE:
        particle_vs_sphere(objA, objB, outList)
    elif objAType == PhyObjType.SPHERE and objBType == PhyObjType.PARTICLE:
        particle_vs_sphere(objB, objA, outList)
    elif objAType == PhyObjType.BOX and objBType == PhyObjType.BOX:
        box_vs_box(objA, objB, outList)
    elif objAType == PhyObjType.BOX and objBType == PhyObjType.SPHERE:
        sphere_vs_box(objB, objA, outList)
    elif objAType == PhyObjType.SPHERE and objBType == PhyObjType.BOX:
        sphere_vs_box(objA, objB, outList)
    elif objAType == PhyObjType.BOX and objBType == PhyObjType.ALIGNEDBOX:
        box_vs_box(objA, objB, outList)
    elif objAType == PhyObjType.ALIGNEDBOX and objBType == PhyObjType.BOX:
        box_vs_box(objB, objA, outList)
    elif objAType == PhyObjType.BOX and objBType == PhyObjType.PARTICLE:
        particle_vs_box(objB, objA, outList)
    elif objAType == PhyObjType.PARTICLE and objBType == PhyObjType.BOX:
        particle_vs_box(objA, objB, outList)

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