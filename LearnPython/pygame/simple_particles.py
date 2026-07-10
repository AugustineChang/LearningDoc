import pygame
import numpy as np
from simple_physics import *

def particle_main():
    # 1. Initialize & Setup
    pygame.init()
    screen = pygame.display.set_mode((screenWidth, screenHeight))

    objects:list[PhyObject] = [PhyAlignedBox(0.0, -4.0, 6.0, 0.5, getRandomColor())]
    forces:list[ForceGenerator] = [GravityForce()]
    particleSys = PartcleSystem()
    usrCtrl = UserController(None, particleSys)

    # 2. Main Game Loop
    while usrCtrl.isRunning():
        dt = usrCtrl.preTick()

        # controller
        usrCtrl.eventReceiver()

        # simulating
        for force in forces:
            force.simulate(dt)
        particleSys.simulate(dt, forces)
        collision_detect_all(dt, objects, particleSys.listAlive)
        
        # rendering
        if usrCtrl.shouldRender():
            screen.fill("black")  # Clear screen
            for force in forces:
                force.draw(screen)
            for obj in objects:
                obj.draw(screen)
            particleSys.draw(screen)
            pygame.display.flip() # Update the display
        usrCtrl.postTick()

    pygame.quit()
    
if __name__ == "__main__":
    particle_main()