class SpringPendulum
{
    PVector pivotPos;
    float restLenOfArm;
    Mover bob;
    
    float gravity;
    float springCoeff;
    
    SpringPendulum(float pivotX, float pivotY, float len)
    {
        gravity = 0.2;
        springCoeff = 0.002;
        pivotPos = new PVector(pivotX, pivotY);
        restLenOfArm = len;
        
        float bobX = len*cos(PI*0.25) + pivotPos.x;
        float bobY = len*sin(PI*0.25) + pivotPos.y;
        bob = new Mover(bobX, bobY, 50.0f);
    }
    
    void update()
    {
        float gravityForce = gravity * bob.mass;
        bob.addForce(0.0, gravityForce);
        
        PVector toPivot = PVector.sub(pivotPos, bob.pos);
        float lengthOfArm = toPivot.mag();
        float springForce = (lengthOfArm - restLenOfArm) * springCoeff;
        toPivot.mult(springForce / lengthOfArm);
        bob.addForce(toPivot);
        
        
        PVector drag = new PVector(bob.velocity.x, bob.velocity.y);
        drag.normalize();
        float sqrVelo = PVector.dot(bob.velocity, bob.velocity);
        drag.mult(-sqrVelo*0.0001);
        bob.addForce(drag);
        
        bob.update();
    }
    
    void display()
    {
        stroke(0xFF2EF2FF);
        line(pivotPos.x, pivotPos.y, bob.pos.x, bob.pos.y);
      
        bob.display();
    }
};
