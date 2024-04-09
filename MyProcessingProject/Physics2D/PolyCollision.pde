class PolyCollision extends MoveObject
{
    int EdgeNum;
  
    PolyCollision(float inX, float inY, float inRadius, int inEdgeNum)
    {
        super(inX, inY, inRadius);
        
        this.EdgeNum = inEdgeNum;
    }
    
    void drawObject()
    {
        /*rectMode(CENTER);
        
        float heightInPixel = MeterToPixel(boatHeight);
        float widhtInPixel = MeterToPixel(boatWidth);
        float halfHeight = 0.5f * heightInPixel;
        float halfWidth = 0.5f * widhtInPixel;
        float headLen = halfHeight * 0.5f;
        
        beginShape();
        vertex(-halfHeight, -halfWidth);
        vertex(halfHeight-headLen, -halfWidth);
        vertex(halfHeight, 0.0f);
        vertex(halfHeight-headLen, halfWidth);
        vertex(-halfHeight, halfWidth);
        endShape(CLOSE);*/
    }
};
