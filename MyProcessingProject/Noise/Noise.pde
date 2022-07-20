ValueNoise valNoise;
SimpleNoise simpleNoise;
PerlinNoise perlinNoise;
CellNoise cellNoise;
WorleyNoise worleyNoise;
OctavesPerlinNoise octavesPerlin;
CurlNoise curlNoise;

int showIndex;

void setup()
{
    size(600, 600, P2D);
    
    showIndex = -1;
}

void draw()
{ 
    background(205);
    
    switch(showIndex)
    {
        case 0:
        if (simpleNoise == null)
        {
            simpleNoise = new SimpleNoise(500, 500);
            simpleNoise.SetDisplayOffset(50, 50);
        }
        simpleNoise.Display();
        break;
        
        case 1:
        if (valNoise == null)
        {
            valNoise = new ValueNoise(500, 500, 5.0);
            valNoise.SetDisplayOffset(50, 50);
        }
        valNoise.Display();
        break;
        
        case 2:
        if (perlinNoise == null)
        {
            perlinNoise = new PerlinNoise(500, 500, 5.0);
            perlinNoise.SetDisplayOffset(50, 50);
        }
        perlinNoise.Display();
        break;
        
        case 3:
        if (cellNoise == null)
        {
            cellNoise = new CellNoise(500, 500, 20);
            cellNoise.SetDisplayOffset(50, 50);
        }
        cellNoise.Display();
        break;
        
        case 4:
        if (worleyNoise == null)
        {
            worleyNoise = new WorleyNoise(500, 500, 5.0);
            worleyNoise.SetDisplayOffset(50, 50);
        }
        worleyNoise.Display();
        break;
        
        case 5:
        if (octavesPerlin == null)
        {
            octavesPerlin = new OctavesPerlinNoise(500, 500);
            octavesPerlin.SetDisplayOffset(50, 50);
        }
        octavesPerlin.Display();
        break;
        
        case 6:
        if (curlNoise == null)
        {
            curlNoise = new CurlNoise(500, 500, 10.0);
            curlNoise.SetDisplayOffset(50, 50);
        }
        curlNoise.Display();
        break;
        
        default:
        {  
            fill(255);
            textSize(40);
            int posY = 60;
            text("Noise Learning Tool:", 20, posY);
            textSize(30);
            posY += 80;
            text("press \'1\' show SimpleNoise", 60, posY);
            posY += 50;
            text("press \'2\' show ValueNoise", 60, posY);
            posY += 50;
            text("press \'3\' show PerlinNoise", 60, posY);
            posY += 50;
            text("press \'4\' show CellNoise", 60, posY);
            posY += 50;
            text("press \'5\' show WorleyNoise", 60, posY);
            posY += 50;
            text("press \'6\' show OctavesPerlinNoise", 60, posY);
            posY += 50;
            text("press \'7\' show CurlNoise", 60, posY);
        }
        break;
    }
}
 
 
void mouseClicked()
{
    switch(showIndex)
    {
        case 0:
        simpleNoise.OnMouseClick();
        break;
        
        case 1:
        valNoise.OnMouseClick();
        break;
        
        case 2:
        perlinNoise.OnMouseClick();
        break;
        
        case 3:
        cellNoise.OnMouseClick();
        break;
        
        case 4:
        worleyNoise.OnMouseClick();
        break;
        
        case 5:
        octavesPerlin.OnMouseClick();
        break;
        
        case 6:
        curlNoise.OnMouseClick();
        break;
        
        default:
        break;
    }
}

void keyPressed()
{
    if (key >= '1' && key <= '7')
    {
        showIndex = key - '1';
    }
    else
        showIndex = -1;
}
