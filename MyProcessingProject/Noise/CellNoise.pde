class CellNoise extends NoiseBase
{
    int NumOfFeaturePoints;
    ArrayList<PVector> FeaturePoints;
  
    CellNoise(int inWidth, int inHeight, int NumOfPoints)
    {
        super(inWidth, inHeight, 0.0);
        NumOfFeaturePoints = NumOfPoints;
        
        FeaturePoints = new ArrayList<PVector>();
        for (int i =0; i < NumOfFeaturePoints; ++i)
        {
            float randX = random(0, PicWidth);
            float randY = random(0, PicHeight);
            FeaturePoints.add(new PVector(randX, randY, 0));
        }
        
        GenerateNoise();
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
};
