class StaticObject
{
    protected int numOfSubObjects;
    
    StaticObject()
    {
        numOfSubObjects = 1;
    }
    
    void display()
    {
    }
    
    int getNumOfSubObjects()
    {
        return numOfSubObjects;
    }
    
    PVector getNormal(int sub, PVector inLoc)
    {
        println("error!!");
        return new PVector();
    }
    
    PVector getHitPoint(int sub, PVector inLoc)
    {
        println("error!!");
        return new PVector();
    }
};
