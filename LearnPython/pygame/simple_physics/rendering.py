import pygame
import numpy as np

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