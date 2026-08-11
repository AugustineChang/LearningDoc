import pygame
import numpy as np
from .common import epsilon, cross, clip, makeArray, localToWorld, worldToLocal
from .common import getRandomDirection, physicsVectorToPixelVector
from .objects import PhyObjType, PhyObject, PhyPlane, PhySphere, PhyBox, PhyConvexPolygon, PhyParticle
from .objects import isStaticType

################################# Collision Handler #################################

Velocity_Iter = 4
Position_Iter = 2

class PhyCollision:
    def __init__(self, objA:PhyObject, objB:PhyObject, positionA:np.ndarray, positionB:np.ndarray, normal:np.ndarray, penetration:float):
        self.objA = objA
        self.objB = objB
        self.positionA = positionA.copy() if positionA is not None else positionA
        self.positionB = positionB.copy() if positionB is not None else self.positionA
        self.normal = normal.copy()
        self.tangent = makeArray([-normal[1], normal[0]])
        self.penetration = penetration # collision depth
        self.restitution = min(self.objA.restitution, self.objB.restitution) # collision loss
        self.fraction = min(self.objA.fraction, self.objB.fraction) # friction factor

    def calcSeparateVelocity(self):
        rela_velo = self.objB.getVelocity(self.positionB) - self.objA.getVelocity(self.positionA)
        return -np.dot(rela_velo, self.normal)
    
    def calcSeparateAcceleration(self):
        rela_accel = self.objB.acceleration - self.objA.acceleration
        return -np.dot(rela_accel, self.normal)

    def calcFrictionVelocity(self):
        rela_velo = self.objB.getVelocity(self.positionB) - self.objA.getVelocity(self.positionA)
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

        rA = self.positionA - self.objA.position
        rB = self.positionB - self.objB.position
        rA_x_n = cross(rA, self.normal)
        rB_x_n = cross(rB, self.normal)

        denom = totalInvMass + rA_x_n*rA_x_n*self.objA.invInertia + rB_x_n*rB_x_n*self.objB.invInertia
        impulseIntensity = deltaVelo / denom
        impulse = impulseIntensity * self.normal
        
        # apply impulse
        self.objA.velocity += impulse * self.objA.invMass
        self.objB.velocity -= impulse * self.objB.invMass
        
        # angular impulse
        self.objA.angularVelocity += cross(rA, impulse) * self.objA.invInertia
        self.objB.angularVelocity -= cross(rB, impulse) * self.objB.invInertia

        # friction
        if self.fraction > 0.0:
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
    def __init__(self, objA:PhyObject, objB:PhyObject, offsetA:np.ndarray, offsetB:np.ndarray):
        self.objA = objA
        self.objB = objB
        self.offsetA = offsetA.copy() if offsetA is not None else None
        self.offsetB = offsetB.copy() if offsetB is not None else None

    def getEndPosition(self, isEndA:bool):
        obj = self.objA if isEndA else self.objB
        offset = self.offsetA if isEndA else self.offsetB
        return obj.localToWorld(offset) if offset is not None else obj.position

    def simulate(self, dt:float) -> PhyCollision:
        pass

    def draw(self, surface:pygame.Surface):
        pass

class Cable(CollisionGenerator):
    def __init__(self, objA:PhyObject, objB:PhyObject, len:float, restitution:float, color:pygame.Color, offsetA:np.ndarray, offsetB:np.ndarray):
        super().__init__(objA, objB, offsetA, offsetB)
        self.length = len
        self.color = color
        self.restitution = restitution
    
    def simulate(self, dt:float) -> PhyCollision:
        posA = self.getEndPosition(True)
        posB = self.getEndPosition(False)
        vec_a2b = posB - posA
        dist_a2b = np.linalg.norm(vec_a2b)
        self.color.a = int(np.clip(dist_a2b/self.length, 0, 1)*255.0)

        if dist_a2b > self.length:
            normal = vec_a2b / dist_a2b
            penetration = dist_a2b - self.length 
            pc = PhyCollision(self.objA, self.objB, posA, posB, normal, penetration)
            pc.restitution = self.restitution
            pc.fraction = 0.0
            return pc
        else:
            return None

    def draw(self, surface:pygame.Surface):
        pixelPosA = physicsVectorToPixelVector(self.getEndPosition(True))
        pixelPosB = physicsVectorToPixelVector(self.getEndPosition(False))

        pygame.draw.line(surface, self.color.premul_alpha(), pixelPosA, pixelPosB, width=2)   

class Rod(CollisionGenerator):
    def __init__(self, objA:PhyObject, objB:PhyObject, len:float, restitution:float, color:pygame.Color, offsetA:np.ndarray, offsetB:np.ndarray):
        super().__init__(objA, objB, offsetA, offsetB)
        if len is not None:
            self.length = len
        else:
            self.length = np.linalg.norm(self.getEndPosition(True) - self.getEndPosition(False))
        self.color = color
        self.restitution = restitution

    def simulate(self, dt:float) -> PhyCollision:
        posA = self.getEndPosition(True)
        posB = self.getEndPosition(False)
        vec_a2b = posB - posA
        dist_a2b = np.linalg.norm(vec_a2b)

        deltaLen = dist_a2b - self.length
        if np.abs(deltaLen) > epsilon:
            normal = vec_a2b / dist_a2b * np.sign(deltaLen)
            penetration = np.abs(deltaLen)
            pc = PhyCollision(self.objA, self.objB, posA, posB, normal, penetration)
            pc.restitution = self.restitution
            pc.fraction = 0.0
            return pc
        else:
            return None

    def draw(self, surface:pygame.Surface):
        pixelPosA = physicsVectorToPixelVector(self.getEndPosition(True))
        pixelPosB = physicsVectorToPixelVector(self.getEndPosition(False))

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

        hit = PhyCollision(sphereA, sphereB, position, None, normal, penetration)
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
        hit = PhyCollision(sphereA, boxB, position, None, normal, penetration)
        outList.append(hit)

def sphere_vs_convex(sphereA:PhySphere, convexB:PhyBox, outList:list[PhyCollision]):
    pointsB = convexB.localToWorld(convexB.corners)
    centerA = sphereA.position

    edgesB = np.roll(pointsB, -1 , axis=0) - pointsB
    edgesBLen = np.maximum(np.linalg.norm(edgesB, axis=1, keepdims=True), epsilon)
    normEdgesB = edgesB / edgesBLen
    toCenterA = centerA - pointsB

    distOnEdge = np.sum(toCenterA*normEdgesB, axis=1)[:,None]
    ratioOnEdge = np.clip(distOnEdge / edgesBLen, 0.0, 1.0)
    closests = pointsB + edgesB * ratioOnEdge
    dists2 = np.sum((centerA - closests) ** 2, axis=1)

    minIdx = np.argmin(dists2)
    closest, dist_a2b = closests[minIdx], np.sqrt(dists2[minIdx])

    normal = normEdgesB[minIdx, [1, 0]]
    normal[0] *= -1

    isOutside = np.dot(toCenterA[minIdx], normal) > 0.0

    if isOutside:
        if dist_a2b >= sphereA.radius:
            return
        penetration = sphereA.radius - dist_a2b
    else:
        penetration = sphereA.radius + dist_a2b

    hit = PhyCollision(sphereA, convexB, closest, None, normal, penetration)
    outList.append(hit)

def sphere_vs_plane(sphereA:PhySphere, planeB:PhyPlane, outList:list[PhyCollision]):
    vec_b2a = sphereA.position - planeB.position
    distToPlane = np.dot(vec_b2a, planeB.normal)
    if distToPlane >= sphereA.radius:
        return
    else:
        normal = planeB.normal
        position = sphereA.position - planeB.normal * distToPlane
        penetration = sphereA.radius - distToPlane

        hit = PhyCollision(sphereA, planeB, position, None, normal, penetration)
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

        hit = PhyCollision(particleA, sphereB, position, None, normal, penetration)
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
        hit = PhyCollision(particleA, boxB, position, None, normal, penetration)
        outList.append(hit)

def particle_vs_convex(particleA:PhyParticle, convexB:PhyBox, outList:list[PhyCollision]):
    pointsB = convexB.localToWorld(convexB.corners)
    centerA = particleA.position

    edgesB = np.roll(pointsB, -1 , axis=0) - pointsB
    edgesBLen = np.maximum(np.linalg.norm(edgesB, axis=1, keepdims=True), epsilon)
    normEdgesB = edgesB / edgesBLen
    toCenterA = centerA - pointsB

    distOnEdge = np.sum(toCenterA*normEdgesB, axis=1)[:,None]
    ratioOnEdge = np.clip(distOnEdge / edgesBLen, 0.0, 1.0)
    closests = pointsB + edgesB * ratioOnEdge
    dists2 = np.sum((centerA - closests) ** 2, axis=1)

    minIdx = np.argmin(dists2)
    closest, penetration = closests[minIdx], np.sqrt(dists2[minIdx])

    normal = normEdgesB[minIdx, [1, 0]]
    normal[0] *= -1

    isOutside = np.dot(toCenterA[minIdx], normal) > 0.0
    if isOutside:
        return
    else:
        hit = PhyCollision(particleA, convexB, closest, None, normal, penetration)
        outList.append(hit)

def particle_vs_plane(particleA:PhyParticle, planeB:PhyPlane, outList:list[PhyCollision]):
    vec_b2a = particleA.position - planeB.position
    distToPlane = np.dot(vec_b2a, planeB.normal)
    if distToPlane >= 0.0:
        return
    else:
        normal = planeB.normal
        position = particleA.position - planeB.normal * distToPlane
        penetration = -distToPlane

        hit = PhyCollision(particleA, planeB, position, None, normal, penetration)
        outList.append(hit)

def convex_collision_detail(referencePoints:np.ndarray, incidentPoints:np.ndarray, 
                            referenceEdge:int, referenceNormal:np.ndarray) -> tuple[list[np.ndarray], list[np.ndarray]]:
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
        return [], []

    depths = np.sum(-referenceNormal*(inciEnds - refEndA), axis=1)
    contact_indices = np.where(depths > 0)

    return list(inciEnds[contact_indices]), list(depths[contact_indices])

def calcConvexNormals(points:np.ndarray) -> np.ndarray:
    edges = np.roll(points, -1, axis=0) - points
    normals = edges[:, [1, 0]]
    normals[:, 0] *= -1
    normals /= np.maximum(np.linalg.norm(normals, axis=1, keepdims=True), epsilon)
    return normals

def convex_vs_convex(convexA:PhyBox|PhyConvexPolygon, convexB:PhyBox|PhyConvexPolygon, outList:list[PhyCollision]):
    pointsA = convexA.localToWorld(convexA.corners)
    pointsB = convexB.localToWorld(convexB.corners)

    normalsA = calcConvexNormals(pointsA)
    normalsB = calcConvexNormals(pointsB)

    overlapA = np.max(pointsA @ normalsA.T, axis=0) - np.min(pointsB @ normalsA.T, axis=0)
    overlapB = np.max(pointsB @ normalsB.T, axis=0) - np.min(pointsA @ normalsB.T, axis=0)
    overlap = np.concatenate([overlapA, overlapB])
    if np.any(overlap <= 0.0):
        return 
    else:
        minIdx = np.argmin(overlap)

        numA = len(overlapA)
        isRefA = (minIdx < numA)
        if isRefA:
            refEdge = minIdx
            refNormal = normalsA[refEdge,:]
            normalOut = -refNormal
        else:
            refEdge = minIdx - numA
            refNormal = normalsB[refEdge,:]
            normalOut = refNormal

        referencePoints = pointsA if isRefA else pointsB
        incidentPoints = pointsB if isRefA else pointsA
        hitPositions, hitDepths = convex_collision_detail(referencePoints, incidentPoints, refEdge, refNormal)
        for hitPos, hitDep in zip(hitPositions, hitDepths):
            hit = PhyCollision(convexA, convexB, hitPos, None, normalOut, hitDep)
            outList.append(hit)

def convex_vs_plane(convexA:PhyBox|PhyConvexPolygon, planeB:PhyPlane, outList:list[PhyCollision]):
    vec_b2a = convexA.position - planeB.position
    distToPlane = np.dot(vec_b2a, planeB.normal)
    if distToPlane >= convexA.boundingRadius:
        return
    else:
        normal = planeB.normal
        pointsA = convexA.localToWorld(convexA.corners)

        vecs_b2a = pointsA - planeB.position
        dists = np.sum(vecs_b2a*planeB.normal, axis=1)
        hitIndices = np.where(dists < 0.0)
        hitPositions = list(pointsA[hitIndices])
        hitDepths = list(dists[hitIndices])
        for position, depth in zip(hitPositions, hitDepths):
            hit = PhyCollision(convexA, planeB, position, None, normal, -depth)
            outList.append(hit)

def flatten_combined_objects(obj:PhyObject, root:PhyObject, out:list[PhyObject]):
    if obj.objType == PhyObjType.COMBINED:
        for subObj in obj.subObjects:
            flatten_combined_objects(subObj, root, out)
    else:
        out.append((obj, root))

def collision_detect(objA:PhyObject, objB:PhyObject, outList:list[PhyCollision]):
    objAType = objA.objType
    objBType = objB.objType
    if (isStaticType(objA.mass) and isStaticType(objB.mass)) or \
        (objAType == PhyObjType.PARTICLE and objBType == PhyObjType.PARTICLE):
        return

    # broad phase: 包围球
    rA = objA.boundingRadius
    rB = objB.boundingRadius
    delta = objB.position - objA.position
    if np.dot(delta, delta) > (rA + rB)**2:
        return

    if objAType == PhyObjType.COMBINED or objBType == PhyObjType.COMBINED:
        subobjectsA = []
        flatten_combined_objects(objA, objA, subobjectsA)
        subobjectsB = []
        flatten_combined_objects(objB, objB, subobjectsB)

        objToRoot = { obj : root for (obj, root) in subobjectsA + subobjectsB }

        subHits:list[PhyCollision] = []
        for subA, _ in subobjectsA:
            for subB, _ in subobjectsB:
                collision_detect(subA, subB, subHits)
        for hit in subHits:
            hit.objA = objToRoot[hit.objA]
            hit.objB = objToRoot[hit.objB]
            outList.append(hit)
        return

    isObjAConvex = objAType in (PhyObjType.BOX, PhyObjType.CONVEX)
    isObjBConvex = objBType in (PhyObjType.BOX, PhyObjType.CONVEX)

    if objAType == PhyObjType.SPHERE and objBType == PhyObjType.SPHERE:
        sphere_vs_sphere(objA, objB, outList)
    elif objAType == PhyObjType.SPHERE and objBType == PhyObjType.PLANE:
        sphere_vs_plane(objA, objB, outList)
    elif objAType == PhyObjType.PLANE and objBType == PhyObjType.SPHERE:
        sphere_vs_plane(objB, objA, outList)
    elif objAType == PhyObjType.SPHERE and objBType == PhyObjType.BOX:
        sphere_vs_box(objA, objB, outList)
    elif objAType == PhyObjType.BOX and objBType == PhyObjType.SPHERE:
        sphere_vs_box(objB, objA, outList)
    elif objAType == PhyObjType.SPHERE and objBType == PhyObjType.CONVEX:
        sphere_vs_convex(objA, objB, outList)
    elif objAType == PhyObjType.CONVEX and objBType == PhyObjType.SPHERE:
        sphere_vs_convex(objB, objA, outList)
    elif objAType == PhyObjType.PARTICLE and objBType == PhyObjType.SPHERE:
        particle_vs_sphere(objA, objB, outList)
    elif objAType == PhyObjType.SPHERE and objBType == PhyObjType.PARTICLE:
        particle_vs_sphere(objB, objA, outList)
    elif objAType == PhyObjType.PARTICLE and objBType == PhyObjType.BOX:
        particle_vs_box(objA, objB, outList)
    elif objAType == PhyObjType.BOX and objBType == PhyObjType.PARTICLE:
        particle_vs_box(objB, objA, outList)
    elif objAType == PhyObjType.PARTICLE and objBType == PhyObjType.CONVEX:
        particle_vs_convex(objA, objB, outList)
    elif objAType == PhyObjType.CONVEX and objBType == PhyObjType.PARTICLE:
        particle_vs_convex(objB, objA, outList)
    elif objAType == PhyObjType.PARTICLE and objBType == PhyObjType.PLANE:
        particle_vs_plane(objA, objB, outList)
    elif objAType == PhyObjType.PLANE and objBType == PhyObjType.PARTICLE:
        particle_vs_plane(objB, objA, outList)
    elif isObjAConvex and isObjBConvex:
        convex_vs_convex(objA, objB, outList)
    elif isObjAConvex and objBType == PhyObjType.PLANE:
        convex_vs_plane(objA, objB, outList)
    elif objAType == PhyObjType.PLANE and isObjBConvex:
        convex_vs_plane(objB, objA, outList)

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