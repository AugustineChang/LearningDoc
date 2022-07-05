class ValueNoise extends NoiseBase
{
    private PImage ValueNoiseImage_Smoothed;
    
    private FloatList RandInGrids;
    private int GridSizeX;
    private int GridSizeY;
    private int NumGridsX;
    private int NumGridsY;
    
    private int AnimTime;
    
    ValueNoise(int inWidth, int inHeight, float inFraq)
    {
        super(inWidth, inHeight, inFraq);
        ValueNoiseImage_Smoothed = createImage(PicWidth, PicHeight, RGB);
        RandInGrids = new FloatList();
        
        AnimTime = 0;
        MAX_STATE = 10;
        
        GridSizeX = floor(PicWidth / Frequency);
        GridSizeY = floor(PicHeight / Frequency);
        NumGridsX = ceil(float(PicWidth) / float(GridSizeX));
        NumGridsY = ceil(float(PicHeight) / float(GridSizeY));
        
        ResetRandoms();
        GenerateNoise(false);
        GenerateNoise(true);
    }
    
    void GenerateNoise(boolean bUseSmooth)
    {
        PImage TargetImage;
        if (bUseSmooth)
            TargetImage = ValueNoiseImage_Smoothed;
        else
            TargetImage = NoiseImage;
        
        TargetImage.loadPixels();
        
        //fill pixels
        int GridIdX = 0;
        int GridIdY = 0;
        int NextGridIdX = 0;
        int NextGridIdY = 0;             
        float UInGrid = 0.0;
        float VInGrid = 0.0;
        for (int y = 0; y < PicHeight; ++y)
        {
            GridIdY = y / GridSizeY;
            VInGrid = float(y % GridSizeY) / float(GridSizeY);
            NextGridIdY = (GridIdY + 1) % NumGridsY;
            if (bUseSmooth)
                VInGrid = SmoothValue(VInGrid);
            
            for (int x = 0; x < PicWidth; ++x)
            {
                GridIdX = x / GridSizeX;
                UInGrid = float(x % GridSizeX) / float(GridSizeX);
                NextGridIdX = (GridIdX + 1) % NumGridsX;
                if (bUseSmooth)
                    UInGrid = SmoothValue(UInGrid);
                
                float val00 = RandInGrids.get(GridIdX + GridIdY*NumGridsX);
                float val10 = RandInGrids.get(NextGridIdX + GridIdY*NumGridsX);
                float val01 = RandInGrids.get(GridIdX + NextGridIdY*NumGridsX);
                float val11 = RandInGrids.get(NextGridIdX + NextGridIdY*NumGridsX);
                
                float lerp0 = lerp(val00, val10, UInGrid);
                float lerp1 = lerp(val01, val11, UInGrid);
                float finalLerp = lerp(lerp0, lerp1, VInGrid) * 255.0;
                
                int index = x + y * PicWidth;
                TargetImage.pixels[index] = color(finalLerp, finalLerp, finalLerp);
            }
        }
        TargetImage.updatePixels();
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
            break;
            
            case 1:
            noStroke();
            fill(0);
            rect(DisplayOffX, DisplayOffY, PicWidth, PicHeight);
            DrawGrids(true);
            break;
            
            case 2:
            noStroke();
            fill(0);
            rect(DisplayOffX, DisplayOffY, PicWidth, PicHeight);
            DrawGrids(false);
            DrawGridRandoms(true);
            break;
            
            case 3:
            case 4:
            case 5:
            case 6:
            noStroke();
            fill(0);
            rect(DisplayOffX, DisplayOffY, PicWidth, PicHeight);
            DrawGrids(false);
            DrawInterpolation();
            break;
            
            case 7:
            image(NoiseImage, DisplayOffX, DisplayOffY);
            DrawGrids(false);
            DrawGridRandoms(false);
            break;
            
            case 8:
            image(NoiseImage, DisplayOffX, DisplayOffY);
            break;
            
            case 9:
            image(ValueNoiseImage_Smoothed, DisplayOffX, DisplayOffY);
            break;
        }
        
        ++AnimTime;
    }
    
    void OnMouseClick()
    {
        super.OnMouseClick();
        AnimTime = 0;
    }
    
    private void DrawGrids(boolean bAnime)
    {
        stroke(255, 70, 0);
        int drawLineInterval = bAnime ? 10 : 0;
        
        //draw row
        int lineRowS = DisplayOffX;
        int lineRowE = DisplayOffX + PicWidth;
        for (int y = 0; y < NumGridsY; ++y)
        {
            if (AnimTime < y*drawLineInterval)  
                continue;
            
            int lineY = y*GridSizeY + DisplayOffY;
            line(lineRowS, lineY, lineRowE, lineY);
        }
        
        //draw column
        int lineColS = DisplayOffY;
        int lineColE = DisplayOffY + PicHeight;
        for (int x = 0; x < NumGridsX; ++x)
        {
            if (AnimTime < (x + NumGridsY)*drawLineInterval)
                continue;
            
            int lineX = x*GridSizeX + DisplayOffX;
            line(lineX, lineColS, lineX, lineColE);
        }
    }
    
    private void DrawGridRandoms(boolean bAnime)
    {
        stroke(0, 185, 255);
        ellipseMode(CENTER);
        
        int drawLineInterval = bAnime ? 2 : 0;
        
        for (int y = 0; y < NumGridsY; ++y)
        {
            int centerY = y*GridSizeY + DisplayOffY;
            for (int x = 0; x < NumGridsX; ++x)
            {
                if (AnimTime < (x + y*NumGridsX)*drawLineInterval)
                    continue;
              
                int centerX = x*GridSizeX + DisplayOffX;
                fill(RandInGrids.get(x + y*NumGridsX) * 255.0);
                ellipse(centerX, centerY, 11.0, 11.0);
            }
        }
    }
    
    private void DrawInterpolation()
    {
        stroke(0, 185, 255);
        ellipseMode(CENTER);
        rectMode(CENTER);
        
        int centerX = floor(GridSizeX*0.5) + DisplayOffX;
        int centerY = floor(GridSizeY*0.5) + DisplayOffY;
        
        if (DisplayState >= 3)
        {  
            fill(0, 185, 255);
            ellipse(centerX, centerY, 5.0, 5.0);
            fill(255);
            textSize(20);
            text("?", centerX+4, centerY+6);
        }
        
        if (DisplayState >= 4)
        {
            fill(RandInGrids.get(0) * 255.0);
            ellipse(DisplayOffX, DisplayOffY, 11.0, 11.0);
            
            fill(RandInGrids.get(1) * 255.0);
            ellipse(DisplayOffX+GridSizeX, DisplayOffY, 11.0, 11.0);
            
            fill(RandInGrids.get(NumGridsX) * 255.0);
            ellipse(DisplayOffX, DisplayOffY+GridSizeY, 11.0, 11.0);
            
            fill(RandInGrids.get(NumGridsX+1) * 255.0);
            ellipse(DisplayOffX+GridSizeX, DisplayOffY+GridSizeY, 11.0, 11.0);
        }
        
        if (DisplayState >= 5)
        {
            fill(0, 185, 255);
            ellipse(centerX, DisplayOffY, 5.0, 5.0);
            ellipse(centerX, DisplayOffY+GridSizeY, 5.0, 5.0);
            
            stroke(54, 255, 0);
            line(DisplayOffX+7, DisplayOffY, centerX-6, DisplayOffY);
            line(DisplayOffX+GridSizeX-8, DisplayOffY, centerX+5, DisplayOffY);
            line(DisplayOffX+7, DisplayOffY+GridSizeY, centerX-6, DisplayOffY+GridSizeY);
            line(DisplayOffX+GridSizeX-8, DisplayOffY+GridSizeY, centerX+5, DisplayOffY+GridSizeY);
        }
        
        if (DisplayState >= 6)
        {
            stroke(54, 255, 0);
            line(centerX, DisplayOffY+5, centerX, centerY-6);
            line(centerX, DisplayOffY+GridSizeY-6, centerX, centerY+5);
        }
    }
    
    private void ResetRandoms()
    {
        //random grids
        RandInGrids.clear();
        for (int y = 0; y < NumGridsY; ++y)
        {
            for (int x = 0; x < NumGridsX; ++x)
            {
                RandInGrids.append(random(0.0, 1.0));
            }
        }
    }
    
    private float SmoothValue(float InVal)
    {
        return InVal * InVal * (3.0 - 2.0 * InVal);// y = -2x^3 + 3x^2
    }
};
