static int[] permutation = { 151,160,137,91,90,15,
   131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,99,37,240,21,10,23,
   190, 6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,57,177,33,
   88,237,149,56,87,174,20,125,136,171,168, 68,175,74,165,71,134,139,48,27,166,
   77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,55,46,245,40,244,
   102,143,54, 65,25,63,161, 1,216,80,73,209,76,132,187,208, 89,18,169,200,196,
   135,130,116,188,159,86,164,100,109,198,173,186, 3,64,52,217,226,250,124,123,
   5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,189,28,42,
   223,183,170,213,119,248,152, 2,44,154,163, 70,221,153,101,155,167, 43,172,9,
   129,22,39,253, 19,98,108,110,79,113,224,232,178,185, 112,104,218,246,97,228,
   251,34,242,193,238,210,144,12,191,179,162,241, 81,51,145,235,249,14,239,107,
   49,192,214, 31,181,199,106,157,184, 84,204,176,115,121,50,45,127, 4,150,254,
   138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180
};

class PerlinNoise extends NoiseBase
{
    private int[] permTable;
    
    private int GridSizeX;
    private int GridSizeY;
    private int NumGridsX;
    private int NumGridsY;
    
    PerlinNoise(int inWidth, int inHeight, float inFraq)
    {
        super(inWidth, inHeight, inFraq);
        
        GridSizeX = floor(PicWidth / Frequency);
        GridSizeY = floor(PicHeight / Frequency);
        NumGridsX = ceil(float(PicWidth) / float(GridSizeX));
        NumGridsY = ceil(float(PicHeight) / float(GridSizeY));
        
        // init perm table
        permTable = new int[512];
        for (int i = 0; i < 256; ++i)
        {
            permTable[i] = permutation[i];
            permTable[256+i] = permutation[i];
        }
        
        GenerateNoise();
    }
    
    void GenerateNoise()
    {
        NoiseImage.loadPixels();
        
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
                
                finalLerp = (0.5 + 0.5*finalLerp) * 255.0;// perlin gridValue range(-1, 1)
                
                int index = x + y * PicWidth;
                NoiseImage.pixels[index] = color(finalLerp, finalLerp, finalLerp);
            }
        }
        
        NoiseImage.updatePixels();
    }
    
    void OnMouseClick()
    {
        super.OnMouseClick();
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
};
