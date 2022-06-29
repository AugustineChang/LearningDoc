class NoiseBase
{
    int PicWidth;
    int PicHeight;
    float Frequency;
    
    protected PImage NoiseImage;
    
    protected int DisplayOffX;
    protected int DisplayOffY;
    
    protected int DisplayState;
    protected int MAX_STATE;
    
    NoiseBase(int inWidth, int inHeight, float inFraq)
    {
        PicWidth = inWidth;
        PicHeight = inHeight;
        Frequency = inFraq;
        
        NoiseImage = createImage(PicWidth, PicHeight, RGB);
        
        DisplayState = 0;
        MAX_STATE = 1;
    }
    
    void SetDisplayOffset(int offX, int offY)
    {
        DisplayOffX = offX;
        DisplayOffY = offY;
    }
    
    void GenerateNoise()
    {
        NoiseImage.loadPixels();
        
        for (int y = 0; y < PicHeight; ++y)
        {
            float G = float(y) / float(PicHeight) * 255.0;
            for (int x = 0; x < PicWidth; ++x)
            {
                int index = x + y * PicWidth;
                float R = float(x) / float(PicWidth) * 255.0;
                NoiseImage.pixels[index] = color(R, G, 0.0);
            }
        }
        
        NoiseImage.updatePixels();
    }
    
    void Display()
    {
        image(NoiseImage, DisplayOffX, DisplayOffY);
    }
    
    void OnMouseClick()
    {
        if (mouseButton == LEFT)
            DisplayState = (DisplayState+1) % MAX_STATE;
        else if (mouseButton == RIGHT)
            DisplayState = (DisplayState+(MAX_STATE-1)) % MAX_STATE;
    }
};
