class CellNoise extends NoiseBase
{
    private boolean bGenerated;
    int NumOfFeaturePoints;
    ArrayList<PVector> FeaturePoints;
  
    CellNoise(int inWidth, int inHeight, int NumOfPoints)
    {
        super(inWidth, inHeight, 0.0);
        
        bGenerated = false;
        NumOfFeaturePoints = NumOfPoints;
        FeaturePoints = new ArrayList<PVector>();
        
        MAX_STATE = 4;
    }
    
    void GenerateNoise()
    {
        NoiseImage.loadPixels();
        
        float AvgSize = (PicWidth + PicHeight) * 0.125f;
        for (int y = 0; y < PicHeight; ++y)
        {
            for (int x = 0; x < PicWidth; ++x)
            {
                PVector curPos = new PVector(x, y, 0);
                float MinDist = Float.MAX_VALUE;
                for (int i = 0; i < NumOfFeaturePoints; ++i)
                {
                    PVector diff = PVector.sub(curPos, FeaturePoints.get(i));
                    float distSquared = diff.magSq();
                    if (distSquared < MinDist)
                    {
                       MinDist = distSquared;
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
    
    private void ResetRandom()
    {
        FeaturePoints.clear();
        for (int i =0; i < NumOfFeaturePoints; ++i)
        {
            float randX = random(0, PicWidth);
            float randY = random(0, PicHeight);
            FeaturePoints.add(new PVector(randX, randY, 0));
        }
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
            DrawFeaturePoints();
            break;
            
            case 2:
            image(NoiseImage, DisplayOffX, DisplayOffY);
            DrawFeaturePoints();
            break;
            
            case 3:
            image(NoiseImage, DisplayOffX, DisplayOffY);
            bGenerated = false;
            break;
        }
        
        fill(255);
        textSize(30);
        text("Worley Noise (1)", 20, 35);
    }
};
