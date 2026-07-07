import pygame
import numpy as np
from simple_physics import *

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
    spring_main()