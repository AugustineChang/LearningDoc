class Hovercraft extends MoveObject
{
    protected float boatWidth;
    protected float boatHeight;
    
    protected PVector turnLeftForcePoint;
    protected PVector turnRightForcePoint;
    
    protected int isMoveForward;
    protected boolean isTurnLeft;
    protected boolean isTurnRight;
    
    Hovercraft(float inX, float inY, float inRadius)
    {
        super(inX, inY, inRadius);
        
        boatHeight = PixelToMeter(inRadius * 2.0f);
        boatWidth = boatHeight * 0.375f;
        
        this.momentOfInertia = (1.0/12.0) * mass * (boatWidth * boatWidth + boatHeight*boatHeight);// (1/12)m(a^2+b^2)
        
        turnLeftForcePoint = new PVector(boatHeight*0.25f, boatWidth*0.5f);
        turnRightForcePoint = new PVector(boatHeight*0.25f, -boatWidth*0.5f);
        this.dragForcePoint = new PVector(-boatHeight*0.25f, 0.0f);
        
        useGravity = false;
        useWindForce = false;
    }
    
    void drawObject()
    {
        rectMode(CENTER);
        
        float heightInPixel = MeterToPixel(boatHeight);
        float widhtInPixel = MeterToPixel(boatWidth);
        float halfHeight = 0.5f * heightInPixel;
        float halfWidth = 0.5f * widhtInPixel;
        float headLen = halfHeight * 0.5f;
        
        beginShape();
        vertex(-halfHeight, -halfWidth);
        vertex(halfHeight-headLen, -halfWidth);
        vertex(halfHeight, 0.0f);
        vertex(halfHeight-headLen, halfWidth);
        vertex(-halfHeight, halfWidth);
        endShape(CLOSE);
    }
    
    void setMoveForward(int inVal) { isMoveForward = inVal; }
    void setTurnLeft(boolean inVal) { isTurnLeft = inVal; }
    void setTurnRight(boolean inVal) { isTurnRight = inVal; }
    
    void update()
    {
        PVector localForces = new PVector();
        PVector localMoments = new PVector();
      
        if (isMoveForward != 0)
        {
            localForces.add(10.0f*isMoveForward, 0.0f);
        }
        
        if (isTurnLeft)
        {
            PVector turnLeftForce = new PVector(0.0f, -5.0f);
            localForces.add(turnLeftForce);
          
            localMoments.add(turnLeftForcePoint.cross(turnLeftForce));
        }
        
        if (isTurnRight)
        {
            PVector turnRightForce = new PVector(0.0f, 5.0f);
            localForces.add(turnRightForce);
          
            localMoments.add(turnRightForcePoint.cross(turnRightForce));
        }
        
        localForces.rotate(angle.z);
        forces.add(localForces);
        moments.add(localMoments);
        
        super.update();
    }
};
