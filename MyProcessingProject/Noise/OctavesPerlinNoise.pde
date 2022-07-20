class OctavesPerlinNoise extends NoiseBase
{
    private int[] permTable;
    private int GeneratedState;
    
    OctavesPerlinNoise(int inWidth, int inHeight)
    {
        super(inWidth, inHeight, 0.0);
        
        // init perm table
        permTable = new int[512];
        for (int i = 0; i < 256; ++i)
        {
            permTable[i] = permutation[i];
            permTable[256+i] = permutation[i];
        }
        
        GeneratedState = DisplayState;
        MAX_STATE = 7;
        
        GenerateNoise(DisplayState+1);
    }
    
    void GenerateNoise(int NumOfLayers)
    {
        NoiseImage.loadPixels();
        
        int NumOfPixels = NoiseImage.pixels.length;
        FloatList TempList = new FloatList();
        for (int i = 0; i < NumOfPixels; ++i)
            TempList.append(0.0);
        
        float Amplitude = 1.0;
        Frequency = 2.0;
        for (int i = 0; i < NumOfLayers; ++i)
        {
            GenerateOneNoise(Amplitude, TempList);
            
            Amplitude *= 0.5;
            Frequency *= 2.0;
        }
        
        for (int i = 0; i < NumOfPixels; ++i)
        {
            // octaves perlin range(-1.984375, 1.984375), but min/max value is very rare
            // we still use (-1,1)
            float finalLerp = (0.5 + 0.5*TempList.get(i)) * 255.0;
            NoiseImage.pixels[i] = color(finalLerp, finalLerp, finalLerp);
        }
        
        NoiseImage.updatePixels();
    }
    
    private void GenerateOneNoise(float Amplitude, FloatList OutList)
    {
        int GridSizeX = floor(PicWidth / Frequency);
        int GridSizeY = floor(PicHeight / Frequency);
        int NumGridsX = ceil(float(PicWidth) / float(GridSizeX));
        int NumGridsY = ceil(float(PicHeight) / float(GridSizeY));
        
        int GridIdX = 0;
        int GridIdY = 0;
        int NextGridIdX = 0;
        int NextGridIdY = 0;             
        PVector UVInGrid = new PVector(0.0, 0.0);
        PVector SmoothedUVInGrid = new PVector(0.0, 0.0);
        PVector GridVert10 = new PVector(1.0, 0.0);
        PVector GridVert01 = new PVector(0.0, 1.0);
        PVector GridVert11 = new PVector(1.0, 1.0);
        
        for (int y = 0; y < PicHeight; ++y)
        {
            GridIdY = y / GridSizeY;
            UVInGrid.y = float(y % GridSizeY) / float(GridSizeY);
            NextGridIdY = (GridIdY + 1) % NumGridsY;
            SmoothedUVInGrid.y = PerlinSmoothValue(UVInGrid.y);
                
            for (int x = 0; x < PicWidth; ++x)
            {
                GridIdX = x / GridSizeX;
                UVInGrid.x = float(x % GridSizeX) / float(GridSizeX);
                NextGridIdX = (GridIdX + 1) % NumGridsX;
                SmoothedUVInGrid.x = PerlinSmoothValue(UVInGrid.x);
                
                float val00 = GetPerlinGridValue(GridIdX, GridIdY, 0, UVInGrid);
                float val10 = GetPerlinGridValue(NextGridIdX, GridIdY, 0, PVector.sub(UVInGrid, GridVert10));
                float val01 = GetPerlinGridValue(GridIdX, NextGridIdY, 0, PVector.sub(UVInGrid, GridVert01));
                float val11 = GetPerlinGridValue(NextGridIdX, NextGridIdY, 0, PVector.sub(UVInGrid, GridVert11));
                
                float lerp0 = lerp(val00, val10, SmoothedUVInGrid.x);
                float lerp1 = lerp(val01, val11, SmoothedUVInGrid.x);
                float finalLerp = lerp(lerp0, lerp1, SmoothedUVInGrid.y);
                
                int index = x + y * PicWidth;
                OutList.set(index, OutList.get(index) + finalLerp * Amplitude);
            }
        }
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
    
    void Display()
    {
        super.Display();
        
        fill(255);
        textSize(30);
        text("Octaves Perlin Noise", 20, 35);
        text("Additive Layers: " + (DisplayState+1), 360, 35);
    }
    
    void OnMouseClick()
    {
        super.OnMouseClick();
        
        if (DisplayState != GeneratedState)
        {
            GenerateNoise(DisplayState+1);
            GeneratedState = DisplayState;
        }
    }
};
