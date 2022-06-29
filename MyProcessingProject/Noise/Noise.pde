ValueNoise valNoise;
SimpleNoise simpleNoise;
PerlinNoise perlinNoise;

void setup()
{
    size(600, 600, P2D);
    
    //valNoise = new ValueNoise(500, 500, 10.0);
    //valNoise.SetDisplayOffset(50, 50);
    
    //simpleNoise = new SimpleNoise(500, 500);
    //simpleNoise.SetDisplayOffset(50, 50);
    
    perlinNoise = new PerlinNoise(500, 500, 20.0);
    perlinNoise.SetDisplayOffset(50, 50);
}

void draw()
{ 
    background(205);
  
    //valNoise.Display();
    
    //simpleNoise.Display();
    
    perlinNoise.Display();
}
 
 
void mouseClicked()
{
    //valNoise.OnMouseClick();
    
    //simpleNoise.OnMouseClick();
    
    perlinNoise.OnMouseClick();
}
