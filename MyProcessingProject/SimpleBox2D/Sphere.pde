class Sphere extends Shape
{
    float radius;
    ArrayList<Vec2> verts;
    
    Sphere(Box2DProcessing box2d, float posX, float posY, float r)
    {
      super(box2d, posX, posY);
      
      radius = r;
      verts = new ArrayList<Vec2>();
      
      int R = floor(random(256));
      int G = floor(random(256));
      int B = floor(random(256));
      filledCol = color(R, G, B);
      
      createShapeEnd(box2d);
    }
    
    void defineShape(Box2DProcessing box2d, PolygonShape ps)
    {
        int numOfVerts = 8;      
        for (int i = 0; i < numOfVerts; ++i)
        {
            verts.add(
              new Vec2(
                radius*cos(2.0*PI*i / float(numOfVerts)),
                radius*sin(-2.0*PI*i / float(numOfVerts))
              )
            );
        }
        
        Vec2 shapeVert[] = new Vec2[numOfVerts];
        for(int i = 0; i < numOfVerts; ++i)
        {
            shapeVert[i] = box2d.vectorPixelsToWorld(verts.get(i));
        }
        ps.set(shapeVert, shapeVert.length); //<>//
    }
    
    void drawShape()
    {
        beginShape();
        int numOfVerts = verts.size();
        for(int i = 0; i < numOfVerts; ++i)
        {
            Vec2 curVert = verts.get(i);
            vertex(curVert.x, curVert.y);
        }
        endShape(CLOSE);
    }
};
