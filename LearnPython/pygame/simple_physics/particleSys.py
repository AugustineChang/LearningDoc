import pygame
from .objects import PhyParticle
from .forces import ForceGenerator
from .common import getRandomColor

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