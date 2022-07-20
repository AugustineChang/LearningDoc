class SimpleNoise extends NoiseBase
{
    SimpleNoise(int inWidth, int inHeight)
    {
        super(inWidth, inHeight, 0.0);
        
        GenerateNoise();
    }
    
    void GenerateNoise()
    {
        NoiseImage.loadPixels();
        
        for (int y = 0; y < PicHeight; ++y)
        {
            for (int x = 0; x < PicWidth; ++x)
            {
                int index = x + y * PicWidth;
                float randVal = random(0, 256);// format R8G8B8
                NoiseImage.pixels[index] = color(randVal, randVal, randVal);
            }
        }
        
        NoiseImage.updatePixels();
    }
    
    void Display()
    {
        super.Display();
        
        fill(255);
        textSize(30);
        text("Simple Noise", 20, 35);
    }
    
    void OnMouseClick()
    {
        super.OnMouseClick();
        GenerateNoise();
    }
};
