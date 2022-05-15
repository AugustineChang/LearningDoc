
Pendulum pendu;
SpringPendulum sPendu;
ParticleSystem particleSys;
ForceBall fBall;

void setup()
{
    size(800, 600);
    
    //pendu = new Pendulum(400, 30, 200);
    //sPendu = new SpringPendulum(400, 30, 300);
    particleSys = new ParticleSystem(width*0.5, height*0.5);
    fBall = new ForceBall(0.0, 0.0);
}

void mousePressed()
{
    //particleSys.addParticle(mouseX, mouseY);
}

void draw()
{
    background(205);
  
    //pendu.update();
    //sPendu.update();
    fBall.update();
    particleSys.addForce(fBall);
    particleSys.update();
    
    //pendu.display();
    //sPendu.display();
    fBall.display();
    particleSys.display();
}
