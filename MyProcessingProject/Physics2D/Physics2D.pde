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
BorderArea border;

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
    
    border = new BorderArea();
    
    int numOfObjs = 1;
    float radius = 10.0;
    objects = new ArrayList<MoveObject>();
    
    PVector rangeX = new PVector(
        border.AreaWidX+radius,
        width-border.AreaWidX-radius
    );
    PVector rangeY = new PVector(
        border.AreaWidY+radius,
        height-border.AreaWidY-radius
    );
    for (int i = 0; i < numOfObjs; ++i)
    {
        float randPosX = random(rangeX.x, rangeX.y);
        float randPosY = random(rangeY.x, rangeY.y);
        objects.add(new MoveObject(randPosX, randPosY, radius));
    }
    
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
    
    int numOfObjs = objects.size();
    for (int i = 0; i < numOfObjs; ++i)
    {
        MoveObject obj = objects.get(i);
        obj.update();
        
        //collision detect
        /*PVector hitNormal = new PVector();
        for (int j = i+1; j < numOfObjs; ++j)
        {
            MoveObject other = objects.get(j);
            if (obj.collisionDetect(other, hitNormal))
            {
                obj.onHitObject(other, hitNormal);
            }
        }
        
        if (obj.collisionDetect(border, hitNormal))
        {
            obj.onHitBorder(hitNormal);
        }*/
    }
    
    //draw
    border.display();
    for (int i = 0; i < numOfObjs; ++i)
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
