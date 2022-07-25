import shiffman.box2d.Box2DProcessing;

ArrayList<Shape> shapes;
Boundary bound;
WaveSurface surf;
Bridge bridge;

Box2DProcessing box2d;
MouseDragger mouseDrg;

int interactiveMode;//0-create 1-drag

void setup()
{
    interactiveMode = 0;
  
    size(800,600);
    box2d = new Box2DProcessing(this);
    box2d.createWorld();
    box2d.setGravity(0.0, -9.8f);
    
    mouseDrg = new MouseDragger();
    shapes = new ArrayList<Shape>();
    
    bound = new Boundary(width/2, height - 50, floor(width*0.8f), 16);
    surf = new WaveSurface(width/2, height/2);
    bridge = new Bridge();
}

void draw()
{
    background(208);
    
    box2d.step();
    
    if (interactiveMode == 0 && mousePressed) 
    {
        int type = 0;//floor(random(0,4));
        Shape p = null;
        switch(type)
        {
            case 0:
            p = new Box(mouseX, mouseY, 16, 16);
            break;
            
            case 1:
            p = new Sphere(mouseX, mouseY, 8);
            break;
            
            case 2:
            p = new Piece(mouseX, mouseY);
            break;
            
            case 3:
            p = new Spoon(mouseX, mouseY, 6, 24);
            break;
            
            default:
            println("type error!!");
        }
        shapes.add(p);
    }
    
    for (Shape b: shapes) 
    { 
        b.display();
    }
    bound.display();
    surf.display();
    bridge.display();
    mouseDrg.display();
    
    // remove dead particles
    int numOfBoxes = shapes.size();
    for (int i = numOfBoxes-1; i >= 0; --i)
    {
        Shape one = shapes.get(i);
        if (one.bIsDead)
        {
            shapes.remove(i);
        }
    }
    
    println("Num Of Shapes:%d", shapes.size());
}

void mousePressed()
{
    if (interactiveMode == 1)
        mouseDrg.createJoint(shapes);
}

void mouseReleased()
{
    if (interactiveMode == 1)
        mouseDrg.destroyJoint();
}

void keyPressed()
{
    if (key == '1')
        interactiveMode = 0;
    else if (key == '2')
        interactiveMode = 1;
}
