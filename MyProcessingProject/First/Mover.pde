class Mover
{
  PVector pos;
  float radius;
  float mass;
  PVector velocity;
  PVector acceleration;
  
  Mover(float inX, float inY, float inRadius)
  {
      this.pos = new PVector(inX, inY);
      this.radius = inRadius;
      this.mass = PI * inRadius * inRadius * 0.0001;
      
      this.velocity = new PVector(0.0, 0.0);
      this.acceleration = new PVector(0.0, 0.0);
  }
  
  void addForce(PVector force)
  {
      acceleration.add(force.div(this.mass));
  }
  
  void addForce(float forceX, float forceY)
  {
      addForce(new PVector(forceX, forceY));
  }
  
  void update()
  {
      velocity.add(acceleration);
      pos.add(velocity); 
      acceleration.set(0.0, 0.0);
  }
  
  void display()
  {
      noStroke();
      fill(153);
      ellipse(pos.x, pos.y, radius, radius);
      
      //stroke(0xFFFF0000);
      //line(pos.x, pos.y, pos.x + velocity.x*10, pos.y + velocity.y*10);
  }
};
