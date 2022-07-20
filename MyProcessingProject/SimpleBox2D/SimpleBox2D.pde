import shiffman.box2d.Box2DProcessing;

ArrayList<Shape> shapes;
Boundary bound;
WaveSurface surf;

Box2DProcessing box2d;

void setup()
{
    size(800,600);
    box2d = new Box2DProcessing(this);
    box2d.createWorld();
    box2d.setGravity(0.0, -9.8f);
     
    shapes = new ArrayList<Shape>();
    bound = new Boundary(box2d, width/2, height - 50, floor(width*0.8f), 16);
    
    surf = new WaveSurface(box2d, width/2, height/2);
}

void draw()
{
    background(208);
    
    box2d.step();
    
    if (mousePressed) 
    {
        int type = floor(random(0,4));
        Shape p = null;
        switch(type)
        {
            case 0:
            p = new Box(box2d, mouseX, mouseY, 16, 16);
            break;
            
            case 1:
            p = new Sphere(box2d, mouseX, mouseY, 8);
            break;
            
            case 2:
            p = new Piece(box2d, mouseX, mouseY);
            break;
            
            case 3:
            p = new Spoon(box2d, mouseX, mouseY, 6, 24);
            break;
            
            default:
            println("type error!!");
        }
        shapes.add(p);
    }
    
    for (Shape b: shapes) 
    { 
        b.display(box2d);
    }
    bound.display();
    surf.display();
    
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
