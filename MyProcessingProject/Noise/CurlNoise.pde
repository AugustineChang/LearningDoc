class CurlNoise extends NoiseBase
{
    private int[] permTable;
    private ArrayList<PVector> CurlList;
    private PImage NoiseNormalImage;
    private PImage NoiseFlowImage;
    private ParticleSystem particlSys;
    
    private int GridSizeX;
    private int GridSizeY;
    private int NumGridsX;
    private int NumGridsY;
    
    PVector GridVert10;
    PVector GridVert01;
    PVector GridVert11;
    
    CurlNoise(int inWidth, int inHeight, float inFraq)
    {
        super(inWidth, inHeight, inFraq);
        
        CurlList = new ArrayList<PVector>();
        NoiseNormalImage = createImage(PicWidth, PicHeight, RGB);
        NoiseFlowImage = createImage(PicWidth, PicHeight, RGB);
        particlSys = new ParticleSystem(inWidth, inHeight);
        
        MAX_STATE = 5;
        
        GridSizeX = floor(PicWidth / Frequency);
        GridSizeY = floor(PicHeight / Frequency);
        NumGridsX = ceil(float(PicWidth) / float(GridSizeX));
        NumGridsY = ceil(float(PicHeight) / float(GridSizeY));
        
        GridVert10 = new PVector(1.0, 0.0);
        GridVert01 = new PVector(0.0, 1.0);
        GridVert11 = new PVector(1.0, 1.0);
        
        // init perm table
        permTable = new int[512];
        for (int i = 0; i < 256; ++i)
        {
            permTable[i] = permutation[i];
            permTable[256+i] = permutation[i];
        }
        
        GenerateNoise();
        GenerateNoise2();
        GenerateNoise3();
    }
    
    void SetDisplayOffset(int offX, int offY)
    {
        super.SetDisplayOffset(offX, offY);
        particlSys.offset.set(DisplayOffX, DisplayOffX);
    }
    
    void GenerateNoise2()
    {
        NoiseNormalImage.loadPixels();
        
        float delta = 2.0 / (GridSizeX + GridSizeY);
        for (int y = 0; y < PicHeight; ++y)
        {
            for (int x = 0; x < PicWidth; ++x)
            {
                float curVal = GetPerlinValue(x, y);
              
                float ddx = (GetPerlinValue(x+1, y) - curVal) / delta;
                float ddy = (GetPerlinValue(x, y+1) - curVal) / delta;
                
                PVector normalDir = new PVector(ddx, ddy, 2.0);
                normalDir.normalize();
                
                normalDir.x = (0.5 + 0.5*normalDir.x) * 255.0;
                normalDir.y = (0.5 + 0.5*normalDir.y) * 255.0;
                normalDir.z = (0.5 + 0.5*normalDir.z) * 255.0;
                
                int index = x + y * PicWidth;
                NoiseNormalImage.pixels[index] = color(normalDir.x, normalDir.y, normalDir.z);
            }
        }
        
        NoiseNormalImage.updatePixels();
    }
    
    void GenerateNoise3()
    {
        NoiseImage.loadPixels();
        
        for (int y = 0; y < PicHeight; ++y)
        {
            for (int x = 0; x < PicWidth; ++x)
            {
                float curVal = GetPerlinValue(x, y);

                curVal = (0.5 + 0.5*curVal) * 255.0;
                
                int index = x + y * PicWidth;
                NoiseImage.pixels[index] = color(curVal, curVal, curVal);
            }
        }
        
        NoiseImage.updatePixels();
    }
    
    void GenerateNoise()
    {
        NoiseFlowImage.loadPixels();
        
        float delta = 2.0 / (GridSizeX + GridSizeY);
        for (int y = 0; y < PicHeight; ++y)
        {
            for (int x = 0; x < PicWidth; ++x)
            {
                float curVal = GetPerlinValue(x, y);
              
                float ddx = (GetPerlinValue(x+1, y) - curVal) / delta;
                float ddy = (GetPerlinValue(x, y+1) - curVal) / delta;
                
                PVector curlDir = new PVector(ddy, -ddx, 0.0);
                curlDir.normalize();
                CurlList.add(curlDir);
                
                float curlDirX = (0.5 + 0.5*curlDir.x) * 255.0;
                float curlDirY = (0.5 + 0.5*curlDir.y) * 255.0;
                
                int index = x + y * PicWidth;
                NoiseFlowImage.pixels[index] = color(curlDirX, curlDirY, 0);
            }
        }
        
        NoiseFlowImage.updatePixels();
    }
    
    void Display()
    {
        fill(255);
        textSize(30);
        switch(DisplayState)
        {
            case 0:
            image(NoiseImage, DisplayOffX, DisplayOffY);
            text("Perlin Noise", 420, 35);
            break;
            
            case 1:
            image(NoiseNormalImage, DisplayOffX, DisplayOffY);
            text("Perlin Noise -> Normal", 300, 35);
            break;
            
            case 2:
            image(NoiseFlowImage, DisplayOffX, DisplayOffY);
            break;
            
            case 3:
            image(NoiseFlowImage, DisplayOffX, DisplayOffY);
            DrawCurlArrow();
            break;
            
            case 4:
            image(NoiseFlowImage, DisplayOffX, DisplayOffY);
            DrawCurlArrow();
            particlSys.setVelosity(CurlList, PicWidth, PicHeight);
            particlSys.update();
            particlSys.display();
            fill(255);
            break;
        }
        
        if (DisplayState >= 2 && DisplayState <= 4)
        {
            text("Perlin Noise -> Curl Noise", 270, 35);
        }
        
        text("Curl Noise", 20, 35);
    }
    
    private float GetPerlinGridValue(int GridIdX, int GridIdY, int GridIdZ, PVector ToGridVert)
    {
        int v1 = permTable[GridIdX];
        int v2 = permTable[v1 + GridIdY];
        int v3 = permTable[v2 + GridIdZ];
        
        switch (v3 & 0xF) // v3 % 16
        {
        case 0:
          return  ToGridVert.x + ToGridVert.y;// dot(ToGridVert, float3(1,1,0))
        case 1:     
          return -ToGridVert.x + ToGridVert.y;// dot(ToGridVert, float3(-1,1,0))
        case 2:     
          return  ToGridVert.x - ToGridVert.y;// dot(ToGridVert, float3(1,-1,0))
        case 3:
          return -ToGridVert.x - ToGridVert.y;// dot(ToGridVert, float3(-1,-1,0))
        case 4:     
          return  ToGridVert.x + ToGridVert.z;// dot(ToGridVert, float3(1,0,1))
        case 5:     
          return -ToGridVert.x + ToGridVert.z;// dot(ToGridVert, float3(-1,0,1))
        case 6:     
          return  ToGridVert.x - ToGridVert.z;// dot(ToGridVert, float3(1,0,-1))
        case 7:     
          return -ToGridVert.x - ToGridVert.z;// dot(ToGridVert, float3(-1,0,-1))
        case 8:     
          return  ToGridVert.y + ToGridVert.z;// dot(ToGridVert, float3(0,1,1))
        case 9:     
          return -ToGridVert.y + ToGridVert.z;// dot(ToGridVert, float3(0,-1,1))
        case 10:  
          return  ToGridVert.y - ToGridVert.z;// dot(ToGridVert, float3(0,1,-1))
        case 11:   
          return -ToGridVert.y - ToGridVert.z;// dot(ToGridVert, float3(0,-1,-1))
        case 12:   
          return  ToGridVert.y + ToGridVert.x;// dot(ToGridVert, float3(1,1,0))
        case 13:   
          return -ToGridVert.y + ToGridVert.z;// dot(ToGridVert, float3(0,-1,1))
        case 14:   
          return  ToGridVert.y - ToGridVert.x;// dot(ToGridVert, float3(-1,1,0))
        case 15:   
          return -ToGridVert.y - ToGridVert.z;// dot(ToGridVert, float3(0,-1,-1))
        default:   
          return 0.0f;
        }
    }
    
    private float PerlinSmoothValue(float InVal)
    {
        return InVal*InVal*InVal*(InVal* (6.0*InVal - 15.0) + 10.0);// y= 6x^5 - 15x^4 + 10x^3
    }
    
    private float GetPerlinValue(int x, int y)
    {
        return GetPerlinValue(x, y, 0, 0);
    }
    
    private float GetPerlinValue(int x, int y, int offsetX, int offsetY)
    {
        int GridIdY = (y / GridSizeY) % NumGridsY;
        int GridIdX = (x / GridSizeX) % NumGridsX;
        int NextGridIdY = (GridIdY + 1) % NumGridsY;
        int NextGridIdX = (GridIdX + 1) % NumGridsX;
        
        GridIdY += offsetY;
        GridIdX += offsetX;
        NextGridIdY += offsetY;
        NextGridIdX += offsetX;
        
        PVector UVInGrid = new PVector(0.0, 0.0);
        PVector SmoothedUVInGrid = new PVector(0.0, 0.0);
        
        UVInGrid.x = float(x % GridSizeX) / float(GridSizeX);
        UVInGrid.y = float(y % GridSizeY) / float(GridSizeY);
        
        SmoothedUVInGrid.y = PerlinSmoothValue(UVInGrid.y);
        SmoothedUVInGrid.x = PerlinSmoothValue(UVInGrid.x);
        
        float val00 = GetPerlinGridValue(GridIdX, GridIdY, 0, UVInGrid);
        float val10 = GetPerlinGridValue(NextGridIdX, GridIdY, 0, PVector.sub(UVInGrid, GridVert10));
        float val01 = GetPerlinGridValue(GridIdX, NextGridIdY, 0, PVector.sub(UVInGrid, GridVert01));
        float val11 = GetPerlinGridValue(NextGridIdX, NextGridIdY, 0, PVector.sub(UVInGrid, GridVert11));
        
        float lerp0 = lerp(val00, val10, SmoothedUVInGrid.x);
        float lerp1 = lerp(val01, val11, SmoothedUVInGrid.x);
        float finalLerp = lerp(lerp0, lerp1, SmoothedUVInGrid.y);
        
        return finalLerp;
    }
    
    private void DrawCurlArrow()
    {
        stroke(0);
        fill(255);
        int step = 10;
        for (int y = 0; y < PicHeight; y+=step)
        {
            for (int x = 0; x < PicWidth; x+=step)
            {
                int index = x + y * PicWidth;                
                PVector curlDir = CurlList.get(index);
                PVector start = new PVector(x+DisplayOffX, y+DisplayOffY);
                DrawArrow(start, curlDir, step*0.8);
            }
        }
    }
    
    private void DrawArrow(PVector start, PVector dir, float len)
    {
        float endX = start.x + len*dir.x;
        float endY = start.y + len*dir.y;
        
        //ellipse(start.x, start.y, 5.0, 5.0);
        line(start.x, start.y, endX, endY);
        
        PVector dir2 = dir.copy();
        dir2.rotate(PI/12.0);
        float wing1EndX = endX - 8.0*dir2.x;
        float wing1EndY = endY - 8.0*dir2.y;
        dir2.rotate(-PI/6.0);
        float wing2EndX = endX - 8.0*dir2.x;
        float wing2EndY = endY - 8.0*dir2.y;
        
        triangle(endX, endY, wing1EndX, wing1EndY, wing2EndX, wing2EndY);
    }
};
