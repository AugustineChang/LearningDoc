static float PixelToMeterFactor = 0.01;
static float MeterToPixelFactor = 100.0;

static float PixelToMeter(float inPixel)
{
    return inPixel * PixelToMeterFactor;
}

static PVector PixelToMeter(float inPixelX, float inPixelY)
{
    return new PVector(
        PixelToMeter(inPixelX), 
        PixelToMeter(inPixelY)
    );
}

static float MeterToPixel(float inMeter)
{
    return inMeter * MeterToPixelFactor;
}

static PVector MeterToPixel(PVector inMeter)
{
    return new PVector(
        MeterToPixel(inMeter.x),
        MeterToPixel(inMeter.y)
    );
}

//////////////////////////////////////////////////

ArrayList<MoveObject> objects;
ArrayList<StaticObject> obstacles;

float Time;
float DeltaTime;
float Restitution;
PVector Gravity;
float AirDensity;
float DragCoefficient;

float WindSpeed;

void setup()
{
    size(800, 800, P2D);
    
    obstacles = new ArrayList<StaticObject>();
    BorderArea border = new BorderArea();
    obstacles.add(border);
    
    int numOfObjs = 50;
    float radius = 5.0;
    objects = new ArrayList<MoveObject>();
    
    PVector rangeX = new PVector(
        border.AreaWidX+radius,
        width-border.AreaWidX-radius
    );
    PVector rangeY = new PVector(
        border.AreaWidY+radius,
        height*0.5
    );
    for (int i = 0; i < numOfObjs; ++i)
    {
        float randPosX = random(rangeX.x, rangeX.y);
        float randPosY = random(rangeY.x, rangeY.y);
        objects.add(new MoveObject(randPosX, randPosY, radius));
    }
    //objects.add(new MoveObject(width*0.5, height*0.5-100, radius));
    
    obstacles.add(new SphereObstacle(width*0.5 - 150.0, height*0.5 + 20.0, 50.0));
    obstacles.add(new SphereObstacle(width*0.5 - 90.0, height*0.5 + 70.0, 50.0));
    obstacles.add(new SphereObstacle(width*0.5 - 30.0, height*0.5 + 100.0, 50.0));
    obstacles.add(new SphereObstacle(width*0.5 + 30.0, height*0.5 + 100.0, 50.0));
    obstacles.add(new SphereObstacle(width*0.5 + 90.0, height*0.5 + 70.0, 50.0));
    obstacles.add(new SphereObstacle(width*0.5 + 150.0, height*0.5 + 20.0, 50.0));
    
    Time = 0.0f;
    DeltaTime = 0.0f;
    Restitution = 0.8f;
    Gravity = new PVector(0.0, 9.8f);
    AirDensity = 1.23f;
    DragCoefficient = 0.6f;
    
    WindSpeed = 10.0f;
}


void draw()
{
    background(205);
    
    DeltaTime = 1.0 / frameRate;
    Time += DeltaTime;
    
    int numOfMovObjs = objects.size();
    for (int i = 0; i < numOfMovObjs; ++i)
    {
        MoveObject obj = objects.get(i);
        obj.update();
    }
    
    //draw
    int numOfStaObjs = obstacles.size();
    for (int i = 0; i < numOfStaObjs; ++i)
    {
        obstacles.get(i).display();
    }
    for (int i = 0; i < numOfMovObjs; ++i)
    {
        objects.get(i).display();
    }
    
    textSize(20);
    fill(0);
    String timeText = "Time = "+Time;
    text(timeText, 20.0, 30.0);
    String frameText = "FrameRate = "+frameRate;
    text(frameText, 20.0, 55.0);
}
