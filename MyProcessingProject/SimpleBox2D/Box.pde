class Box extends Shape
{
    int boxWidth;
    int boxHeight;
    
    Box(Box2DProcessing box2d, float posX, float posY, int w, int h)
    {
      super(box2d, posX, posY);
      
      boxWidth = w;
      boxHeight = h;
      
      int R = floor(random(256));
      int G = floor(random(256));
      int B = floor(random(256));
      filledCol = color(R, G, B);
      
      createShapeEnd(box2d);
    }
    
    void defineShape(Box2DProcessing box2d, PolygonShape ps)
    {
        ps.setAsBox( //<>//
          box2d.scalarPixelsToWorld(boxWidth/2.0f),
          box2d.scalarPixelsToWorld(boxHeight/2.0f)
        );
    }
    
    void drawShape()
    {
        rect(0, 0, boxWidth, boxHeight);
    }
};
