import pygame
import numpy as np

################################# GlobalCommon #################################
screenWidth = 1024
screenHeight = 1024
meterPerPixel = 100.0
epsilon = 1e-6

rng = np.random.default_rng()
precision_type = np.float32
zero_vector = np.array([0, 0], dtype=precision_type)

def lerp(a:np.ndarray|float, b:np.ndarray|float, alpha:float):
    return a + alpha * (b - a)

def physicsVectorToPixelVector(physicsPos:np.ndarray, isPoint:bool = True, tolist:bool = True):
    pixelPos = physicsPos * np.array([meterPerPixel, -meterPerPixel], dtype=precision_type)
    if isPoint:
        pixelPos += np.array([screenWidth*0.5, screenHeight*0.5], dtype=precision_type)

    if tolist:
        return np.round(pixelPos).tolist()
    else:
        return np.round(pixelPos)

def physicsScalarToPixelScalar(physicsScalar:float|np.ndarray, tolist:bool = True):
    result = physicsScalar * meterPerPixel
    if tolist:
        return result.tolist() if isinstance(result, np.ndarray) else result
    else:
        return result

def getSceneSize():
    return (screenWidth / meterPerPixel, screenHeight / meterPerPixel)

def getRandomColor():
    rand_rgb = rng.integers(0, 256, size=3)
    return pygame.Color(*rand_rgb.tolist())

def getRandomDirection():
    rand_theta = rng.uniform(0, 2 * np.pi)
    return np.array([np.cos(rand_theta), np.sin(rand_theta)], dtype=precision_type)

def getRandomIntRange(min:int, max:int):
    return rng.integers(min, max)

def getRandomFloatRange(min:float, max:float):
    return rng.uniform(min, max)