import org.jbox2d.common.Vec2;
import org.jbox2d.common.Transform;

import org.jbox2d.dynamics.Body;
import org.jbox2d.dynamics.BodyDef;
import org.jbox2d.dynamics.BodyType;
import org.jbox2d.dynamics.FixtureDef;

import org.jbox2d.collision.shapes.PolygonShape;

class Box
{
    int boxWidth;
    int boxHeight;
    color filledCol;
    boolean bIsDead;
    
    Body boxBody;
    
    Box(Box2DProcessing box2d, float posX, float posY, int w, int h)
    {
      {
        boxWidth = w;
        boxHeight = h;
        
        int R = floor(random(256));
        int G = floor(random(256));
        int B = floor(random(256));
        filledCol = color(R, G, B);
        
        bIsDead = false;
      }
      
      {
        BodyDef bd = new BodyDef();
        bd.position = box2d.coordPixelsToWorld(posX, posY);
        bd.type = BodyType.DYNAMIC;
        
        boxBody = box2d.createBody(bd);
        boxBody.setLinearVelocity(new Vec2(1.0f, 2.0f));
        
        PolygonShape ps = new PolygonShape();
        ps.setAsBox(
          box2d.scalarPixelsToWorld(boxWidth/2.0f),
          box2d.scalarPixelsToWorld(boxHeight/2.0f)
        );
        
        FixtureDef fd = new FixtureDef();
        fd.shape = ps;
        fd.friction = 0.3f;
        fd.restitution = 0.5f;
        fd.density = 1.0f;
        
        boxBody.createFixture(fd);
      }
    }
    
    void display(Box2DProcessing box2d)
    {
        stroke(0);
        fill(filledCol);
        rectMode(CENTER);
        
        Transform trans = boxBody.getTransform();
        Vec2 location = box2d.coordWorldToPixels(trans.p);
        float angle = trans.q.getAngle();
        
        if (location.x - boxWidth > width || location.x + boxWidth < 0.0 ||
        location.y - boxHeight > height || location.y + boxHeight < 0.0)
        {
            bIsDead = true;
            box2d.destroyBody(boxBody);
            return;
        }
        
        pushMatrix();
        translate(location.x, location.y);
        rotate(-angle);
        rect(0, 0, boxWidth, boxHeight);
        popMatrix();
    }
};
