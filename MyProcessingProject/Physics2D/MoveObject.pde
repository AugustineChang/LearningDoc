class MoveObject
{
    protected PVector forces;   // N(kg*m/s^2)
    protected PVector location; // m
    protected PVector velocity; // m/s
    protected PVector localVelocity; // m/s
    protected float speed;
    
    protected PVector moments;  // N·m(kg*(m/s)^2)
    protected PVector angularVelo; // rad / s
    protected PVector angle;     // rad
  
    protected float mass;      // kg
    protected float momentOfInertia;// kg·m^2
    protected float radius;    // m
    protected float density;   // kg/m^2
    
    protected PVector dragForcePoint;
    
    protected color fillCol;
    
    protected boolean useGravity;
    protected boolean useDrag;
    protected boolean useWindForce;
    
    MoveObject(float inX, float inY, float inRadius)
    {
        this.forces = new PVector(0.0f, 0.0f);
        this.location = PixelToMeter(inX, inY);
        this.velocity = new PVector(0.0f, 0.0f);
        this.localVelocity = new PVector(0.0f, 0.0f); 
        this.speed = velocity.mag();
        
        this.moments = new PVector(0.0f, 0.0f, 0.0f);
        this.angularVelo = new PVector(0.0f, 0.0f, 0.0f);
        this.angle = new PVector(0.0f, 0.0f, 0.0f);
      
        this.radius = PixelToMeter(inRadius);
        this.density = 500.0f;
        
        this.mass = PI * radius * radius * density;
        this.momentOfInertia = 0.4 * mass * radius * radius;// (2/5)mr^2
        
        this.dragForcePoint = new PVector(0.0f, 0.0f, 0.0f);
        
        fillCol = color(50, 100, 200);
        useGravity = true;
        useDrag = true;
        useWindForce = true;
    }
    
    PVector getLocation()
    {
        return MeterToPixel(location);
    }
    
    void addForce(PVector inF)
    {
        forces.add(inF);
    }
    
    //void custom
    
    void update()
    {
        // add gravity
        if (useGravity)
            forces.add(PVector.mult(Gravity, mass));
        
        // add air drag
        if (useDrag)
        {
            PVector totalLocalVelo = angularVelo.cross(dragForcePoint);
            println(totalLocalVelo);
            totalLocalVelo.add(localVelocity);
            float totalSpeed = totalLocalVelo.mag();
            
            //if (totalSpeed > 0.1f)
            {
                PVector airDrag = PVector.mult(totalLocalVelo, -1.0f);
                airDrag.normalize();
                airDrag.mult(AirDensity * totalSpeed*totalSpeed *radius *DragCoefficient);
                
                moments.add(dragForcePoint.cross(airDrag));
                airDrag.rotate(angle.z);
                forces.add(airDrag);
            }
        }
        
        // add wind force
        if (useWindForce)
        {
            PVector windForce = new PVector(1.0f, 0.0f);
            windForce.mult(AirDensity * WindSpeed*WindSpeed *radius *DragCoefficient);
            //forces.add(windForce);
        }
        
        // basic euler integral
        {
            //println("forces=("+forces.x+","+forces.y+")");
            
            // linear velocity
            PVector accel = PVector.div(forces, mass);
            forces.set(0.0f, 0.0f, 0.0f);
        
            PVector dv = PVector.mult(accel, DeltaTime);
            velocity.add(dv);
            speed = velocity.mag();
        
            PVector ds = PVector.mult(velocity, DeltaTime);
            PVector newLoc = PVector.add(location, ds);
            
            // angular velocity
            PVector angularAccel = PVector.div(moments, momentOfInertia);
            moments.set(0.0f, 0.0f, 0.0f);
            
            PVector dav = PVector.mult(angularAccel, DeltaTime);
            angularVelo.add(dav);
            
            PVector da = PVector.mult(angularVelo, DeltaTime);
            angle.add(da);
            
            localVelocity.set(velocity);
            localVelocity.rotate(-angle.z);
            
            // collision detect
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
    
    void drawObject()
    {
        ellipseMode(CENTER);
      
        float pixelSize = MeterToPixel(radius) * 2.0;
        ellipse(0, 0, pixelSize, pixelSize);
        line(0, 0, pixelSize*0.5, 0);
    }
    
    void display()
    {
        stroke(0);
        fill(fillCol);
        
        pushMatrix();
        translate(MeterToPixel(location.x), MeterToPixel(location.y));
        rotate(angle.z);
        drawObject();
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
