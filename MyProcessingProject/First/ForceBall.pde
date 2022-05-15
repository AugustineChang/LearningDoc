class ForceBall
{
    Mover ball;
    
    boolean isEnable;
    boolean isAttract;
    float forceFactor;
  
    ForceBall(float x, float y)
    {
        ball = new Mover(x, y, 50.0);
        isEnable = false;
        isAttract = true;
        forceFactor = 5000;
    }
    
    void addForceTo(Particle p)
    {
        if (!isEnable)
          return;
      
        PVector diffV = PVector.sub(ball.pos, p.location);
        float dist = diffV.mag();
        if (dist >= ball.radius)
        {
            float force = (forceFactor * ball.mass * p.mass) / dist;
            force *= isAttract ? 1.0 : -1.0;
            
            diffV.normalize();
            diffV.mult(force);
            p.addForce(diffV);
        }
    }
    
    void update()
    {
        isEnable = mousePressed;
        isAttract = mouseButton == LEFT;
      
        if (isEnable)
          ball.pos.set(mouseX, mouseY);
    }
    
    void display()
    {
        if (isEnable)
          ball.display();
    }
};
