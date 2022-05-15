class Confetti extends Particle
{
    color filledCol;
    
    float angleVelo;
    float angle;
  
    Confetti(PVector loc)
    {
        super(loc);
    }
    
    Confetti(float x, float y, float life)
    {
        super(x, y, life);
    }
    
    protected void init(PVector loc, float life)
    {
        super.init(loc, life);
        
        angleVelo = random(-0.5, 0.5);
        
        int R = floor(random(256));
        int G = floor(random(256));
        int B = floor(random(256));
        filledCol = color(R, G, B);
    }
    
    void update()
    {
        super.update();
        
        angle += angleVelo;
    }
    
    void display()
    {
        stroke(0);
        fill(filledCol);
        rectMode(CENTER);
        
        pushMatrix();
        translate(location.x, location.y);
        rotate(angle);
        rect(0, 0, radius, radius);
        popMatrix();
    }
};
