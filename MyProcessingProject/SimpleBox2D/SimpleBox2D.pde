import shiffman.box2d.Box2DProcessing;

ArrayList<Box> boxes;
Boundary bound;

Box2DProcessing box2d;

void setup()
{
    size(800,600);
    box2d = new Box2DProcessing(this);
    box2d.createWorld();
    box2d.setGravity(0.0, -9.8f);
     
    boxes = new ArrayList<Box>();
    bound = new Boundary(box2d, width/2, height - 50, floor(width*0.8f), 16);
}


void draw()
{
    background(208);
    
    box2d.step();
    
    if (mousePressed) 
    {
        Box p = new Box(box2d, mouseX, mouseY, 16, 16);
        boxes.add(p);
    }
    
    
    for (Box b: boxes) 
    { 
        b.display(box2d);
    }
    bound.display();
    
    // remove dead particles
    int numOfBoxes = boxes.size();
    for (int i = numOfBoxes-1; i >= 0; --i)
    {
        Box one = boxes.get(i);
        if (one.bIsDead)
        {
            boxes.remove(i);
        }
    }
    
    println("Num Of Boxes:%d", boxes.size());
}
