import pygame
from .objects import PhyObject, PhySphere
from .particleSys import PartcleSystem
from .common import getRandomColor, getRandomDirection, getRandomFloatRange, getRandomIntRange

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
        randIdx = getRandomIntRange(0, numOfObj)
        randObj = validObjs[randIdx]
        randObj.velocity += getRandomDirection() * getRandomFloatRange(5.0, 10.0)
    
    def rightClick(self):
        if self.allObjects is None: return

        newObj = PhySphere(2.0, 0.0, 0.1, getRandomColor())
        newObj.velocity = getRandomDirection() * getRandomFloatRange(0.5, 2.0)
        self.allObjects.append(newObj)

    def isRunning(self):
        return self.status == 1

    def shouldRender(self):
        return self.frameCounter % 4 == 0

    def quit(self):
        self.status = 0