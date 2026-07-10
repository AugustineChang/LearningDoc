import pygame
import numpy as np
from simple_physics import *

def init_spring_one(objects:list[PhyObject], forces:list[ForceGenerator]):
    fixPos = PhyObject(0.0, 3.0)
    
    objects.append(PhySphere(0.0, 2.0, 0.5, getRandomColor()))
    forces.append(SpringForce(fixPos, objects[0], 0.8, 3.0, 0.08))

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
    fixPosA = PhyObject(-3.0, 4.0)
    fixPosB = PhyObject( 3.0, 4.0)
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
    forces.pop(0)

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

def init_stack_spheres(objects:list[PhyObject], forces:list[ForceGenerator]):
    objects.append(PhySphere(-0.4,-1.5, 0.2, getRandomColor()))
    objects.append(PhySphere( 0.0,-1.5, 0.2, getRandomColor()))
    objects.append(PhySphere( 0.4,-1.5, 0.2, getRandomColor()))
    
    objects.append(PhySphere(-0.2,-1.0, 0.2, getRandomColor()))
    objects.append(PhySphere( 0.2,-1.0, 0.2, getRandomColor()))

    objects.append(PhySphere( 0.0,-0.5, 0.1, getRandomColor()))

    objects.append(PhyAlignedBox( 0.0, -2.0, 6.0, 0.5, getRandomColor()))
    objects.append(PhyAlignedBox( 0.90, -1.5, 0.5, 0.5, getRandomColor()))
    objects.append(PhyAlignedBox(-0.90, -1.5, 0.5, 0.5, getRandomColor()))

def init_stack_spheres2(objects:list[PhyObject], forces:list[ForceGenerator]):
    objects.append(PhySphere(0.0, 0.5, 0.2, getRandomColor()))
    objects.append(PhySphere(0.0, 0.0, 0.2, getRandomColor()))
    objects.append(PhySphere(0.0,-0.5, 0.2, getRandomColor()))
    objects.append(PhySphere(0.0,-1.0, 0.2, getRandomColor()))
    objects.append(PhySphere(0.0,-1.5, 0.2, getRandomColor()))

    objects.append(PhyAlignedBox( 0.0, -2.0, 6.0, 0.5, getRandomColor()))

def init_links(objects:list[PhyObject], links:list[CollisionGenerator]):
    objects.append(PhyObject(0.0, 3.0))
    objects.append(PhySphere(0.0, 2.0, 0.5, getRandomColor()))
    objects.append(PhySphere(1.0, 2.0, 0.3, getRandomColor()))
    links.append(Cable(objects[0], objects[1], 2.0, 0.5))
    links.append(Rod(objects[1], objects[2], 1.0, 0.5))

def spring_main():
    # 1. Initialize & Setup
    pygame.init()
    screen = pygame.display.set_mode((screenWidth, screenHeight))

    objects:list[PhyObject] = []
    forces:list[ForceGenerator] = [GravityForce(), DragForce()]
    links:list[CollisionGenerator] = []
    #init_spring_one(objects, forces)
    #init_spring_three(objects, forces)
    #init_spring_net(objects, forces)
    #init_collision(objects, forces)
    #init_stack_spheres(objects, forces)
    #init_stack_spheres2(objects, forces)
    init_links(objects, links)

    usrCtrl = UserController(objects)

    # 2. Main Game Loop
    while usrCtrl.isRunning():
        dt = usrCtrl.preTick()

        # controller
        usrCtrl.eventReceiver()

        # simulating
        for force in forces:
            force.simulate(dt)
        for obj in objects:
            for force in forces:
                force.addForce(obj)
            obj.simulate(dt)   
        collision_detect_all(dt, objects, None, links)

        # rendering
        if usrCtrl.shouldRender():
            screen.fill("black")  # Clear screen
            for force in forces:
                force.draw(screen)
            for lk in links:
                lk.draw(screen)
            for obj in objects:
                obj.draw(screen)
            pygame.display.flip() # Update the display
        usrCtrl.postTick()

    pygame.quit()

if __name__ == "__main__":
    spring_main()