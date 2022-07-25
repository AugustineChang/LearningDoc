class Piece extends Shape
{
    ArrayList<Vec2> verts;
    
    Piece(float posX, float posY)
    {
      super(posX, posY);
      
      verts = new ArrayList<Vec2>();
      
      int R = floor(random(256));
      int G = floor(random(256));
      int B = floor(random(256));
      filledCol = color(R, G, B);
      
      createShapeEnd();
    }
    
    void defineShape(PolygonShape ps)
    {
        int numOfVerts = 5;      
        verts.add(new Vec2(-12.0, 0.0));
        verts.add(new Vec2(0.0, 6.0));
        verts.add(new Vec2(4.26, 4.26));
        verts.add(new Vec2(6.0, 0.0));
        verts.add(new Vec2(0.0, -6.0));
        
        Vec2 shapeVert[] = new Vec2[numOfVerts];
        for(int i = 0; i < numOfVerts; ++i)
        {
            shapeVert[i] = box2d.vectorPixelsToWorld(verts.get(i));
        }
        ps.set(shapeVert, shapeVert.length);
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
