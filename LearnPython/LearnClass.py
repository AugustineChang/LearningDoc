import turtle
import math
import tkinter
import random

class Shape:
    def __init__(self, x=0, y=0, color="transparent", edgeWidth=1, edgeColor="black"):
        self.x = x
        self.y = y
        self.color = color
        self.edgeWidth = edgeWidth
        self.edgeColor = edgeColor
        self.NumOfsteps = 20
        
    def setPosition(self, x, y):
        self.x = x
        self.y = y
    
    def setColor(self, color):
        self.color = color
    
    def setEdge(self, width, color):
        self.edgeWidth = width
        self.edgeColor = color
    
    def draw(self, t:turtle.Turtle):
        pass

class Circle(Shape):
    def __init__(self, x=0, y=0, radius = 30, color="transparent", edgeWidth=1, edgeColor="black"):
        super().__init__(x, y, color, edgeWidth, edgeColor)
        self.radius = radius
    
    def draw(self, t:turtle.Turtle):
        t.penup()
        t.goto(self.x, self.y)
        t.width(self.edgeWidth)
        t.color(self.edgeColor)
        t.setheading(0)
        t.forward(self.radius)
        
        shouldFill = self.color != "transparent"
        if shouldFill:
            t.fillcolor(self.color)
            t.begin_fill()
        
        t.pendown()
        for step in range(self.NumOfsteps+1):
            angle = 2.0*math.pi * step / self.NumOfsteps;
            nextX = self.x + self.radius * math.cos(angle)
            nextY = self.y + self.radius * math.sin(angle)
            t.goto(nextX, nextY)
        t.penup()
        
        if shouldFill:
            t.end_fill()



class Rectangle(Shape):
    def __init__(self, x=0, y=0, width=100, height=50, color="transparent", edgeWidth=1, edgeColor="black"):
        super().__init__(x, y, color, edgeWidth, edgeColor)
        self.width = width
        self.height = height

    
    def draw(self, t:turtle.Turtle):
        t.penup()
        t.goto(self.x, self.y)
        t.width(self.edgeWidth)
        t.color(self.edgeColor)        
        
        halfW = self.width*0.5
        halfH = self.height*0.5
        t.goto(self.x-halfW, self.y-halfH)
        
        shouldFill = self.color != "transparent"
        if shouldFill:
            t.fillcolor(self.color)
            t.begin_fill()        
        
        t.pendown()
        t.goto(self.x+halfW, self.y-halfH)
        t.goto(self.x+halfW, self.y+halfH)
        t.goto(self.x-halfW, self.y+halfH)
        t.goto(self.x-halfW, self.y-halfH)
        t.penup()
        
        if shouldFill:
            t.end_fill()
        
    
def TestDraw():
    t = turtle.Turtle()
    t.radians()
    screen = t.getscreen()
    
    myCircle = Circle(10,10, 50, "red")
    myCircle.draw(t)
    
    myRect = Rectangle(-50, -60, 100, 50, "cyan")
    myRect.draw(t) 
    
    screen.exitonclick()
    
if __name__ == "__main__":
    root = tkinter.Tk()
    root.title("Let Turtle Draw!!!")
    root.geometry("800x600")
    root.resizable(False,False)    
    
    #DrawSpace
    canvas = tkinter.Canvas(root, width=600, height=600)
    canvas.pack(side="left")
    
    #Turtle
    t = turtle.RawTurtle(canvas)
    t.radians()     
    t.penup()
    
    #ControlSpace
    mainFrame = tkinter.Frame(root)
    mainFrame.pack(side="left")
    
    #Lable
    lable = tkinter.Label(mainFrame, text="Let the Turtle Draw!", width=28)
    lable.grid(row=1, column=1, columnspan=2)
    
    #Control Button
    def ClickBtn():        
        drawType = random.randint(0,1)
        drawPosX = random.random()*600.0 - 300.0
        drawPosY = random.random()*600.0 - 300.0
        drawCol = "#%02x%02x%02x" % (random.randint(0,255), random.randint(0,255), random.randint(0,255))
        
        if drawType == 0:
            drawRadius = 10.0+random.random()*90.0
            newCircle = Circle(drawPosX, drawPosY, drawRadius, drawCol)
            newCircle.draw(t)            
        elif drawType == 1:
            drawWidth = 10.0+random.random()*90.0
            drawHeight = 10.0+random.random()*90.0
            newRect = Rectangle(drawPosX, drawPosY, drawWidth, drawHeight, drawCol)
            newRect.draw(t)
    
    btn = tkinter.Button(mainFrame, text="Click To Draw", command=ClickBtn)
    btn.grid(row=2, column=1, columnspan=2) 
    
    root.mainloop()
    
    
    
    
    