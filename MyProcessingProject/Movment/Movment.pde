ArrayList<Seeker> seekers;
BorderArea border;
FlowField field;

void setup()
{
    size(800, 600);
    
    int numOfSeekers = 10;
    seekers = new ArrayList<Seeker>();
    for (int i = 0; i < numOfSeekers; ++i)
    {
        float randPosX = random(width*0.2,width*0.8);
        float randPosY = random(height*0.2,height*0.8);
        
        int type = floor(random(2));
        switch(type)
        {
            case 1:
            seekers.add(new Wanderer(randPosX, randPosY));
            break;
          
            default:
            case 0:
            seekers.add(new Seeker(randPosX, randPosY));
            break;
        }
    }
    
    border = new BorderArea();
    field = new FlowField(width*0.5, height*0.5, width-2.0*border.AreaWidX, height-2.0*border.AreaWidY, 15.0);
}

void draw()
{
    background(205);
  
    int numOfSeekers = seekers.size();
    for (int i = 0; i< numOfSeekers; ++i)
    {
        Seeker curSeeker = seekers.get(i);
        curSeeker.setTarget(mousePressed, mouseX, mouseY);
        border.applyBorderAreaForce(curSeeker);
        field.applyFlowForce(curSeeker);
        curSeeker.update();
    }
    
    border.display();
    field.display();
    for (int i = 0; i< numOfSeekers; ++i)
    {
        Seeker curSeeker = seekers.get(i);
        curSeeker.display();
    }
}
