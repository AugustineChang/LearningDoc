class ParticleSystem
{
    ArrayList<Particle> particles;
    
    int rangeX;
    int rangeY;
    PVector offset;
    
    ParticleSystem(int inWidth, int inHeight)
    {
        particles = new ArrayList<Particle>();
        offset = new PVector();
        
        rangeX = inWidth;
        rangeY = inHeight;
    }
    
    void addParticle()
    {
        float randPosX = random(0, rangeX);
        float randPosY = random(0, rangeY);
        
        float lifetime = -10.0;
        Particle one = new Particle(randPosX, randPosY, lifetime);
        
        particles.add(one);
    }
     
    void addForce(PVector force)
    {
        for (Particle p : particles)
        {
            p.addForce(force);
        }
    }
    
    void setVelosity(ArrayList<PVector> flowMap, int PicWidth, int PicHeight)
    {
        for (Particle p : particles)
        {
            p.setVelosity(flowMap, PicWidth, PicHeight);
        }
    }
    
    void update()
    {
        // add new particle
        if (particles.size() < 30)
        {
            addParticle();
        }
        
        // update particles
        int NumOfList = particles.size();
        for (int i = 0; i < NumOfList; ++i)
        {
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
        
        //println("Num Of Particles:%d", particles.size());
    }
    
    void display()
    {
        int NumOfList = particles.size();
        for (int i = 0; i < NumOfList; ++i)
        {
            particles.get(i).display(offset);
        }
    }
};
