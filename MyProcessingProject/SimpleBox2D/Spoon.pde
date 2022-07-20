import org.jbox2d.collision.shapes.CircleShape;

class Spoon extends Shape
{
    float headRadius;
    float handleLen;
    float handleWid;
    float headToHandle;
    
    Spoon(Box2DProcessing box2d, float posX, float posY, float r, float l)
    {
      super(box2d, posX, posY);
      
      headRadius = r;
      handleLen = l;
      handleWid = l*0.15;
      headToHandle = handleLen/2.0 + headRadius;// + sqrt(headRadius*headRadius - 0.25*handleWid*handleWid);
      
      int R = floor(random(256));
      int G = floor(random(256));
      int B = floor(random(256));
      filledCol = color(R, G, B);
      
      createShapeEnd(box2d);
    }
    
    void createShapeEnd(Box2DProcessing box2d)
    {
        PolygonShape handle = new PolygonShape();
        handle.setAsBox(
          box2d.scalarPixelsToWorld(handleWid/2.0),
          box2d.scalarPixelsToWorld(handleLen/2.0)
        );
        
        CircleShape head = new CircleShape();
        head.m_radius = box2d.scalarPixelsToWorld(headRadius);
        head.m_p.set(box2d.vectorPixelsToWorld(new Vec2(0.0, -headToHandle)));
        
        FixtureDef fd = new FixtureDef();
        fd.friction = 0.3f;
        fd.restitution = 0.5f;
        fd.density = 1.0f;
        
        fd.shape = handle;
        shapeBody.createFixture(fd);
        fd.shape = head;
        shapeBody.createFixture(fd);
    }
    
    void drawShape()
    {  
        ellipseMode(CENTER);  
        
        rect(0, 0, handleWid, handleLen);
        ellipse(0, -headToHandle, headRadius*2.0, headRadius*2.0);
    }
};
