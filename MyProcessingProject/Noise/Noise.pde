ValueNoise valNoise;
SimpleNoise simpleNoise;
PerlinNoise perlinNoise;
CellNoise cellNoise;
WorleyNoise worleyNoise;
OctavesPerlinNoise octavesPerlin;
CurlNoise curlNoise;

void setup()
{
    size(600, 600, P2D);
    
    //valNoise = new ValueNoise(500, 500, 5.0);
    //valNoise.SetDisplayOffset(50, 50);
    
    //simpleNoise = new SimpleNoise(500, 500);
    //simpleNoise.SetDisplayOffset(50, 50);
    
    //perlinNoise = new PerlinNoise(500, 500, 5.0);
    //perlinNoise.SetDisplayOffset(50, 50);
    
    //cellNoise = new CellNoise(500, 500, 20);
    //cellNoise.SetDisplayOffset(50, 50);
    
    //worleyNoise = new WorleyNoise(500, 500, 5.0);
    //worleyNoise.SetDisplayOffset(50, 50);
    
    //octavesPerlin = new OctavesPerlinNoise(500, 500);
    //octavesPerlin.SetDisplayOffset(50, 50);
    
    curlNoise = new CurlNoise(500, 500, 10.0);
    curlNoise.SetDisplayOffset(50, 50);
}

void draw()
{ 
    background(205);
  
    //valNoise.Display();
    
    //simpleNoise.Display();
    
    //perlinNoise.Display();
    
    //cellNoise.Display();
    
    //worleyNoise.Display();
    
    //octavesPerlin.Display();
    
    curlNoise.Display();
}
 
 
void mouseClicked()
{
    //valNoise.OnMouseClick();
    
    //simpleNoise.OnMouseClick();
    
    //perlinNoise.OnMouseClick();
    
    //cellNoise.OnMouseClick();
    
    //worleyNoise.OnMouseClick();
    
    //octavesPerlin.OnMouseClick();
    
    curlNoise.OnMouseClick();
}
