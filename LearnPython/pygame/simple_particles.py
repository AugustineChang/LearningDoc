import pygame
import numpy as np

################################# GlobalCommon #################################
screenWidth = 1024
screenHeight = 1024
meterPerPixel = 100.0
epsilon = 1e-6

rng = np.random.default_rng()
precision_type = np.float32
zero_vector = np.array([0, 0], dtype=precision_type)

def lerp(a:np.ndarray|float, b:np.ndarray|float, alpha:float):
    return a + alpha * (b - a)

def physicsVectorToPixelVector(physicsPos:np.ndarray, isPoint:bool = True, tolist:bool = True):
    pixelPos = physicsPos * np.array([meterPerPixel, -meterPerPixel], dtype=precision_type)
    if isPoint:
        pixelPos += np.array([screenWidth*0.5, screenHeight*0.5], dtype=precision_type)

    if tolist:
        return np.round(pixelPos).tolist()
    else:
        return np.round(pixelPos)

def physicsScalarToPixelScalar(physicsScalar:float|np.ndarray, tolist:bool = True):
    result = physicsScalar * meterPerPixel
    if tolist:
        return result.tolist() if isinstance(result, np.ndarray) else result
    else:
        return result

def getSceneSize():
    return (screenWidth / meterPerPixel, screenHeight / meterPerPixel)

def getRandomColor():
    rand_rgb = rng.integers(0, 256, size=3)
    return pygame.Color(*rand_rgb.tolist())

def getRandomDirection():
    rand_theta = rng.uniform(0, 2 * np.pi)
    return np.array([np.cos(rand_theta), np.sin(rand_theta)], dtype=precision_type)

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
        self.velocity = getRandomDirection() * rng.uniform(0.5, 2.0)
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
            self.status = 2 if rng.integers(0, 2) == 0 else 0

    def draw(self, surface:pygame.Surface):
        if self.status != 1: return
        pixelPos = physicsVectorToPixelVector(self.position)
        pixelRadius = physicsScalarToPixelScalar(self.radius)

        drawCol = pygame.Color(self.color)
        drawCol.a = int(255.0 * self.age / self.maxAge + 0.5)
        pygame.draw.circle(surface, drawCol.premul_alpha(), pixelPos, pixelRadius)

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

################################# Particle System #################################

class PartcleSystem:
    def __init__(self):
        self.listAlive:list[PhyParticle] = []
        self.listDead:list[PhyParticle] = []
        
        self.maxParticles = 200
        self.initSpawns = 10
        self.deadSpawns = 5

        for idx in range(self.initSpawns):
            self.create_to_alive(0.0, 0.0)

    def remove_to_dead(self, idx:int):
        dead_par = self.listAlive[idx]
        if len(self.listDead) < self.maxParticles:
            self.listDead.append(dead_par)
        self.listAlive[idx] = self.listAlive[-1]
        self.listAlive.pop()

    def create_to_alive(self, x:float, y:float, vx:float=0.0, vy:float=0.0):
        if len(self.listAlive) >= self.maxParticles:
            return

        if len(self.listDead) > 0:
            reuse_particle = self.listDead.pop()
            reuse_particle.__init__(x, y, 0.02, getRandomColor())
            reuse_particle.velocity[:] += [vx,vy]
            self.listAlive.append(reuse_particle)
        else:
            new_particle = PhyParticle(x, y, 0.02, getRandomColor())
            new_particle.velocity[:] += [vx,vy]
            self.listAlive.append(new_particle)

    def simulate(self, dt:float, forces:list[ForceGenerator]):
        for par in self.listAlive:
            for force in forces:
                force.addForce(par)
            par.simulate(dt)       

        # spawn lifetime
        spawnParams = []
        for idx in range(len(self.listAlive) - 1, -1, -1):
            par = self.listAlive[idx]
            if par.status != 1:
                self.remove_to_dead(idx)
                if par.status == 2:
                    spawnParams.append((par.position.copy(), par.velocity.copy()))
        
        for pos, velo in spawnParams:
            for idx in range(self.deadSpawns):
                self.create_to_alive(pos[0], pos[1], velo[0], velo[1])

    def draw(self, surface:pygame.Surface):
        for par in self.listAlive:
            par.draw(surface)

    def isRunning(self):
        return len(self.listAlive) > 0
################################# User Control #################################

class UserController:
    def __init__(self, objects:list[PhyObject] = None, particleSys:PartcleSystem = None):
        self.status = 1 # 0-stop 1-running
        self.frameCounter = 0
        self.allObjects = objects
        self.particleSystem = particleSys

    def eventReceiver(self):
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                self.quit()
            elif event.type == pygame.MOUSEBUTTONDOWN:
                if event.button == 1: self.leftClick()
                elif event.button == 3: self.rightClick()

        isNoObjects = self.allObjects is None or len(self.allObjects) <= 0
        isParticlesStop = self.particleSystem is None or not self.particleSystem.isRunning()
        if isNoObjects and isParticlesStop:
            self.quit()

    def tick(self):
        self.frameCounter += 1

    def leftClick(self):
        if self.allObjects is None: return

        validObjs = []
        for obj in self.allObjects:
            if obj.objType == 'sphere' and obj.status == 1:
                validObjs.append(obj)
        numOfObj = len(validObjs)
        randIdx = rng.integers(0, numOfObj)
        randObj = validObjs[randIdx]
        randObj.velocity += getRandomDirection() * rng.uniform(5.0, 10.0)
    
    def rightClick(self):
        if self.allObjects is None: return

        newObj = PhySphere(2.0, 0.0, 0.1, getRandomColor())
        newObj.velocity = getRandomDirection() * rng.uniform(0.5, 2.0)
        self.allObjects.append(newObj)

    def isRunning(self):
        return self.status == 1

    def shouldRender(self):
        return self.frameCounter % 4 == 0

    def quit(self):
        self.status = 0

###################################################################################

def particle_main():
    # 1. Initialize & Setup
    pygame.init()
    screen = pygame.display.set_mode((screenWidth, screenHeight))
    clock = pygame.time.Clock()

    objects:list[PhyObject] = [PhyAlignedBox(0.0, -4.0, 6.0, 0.5, getRandomColor())]
    forces:list[ForceGenerator] = [GravityForce()]
    particleSys = PartcleSystem()
    usrCtrl = UserController(None, particleSys)

    # 2. Main Game Loop
    while usrCtrl.isRunning():
        dt = clock.tick_busy_loop(240) * 0.001  # Delta time in seconds

        # controller
        usrCtrl.eventReceiver()

        # simulating
        for force in forces:
            force.simulate(dt)
        particleSys.simulate(dt, forces)
        collision_detect_all(objects, particleSys.listAlive)
        
        # rendering
        if usrCtrl.shouldRender():
            screen.fill("black")  # Clear screen
            for force in forces:
                force.draw(screen)
            for obj in objects:
                obj.draw(screen)
            particleSys.draw(screen)
            pygame.display.flip() # Update the display
        usrCtrl.tick()

    pygame.quit()

def init_spring_one(objects:list[PhyObject], forces:list[ForceGenerator]):
    fixPos = np.array([0,3], dtype=precision_type)
    
    objects.append(PhySphere(0.0, 2.0, 0.2, getRandomColor()))
    forces.append(SpringForce(fixPos, objects[0], 0.8, 1.0, 0.08))

def init_spring_three(objects:list[PhyObject], forces:list[ForceGenerator]):
    objects.append(PhySphere(2.0, 0.0, 0.1, getRandomColor()))
    objects.append(PhySphere(0.0, 2.0, 0.1, getRandomColor()))
    objects.append(PhySphere(0.0, 0.0, 0.1, getRandomColor()))

    forces.append(SpringForce(objects[0], objects[1], 0.8, 1000.0, 0.08))
    forces.append(SpringForce(objects[1], objects[2], 0.8, 1000.0, 0.08))
    forces.append(SpringForce(objects[2], objects[0], 0.8, 1000.0, 0.08))

def init_spring_net(objects:list[PhyObject], forces:list[ForceGenerator]):
    # init objects
    sizeX, sizeY = (5,5)
    for y in range(sizeY-1,-1,-1):
        for x in range(sizeX):
            objects.append(PhySphere(x-sizeX*0.5, y-sizeY*0.5, 0.1, getRandomColor()))

    # init forces
    fixPosA = np.array([-3,4], dtype=precision_type)
    fixPosB = np.array([3,4], dtype=precision_type)
    forces.append(SpringForce(fixPosA, objects[0], 1.0, 50.0, 0.5))
    forces.append(SpringForce(fixPosB, objects[sizeX-1], 1.0, 50.0, 0.5))
    for y in range(sizeY):
        for x in range(sizeX-1):
            objIdx = y * sizeX + x
            forces.append(SpringForce(objects[objIdx], objects[objIdx+1], 0.8, 20.0, 0.5))
    for x in range(sizeX):
        for y in range(sizeY-1):
            objIdx = y * sizeX + x
            forces.append(SpringForce(objects[objIdx], objects[objIdx+sizeX], 0.8, 20.0, 0.5))

def init_collision(objects:list[PhyObject], forces:list[ForceGenerator]):
    objects.append(PhySphere(0.0, 2.0, 0.2, getRandomColor()))
    objects.append(PhySphere(2.0, 0.0, 0.1, getRandomColor()))
    
    sceneW, sceneH = getSceneSize()
    aboxThick = 0.5
    objects.append(PhyAlignedBox( 0.0, (sceneH-aboxThick)*0.5, sceneW, aboxThick, getRandomColor()))
    objects.append(PhyAlignedBox( 0.0,-(sceneH-aboxThick)*0.5, sceneW, aboxThick, getRandomColor()))
    objects.append(PhyAlignedBox( (sceneW-aboxThick)*0.5, 0.0, aboxThick, sceneH - 2.0*aboxThick, getRandomColor()))
    objects.append(PhyAlignedBox(-(sceneW-aboxThick)*0.5, 0.0, aboxThick, sceneH - 2.0*aboxThick, getRandomColor()))

    objects[0].velocity[1] = -2.0 
    objects[1].velocity[0] = -2.5 

def spring_main():
    # 1. Initialize & Setup
    pygame.init()
    screen = pygame.display.set_mode((screenWidth, screenHeight))
    clock = pygame.time.Clock()

    objects:list[PhyObject] = []
    forces:list[ForceGenerator] = [DragForce()]#[GravityForce(), DragForce()]
    #init_spring_one(objects, forces)
    #init_spring_three(objects, forces)
    #init_spring_net(objects, forces)
    init_collision(objects, forces)

    usrCtrl = UserController(objects)

    # 2. Main Game Loop
    while usrCtrl.isRunning():
        dt = clock.tick_busy_loop(240) * 0.001  # Delta time in seconds

        # controller
        usrCtrl.eventReceiver()

        # simulating
        for force in forces:
            force.simulate(dt)
        for obj in objects:
            for force in forces:
                force.addForce(obj)
            obj.simulate(dt)   
        collision_detect_all(objects)

        # rendering
        if usrCtrl.shouldRender():
            screen.fill("black")  # Clear screen
            for force in forces:
                force.draw(screen)
            for obj in objects:
                obj.draw(screen)
            pygame.display.flip() # Update the display
        usrCtrl.tick()

    pygame.quit()

    
if __name__ == "__main__":
    #spring_main()
    particle_main()