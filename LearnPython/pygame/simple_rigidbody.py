import pygame
import numpy as np
from simple_physics import *

def init_demo(objects:list[PhyObject], forces:list[ForceGenerator], links:list[CollisionGenerator]):
    objects.append(PhyObject.from_xy(0.0, 2.0))
    objects.append(PhyBox.from_xy(1.0, 0.0, 1.0, 0.5, getRandomColor()))

    objects.append(PhyPlane.from_xy(0.0,-4.0, 0.0, 1.0, getRandomColor()))

    offset = makeArray([0.5, 0.25])
    forces.append(SpringForce(objects[0], objects[1], 3.0, 100.0, None, offset, 0.1, False))
    forces.append(GravityForce())

def init_demo2(objects:list[PhyObject], forces:list[ForceGenerator], links:list[CollisionGenerator]):
    objects.append(PhySphere.from_xy(5.0, 0.0, 0.7, getRandomColor()))
    
    objects.append(PhyBoat.from_xy(4.0, 0.0, 1.5, 0.5, getRandomColor()))
    #objects.append(PhySail.from_xy(4.0, 0.85, 1.0, getRandomColor()))
    #objects[2].orientation = np.pi * 0.5

    offset1 = makeArray([0.3, 0.25])
    offset2 = makeArray([-0.3, 0.25])
    #forces.append(SpringForce(objects[2], objects[1], 0.7, 300.0, None, offset1, 0.1))
    #forces.append(SpringForce(objects[2], objects[1], 0.7, 300.0, None, offset2, 0.1))

    forces.append(GravityForce())
    forces.append(BuoyancyForce(30.0, -4.0))
    forces.append(AeroForce(0.1, makeArray([-2.0, 0.0])))

def init_demo3(objects:list[PhyObject], forces:list[ForceGenerator], links:list[CollisionGenerator]):
    objects.append(PhyStaticBox.from_xy(0.0, -4.0, 8.0, 0.5, getRandomColor()))
    objects.append(PhyBox.from_xy(1.0, 0.0, 1.0, 0.5, getRandomColor()))
    objects.append(PhyBox.from_xy(-1.0, 0.0, 0.5, 1.0, getRandomColor()))
    objects[1].angularVelocity = np.pi * 0.1666

    forces.append(GravityForce())
    forces.append(AeroForce(0.1))

def init_demo4(objects:list[PhyObject], forces:list[ForceGenerator], links:list[CollisionGenerator]):

    obj1 = PhyBox.from_xy(0.25, 0.0, 1.0, 0.5, getRandomColor())
    obj2 = PhyBox.from_xy(-0.5, 0.0, 1.0, 0.5, getRandomColor())
    #obj2 = PhySphere.from_xy(-0.5, 0.0, 0.5, getRandomColor())
    obj1.orientation = np.pi * 0.5

    objects.append(PhyCombinedObj.from_xy(0.0, 0.0, [obj1, obj2]))
    #objects.append(PhySphere.from_xy(0.0, 0.0, 0.5, getRandomColor()))
    objects[0].angularVelocity = np.pi * 0.1666

    objects.append(PhyStaticBox.from_xy(-1.0, -3.0, 4.0, 0.5, getRandomColor()))

    forces.append(GravityForce())
    forces.append(BuoyancyForce(30.0, -4.0))
    forces.append(AeroForce(0.1))

def init_demo5(objects:list[PhyObject], forces:list[ForceGenerator], links:list[CollisionGenerator]):
    objects.append(PhyPlane.from_xy(0.0, -4.0, 0.0, getRandomColor()))

    objects.append(PhyObject.from_xy(0.0, 4.0))
    objects.append(PhySphere.from_xy(0.0, 5.0, 0.2, getRandomColor()))
    objects.append(PhyRegularPolygon(1.0, 5.0, 6, 0.2, getRandomColor()))
    links.append(PhyRod(objects[1], objects[2], 1.0, getRandomColor(), None, None))
    links.append(PhyRod(objects[2], objects[3], 1.0, getRandomColor(), None, makeArray([-0.15, 0.0])))


    objects.append(PhyRegularPolygon(-1.0, 0.0, 3, 1.0, getRandomColor()))
    objects.append(PhyRegularPolygon(1.0, 0.0, 3, 1.0, getRandomColor()))
    objects[4].orientation = np.pi * -0.16667
    objects[5].orientation = np.pi * -0.16667
    objects[4].setMass(0.0, 0.0)
    objects[5].setMass(0.0, 0.0)
    
    forces.append(GravityForce())
    #forces.append(AeroForce(0.1))

def main():
    # 1. Initialize & Setup
    pygame.init()
    screen = pygame.display.set_mode((screenWidth, screenHeight))

    objects:list[PhyObject] = []
    forces:list[ForceGenerator] = []
    links:list[CollisionGenerator] = []
    init_demo5(objects, forces, links)

    usrCtrl = UserController(objects)
    #usrCtrl.registerPawn(objects[1])

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