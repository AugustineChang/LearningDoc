class Boundary
{
    int boundWidth;
    int boundHeight;
    Vec2 boundPos;
    
    Body boundBody;
    
    Boundary(Box2DProcessing box2d, float posX, float posY, int w, int h)
    {
        boundWidth = w;
        boundHeight = h;
        boundPos = new Vec2(posX, posY);
        
        BodyDef bd = new BodyDef();
        bd.position = box2d.coordPixelsToWorld(posX, posY);
        bd.type = BodyType.STATIC;
        
        boundBody = box2d.createBody(bd);
        
        PolygonShape ps = new PolygonShape();
        ps.setAsBox(
          box2d.scalarPixelsToWorld(boundWidth/2.0f),
          box2d.scalarPixelsToWorld(boundHeight/2.0f)
        );
        
        boundBody.createFixture(ps, 1.0f);
    }
    
    
    void display()
    {
        stroke(0);
        fill(25);
        rectMode(CENTER);
        rect(boundPos.x, boundPos.y, boundWidth, boundHeight);
    }
}
