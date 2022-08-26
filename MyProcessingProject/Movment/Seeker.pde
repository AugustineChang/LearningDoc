class Seeker extends MoveAgent
{
    boolean bHasTarget;
    PVector targetPos;
    float goForceRandom;
    
    boolean bArrived;
    float ArriveDist;
  
    Seeker(float posX, float posY)
    {
        super(posX, posY);
        
        bHasTarget = false;
        targetPos = new PVector();
        goForceRandom = random(0.1, 0.2);
        
        bArrived = false;
        ArriveDist = 2.0;
        
        PVector randVelo = PVector.random2D();
        randVelo.mult(random(0.01*maxVelo, 0.05*maxVelo));
        velocity.set(randVelo);
    }
    
    void setTarget(boolean hasTar, float tarX, float tarY)
    {
        bHasTarget = hasTar;
        targetPos.set(tarX, tarY);
    }
    
    private void goToTarget()
    {
        if (!bHasTarget)
            return;
        
        float slowRadius = 50.0;
        
        PVector toTar = PVector.sub(targetPos, location);
        float dist = toTar.mag();
        
        bArrived = dist < ArriveDist;
        if (bArrived)
            return;
        
        PVector desired = null;
        if (dist > slowRadius)
        {
            desired = PVector.mult(toTar, maxVelo/dist);
        }
        else
        {
            float ratio = dist / slowRadius;
            desired = PVector.mult(toTar, ratio*ratio*maxVelo/max(dist, 0.001));
        }
        
        PVector force = PVector.sub(desired, velocity);
        force.mult(mass*goForceRandom);
        addForce(force);
    }
    
    void doAction()
    {
        goToTarget();
    }
};
