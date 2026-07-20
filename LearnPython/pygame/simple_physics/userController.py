import pygame
import numpy as np
from .objects import PhyObject, PhySphere
from .particleSys import PartcleSystem
from .common import epsilon, getRandomColor, getRandomDirection, getRandomFloatRange, pixelVectorToPhysicsVector

################################# User Control #################################

class UserController:
    def __init__(self, objects:list[PhyObject] = None, particleSys:PartcleSystem = None):
        self.status = 1 # 0-stop 1-running
        self.clock = pygame.time.Clock()
        self.framerate = 240
        self.maxTimestep = 5.0 / self.framerate
        self.frameCounter = 0
        
        self.allObjects = objects
        self.particleSystem = particleSys
        self.pawn:PhyObject = None

        self.leftClickObj = None
        self.leftDownPos = None
        self.leftClickPos = None
        self.leftClickStart = -1.0
        self.rightClickStart = -1.0

    def preTick(self) -> float:
        dt = self.clock.tick_busy_loop(self.framerate) * 0.001  # Delta time in seconds
        dt = min(self.maxTimestep, dt)

        if self.leftClickStart >= 0.0:
            self.leftClickStart += dt
        if self.rightClickStart >= 0.0:
            self.rightClickStart += dt

        return dt

    def eventReceiver(self):
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                self.quit()
            elif event.type == pygame.MOUSEBUTTONDOWN:
                if event.button == 1: self.leftMouseDown(event.pos)
                elif event.button == 3: self.rightMouseDown()
            elif event.type == pygame.MOUSEBUTTONUP:
                if event.button == 1: self.leftMouseUp(event.pos)
                elif event.button == 3: self.rightMouseUp(event.pos)
            elif event.type == pygame.KEYDOWN:
                self.keyDown(event.key)
            elif event.type == pygame.KEYUP:
                self.keyUp(event.key)

        isNoObjects = self.allObjects is None or len(self.allObjects) <= 0
        isParticlesStop = self.particleSystem is None or not self.particleSystem.isRunning()
        if isNoObjects and isParticlesStop:
            self.quit()

    def postTick(self):
        self.frameCounter += 1

    def registerPawn(self, newPawn:PhyObject):
        self.pawn = newPawn
    def unregisterPawn(self):
        self.pawn = None

    def keyDown(self, keyType:int):
        if self.pawn is None: return
        self.pawn.keyDown(keyType)

    def keyUp(self, keyType:int):
        if self.pawn is None: return
        self.pawn.keyUp(keyType)

    ###########################################################################################

    def leftMouseDown(self, mousePos):
        if self.allObjects is None: return

        downPos = pixelVectorToPhysicsVector(mousePos[0], mousePos[1])
        for obj in self.allObjects:
            if obj.objType == 'sphere' and obj.status == 1:
                obj2Click = downPos - obj.position
                if np.dot(obj2Click, obj2Click) <= obj.radius * obj.radius:
                    self.leftDownPos = downPos
                    self.leftClickPos = obj.worldToLocal(downPos)
                    self.leftClickObj = obj
            elif obj.objType == 'box' and obj.status == 1:
                obj2Click = obj.worldToLocal(downPos)
                if (np.abs(obj2Click) <= obj.size * 0.5).all():
                    self.leftDownPos = downPos
                    self.leftClickPos = obj2Click
                    self.leftClickObj = obj

        self.leftClickStart = 0.0

    def leftMouseUp(self, mousePos):
        if self.allObjects is None: return

        if self.leftDownPos is not None and self.leftClickPos is not None and self.leftClickObj is not None:
            upPos = pixelVectorToPhysicsVector(mousePos[0], mousePos[1])
            clickDir = upPos - self.leftDownPos
            clickLen = np.linalg.norm(clickDir)
            if clickLen <= epsilon:
                clickDir = getRandomDirection()
            else:
                clickDir /= clickLen

            strength = (20.0 + self.leftClickStart * 80.0) * self.leftClickObj.mass
            force = clickDir * strength
            forcePos = self.leftClickObj.localToWorld(self.leftClickPos)
            self.leftClickObj.addForce(force, forcePos)
            #print(f"add force({strength:0.4f}) to obj({self.leftClickObj.mass}, {self.leftClickObj.inertia})!")

        self.leftDownPos = None
        self.leftClickPos = None
        self.leftClickObj = None
        self.leftClickStart = -1.0

    def rightMouseDown(self):
        if self.allObjects is None: return

        self.rightClickStart = 0.0

    def rightMouseUp(self, mousePos):
        if self.allObjects is None: return

        pos = pixelVectorToPhysicsVector(mousePos[0], mousePos[1])
        radius = 0.1 + self.rightClickStart * 0.1
        newObj = PhySphere(pos, radius, getRandomColor())
        print(f"create sphere({pos},{radius})!")
        newObj.velocity = getRandomDirection() * getRandomFloatRange(0.5, 2.0)
        self.allObjects.append(newObj)

        self.rightClickStart = -1.0

    def isRunning(self):
        return self.status == 1

    def shouldRender(self):
        return self.frameCounter % 4 == 0

    def quit(self):
        self.status = 0