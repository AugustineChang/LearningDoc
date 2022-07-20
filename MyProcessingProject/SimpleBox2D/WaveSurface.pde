class WaveSurface extends Surface
{
    WaveSurface(Box2DProcessing box2d, float posX, float posY)
    {
        super(box2d, posX, posY);
    }
    
    void generateVerts(float centerX, float centerY)
    {
        int NumOfVerts = 20;
        
        Vec2 center = new Vec2(0.0, 0.0);
        for (int i = 0; i < NumOfVerts; ++i)
        {
            Vec2 curVert = new Vec2(i*30.0, 20.0*sin(2.0*PI*i / float(NumOfVerts)));
            surfaceVerts.add(curVert);
            center.addLocal(curVert);
        }
        center.mulLocal(1.0/NumOfVerts);
      
        for (int i = 0; i < NumOfVerts; ++i)
        {
            Vec2 curVert = surfaceVerts.get(i);
            curVert.subLocal(center);
            curVert.addLocal(new Vec2(centerX, centerY));
        }
    }
};
