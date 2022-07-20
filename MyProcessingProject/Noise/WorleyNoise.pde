class WorleyNoise extends NoiseBase
{
    ArrayList<PVector> FeaturePoints;
    private boolean bGenerated;
    private int GridSizeX;
    private int GridSizeY;
    private int NumGridsX;
    private int NumGridsY;
  
    WorleyNoise(int inWidth, int inHeight, float inFraq)
    {
        super(inWidth, inHeight, inFraq);
        
        MAX_STATE = 5;
        
        GridSizeX = floor(PicWidth / Frequency);
        GridSizeY = floor(PicHeight / Frequency);
        NumGridsX = ceil(float(PicWidth) / float(GridSizeX));
        NumGridsY = ceil(float(PicHeight) / float(GridSizeY));
        
        bGenerated = false;
        FeaturePoints = new ArrayList<PVector>();
    }
    
    private void ResetRandom()
    {
        FeaturePoints.clear();
        for (int y = 0; y < NumGridsY; ++y)
        {
            for (int x = 0; x < NumGridsX; ++x)
            {
                float randX = random(x*GridSizeX, (x+1)*GridSizeX);
                float randY = random(y*GridSizeY, (y+1)*GridSizeY);
                FeaturePoints.add(new PVector(randX, randY, 0));
            }
        }
    }
    
    void GenerateNoise()
    {
        NoiseImage.loadPixels();
        
        int GridIdX = 0;
        int GridIdY = 0;
        float AvgSize = (GridSizeX + GridSizeY)*0.6;
        for (int y = 0; y < PicHeight; ++y)
        {
            GridIdY = y / GridSizeY;
            for (int x = 0; x < PicWidth; ++x)
            {
                GridIdX = x / GridSizeX;
                PVector curPos = new PVector(x, y, 0);
                float MinDist = Float.MAX_VALUE;
                for (int deltaY = -1; deltaY <= 1; ++deltaY)
                {
                    int curGridIdY = (GridIdY + deltaY + NumGridsY) % NumGridsY;
                    for (int deltaX = -1; deltaX <= 1; ++deltaX)
                    {
                        int curGridIdX = (GridIdX + deltaX + NumGridsX) % NumGridsX;
                        int pointIndex = curGridIdX + curGridIdY*NumGridsX;
                        PVector diff = PVector.sub(curPos, FeaturePoints.get(pointIndex));
                        float distSquared = diff.magSq();
                        if (distSquared < MinDist)
                        {
                           MinDist = distSquared;
                        }
                    }
                }
                
                int index = x + y * PicWidth;
                float randVal = sqrt(MinDist) / AvgSize * 255.0;
                randVal = min(randVal, 255.0);
                NoiseImage.pixels[index] = color(randVal, randVal, randVal);
            }
        }
        
        NoiseImage.updatePixels();
    }
    
    void Display()
    {
        rectMode(CORNER);
        switch(DisplayState)
        {
            case 0:
            noStroke();
            fill(0);
            rect(DisplayOffX, DisplayOffY, PicWidth, PicHeight);
            if (!bGenerated)
            {
                ResetRandom();
                GenerateNoise();
                bGenerated = true;
            }
            break;
            
            case 1:
            noStroke();
            fill(0);
            rect(DisplayOffX, DisplayOffY, PicWidth, PicHeight);
            DrawGrids();
            break;
            
            case 2:
            noStroke();
            fill(0);
            rect(DisplayOffX, DisplayOffY, PicWidth, PicHeight);
            DrawGrids();
            DrawFeaturePoints();
            break;
            
            case 3:
            image(NoiseImage, DisplayOffX, DisplayOffY);
            DrawGrids();
            DrawFeaturePoints();
            break;
            
            case 4:
            image(NoiseImage, DisplayOffX, DisplayOffY);
            bGenerated = false;
            break;
        }
        
        fill(255);
        textSize(30);
        text("Worley Noise (2)", 20, 35);
    }
    
    private void DrawGrids()
    {
        stroke(255, 70, 0);
       
        //draw row
        int lineRowS = DisplayOffX;
        int lineRowE = DisplayOffX + PicWidth;
        for (int y = 0; y < NumGridsY; ++y)
        {
            int lineY = y*GridSizeY + DisplayOffY;
            line(lineRowS, lineY, lineRowE, lineY);
        }
        
        //draw column
        int lineColS = DisplayOffY;
        int lineColE = DisplayOffY + PicHeight;
        for (int x = 0; x < NumGridsX; ++x)
        {
            int lineX = x*GridSizeX + DisplayOffX;
            line(lineX, lineColS, lineX, lineColE);
        }
    }
    
    private void DrawFeaturePoints()
    {
        stroke(0, 185, 255);
        fill(0, 185, 255);
        ellipseMode(CENTER);
      
        int NumOfPoints = FeaturePoints.size();
        for (int i = 0; i < NumOfPoints; ++i)
        {
            PVector curPoint = FeaturePoints.get(i);
            ellipse(curPoint.x+DisplayOffX, curPoint.y+DisplayOffY, 11.0, 11.0);
        }
    }
};
