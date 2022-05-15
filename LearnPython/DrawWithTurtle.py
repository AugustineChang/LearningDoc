import turtle
import math
from xml.dom import minidom

def GetTurtleDirection(t:turtle.Turtle):
    angle = t.heading()
    return (math.cos(angle), math.sin(angle))

def VectorAdd(a:tuple, b:tuple):
    return (a[0]+b[0], a[1]+b[1])

def VectorSub(a:tuple, b:tuple):
    return (a[0]-b[0], a[1]-b[1])

def VectorLen(v:tuple):
    return math.sqrt(v[0]*v[0] + v[1]*v[1])

def VectorNormalize(v:tuple, vecLen = -1):
    if vecLen <= 0:
        vecLen = VectorLen(v)
    vecLen = max(vecLen, 0.001)
    return (v[0]/vecLen, v[1]/vecLen)

def VectorDot(a:tuple, b:tuple):
    return a[0]*b[0] + a[1]*b[1]

def VectorCross(a:tuple, b:tuple):
    return a[0]*b[0] + a[1]*b[1]

def DrawLine(t:turtle.Turtle, target:tuple):
    curDir = GetTurtleDirection(t)
    tarDir = VectorSub(target, t.pos())
    tarDist = VectorLen(tarDir)
    tarDir = VectorNormalize(tarDir, tarDist)
    cosA = VectorDot(curDir, tarDir)
    rotAngle = math.acos(cosA)
    rotDir = curDir[0]*tarDir[1] - curDir[1]*tarDir[0]
    if rotDir > 0:
        t.left(rotAngle)
        #print("ture left", rotAngle)
    else:
        t.right(rotAngle)
        #print("ture right", rotAngle)
    t.forward(tarDist)
    #print("move", tarDist)

def GetPointOnCircle(angle:float, center:tuple=(0.0, 0.0), Radius:float=100.0):
    point = (math.cos(angle)*Radius, math.sin(angle)*Radius)
    return VectorAdd(point, center)

def DrawPolygon(t:turtle.Turtle, sides:int, center:tuple=(0.0, 0.0), Radius:float=100.0, rotAngle:float=0.0):
    if sides < 3:
        return
    
    t.penup()
    rotAngle = rotAngle / 180 * math.pi
    startPoint = GetPointOnCircle(rotAngle)
    t.goto(startPoint)
    t.pendown()
    
    step = 2 * math.pi / sides
    for n in range(1, sides+1):
        tarPoint = GetPointOnCircle(n*step + rotAngle)
        DrawLine(t, tarPoint)

def DrawSquare(t:turtle.Turtle):
    size = 100
    t.penup()
    t.backward(size/2)
    t.right(90)
    t.backward(size/2)
    t.pendown()
    t.left(90)
    
    for i in range(0,4):
        t.forward(size)
        t.right(90)


class DrawCommand():
    def __init__(self, commandObj):
        self.command = commandObj.firstChild.data.strip()
        
        attr = commandObj.attributes
        self.x = float(attr["x"].value) if "x" in attr else None
        self.y = float(attr["y"].value) if "y" in attr else None
        self.width = float(attr["width"].value) if "width" in attr else None
        self.color = attr["color"].value if "color" in attr else None
        self.radius = float(attr["radius"].value) if "radius" in attr else None
    
    def printCommand(self):
        print("x=%s y=%s width=%s color=%s radius=%s" % (self.x, self.y, self.width, self.color, self.radius))

    def doCommand(self, t:turtle.Turtle):
        if self.command == "PenUp":
            t.penup()
        elif self.command == "PenDown":
            t.pendown()
        elif self.command == "GoTo":
            t.width(self.width)
            t.color(self.color)
            t.goto(self.x, self.y)
        elif self.command == "BeginFill":
            t.fillcolor(self.color)
            t.begin_fill()
        elif self.command == "EndFill":
            t.end_fill()
        elif self.command == "Circle":
            t.width(self.width)
            t.pencolor(self.color)
            t.circle(self.radius)
        
def ReadFromXML(filename:str, t:turtle.Turtle):

    xmldoc = minidom.parse(filename)
    graphicCommands = xmldoc.getElementsByTagName("GraphicsCommands")[0]
    commandDOMs = graphicCommands.getElementsByTagName("Command")
    for dom in commandDOMs:
        cmdObj = DrawCommand(dom)
        cmdObj.printCommand()
        cmdObj.doCommand(t)
    

def main():
    t = turtle.Turtle()
    t.radians()
    screen = t.getscreen()
    #DrawPolygon(t, 3, (0,0), 100, 90)
    #DrawPolygon(t, 4, (0,0), 100, 45)
    #DrawPolygon(t, 5, (0,0), 100, 18)
    #DrawPolygon(t, 8, (0,0), 100)
    #DrawPolygon(t, 12, (0,0), 100)
    #DrawPolygon(t, 20, (0,0), 100)
    ReadFromXML("drawcommand.xml", t)
    screen.exitonclick()    

if __name__ == "__main__":
    main()