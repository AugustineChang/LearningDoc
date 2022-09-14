class BorderArea extends StaticObject
{
    float AreaWidX;
    float AreaWidY;
    
    float BorderMinX;
    float BorderMinY;
    float BorderMaxX;
    float BorderMaxY;
    
    PVector[] BorderNormals;
    
    BorderArea()
    {
        super();
        numOfSubObjects = 4;
        
        AreaWidX = 25.0;
        AreaWidY = 25.0;
        
        BorderMinX = PixelToMeter(AreaWidX);
        BorderMinY = PixelToMeter(AreaWidY);
        BorderMaxX = PixelToMeter(width-AreaWidX);
        BorderMaxY = PixelToMeter(height-AreaWidX);
        
        BorderNormals = new PVector[numOfSubObjects];
        BorderNormals[0]= new PVector(1.0, 0.0);
        BorderNormals[1]= new PVector(-1.0, 0.0);
        BorderNormals[2]= new PVector(0.0, 1.0);
        BorderNormals[3]= new PVector(0.0, -1.0);
    }
    
    void display()
    {
        rectMode(CORNER);
        noStroke();
        fill(230, 128, 103);
        
        rect(0, 0, AreaWidX, height);
        rect(0, 0, width, AreaWidY);
        rect(width-AreaWidX, 0, width, height);
        rect(0, height-AreaWidY, width, height);
        
        stroke(0);
        noFill();
        rect(AreaWidX, AreaWidY, width-AreaWidX*2, height-AreaWidY*2);
    }
    
    PVector getNormal(int sub, PVector inLoc)
    {
        return BorderNormals[sub];
    }
    
    PVector getHitPoint(int sub, PVector inLoc)
    {
        switch(sub)
        {
            case 0:
              return new PVector(BorderMinX, inLoc.y);
            case 1:
              return new PVector(BorderMaxX, inLoc.y);
            case 2:
              return new PVector(inLoc.x, BorderMinY);
            case 3:
              return new PVector(inLoc.x, BorderMaxY);
            default:
              println("BorderArea Error: can not get hit point, no such subIndex("+sub+")!!!");
              return super.getHitPoint(sub, inLoc);
        }
    }
}
