class MoveAgent
{
    PVector location;
    PVector velocity;
    PVector acceleration;
    
    float mass;
    float maxAccel;
    float maxVelo;
    float damp;
    
    boolean bInBorder;
    PVector randomFactor;
    
    color fillCol;
    float agentSize;
    
    MoveAgent(float posX, float posY)
    {
        location = new PVector(posX, posY);
        velocity = new PVector();
        acceleration = new PVector();
        
        mass = 1.0;
        maxAccel = 10.0;
        maxVelo = 20.0;
        damp = 0.998;
        
        bInBorder = false;
        randomFactor = new PVector(); 
        
        fillCol = color(153, 153, 153);
        agentSize = 20.0;
    }
    
    protected void doAction()
    {
        // do something
    }
    
    void addForce(PVector force)
    {
        acceleration.add(PVector.div(force, mass));
    }
    
    void update()
    {
        doAction();
        
        acceleration.limit(maxAccel);
        velocity.add(acceleration);
        velocity.mult(damp);
        velocity.limit(maxVelo);
        location.add(velocity); 
        acceleration.set(0.0, 0.0);
    }
    
    void display()
    {
        ellipseMode(CENTER);
        stroke(0);
        fill(fillCol);
        
        PVector veloDir = velocity.copy();
        veloDir.normalize();
        float rotation = atan2(veloDir.y, veloDir.x);
        
        pushMatrix();
        translate(location.x, location.y);
        rotate(rotation);
        ellipse(0, 0, agentSize, agentSize);
        line(0, 0, agentSize*0.5, 0);
        popMatrix();
    }
};
