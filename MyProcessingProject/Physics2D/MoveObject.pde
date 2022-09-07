class MoveObject
{
    protected PVector forces;   // N(kg*m/s^2)
    protected PVector location; // m
    protected PVector previousLocation; // m
    protected PVector velocity; // m/s
    protected float speed;
    
    protected float angleVelo; // rad / s
    protected float angle;     // rad
  
    protected float mass;      // kg
    protected float momentOfInertia;// kg·m^2
    protected float radius;    // m
    protected float density;   // kg/m^2
    
    protected color fillCol;
    
    MoveObject(float inX, float inY, float inRadius)
    {
        this.forces = new PVector(0.0f, 0.0f);
        this.location = PixelToMeter(inX, inY);
        this.velocity = new PVector(0.0, 0.0);
        this.speed = velocity.mag();
        
        this.angleVelo = 0.0f;
        this.angle = 0.0f;
      
        this.radius = PixelToMeter(inRadius);
        this.density = 500.0f;
        
        this.mass = PI * radius * radius * density;
        this.momentOfInertia = 0.4 * mass * radius * radius;// (2/5)mr^2
        
        fillCol = color(50, 100, 200);
    }
    
    PVector getLocation()
    {
        return MeterToPixel(location);
    }
    
    void addForce(PVector inF)
    {
        forces.add(inF);
    }
    
    void update()
    {
        // add gravity
        forces.add(PVector.mult(Gravity, mass));
        
        // add air drag
        {
            PVector airDrag = PVector.mult(velocity, -1.0f);
            airDrag.normalize();
            airDrag.mult(AirDensity * speed*speed *radius *DragCoefficient);
            forces.add(airDrag);
        }
        
        // add wind force
        {
            PVector windForce = new PVector(1.0f, 0.0f);
            windForce.mult(AirDensity * WindSpeed*WindSpeed *radius *DragCoefficient);
            //forces.add(windForce);
        }
        
        // basic euler integral
        {
            //println("forces=("+forces.x+","+forces.y+")");
            PVector accel = PVector.div(forces, mass);
            forces.set(0.0f, 0.0f, 0.0f);
        
            PVector dv = PVector.mult(accel, DeltaTime);
            velocity.add(dv);
            speed = velocity.mag();
        
            PVector ds = PVector.mult(velocity, DeltaTime);
            PVector newLoc = PVector.add(location, ds);
            
            PVector locationOffset = new PVector();
            PVector hitNormal = new PVector();
            int hitTimes = 0;
            
            for(StaticObject staObj : obstacles)
            {
                hitTimes += collisionDetect(staObj, newLoc, locationOffset, hitNormal);
            }
            
            if (hitTimes > 0)
            {
                hitNormal.normalize();
              
                float vrn = PVector.dot(velocity, hitNormal);
                float J = -(1 + Restitution) * vrn * mass;
                PVector impulse = PVector.mult(hitNormal, J/DeltaTime);
                forces.add(impulse);
                location.set(PVector.add(newLoc, locationOffset));
            }
            else
                location.set(newLoc);
            //println("hitTimes", hitTimes);
        }     
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
        translate(MeterToPixel(location.x), MeterToPixel(location.y));
        rotate(rotation);
        float pixelSize = MeterToPixel(radius) * 2.0;
        ellipse(0, 0, pixelSize, pixelSize);
        line(0, 0, pixelSize*0.5, 0);
        popMatrix();
    }
    
    int collisionDetect(StaticObject obstacle, PVector newLoc, PVector outLocOffset, PVector outHitNormal)
    {
        int hitTimes = 0;
        
        int numOfSubs = obstacle.getNumOfSubObjects();
        for (int i = 0; i < numOfSubs; ++i)
        {
            PVector borderPoint = obstacle.getHitPoint(i, location);
            PVector borderNormal = obstacle.getNormal(i, location);
          
            PVector toParticle1 = PVector.sub(location, borderPoint);
            PVector toParticle2 = PVector.sub(newLoc, borderPoint);
            
            float dot1 = PVector.dot(toParticle1, borderNormal);
            float dot2 = PVector.dot(toParticle2, borderNormal);
            
            if ((dot2 - dot1) < 0.0f && dot2 <= radius)
            {
                ++hitTimes;
                
                float weight = abs(PVector.dot(velocity, borderNormal));
                
                outHitNormal.add(PVector.mult(borderNormal, weight));
                outLocOffset.add(PVector.mult(borderNormal, radius - dot2));
            }
        }
        
        return hitTimes;
    }
    
    boolean collisionDetect(MoveObject other, PVector outNormal)
    {
        PVector diff = PVector.sub(other.location, location);
        float diffLen = diff.mag();
        
        if (diffLen <= (radius + other.radius))
        {
            diff.normalize();
            outNormal.set(diff);
            return true;
        }
        
        outNormal.set(0.0, 0.0);
        return false;
    }
    
    void onHitBorder(PVector hitNormal)
    {
        float veloInNormal = PVector.dot(velocity, hitNormal);
        if (veloInNormal >= 0.0)
            return;
        
        PVector velo_norm = PVector.mult(hitNormal, veloInNormal);
        PVector velo_tang = PVector.sub(velocity, velo_norm);
        
        this.velocity = PVector.add(velo_tang, PVector.mult(velo_norm, -1.0));
    }
    
    void onHitObject(MoveObject other, PVector hitNormal)
    {
        /*
        ball1 hit ball2
        positive direction: 1 -> 2
        -J = m1 * (v12 - v11);
        J = m2 * (v22 - v21);
        e = -(v12 - v22) / (v11 - v21);
        
        -J/m1 + v11 = v12;
        J/m2 + v21 = v22;
        
        e*(v11 - v21) = (J/m1 - v11 + J/m2 + v21);
        e*(v11 - v21) = (J/m1 + J/m2) + (v21 - v11);
        (1+e)*(v11 - v21) = J*(1/m1 + 1/m2);
        
        J = (1+e)*(v11 - v21) / (1/m1 + 1/m2);
        */
      
        float v11 = PVector.dot(velocity, hitNormal);
        PVector v1_n = PVector.mult(hitNormal, v11);
        PVector v1_t = PVector.sub(velocity, v1_n);
        
        float v21 = PVector.dot(other.velocity, hitNormal);
        PVector v2_n = PVector.mult(hitNormal, v21);
        PVector v2_t = PVector.sub(other.velocity, v2_n);
        
        float J = (1.0 + Restitution) * (v11 - v21) / (1.0 / mass + 1.0 / other.mass);
        
        float v12 = -J/mass + v11;
        float v22 = J/other.mass + v21;
        
        this.velocity = PVector.add(v1_t, PVector.mult(hitNormal, v12));
        other.velocity = PVector.add(v2_t, PVector.mult(hitNormal, v22));
    }
};
