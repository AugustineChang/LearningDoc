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
    //private ArrayList<PVector> RandDirs;
    
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
        
        MAX_STATE = 12;
        
        // init perm table
        permTable = new int[512];
        for (int i = 0; i < 256; ++i)
        {
            permTable[i] = permutation[i];
            permTable[256+i] = permutation[i];
        }
        
        //random grids
        /*RandDirs = new ArrayList<PVector>();
        for (int y = 0; y < NumGridsY; ++y)
        {
            for (int x = 0; x < NumGridsX; ++x)
            {
                RandDirs.add(PVector.random2D());
            }
        }*/
        
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
    
    /*private float GetPerlinGridValue2(int GridIdX, int GridIdY, int GridIdZ, PVector ToGridVert)
    {
        PVector randDir = RandDirs.get(GridIdX + GridIdY*NumGridsX);
        
        return PVector.dot(randDir, ToGridVert);
    }*/
    
    private PVector GetPerlinGridDir(int GridIdX, int GridIdY, int GridIdZ)
    {
        int v1 = permTable[GridIdX];
        int v2 = permTable[v1 + GridIdY];
        int v3 = permTable[v2 + GridIdZ];
        
        PVector OutVec;
        switch (v3 & 0xF) // v3 % 16
        {
        case 0:
          OutVec = new PVector(1,1,0); // dot(ToGridVert, float3(1,1,0))
          break;
        case 1:     
          OutVec = new PVector(-1,1,0); // dot(ToGridVert, float3(-1,1,0))
          break;
        case 2:     
          OutVec = new PVector(1,-1,0); // dot(ToGridVert, float3(1,-1,0))
          break;
        case 3:
          OutVec = new PVector(-1,-1,0); // dot(ToGridVert, float3(-1,-1,0))
          break;
        case 4:     
          OutVec = new PVector(1,0,1); // dot(ToGridVert, float3(1,0,1))
          break;
        case 5:     
          OutVec = new PVector(-1,0,1); // dot(ToGridVert, float3(-1,0,1))
          break;
        case 6:     
          OutVec = new PVector(1,0,-1); // dot(ToGridVert, float3(1,0,-1))
          break;
        case 7:     
          OutVec = new PVector(-1,0,-1); // dot(ToGridVert, float3(-1,0,-1))
          break;
        case 8:     
          OutVec = new PVector(0,1,1); // dot(ToGridVert, float3(0,1,1))
          break;
        case 9:     
          OutVec = new PVector(0,-1,1); // dot(ToGridVert, float3(0,-1,1))
          break;
        case 10:  
          OutVec = new PVector(0,1,-1); // dot(ToGridVert, float3(0,1,-1))
          break;
        case 11:   
          OutVec = new PVector(0,-1,-1); // dot(ToGridVert, float3(0,-1,-1))
          break;
        case 12:   
          OutVec = new PVector(1,1,0); // dot(ToGridVert, float3(1,1,0))
          break;
        case 13:   
          OutVec = new PVector(1,-1,1); // dot(ToGridVert, float3(0,-1,1))
          break;
        case 14:   
          OutVec = new PVector(-1,1,0); // dot(ToGridVert, float3(-1,1,0))
          break;
        
        default: 
        case 15:   
          OutVec = new PVector(0,-1,-1); // dot(ToGridVert, float3(0,-1,-1))
          break;
        }
        
        float size2D = sqrt(OutVec.x*OutVec.x + OutVec.y*OutVec.y);
        OutVec.div(size2D);//normalize2D
                
        return OutVec;
    }
    
    private float PerlinSmoothValue(float InVal)
    {
        return InVal*InVal*InVal*(InVal* (6.0*InVal - 15.0) + 10.0);// y= 6x^5 - 15x^4 + 10x^3
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
            DrawGrids();
            break;
            
            case 2:
            noStroke();
            fill(0);
            rect(DisplayOffX, DisplayOffY, PicWidth, PicHeight);
            DrawGrids();
            DrawGridDirections();
            break;
            
            case 3:
            case 4:
            case 5:
            case 6:
            case 7:
            case 8:
            case 9:
            noStroke();
            fill(0);
            rect(DisplayOffX, DisplayOffY, PicWidth, PicHeight);
            DrawGrids();
            DrawInterpolation();
            break;
            
            case 10:
            image(NoiseImage, DisplayOffX, DisplayOffY);
            DrawGrids();
            DrawInterpolation();
            break;
            
            case 11:
            image(NoiseImage, DisplayOffX, DisplayOffY);
            break;
        }
        
        fill(255);
        textSize(30);
        text("Perlin Noise", 20, 35);
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
    
    private void DrawGridDirections()
    {
        stroke(0, 185, 255);
        fill(0, 185, 255);
        ellipseMode(CENTER);
        
        float arrowLen = (GridSizeX+GridSizeY)*0.2;
        for (int y = 0; y < NumGridsY; ++y)
        {
            int centerY = y*GridSizeY + DisplayOffY;
            for (int x = 0; x < NumGridsX; ++x)
            {
                int centerX = x*GridSizeX + DisplayOffX;
                PVector gridDir = GetPerlinGridDir(x, y, 0);
                
                DrawArrow(new PVector(centerX, centerY, 0), gridDir, arrowLen);
            }
        }
    }
    
    private void DrawArrow(PVector start, PVector dir, float len)
    {
        float endX = start.x + len*dir.x;
        float endY = start.y + len*dir.y;
        
        ellipse(start.x, start.y, 5.0, 5.0);
        line(start.x, start.y, endX, endY);
        
        dir.rotate(PI/12.0);
        float wing1EndX = endX - 12.0*dir.x;
        float wing1EndY = endY - 12.0*dir.y;
        dir.rotate(-PI/6.0);
        float wing2EndX = endX - 12.0*dir.x;
        float wing2EndY = endY - 12.0*dir.y;
        
        triangle(endX, endY, wing1EndX, wing1EndY, wing2EndX, wing2EndY);
    }
    
    private void DrawInterpolation()
    {
        stroke(0, 185, 255);
        ellipseMode(CENTER);
        rectMode(CENTER);
        
        PVector pointPos = new PVector(
          floor(GridSizeX*0.5) + DisplayOffX, 
          floor(GridSizeY*0.5) + DisplayOffY);
      
        if (DisplayState >= 3 && DisplayState < 9)
        {  
            fill(0, 185, 255);
            ellipse(pointPos.x, pointPos.y, 5.0, 5.0);
            fill(255);
            textSize(20);
            text("?", pointPos.x+4, pointPos.y+6);
        }
        
        if (DisplayState == 4)
        {
            fill(0, 185, 255);
            
            PVector gridPos = new PVector(DisplayOffX, DisplayOffY, 0);
            PVector gridDir = GetPerlinGridDir(0, 0, 0);
            float arrowLen = (GridSizeX+GridSizeY)*0.2;
            DrawArrow(gridPos, gridDir, arrowLen);
            
            PVector toPoint = PVector.sub(pointPos, gridPos);
            arrowLen = toPoint.mag();
            toPoint.div(arrowLen);
            DrawArrow(gridPos, toPoint, arrowLen);
            
            text("calc dot product", pointPos.x, pointPos.y-GridSizeY*0.25);
        }
        
        float val00 = 0.0;
        PVector toPos = new PVector();
        if (DisplayState >= 5)
        {
            PVector gridPos = new PVector(DisplayOffX, DisplayOffY, 0);
            toPos = PVector.sub(pointPos, gridPos);
            toPos.x = toPos.x / GridSizeX;
            toPos.y = toPos.y / GridSizeY;
            
            val00 = GetPerlinGridValue(0, 0, 0, toPos);
            if (DisplayState < 9)
            {
                fill((0.5 + 0.5*val00) * 255.0);
                ellipse(DisplayOffX, DisplayOffY, 11.0, 11.0);
            }
        }
        
        float val10 = 0.0;
        if (DisplayState >= 6 && DisplayState < 9)
        {
            val10 = GetPerlinGridValue(1, 0, 0, PVector.sub(toPos, new PVector(1, 0, 0)));
            if (DisplayState < 9)
            {
                fill((0.5 + 0.5*val10) * 255.0);
                ellipse(DisplayOffX+GridSizeX, DisplayOffY, 11.0, 11.0);
            }
        }
        
        float val01 = 0.0;
        if (DisplayState >= 7 && DisplayState < 9)
        {
            val01 = GetPerlinGridValue(0, 1, 0, PVector.sub(toPos, new PVector(0, 1, 0)));
            if (DisplayState < 9)
            {
                fill((0.5 + 0.5*val01) * 255.0);
                ellipse(DisplayOffX, DisplayOffY+GridSizeY, 11.0, 11.0);
            }
        }
        
        float val11 = 0.0;
        if (DisplayState >= 8 && DisplayState < 9)
        {
            val11 = GetPerlinGridValue(1, 1, 0, PVector.sub(toPos, new PVector(1, 1, 0)));
            if (DisplayState < 9)
            {
                fill((0.5 + 0.5*val11) * 255.0);
                ellipse(DisplayOffX+GridSizeX, DisplayOffY+GridSizeY, 11.0, 11.0);
            }
        }
        
        if (DisplayState >= 9)
        {
            float SmoothedUVInGridX = PerlinSmoothValue(toPos.x);
            float SmoothedUVInGridY = PerlinSmoothValue(toPos.y);
          
            float lerp0 = lerp(val00, val10, SmoothedUVInGridX);
            float lerp1 = lerp(val01, val11, SmoothedUVInGridX);
            float finalLerp = lerp(lerp0, lerp1, SmoothedUVInGridY);
            
            fill((0.5 + 0.5*finalLerp) * 255.0);
            ellipse(pointPos.x, pointPos.x, 11.0, 11.0);
        }
    }
};
