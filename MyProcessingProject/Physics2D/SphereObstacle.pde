class SphereObstacle extends StaticObject
{
    protected PVector location; // m
    protected float radius;    // m
    
    SphereObstacle(float inX, float inY, float inRadius)
    {
        super();
        
        this.location = PixelToMeter(inX, inY);
        this.radius = PixelToMeter(inRadius);
    }
    
    void display()
    {
        ellipseMode(CENTER);
        stroke(0);
        fill(130, 200, 30);
      
        float pixelSize = MeterToPixel(radius) * 2.0;
        float locX = MeterToPixel(location.x);
        float locY = MeterToPixel(location.y);
      
        ellipse(locX, locY, pixelSize, pixelSize);
    }
    
    PVector getNormal(int sub, PVector inLoc)
    {
        PVector normal = PVector.sub(inLoc, location);
        normal.normalize();
        return normal;
    }
    
    PVector getHitPoint(int sub, PVector inLoc)
    {
        PVector hitPoint = getNormal(sub, inLoc);
        hitPoint.mult(radius);
        hitPoint.add(location);
        return hitPoint;
    }
};
