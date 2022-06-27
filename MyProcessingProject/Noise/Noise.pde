ValueNoise valNoise;

void setup()
{
    size(600, 600, P2D);
    
    valNoise = new ValueNoise(500, 500, 10.0);
    valNoise.SetDisplayOffset(50, 50);
}

void draw()
{ 
    background(205);
  
    valNoise.Display();
}
 
 
void mouseClicked()
{
    valNoise.OnMouseClick();
}
