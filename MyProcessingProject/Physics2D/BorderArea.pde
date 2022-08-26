class BorderArea
{
    float AreaWidX;
    float AreaWidY;
    
    float BorderMinX;
    float BorderMinY;
    float BorderMaxX;
    float BorderMaxY;
    
    PVector[] BorderPoints;
    PVector[] BorderNormals;
    
    BorderArea()
    {
        AreaWidX = 25.0;
        AreaWidY = 25.0;
        
        BorderMinX = PixelToMeter(AreaWidX);
        BorderMinY = PixelToMeter(AreaWidY);
        BorderMaxX = PixelToMeter(width-AreaWidX);
        BorderMaxY = PixelToMeter(height-AreaWidX);
        
        BorderPoints = new PVector[4];
        BorderPoints[0]= new PVector(BorderMinX, (BorderMinY+BorderMaxY)*0.5);
        BorderPoints[1]= new PVector(BorderMaxX, (BorderMinY+BorderMaxY)*0.5);
        BorderPoints[2]= new PVector((BorderMinX+BorderMaxX)*0.5, BorderMinY);
        BorderPoints[3]= new PVector((BorderMinX+BorderMaxX)*0.5, BorderMaxY);
        
        BorderNormals = new PVector[4];
        BorderNormals[0]= new PVector(1.0, 0.0);
        BorderNormals[1]= new PVector(-1.0, 0.0);
        BorderNormals[2]= new PVector(0.0, 1.0);
        BorderNormals[3]= new PVector(0.0, -1.0);
    }
    
    void display()
    {
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
}
