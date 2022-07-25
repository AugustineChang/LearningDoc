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
    boolean bIsDragged;
    
    Body shapeBody;
    
    Shape(float posX, float posY)
    {
        filledCol = color(188);
        bIsDead = false;
        bIsDragged = false;
      
        BodyDef bd = new BodyDef();
        bd.position = box2d.coordPixelsToWorld(posX, posY);
        bd.type = BodyType.DYNAMIC;
        
        shapeBody = box2d.createBody(bd);
        //shapeBody.setLinearVelocity(new Vec2(1.0f, 2.0f));
    }
    
    void createShapeEnd()
    {
        PolygonShape ps = new PolygonShape();
        defineShape(ps);
        
        FixtureDef fd = new FixtureDef();
        fd.shape = ps;
        fd.friction = 0.3f;
        fd.restitution = 0.5f;
        fd.density = 1.0f;
        
        shapeBody.createFixture(fd);
    }
    
    void defineShape(PolygonShape ps)
    {
        // should do something
        println("error: not override defineShape");
    }
    
    void drawShape()
    {
        // should do something
        println("error: not override drawShape");
    }
    
    boolean checkDrag(float dragPosX, float dragPosY)
    {
        return false;
    }
    
    boolean checkDead(Vec2 location)
    {
        if (bIsDragged)
            return false;
      
        if (location.x > width || location.x < 0.0 ||
            location.y > height || location.y < 0.0)
        {
            bIsDead = true;
            box2d.destroyBody(shapeBody);
            return true;
        }
        
        return false;
    }
    
    void display()
    {
        rectMode(CENTER);
        stroke(0);
        fill(filledCol);
        
        Transform trans = shapeBody.getTransform();
        Vec2 location = box2d.coordWorldToPixels(trans.p);
        float angle = trans.q.getAngle();
        
        if (checkDead(location))
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
