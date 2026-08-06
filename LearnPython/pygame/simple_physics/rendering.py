import pygame
import numpy as np
from .common import makeRotationMatrix

def drawDottedLine(surface:pygame.Surface, color:pygame.Color, start:np.ndarray, end:np.ndarray, width:int = 1, segLen:float = 10.0):
    drawDir = end - start
    drawDist = np.linalg.norm(drawDir)
    drawDir /= drawDist

    curStart = start
    while(drawDist > 0.0):
        curEnd = curStart + drawDir * min(segLen, drawDist)
        pygame.draw.line(surface, color, curStart.tolist(), curEnd.tolist(), width)
        curStart = curEnd + drawDir * segLen
        drawDist -= segLen * 2.0

def drawShadowedLine(surface:pygame.Surface, color:pygame.Color, start:np.ndarray, end:np.ndarray, shadowDir:np.ndarray, width:int = 1):
    drawDir = end - start
    drawDist = np.linalg.norm(drawDir)
    drawDir /= drawDist

    rotMat = makeRotationMatrix(np.deg2rad(30.0))
    rottedShadowDir = rotMat @ shadowDir

    pygame.draw.line(surface, color, start.tolist(), end.tolist(), width=width)

    shadowWidth = max(width // 2, 1)
    for dist in np.arange(0.0, drawDist, 10.0):
        shadowStart = start + drawDir * dist
        shadowEnd = shadowStart + rottedShadowDir * 30.0
        pygame.draw.line(surface, color, shadowStart.tolist(), shadowEnd.tolist(), width=shadowWidth)

def drawGradientLine(surface:pygame.Surface, color0:pygame.Color, color1:pygame.Color, start:np.ndarray, end:np.ndarray, width:int = 1):
    gradient_surf = pygame.Surface((2, 1), pygame.SRCALPHA)
    gradient_surf.set_at((0, 0), color0)
    gradient_surf.set_at((1, 0), color1)

    drawDir = end - start
    drawDist = float(np.linalg.norm(drawDir))
    angle = np.rad2deg(np.arctan2(-drawDir[1], drawDir[0]))
    
    line_image = pygame.transform.smoothscale(gradient_surf, (drawDist, width))
    line_image = pygame.transform.rotate(line_image, angle)

    rect = line_image.get_rect()
    rect.center = ((start + end) * 0.5).tolist()

    surface.blit(line_image, rect.topleft)