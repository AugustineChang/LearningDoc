import pygame
import numpy as np
from simple_physics import *

def init_spring_one(objects:list[PhyObject], forces:list[ForceGenerator]):
    fixPos = PhyObject.from_xy(0.0, 3.0)
    
    objects.append(PhySphere.from_xy(0.0, 2.0, 0.5, getRandomColor()))
    forces.append(SpringForce(fixPos, objects[0], 2, 50.0, None, None, 0.15))

def init_spring_three(objects:list[PhyObject], forces:list[ForceGenerator]):
    objects.append(PhySphere.from_xy(2.0, 0.0, 0.1, getRandomColor()))
    objects.append(PhySphere.from_xy(0.0, 2.0, 0.1, getRandomColor()))
    objects.append(PhySphere.from_xy(0.0, 0.0, 0.1, getRandomColor()))

    forces.append(SpringForce(objects[0], objects[1], 0.8, 1000.0, None, None, 0.15))
    forces.append(SpringForce(objects[1], objects[2], 0.8, 1000.0, None, None, 0.15))
    forces.append(SpringForce(objects[2], objects[0], 0.8, 1000.0, None, None, 0.15))

def init_spring_net(objects:list[PhyObject], forces:list[ForceGenerator]):
    # init objects
    sizeX, sizeY = (5,5)
    for y in range(sizeY-1,-1,-1):
        for x in range(sizeX):
            objects.append(PhySphere.from_xy(x-sizeX*0.5, y-sizeY*0.5, 0.1, getRandomColor()))

    # init forces
    fixPosA = PhyObject.from_xy(-3.0, 4.0)
    fixPosB = PhyObject.from_xy( 3.0, 4.0)
    forces.append(SpringForce(fixPosA, objects[0], 1.0, 50.0, None, None, 0.5))
    forces.append(SpringForce(fixPosB, objects[sizeX-1], 1.0, 50.0, None, None, 0.5))
    for y in range(sizeY):
        for x in range(sizeX-1):
            objIdx = y * sizeX + x
            forces.append(SpringForce(objects[objIdx], objects[objIdx+1], 0.8, 20.0, None, None, 0.5))
    for x in range(sizeX):
        for y in range(sizeY-1):
            objIdx = y * sizeX + x
            forces.append(SpringForce(objects[objIdx], objects[objIdx+sizeX], 0.8, 20.0, None, None, 0.5))

def init_collision(objects:list[PhyObject], forces:list[ForceGenerator]):
    forces.pop(0)

    objects.append(PhySphere.from_xy(0.0, 2.0, 0.2, getRandomColor()))
    objects.append(PhySphere.from_xy(2.0, 0.0, 0.1, getRandomColor()))
    
    sceneW, sceneH = getSceneSize()
    aboxThick = 0.5
    objects.append(PhyAlignedBox.from_xy( 0.0, (sceneH-aboxThick)*0.5, sceneW, aboxThick, getRandomColor()))
    objects.append(PhyAlignedBox.from_xy( 0.0,-(sceneH-aboxThick)*0.5, sceneW, aboxThick, getRandomColor()))
    objects.append(PhyAlignedBox.from_xy( (sceneW-aboxThick)*0.5, 0.0, aboxThick, sceneH - 2.0*aboxThick, getRandomColor()))
    objects.append(PhyAlignedBox.from_xy(-(sceneW-aboxThick)*0.5, 0.0, aboxThick, sceneH - 2.0*aboxThick, getRandomColor()))

    objects[0].velocity[1] = -2.0 
    objects[1].velocity[0] = -2.5 

def init_stack_spheres(objects:list[PhyObject], forces:list[ForceGenerator]):
    objects.append(PhySphere.from_xy(-0.4,-1.5, 0.2, getRandomColor()))
    objects.append(PhySphere.from_xy( 0.0,-1.5, 0.2, getRandomColor()))
    objects.append(PhySphere.from_xy( 0.4,-1.5, 0.2, getRandomColor()))
    
    objects.append(PhySphere.from_xy(-0.2,-1.0, 0.2, getRandomColor()))
    objects.append(PhySphere.from_xy( 0.2,-1.0, 0.2, getRandomColor()))

    objects.append(PhySphere.from_xy( 0.0,-0.5, 0.1, getRandomColor()))

    objects.append(PhyAlignedBox.from_xy( 0.0, -2.0, 6.0, 0.5, getRandomColor()))
    objects.append(PhyAlignedBox.from_xy( 0.90, -1.5, 0.5, 0.5, getRandomColor()))
    objects.append(PhyAlignedBox.from_xy(-0.90, -1.5, 0.5, 0.5, getRandomColor()))

def init_stack_spheres2(objects:list[PhyObject], forces:list[ForceGenerator]):
    objects.append(PhySphere.from_xy(0.0, 0.5, 0.2, getRandomColor()))
    objects.append(PhySphere.from_xy(0.0, 0.0, 0.2, getRandomColor()))
    objects.append(PhySphere.from_xy(0.0,-0.5, 0.2, getRandomColor()))
    objects.append(PhySphere.from_xy(0.0,-1.0, 0.2, getRandomColor()))
    objects.append(PhySphere.from_xy(0.0,-1.5, 0.2, getRandomColor()))

    objects.append(PhyAlignedBox.from_xy(0.0, -2.0, 6.0, 0.5, getRandomColor()))

def init_links(objects:list[PhyObject], forces:list[ForceGenerator], links:list[CollisionGenerator]):
    forces.pop()
    objects.append(PhyObject.from_xy(0.0, 3.0))
    objects.append(PhySphere.from_xy(0.0, 2.0, 0.5, getRandomColor()))
    objects.append(PhySphere.from_xy(2.0, 2.0, 0.3, getRandomColor()))
    objects.append(PhySphere.from_xy(2.0, 3.0, 0.2, getRandomColor()))
    links.append(Cable(objects[0], objects[1], 1.5, 0.5, getRandomColor()))
    links.append(Rod(objects[1], objects[2], 2.0, 0.5, getRandomColor()))
    links.append(Rod(objects[2], objects[3], 1.0, 0.5, getRandomColor()))

def init_bridge_demo(objects:list[PhyObject], forces:list[ForceGenerator], links:list[CollisionGenerator]):
    bridgeSeg = 16
    segStep = 0.3
    bridgeLen = bridgeSeg * segStep
    bridgeColor1 = getRandomColor()
    bridgeColor2 = getRandomColor()

    objects.append(PhyObject.from_xy(-bridgeLen*0.5, 0.0))
    for s in range(bridgeSeg-2):
        x_pos = (s+1)*segStep-bridgeLen*0.5
        objects.append(PhySphere.from_xy(x_pos, 0.0, 0.1, bridgeColor1))
    objects.append(PhyObject.from_xy(bridgeLen*0.5, 0.0))

    for s in range(bridgeSeg-1):
        links.append(Cable(objects[s], objects[s+1], segStep+0.05, 0.8, bridgeColor2))

    objects.append(PhySphere.from_xy(-bridgeLen*0.5+segStep, 1.0, 0.2, getRandomColor()))

def init_wheel_demo(objects:list[PhyObject], forces:list[ForceGenerator], links:list[CollisionGenerator]):
    wheelRadius = 1.0
    wheelSeg = 16
    wheelStep = np.deg2rad(360.0 / wheelSeg) 
    wheelColor1 = getRandomColor()
    wheelColor2 = getRandomColor()

    objects.append(PhyObject.from_xy(0.0, 0.0))
    for s in range(wheelSeg):
        posX = wheelRadius * np.cos(s*wheelStep)
        posY = wheelRadius * np.sin(s*wheelStep)
        objects.append(PhySphere.from_xy(posX, posY, 0.1, wheelColor1))

    for s in range(wheelSeg):
        links.append(Rod(objects[0], objects[s+1], None, 0.8, wheelColor2))
    for s in range(wheelSeg-1):
        links.append(Rod(objects[s+1], objects[s+2], None, 0.8, wheelColor2))
    links.append(Rod(objects[wheelSeg], objects[1], None, 0.8, wheelColor2))

    objects.append(PhySphere.from_xy(wheelRadius*0.7, 2.0, 0.2, getRandomColor()))

def main():
    # 1. Initialize & Setup
    pygame.init()
    screen = pygame.display.set_mode((screenWidth, screenHeight))

    objects:list[PhyObject] = []
    forces:list[ForceGenerator] = [GravityForce(), AeroForce(0.01)]
    links:list[CollisionGenerator] = []
    #init_spring_one(objects, forces)
    #init_spring_three(objects, forces)
    #init_spring_net(objects, forces)
    #init_collision(objects, forces)
    #init_stack_spheres(objects, forces)
    #init_stack_spheres2(objects, forces)
    init_links(objects, forces, links)
    #init_bridge_demo(objects, forces, links)
    #init_wheel_demo(objects, forces, links)

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
                force.applyForce(obj)
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
    main()