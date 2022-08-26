class FlowField
{
    PVector fieldSize;
    PVector fieldCenter;
    
    private int rows;
    private int columns;
    private float gridSizeX;
    private float gridSizeY;
    PVector [][] flowDirs;
    
    FlowField(float centerX, float centerY, float w, float h, float density)
    {
        fieldSize = new PVector(w, h);
        fieldCenter = new PVector(centerX, centerY);
      
        rows = round(h / density);
        columns = round(w / density);
        gridSizeX = fieldSize.x / columns;
        gridSizeY = fieldSize.y / rows;
        
        flowDirs = new PVector[rows][columns];
        for (int y = 0; y < rows; ++y)
        {
            for (int x = 0; x < columns; ++x)
            {
                float angle = noise(x*0.1, y*0.1, 0)*TWO_PI;
                flowDirs[y][x] = new PVector(cos(angle), sin(angle), 0.0);
            } 
        }
    }
    
    void applyFlowForce(MoveAgent mover)
    {
        PVector localPos = mover.location.copy();
        localPos.sub(fieldCenter);
        localPos.add(PVector.mult(fieldSize, 0.5));
        
        if (localPos.x < 0.0 || localPos.x >= fieldSize.x ||
            localPos.y < 0.0 || localPos.y >= fieldSize.y)
        {
            return;
        }
        
        /*int v0x = floor(localPos.x / gridSizeX);
        int v0y = floor(localPos.y / gridSizeY);
        int v1x = (v0x + 1) % columns;
        int v1y = (v0y + 1) % rows;
        
        //println("v0x="+v0x+";v0y="+v0y+";localPos.x="+localPos.x+";localPos.y="+localPos.y);
        
        PVector v00 = flowDirs[v0y][v0x];
        PVector v10 = flowDirs[v1y][v0x];
        PVector v01 = flowDirs[v0y][v1x];
        PVector v11 = flowDirs[v1y][v1x];
        
        float u = (localPos.x - v0x*gridSizeX)/gridSizeX;
        float v = (localPos.y - v0y*gridSizeY)/gridSizeY;
        
        PVector lerp0 = PVector.lerp(v00, v10, u);
        PVector lerp1 = PVector.lerp(v01, v11, u);
        PVector finalDir = PVector.lerp(lerp0, lerp1, v);
        
        PVector force = PVector.mult(finalDir, 0.01*mover.mass);
        mover.addForce(force);*/
        
        int v0x = floor(localPos.x / gridSizeX);
        int v0y = floor(localPos.y / gridSizeY);
        
        PVector v00 = flowDirs[v0y][v0x];
        
        PVector force = PVector.mult(v00, 0.01*mover.mass);
        mover.addForce(force);
    }
    
    void display()
    {
        stroke(0);
        
        pushMatrix();
        translate(fieldCenter.x-fieldSize.x*0.5, fieldCenter.y-fieldSize.y*0.5);
        
        line(0, 0, 0, fieldSize.y);
        line(0, fieldSize.y, fieldSize.x, fieldSize.y);
        line(fieldSize.x, fieldSize.y, fieldSize.x, 0);
        line(fieldSize.x, 0, 0, 0);
        
        float arrowLen = (gridSizeX + gridSizeY) * 0.6;
        for (int y = 0; y < rows; ++y)
        {
            for (int x = 0; x < columns; ++x)
            {
                PVector center = new PVector((x + 0.5)*gridSizeX, (y + 0.5)*gridSizeY);
                
                PVector dir = flowDirs[y][x].copy();
                PVector start = PVector.sub(center, PVector.mult(dir, arrowLen*0.5));
                
                drawArrow(start, dir, arrowLen);
            } 
        }
        popMatrix();
    }
    
    private void drawArrow(PVector start, PVector dir, float len)
    {
        float endX = start.x + len*dir.x;
        float endY = start.y + len*dir.y;
        
        line(start.x, start.y, endX, endY);
        
        float wingLen = len*0.4;
        dir.rotate(PI/12.0);
        float wing1EndX = endX - wingLen*dir.x;
        float wing1EndY = endY - wingLen*dir.y;
        dir.rotate(-PI/6.0);
        float wing2EndX = endX - wingLen*dir.x;
        float wing2EndY = endY - wingLen*dir.y;
        
        line(endX, endY, wing1EndX, wing1EndY);
        line(endX, endY, wing2EndX, wing2EndY);
    }
};
