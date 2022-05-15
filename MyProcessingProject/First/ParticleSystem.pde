class ParticleSystem
{
    ArrayList<Particle> particles;
    
    PVector origin;
    float gravity;
    
    ParticleSystem(float posX, float posY)
    {
        particles = new ArrayList<Particle>();
        
        origin = new PVector(posX, posY);
        gravity = 0.4;
    }
    
    void addParticle(float posX, float posY)
    {
        float randX = random(-5.0, 5.0);
        float randY = random(-10.0, 0.0);
        
        Particle one;
        int randType = floor(random(2));
        float lifetime = 100.0;
        switch(randType)
        {
            case 1:
              one = new Confetti(posX, posY,lifetime);
              break;
              
            default:
            case 0:
              one = new Particle(posX, posY, lifetime);
              break;
        }
        
        one.addForce(randX, randY);
        particles.add(one);
    }
     
    void addForce(PVector force)
    {
        for (Particle p : particles)
        {
            p.addForce(force);
        }
    }
    
    void addForce(ForceBall ball)
    {
        if (!ball.isEnable)
          return;
        
        for (Particle p : particles)
        {
            ball.addForceTo(p);
        }
    }
    
    void update()
    {
        // add new particle
        addParticle(origin.x, origin.y);
        
        // update particles
        int NumOfList = particles.size();
        for (int i = 0; i < NumOfList; ++i)
        {
            particles.get(i).addForce(0.0, gravity);
            particles.get(i).update();
        }
        
        // remove dead particles
        for (int i = NumOfList-1; i >= 0; --i)
        {
            Particle one = particles.get(i);
            if (one.isDead())
            {
                particles.remove(i);
            }
        }
        
        println("Num Of Particles:%d", particles.size());
    }
    
    void display()
    {
        int NumOfList = particles.size();
        for (int i = 0; i < NumOfList; ++i)
        {
            particles.get(i).display();
        }
    }
};
