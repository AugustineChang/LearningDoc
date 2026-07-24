import pygame
import numpy as np

################################# GlobalCommon #################################
screenWidth = 1024
screenHeight = 1024
meterPerPixel = 100.0
epsilon = 1e-6

rng = np.random.default_rng()
precision_type = np.float32

def makeArray(alist:list, dtype=precision_type):
    return np.array(alist, dtype=dtype)

zero_vector = makeArray([0, 0])
x_vector = makeArray([1, 0])
y_vector = makeArray([0, 1])
one_vector = makeArray([1, 1])

def lerp(a:np.ndarray|float, b:np.ndarray|float, alpha:float):
    return a + alpha * (b - a)

def cross(a:np.ndarray, b:np.ndarray) -> float:
    return a[0] * b[1] - a[1] * b[0]

def clip(v1, v2, plane_n:np.ndarray, plane_p:np.ndarray):
    d1 = np.dot(plane_n, v1-plane_p) 
    d2 = np.dot(plane_n, v2-plane_p)
    result = []
    if d1 >= 0: result.append(v1)
    if d2 >= 0: result.append(v2)
    if d1 * d2 < 0:# 一内一外,加交点
        t = d1 / (d1 - d2)
        result.append(lerp(v1,v2,t))
    return result

def localToWorld(vec:np.ndarray, rotMat:np.ndarray, translation:np.ndarray=None):
    return (rotMat @ vec) if translation is None else (translation + rotMat @ vec)

def worldToLocal(vec:np.ndarray, invRotMat:np.ndarray, translation:np.ndarray=None):
    return (invRotMat @ vec) if translation is None else (invRotMat @ (vec - translation))

def physicsVectorToPixelVector(physicsPos:np.ndarray, isPoint:bool = True, tolist:bool = True):
    pixelPos = physicsPos * makeArray([meterPerPixel, -meterPerPixel])
    if isPoint:
        pixelPos += makeArray([screenWidth*0.5, screenHeight*0.5])

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

def pixelVectorToPhysicsVector(pixelPosX:float|int, pixelPosY:float|int, isPoint:bool = True):
    physicsPos = makeArray([pixelPosX, pixelPosY])
    if isPoint:
        physicsPos -= makeArray([screenWidth*0.5, screenHeight*0.5])
    physicsPos /= makeArray([meterPerPixel, -meterPerPixel])
    
    return physicsPos

def pixelScalarToPhysicsScalar(pixelScalar:float|int):
    return pixelScalar / meterPerPixel

def getSceneSize():
    return (screenWidth / meterPerPixel, screenHeight / meterPerPixel)

def getRandomColor():
    rand_rgb = rng.integers(0, 256, size=3)
    return pygame.Color(*rand_rgb.tolist())

def getRandomDirection():
    rand_theta = rng.uniform(0, 2 * np.pi)
    return makeArray([np.cos(rand_theta), np.sin(rand_theta)])

def getRandomIntRange(min:int, max:int):
    return rng.integers(min, max)

def getRandomFloatRange(min:float, max:float):
    return rng.uniform(min, max)

def arcAreaAndL(d:float, r:float):
        r2 = r*r
        half_l = np.sqrt(r2-d*d)
        return r2 * np.arccos(d/r) - d * half_l, half_l * 2.0

def polygonAreaAndCenter(vertices:np.ndarray):
    """
    计算按顺序排列的多边形的面积和重心(Centroid)
    vertices: 形如 [[x1,y1], [x2,y2], ...] 的 NumPy 数组或列表
    """
    n = len(vertices)
    
    if n < 3:
        return 0.0, (0.0, 0.0)
        
    # 巧妙利用 np.roll 将顶点错开一位，实现高效的错位相乘
    x = vertices[:, 0]
    y = vertices[:, 1]
    next_x = np.roll(x, -1)
    next_y = np.roll(y, -1)
    
    # 计算每一项的交叉乘积 (xi * yi+1 - xi+1 * yi)
    cross_product = x * next_y - next_x * y
    
    # 1. 计算总面积 (Shoelace Formula)
    area = 0.5 * np.sum(cross_product)
    area_abs = np.abs(area)

    if area_abs < 1e-6: # 防止除以 0
        return 0.0, makeArray([np.mean(x), np.mean(y)])
        
    # 2. 计算重心坐标 Cx, Cy
    cx = np.sum((x + next_x) * cross_product) / (6.0 * area)
    cy = np.sum((y + next_y) * cross_product) / (6.0 * area)
    
    return area_abs, makeArray([cx, cy])