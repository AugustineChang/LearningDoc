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
    size(800, 800, P2D);
    
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
            simpleNoise = new SimpleNoise(width-100, height-100);
            simpleNoise.SetDisplayOffset(50, 50);
        }
        simpleNoise.Display();
        break;
        
        case 1:
        if (valNoise == null)
        {
            valNoise = new ValueNoise(width-100, height-100, 5.0);
            valNoise.SetDisplayOffset(50, 50);
        }
        valNoise.Display();
        break;
        
        case 2:
        if (perlinNoise == null)
        {
            perlinNoise = new PerlinNoise(width-100, height-100, 5.0);
            perlinNoise.SetDisplayOffset(50, 50);
        }
        perlinNoise.Display();
        break;
        
        case 3:
        if (cellNoise == null)
        {
            cellNoise = new CellNoise(width-100, height-100, 20);
            cellNoise.SetDisplayOffset(50, 50);
        }
        cellNoise.Display();
        break;
        
        case 4:
        if (worleyNoise == null)
        {
            worleyNoise = new WorleyNoise(width-100, height-100, 5.0);
            worleyNoise.SetDisplayOffset(50, 50);
        }
        worleyNoise.Display();
        break;
        
        case 5:
        if (octavesPerlin == null)
        {
            octavesPerlin = new OctavesPerlinNoise(width-100, height-100);
            octavesPerlin.SetDisplayOffset(50, 50);
        }
        octavesPerlin.Display();
        break;
        
        case 6:
        if (curlNoise == null)
        {
            curlNoise = new CurlNoise(width-100, height-100, 10.0);
            curlNoise.SetDisplayOffset(50, 50);
        }
        curlNoise.Display();
        break;
        
        default:
        {  
            fill(255);
            textSize(50);
            int posY = 60;
            text("Noise Learning Tool:", 30, posY);
            textSize(40);
            posY += 90;
            text("press \'1\' show SimpleNoise", 70, posY);
            posY += 65;
            text("press \'2\' show ValueNoise", 70, posY);
            posY += 65;
            text("press \'3\' show PerlinNoise", 70, posY);
            posY += 65;
            text("press \'4\' show CellNoise", 70, posY);
            posY += 65;
            text("press \'5\' show WorleyNoise", 70, posY);
            posY += 65;
            text("press \'6\' show OctavesPerlinNoise", 70, posY);
            posY += 65;
            text("press \'7\' show CurlNoise", 70, posY);
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
