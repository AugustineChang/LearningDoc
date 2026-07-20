import pygame
import numpy as np
from simple_physics import *

def init_demo(objects:list[PhyObject], forces:list[ForceGenerator], links:list[CollisionGenerator]):
    objects.append(PhyObject.from_xy(0.0, 2.0))
    objects.append(PhyBox.from_xy(1.0, 0.0, 1.0, 0.5, getRandomColor()))

    offset = makeArray([0.5, 0.25])
    forces.append(SpringForce(objects[0], objects[1], 3.0, 100.0, None, offset, 0.1, True))

def init_demo2(objects:list[PhyObject], forces:list[ForceGenerator], links:list[CollisionGenerator]):
    objects.append(PhySphere.from_xy(3.0, 0.0, 0.7, getRandomColor()))
    
    objects.append(PhyBoat.from_xy(4.0, 0.0, 1.5, 0.5, getRandomColor()))
    #objects.append(PhySail.from_xy(4.0, 0.85, 1.0, getRandomColor()))
    #objects[2].orientation = np.pi * 0.5

    offset1 = makeArray([0.3, 0.25])
    offset2 = makeArray([-0.3, 0.25])
    #forces.append(SpringForce(objects[2], objects[1], 0.7, 300.0, None, offset1, 0.1))
    #forces.append(SpringForce(objects[2], objects[1], 0.7, 300.0, None, offset2, 0.1))

    forces.append(BuoyancyForce(30.0, -4.0))
    forces.append(AeroForce(0.1, makeArray([-2.0, 0.0])))

def main():
    # 1. Initialize & Setup
    pygame.init()
    screen = pygame.display.set_mode((screenWidth, screenHeight))

    objects:list[PhyObject] = []
    forces:list[ForceGenerator] = [GravityForce()]
    links:list[CollisionGenerator] = []
    init_demo2(objects, forces, links)

    usrCtrl = UserController(objects)
    usrCtrl.registerPawn(objects[1])

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