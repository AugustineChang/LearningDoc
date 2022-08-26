class Particle
{
    PVector location;
    PVector volecity;
    PVector acceleration;
    
    float radius;
    float lifetime;
    float mass;
    
    Particle(PVector loc)
    {
        init(loc, -1.0);
    }
    
    Particle(float x, float y, float life)
    {
        init(new PVector(x, y), life);
    }
    
    protected void init(PVector loc, float life)
    {
        location = loc;
        volecity = new PVector();
        acceleration = new PVector();
        
        radius = 8.0;
        lifetime = life;
        mass = PI * radius * radius * 0.0001;
    }
    
    void setVelosity(ArrayList<PVector> flowMap, int PicWidth, int PicHeight)
    {
        if (location.x < 0 || location.x > PicWidth-1 ||
            location.y < 0 || location.y > PicHeight-1)
            return;
        
        int index = round(location.x) + round(location.y) * PicWidth;                
        PVector curlDir = flowMap.get(index);
        volecity.set(PVector.mult(curlDir, 1));
    }
    
    void addForce(PVector force)
    {
        acceleration.add(force);
    }
    
    void addForce(float forceX, float forceY)
    {
        addForce(new PVector(forceX, forceY));
    }
    
    boolean isDead(float maxX, float maxY)
    {  
        return (-0.5f < lifetime && lifetime <= 0.0) || 
        (location.x - radius > maxX || location.x + radius < 0.0 ||
        location.y - radius > maxY || location.y + radius < 0.0);
    }
    
    void update()
    {
        volecity.add(acceleration);
        location.add(volecity);
        acceleration.set(0.0, 0.0);
        
        if (lifetime > 0.0)
          lifetime = max(0.0, lifetime - 1.0);
    }
    
    void display(PVector offset)
    {
        stroke(0);
        fill(175);
        ellipse(offset.x+location.x, offset.y+location.y, radius, radius);
    }
};
