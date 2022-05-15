class Pendulum
{
    PVector pivotPos;
    float lengthOfArm;
    Mover bob;
    
    float gravity;
    
    Pendulum(float pivotX, float pivotY, float len)
    {
        gravity = 0.2;
        pivotPos = new PVector(pivotX, pivotY);
        lengthOfArm = len;
        
        float bobX = lengthOfArm*cos(PI*0.25) + pivotPos.x;
        float bobY = lengthOfArm*sin(PI*0.25) + pivotPos.y;
        bob = new Mover(bobX, bobY, 50.0f);
    }
    
    void update()
    {
        float gravityForce = gravity * bob.mass;
        bob.addForce(0.0, gravityForce);
        
        PVector toPivot = PVector.sub(pivotPos, bob.pos);
        toPivot.normalize();
        float cosTheta = PVector.dot(toPivot, new PVector(0.0, -1.0));
        
        float sqrVelo = PVector.dot(bob.velocity, bob.velocity);
        float toCenter = bob.mass * sqrVelo / lengthOfArm;
        
        toPivot.mult(gravityForce*cosTheta + toCenter);    
        bob.addForce(toPivot);
        
        PVector drag = new PVector(bob.velocity.x, bob.velocity.y);
        drag.normalize();
        drag.mult(-sqrVelo*0.0001);
        bob.addForce(drag);
        
        bob.update();
    }
    
    void display()
    {
        stroke(0xFFF0AD00);
        line(pivotPos.x, pivotPos.y, bob.pos.x, bob.pos.y);
      
        bob.display();
    }
};
