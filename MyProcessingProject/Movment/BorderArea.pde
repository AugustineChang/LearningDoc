class BorderArea
{
    float AreaWidX;
    float AreaWidY;
    
    BorderArea()
    {
        AreaWidX = max(width * 0.1, 50.0);
        AreaWidY = max(height * 0.1, 50.0);
    }
    
    void applyBorderAreaForce(MoveAgent mover)
    {
        float forceX = getBorderForce(mover.location.x, mover.mass, width, AreaWidX);
        float forceY = getBorderForce(mover.location.y, mover.mass, height, AreaWidY);
        
        if (!mover.bInBorder && (forceX != 0.0 || forceY != 0.0))
        {
            mover.bInBorder = true;
            mover.randomFactor.set(
              random(-0.2, 0.2),
              random(-0.2, 0.2)
            );
        }
        else if (mover.bInBorder && forceX == 0.0 && forceY == 0.0)
        {
            mover.bInBorder = false;
            mover.randomFactor.set(0.0, 0.0);
        }
        
        if (mover.bInBorder)
        {
            float randX = mover.randomFactor.x*forceY;
            float randY = mover.randomFactor.y*forceX;
            
            mover.addForce(new PVector(forceX, forceY));
            mover.addForce(new PVector(randX, randY));
        }
    }
    
    boolean checkInBorder(PVector loc)
    {
        float forceX = getBorderForce(loc.x, 1.0, width, AreaWidX);
        float forceY = getBorderForce(loc.y, 1.0, height, AreaWidY);
        
        return (forceX != 0.0 || forceY != 0.0);
    }
    
    private float getBorderForce(float loc, float mass, float totalSize, float awayArea)
    {
        float force = 0.0;
        if (loc < awayArea)
        {
            force = 10.0*mass * (awayArea - loc) / awayArea;
        }
        else if (loc > (totalSize - awayArea))
        {
            force = -10.0*mass * (loc - totalSize + awayArea) / awayArea;
        }
        return force;
    }
    
    void display()
    {
        noStroke();
        fill(230, 128, 103);
        
        rect(0, 0, AreaWidX, height);
        rect(0, 0, width, AreaWidY);
        rect(width-AreaWidX, 0, width, height);
        rect(0, height-AreaWidY, width, height);
    }
}
