class Box extends Shape
{
    int boxWidth;
    int boxHeight;
    
    Box(float posX, float posY, int w, int h)
    {
      super(posX, posY);
      
      boxWidth = w;
      boxHeight = h;
      
      int R = floor(random(256));
      int G = floor(random(256));
      int B = floor(random(256));
      filledCol = color(R, G, B);
      
      createShapeEnd();
    }
    
    void defineShape(PolygonShape ps)
    {
        ps.setAsBox(
          box2d.scalarPixelsToWorld(boxWidth/2.0f),
          box2d.scalarPixelsToWorld(boxHeight/2.0f)
        );
    }
    
    void drawShape()
    {
        rect(0, 0, boxWidth, boxHeight);
    }
    
    boolean checkDrag(float dragPosX, float dragPosY)
    {
        Transform trans = shapeBody.getTransform();
        Vec2 center = box2d.coordWorldToPixels(trans.p);
        
        Vec2 axisX = new Vec2();
        Vec2 axisY = new Vec2();
        trans.q.getXAxis(axisX);
        trans.q.getYAxis(axisY);
        axisX.y *= box2d.yFlip;
        axisY.y *= box2d.yFlip;
        
        Vec2 toCenter = center.sub(new Vec2(dragPosX, dragPosY));
        float distX = abs(Vec2.dot(toCenter, axisX));
        float distY = abs(Vec2.dot(toCenter, axisY));
        
        return distX <= boxWidth*0.5 && distY <= boxHeight*0.5;
    }
};
