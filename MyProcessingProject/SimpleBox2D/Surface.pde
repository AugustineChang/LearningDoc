import org.jbox2d.collision.shapes.ChainShape;

class Surface
{
    Body surfaceBody;
    ArrayList<Vec2> surfaceVerts;
    
    Surface(float posX, float posY)
    {
        surfaceVerts = new ArrayList<Vec2>();
        generateVerts(posX, posY);
        
        int NumOfVerts = surfaceVerts.size();
        Vec2 verts[] = new Vec2[NumOfVerts];
        for(int i = 0; i < NumOfVerts; ++i)
        {
            verts[i] = box2d.coordPixelsToWorld(surfaceVerts.get(i));
        }
        
        BodyDef bd = new BodyDef();
        bd.type = BodyType.STATIC;
        surfaceBody = box2d.createBody(bd);
        
        ChainShape cs = new ChainShape();
        cs.createChain(verts, verts.length);
        
        surfaceBody.createFixture(cs, 1.0f);
    }
    
    void generateVerts(float centerX, float centerY)
    {
        surfaceVerts.add(new Vec2(centerX-100.0, centerY+20.0));
        surfaceVerts.add(new Vec2(centerX-50.0, centerY+0.0));
        surfaceVerts.add(new Vec2(centerX+0.0, centerY+20.0));
        surfaceVerts.add(new Vec2(centerX+50.0, centerY+0.0));
        surfaceVerts.add(new Vec2(centerX+100.0, centerY+20.0));
    }
    
    void display()
    {
        stroke(0);
        fill(25);
        
        beginShape();
        int NumOfVerts = surfaceVerts.size();
        for(int i = 0; i < NumOfVerts; ++i)
        {
            Vec2 curVert = surfaceVerts.get(i);
            vertex(curVert.x, curVert.y);
        }
        for(int i = NumOfVerts-1; i >= 0; --i)
        {
            Vec2 curVert = surfaceVerts.get(i);
            vertex(curVert.x, curVert.y+5.0);
        }
        endShape();
    }
}
