class Wanderer extends Seeker
{
    int seekMode;//0-to random point; 1-to mouse
    PVector wanderStepRange;
    
    float stopFrames;
    
    Wanderer(float posX, float posY)
    {
        super(posX, posY);
        
        maxAccel = 2.0;
        maxVelo = 5.0;
        goForceRandom = random(0.2, 0.5);
        ArriveDist = 8.0;
        
        seekMode = 0;
        wanderStepRange = new PVector(20.0, 100.0);
        stopFrames = 20;
        
        fillCol = color(103, 103, 153);
    }
    
    void setTarget(boolean hasTar, float tarX, float tarY)
    {
        if (!hasTar && seekMode == 0)
            return;
      
        super.setTarget(hasTar, tarX, tarY);
        
        seekMode = hasTar ? 1 : 0;
    }
    
    void doAction()
    {
        //wander
        if (seekMode == 0)
        {
            if (!bHasTarget || (bArrived && stopFrames <= 0))
            {
                PVector newTar = getWanderTarget();
                super.setTarget(true, newTar.x, newTar.y);
                stopFrames = 20;
            }
            else
                --stopFrames;
        }
      
        super.doAction();
    }
    
    private PVector getWanderTarget()
    {        
        PVector destLoc = null;
        do
        {
            PVector wandDir = new PVector(1.0, 0.0, 0.0);
            wandDir.rotate(random(0.0, 2.0*PI));
            wandDir.mult(random(wanderStepRange.x, wanderStepRange.y));
          
            destLoc = PVector.add(location, wandDir);
        }
        while(border.checkInBorder(destLoc));
        
        return destLoc;
    }
    
    /*void display()
    {
        super.display();
        
        ellipse(targetPos.x, targetPos.y, 6, 6);
    }*/
};
