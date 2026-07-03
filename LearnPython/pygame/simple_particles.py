import pygame
import numpy as np

################################# GlobalCommon ############################
screenWidth = 1024
screenHeight = 1024
meterPerPixel = 100.0
epcilon = 1e-6

rng = np.random.default_rng()
precision_type = np.float32
zero_vector = np.array([0, 0], dtype=precision_type)

def lerp(a:np.ndarray|float, b:np.ndarray|float, alpha:float):
    return a + alpha * (b - a)

def physicsVectorToPixelVector(physicsPos:np.ndarray, isPoint:bool = True):
    pixelPos = physicsPos * np.array([meterPerPixel, -meterPerPixel], dtype=precision_type)
    if isPoint:
        pixelPos += np.array([screenWidth*0.5, screenHeight*0.5], dtype=precision_type)

    return np.round(pixelPos).astype(int)

def physicsScalarToPixelScalar(physicsScalar:float):
    return physicsScalar * meterPerPixel

def getRandomColor():
    rand_rgb = np.random.randint(0, 256, size=3)
    return pygame.Color(*rand_rgb.tolist())

def getRandomDirection():
    rand_theta = rng.uniform(0, 2 * np.pi)
    return np.array([np.cos(rand_theta), np.sin(rand_theta)], dtype=precision_type)

###########################################################################

class PhyObject:
    def __init__(self, x:float, y:float):
        self.position = np.array([x, y], dtype=precision_type)
        self.lastPosition = None
        self.velocity = zero_vector.copy()
        self.acceleration = zero_vector.copy()
        self.forceAccum = zero_vector.copy()
        self.mass = 1.0
        self.damping = 1.0#0.65

    def simulate(self, dt:float):
        self.lastPosition = self.position.copy()

        newAcceleration = self.forceAccum / self.mass
        newVelocity = self.velocity + newAcceleration * dt
        newDampVelocity = newVelocity * np.pow(self.damping, dt)

        self.position += (newVelocity + newDampVelocity) * 0.5 * dt
        self.velocity = newDampVelocity
        self.acceleration = newAcceleration
        self.forceAccum[:] = 0.0

    def draw(self, surface:pygame.Surface):
        pass
    
    def isOutScreen(self):
        screenPos = physicsVectorToPixelVector(self.position)

        return screenPos[0] < 0 or screenPos[0] >= screenWidth or screenPos[1] < 0 or screenPos[1] >= screenHeight

class PhySphere(PhyObject):
    def __init__(self, x:float, y:float, radius:float, color:pygame.Color):
        super().__init__(x, y)

        self.color = color
        self.radius = radius
        self.mass = np.pi * radius * radius * 10.0

        self.status = 1

    def simulate(self, dt:float):
        if self.status != 1: return
        super().simulate(dt)

        #if self.isOutScreen(): 
        #    self.status = 0

    def draw(self, surface:pygame.Surface):
        if self.status != 1: return
        pixelPos = physicsVectorToPixelVector(self.position)
        pixelRadius = physicsScalarToPixelScalar(self.radius)

        pygame.draw.circle(surface, self.color, pixelPos, pixelRadius)

class PhyParticle(PhyObject):
    def __init__(self, x:float, y:float, radius:float, color:pygame.Color):
        super().__init__(x, y)

        self.color = color
        self.velocity = getRandomDirection() * rng.uniform(0.5, 2.0)
        self.radius = radius
        self.mass = np.pi * radius * radius * 10.0
        
        self.maxAge = 3.0
        self.age = self.maxAge
        self.status = 1 # 0-dead 1-alive 2-spawn

    def simulate(self, dt:float):
        if self.status != 1: return
        super().simulate(dt)
        self.age -= dt

        if self.isOutScreen(): 
            self.status = 0
        elif self.age <= 0.0: 
            self.status = 2 if rng.integers(0, 2) == 0 else 0

    def draw(self, surface:pygame.Surface):
        if self.status != 1: return
        pixelPos = physicsVectorToPixelVector(self.position)
        pixelRadius = physicsScalarToPixelScalar(self.radius)

        drawCol = pygame.Color(self.color)
        drawCol.a = int(255.0 * self.age / self.maxAge + 0.5)
        pygame.draw.circle(surface, drawCol.premul_alpha(), pixelPos, pixelRadius)

def remove_to_dead(list_alive:list[PhyParticle], list_dead:list[PhyParticle], idx:int):
    dead_par = list_alive[idx]
    if len(list_dead) < 200:
        list_dead.append(dead_par)
    else:
        del dead_par
    list_alive[idx] = list_alive[-1]
    list_alive.pop()

def create_to_alive(list_alive:list[PhyParticle], list_dead:list[PhyParticle], 
                    x:float, y:float, vx:float=0.0, vy:float=0.0):
    if len(list_alive) >= 200:
        return

    if len(list_dead) > 0:
        reuse_particle = list_dead.pop()
        reuse_particle.__init__(x, y, 0.02, getRandomColor())
        reuse_particle.velocity[:] += [vx,vy]
        list_alive.append(reuse_particle)
    else:
        new_particle = PhyParticle(x, y, 0.02, getRandomColor())
        new_particle.velocity[:] += [vx,vy]
        list_alive.append(new_particle)

class ForceGenerator:
    def __init__(self):
        pass

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
        self.k1 = 0.05
        self.k2 = 0.025

    def addForce(self, obj:PhyObject):
        force = obj.velocity.copy()
        veloLen = np.linalg.norm(force)
        force = force / max(veloLen, epcilon)

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
            posDir = obj.position / max(posLen, epcilon)
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
        force = force / max(dist, epcilon)
        obj.forceAccum += force * self.forceIntensity

class SpringForce(ForceGenerator):
    def __init__(self, endA:PhyObject|np.ndarray, endB:PhyObject|np.ndarray, restLen:float, springConst:float, isRope:bool=False):
        super().__init__()
        self.isRope = isRope
        self.endA:PhyObject|np.ndarray = endA
        self.endB:PhyObject|np.ndarray = endB
        self.color  = getRandomColor()
        self.restLength = restLen
        self.springConstant = springConst

        self.force = None

    @staticmethod
    def getEndPosition(end:PhyObject|np.ndarray):
        return end.position if isinstance(end, PhyObject) else end

    def simulate(self, dt:float):
        forceDir = SpringForce.getEndPosition(self.endB) - SpringForce.getEndPosition(self.endA)
        springLen = np.linalg.norm(forceDir)
        if springLen <= epcilon:
            forceDir = getRandomDirection()
        else:
            forceDir = forceDir / springLen

        deltaDist = (springLen - self.restLength)
        if self.isRope:
            deltaDist = max(0.0, deltaDist)
        self.force = deltaDist * -self.springConstant * forceDir

    def draw(self, surface):
        pixelPosA = physicsVectorToPixelVector(SpringForce.getEndPosition(self.endA))
        pixelPosB = physicsVectorToPixelVector(SpringForce.getEndPosition(self.endB))

        pygame.draw.line(surface, self.color, pixelPosA, pixelPosB, width=2)

    def addForce(self, obj:PhyObject):
        if isinstance(self.endA, PhyObject) and obj == self.endA:
            obj.forceAccum -= self.force
        elif isinstance(self.endB, PhyObject) and obj == self.endB:
            obj.forceAccum += self.force

def particle_main():
    # 1. Initialize & Setup
    pygame.init()
    screen = pygame.display.set_mode((screenWidth, screenHeight))
    clock = pygame.time.Clock()
    running = True
    frameCounter = 0

    particlesAlive:list[PhyParticle] = []
    particlesDead:list[PhyParticle] = []

    # init particles
    numOfSpawn = 10
    for idx in range(numOfSpawn):
        create_to_alive(particlesAlive, particlesDead, 0.0, 0.0)

    # init forces
    forces:list[ForceGenerator] = [GravityForce(), TestUpForce()]

    # 2. Main Game Loop
    while running:
        dt = clock.tick_busy_loop(240) * 0.001  # Delta time in seconds

        # quit
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                running = False
        if len(particlesAlive) <= 0:
            running = False

        # simulating
        for force in forces:
            force.simulate(dt)
        for par in particlesAlive:
            for force in forces:
                force.addForce(par)
            par.simulate(dt)       

        # spawn lifetime
        spawnParams = []
        for idx in range(len(particlesAlive) - 1, -1, -1):
            par = particlesAlive[idx]
            if par.status != 1:
                remove_to_dead(particlesAlive, particlesDead, idx)
                if par.status == 2:
                    spawnParams.append((par.position.copy(), par.velocity.copy()))
        
        for pos, velo in spawnParams:
            for idx in range(numOfSpawn):
                create_to_alive(particlesAlive, particlesDead, pos[0], pos[1], velo[0], velo[1])
        
        # rendering
        if frameCounter % 4 == 0:
            screen.fill("black")  # Clear screen
            for force in forces:
                force.draw(screen)
            for par in particlesAlive:
                par.draw(screen)
            pygame.display.flip() # Update the display
        frameCounter += 1

    pygame.quit()

def spring_main():
    # 1. Initialize & Setup
    pygame.init()
    screen = pygame.display.set_mode((screenWidth, screenHeight))
    clock = pygame.time.Clock()
    running = True
    frameCounter = 0

    objects:list[PhyObject] = []

    # init objects
    sizeX, sizeY = (5,5)
    for y in range(sizeY-1,-1,-1):
        for x in range(sizeX):
            objects.append(PhySphere(x-sizeX*0.5, y-sizeY*0.5, 0.1, getRandomColor()))

    # init forces
    forces:list[ForceGenerator] = [GravityForce(), DragForce()]
    
    fixPosA = np.array([-3,4], dtype=precision_type)
    fixPosB = np.array([3,4], dtype=precision_type)
    forces.append(SpringForce(fixPosA, objects[0], 1.0, 50.0, True))
    forces.append(SpringForce(fixPosB, objects[sizeX-1], 1.0, 50.0, True))
    for y in range(sizeY):
        for x in range(sizeX-1):
            objIdx = y * sizeX + x
            forces.append(SpringForce(objects[objIdx], objects[objIdx+1], 0.8, 20.0, True))
    for x in range(sizeX):
        for y in range(sizeY-1):
            objIdx = y * sizeX + x
            forces.append(SpringForce(objects[objIdx], objects[objIdx+sizeX], 0.8, 20.0, True))

    # 2. Main Game Loop
    while running:
        dt = clock.tick_busy_loop(240) * 0.001  # Delta time in seconds

        # quit
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                running = False
        if len(objects) <= 0:
            running = False

        # simulating
        for force in forces:
            force.simulate(dt)
        for obj in objects:
            for force in forces:
                force.addForce(obj)
            obj.simulate(dt)   

        # rendering
        if frameCounter % 4 == 0:
            screen.fill("black")  # Clear screen
            for force in forces:
                force.draw(screen)
            for obj in objects:
                obj.draw(screen)
            pygame.display.flip() # Update the display
        frameCounter += 1

    pygame.quit()

    
if __name__ == "__main__":
    spring_main()
    #particle_main()