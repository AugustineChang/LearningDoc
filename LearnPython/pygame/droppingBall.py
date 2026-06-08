from __future__ import annotations
import pygame
import numpy as np

class PhyCommon:
    def __init__(self):
        self.gravity = np.array([0.0, -9.8], dtype=np.float32)
        self.wind = np.array([0.2, 0.0], dtype=np.float32)
        self.airResistanceCoefficient = 0.3

class DebugPhyCommon(PhyCommon):
    def __init__(self):
        super().__init__()
        self.airResistanceCoefficient = 0.0

class HitInfo:
    def __init__(self, isHit=False, point=None, normal=None, time=0.0):
        self.isHit = isHit
        self.time = time
        self.point = point if point is not None else np.array([0.0, 0.0])
        self.normal = normal if normal is not None else np.array([0.0, 1.0])

class PhyObject:
    def __init__(self, x, y, color, phyParams: PhyCommon):
        self.position = np.array([x, y], dtype=np.float32)
        self.last_position = self.position.copy()
        self.time_remaining = 0.0

        zero_vector = np.array([0, 0], dtype=np.float32)
        self.velocity = zero_vector.copy()
        self.acceleration = zero_vector.copy()
        self.mass = 1.0
        self.color = color
        self.phyParams = phyParams

    def physicsVectorToPixelVector(self, physicsPos, isPoint = True):
        global screenWidth, screenHeight, meterPerPixel
        
        pixelPos = physicsPos * np.array([meterPerPixel, -meterPerPixel], dtype=np.float32)
        if isPoint:
            pixelPos += np.array([screenWidth*0.5, screenHeight], dtype=np.float32)

        return np.round(pixelPos).astype(int)

    def physicsScalarToPixelScalar(self, physicsScalar):
        global meterPerPixel

        return physicsScalar * meterPerPixel

    def lerp(self, a:np.ndarray, b:np.ndarray, alpha:float):
        return a + alpha * (b - a)

    def simulate(self, dt):
        self.last_position = self.position.copy()

    def collisionDetection(self, other:PhyObject, dt:float) -> HitInfo:
        return HitInfo()

    def collisionResponse(self, hit:HitInfo, dt:float):
        pass

    def draw(self, surface):
        pass

class PhyBall(PhyObject):
    def __init__(self, x, y, radius, color, phyParams: PhyCommon):
        super().__init__(x, y, color, phyParams)

        self.velocity = np.array([3.0, 14.0], dtype=np.float32)
        self.acceleration = np.array([0.0, 0.0], dtype=np.float32)
        self.radius = radius
        self.mass = np.pi * radius * radius * 10.0

        self.elasticity = 0.9
        self.friction = 0.05

    def simulate(self, dt):
        timsstep = dt + self.time_remaining
        self.time_remaining = 0.0
        super().simulate(timsstep)

        newAcceleration = self.phyParams.gravity.copy()
        newAcceleration += (self.phyParams.wind - self.velocity) / self.mass * self.phyParams.airResistanceCoefficient

        newVelocity = self.velocity + (self.acceleration + newAcceleration) * 0.5 * timsstep
        self.position += (self.velocity + newVelocity) * 0.5 * timsstep
        self.velocity = newVelocity
        self.acceleration = newAcceleration

    def collisionResponse(self, hit:HitInfo, dt:float):
        velocity_normal = np.dot(self.velocity, hit.normal) * hit.normal
        velocity_tangent = self.velocity - velocity_normal

        len_velo_normal = np.linalg.norm(velocity_normal)
        len_velo_tangent = np.linalg.norm(velocity_tangent)

        velocity_normal *= -self.elasticity
        if len_velo_tangent > epcilon:
            velocity_tangent = velocity_tangent - min(self.friction * len_velo_normal, len_velo_tangent) / len_velo_tangent * velocity_tangent
    
        self.velocity = velocity_normal + velocity_tangent
        self.time_remaining = dt - hit.time

    def draw(self, surface):
        pixelPos = self.physicsVectorToPixelVector(self.position)
        pixelRadius = self.physicsScalarToPixelScalar(self.radius)
        pygame.draw.circle(surface, self.color, pixelPos, pixelRadius)

    def isOutScreen(self):
        global screenWidth, meterPerPixel
        maxWidth = screenWidth / meterPerPixel * 0.5

        return self.position[0] < -maxWidth or self.position[0] > maxWidth

class PhyPlane(PhyObject):
    def __init__(self, cx, cy, nx, ny, color, phyParams: PhyCommon):
        super().__init__(cx, cy, color, phyParams)
        self.normal = np.array([nx, ny], dtype=np.float32)
        self.normal /= np.linalg.norm(self.normal)

    def pointToPlaneDistance(self, point):
        return np.dot(point - self.position, self.normal)

    def collisionDetection(self, other:PhyObject, dt:float) -> HitInfo:
        last_distance = self.pointToPlaneDistance(other.last_position)
        cur_distance = self.pointToPlaneDistance(other.position)
        
        isHit = last_distance >= other.radius and cur_distance < other.radius
        if isHit:
            alpha = (last_distance - other.radius) / (last_distance - cur_distance)
            point = self.lerp(other.last_position, other.position, alpha) - self.normal * other.radius
            return HitInfo(True, point, self.normal, dt*alpha)
        else:
            return HitInfo()

    def draw(self, surface):
        plane_dir = np.array([-self.normal[1], self.normal[0]], dtype=np.float32)

        startPos = self.physicsVectorToPixelVector(self.position - plane_dir * 10)
        endPos = self.physicsVectorToPixelVector(self.position + plane_dir * 10)

        pygame.draw.line(surface, self.color, startPos, endPos, 4)
    

screenWidth = 1024
screenHeight = 1024
meterPerPixel = 100.0
epcilon = 1e-6

def main():
    # 1. Initialize & Setup
    pygame.init()
    screen = pygame.display.set_mode((screenWidth, screenHeight))
    clock = pygame.time.Clock()
    running = True
    frameCounter = 0

    commonParams = PhyCommon()
    DebugParams = DebugPhyCommon()

    #ball1 = PhyBall(-5.0, 0.0, 0.2, "red", DebugParams)
    ball2 = PhyBall(-5.0, 0.0, 0.2, "green", commonParams)
    plane = PhyPlane(0.0, 0.04, 0.0, 1.0, "white", commonParams)

    # 2. Main Game Loop
    while running:
        dt = clock.tick(480) * 0.001  # Delta time in seconds
        #actual_fps = clock.get_fps()
        #print(f"Current FPS: {actual_fps:.2f}")

        # A. Event Handling (Input)
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                running = False
        if ball2.isOutScreen():
            running = False

        # B. Logic/Physics (Update)
        #ball1.simulate(dt)
        ball2.simulate(dt)

        # C. Collision Detection & Response
        #hitInfo1 = plane.collisionDetection(ball1, dt)
        #if hitInfo1.isHit:
        #    ball1.collisionResponse(hitInfo1, dt)
        hitInfo2 = plane.collisionDetection(ball2, dt)
        if hitInfo2.isHit:
            ball2.collisionResponse(hitInfo2, dt)

        # D. Rendering (Draw)
        if frameCounter % 4 == 0:
            screen.fill("black")  # Clear screen
            #ball1.draw(screen)  # Draw the ball
            ball2.draw(screen)  # Draw the ball
            plane.draw(screen)  # Draw the plane
            pygame.display.flip() # Update the display

        frameCounter += 1

    pygame.quit()

if __name__ == "__main__":
    main()