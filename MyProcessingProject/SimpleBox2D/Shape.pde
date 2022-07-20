import org.jbox2d.common.Vec2;
import org.jbox2d.common.Transform;

import org.jbox2d.dynamics.Body;
import org.jbox2d.dynamics.BodyDef;
import org.jbox2d.dynamics.BodyType;
import org.jbox2d.dynamics.FixtureDef;

import org.jbox2d.collision.shapes.PolygonShape;

class Shape
{
    color filledCol;
    boolean bIsDead;
    
    Body shapeBody;
    
    Shape(Box2DProcessing box2d, float posX, float posY)
    {
        filledCol = color(188);
        bIsDead = false;
      
        BodyDef bd = new BodyDef();
        bd.position = box2d.coordPixelsToWorld(posX, posY);
        bd.type = BodyType.DYNAMIC;
        
        shapeBody = box2d.createBody(bd);
        //shapeBody.setLinearVelocity(new Vec2(1.0f, 2.0f));
    }
    
    void createShapeEnd(Box2DProcessing box2d)
    {
        PolygonShape ps = new PolygonShape();
        defineShape(box2d, ps);
        
        FixtureDef fd = new FixtureDef();
        fd.shape = ps;
        fd.friction = 0.3f;
        fd.restitution = 0.5f;
        fd.density = 1.0f;
        
        shapeBody.createFixture(fd);
    }
    
    void defineShape(Box2DProcessing box2d, PolygonShape ps)
    {
        // should do something
        println("error: not override defineShape");
    }
    
    void drawShape()
    {
        // should do something
        println("error: not override drawShape");
    }
    
    boolean checkDead(Box2DProcessing box2d, Vec2 location)
    {
        if (location.x > width || location.x < 0.0 ||
            location.y > height || location.y < 0.0)
        {
            bIsDead = true;
            box2d.destroyBody(shapeBody);
            return true;
        }
        
        return false;
    }
    
    void display(Box2DProcessing box2d)
    {
        rectMode(CENTER);
        stroke(0);
        fill(filledCol);
        
        Transform trans = shapeBody.getTransform();
        Vec2 location = box2d.coordWorldToPixels(trans.p);
        float angle = trans.q.getAngle();
        
        if (checkDead(box2d, location))
        {
            return;
        }
        
        pushMatrix();
        translate(location.x, location.y);
        rotate(-angle);
        drawShape();
        popMatrix();
    }
};
