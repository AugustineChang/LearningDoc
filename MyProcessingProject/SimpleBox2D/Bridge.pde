import org.jbox2d.dynamics.joints.DistanceJoint;
import org.jbox2d.dynamics.joints.DistanceJointDef;

class Bridge
{
    ArrayList<Body> nodeBodys;
    ArrayList<DistanceJoint> nodeLinks;
    
    float nodeSize;
    
    Bridge()
    {
        nodeBodys = new ArrayList<Body>();
        nodeLinks = new ArrayList<DistanceJoint>();
        
        nodeSize = 16.0f;
        
        float FirstX = 200.0;
        float FirstY = 80.0;
        float step = 15.0;
        createBody(FirstX, FirstY, false);
        
        int numOfNodes = 20;
        for (int i = 0; i < numOfNodes; ++i)
        {
            createBody(FirstX + step*(i+1), FirstY, i < numOfNodes-1);
        }
        
        for (int i = 0; i < numOfNodes; ++i)
        {
            linkTwoBodies(i, i+1);
        }
    }
  
    void linkTwoBodies(int indexA, int indexB)
    {
        Body bodyA = nodeBodys.get(indexA);
        Body bodyB = nodeBodys.get(indexB);
        
        Vec2 locA = bodyA.getTransform().p;
        Vec2 locB = bodyB.getTransform().p;
        Vec2 diff = locA.sub(locB);
        float dist = diff.length();
        
        DistanceJointDef djd = new DistanceJointDef();
        djd.bodyA = bodyA;
        djd.bodyB = bodyB;
        djd.length = dist;
        djd.frequencyHz = 6.0;
        djd.dampingRatio = 0.0;
        
        DistanceJoint createdJoint = (DistanceJoint) box2d.createJoint(djd);
        nodeLinks.add(createdJoint);
    }
    
    void createBody(float posX, float posY, boolean bDynamic)
    {
        BodyDef bd = new BodyDef();
        bd.position = box2d.coordPixelsToWorld(posX, posY);
        bd.type = bDynamic ? BodyType.DYNAMIC : BodyType.STATIC;
        
        Body createdBody = box2d.createBody(bd);
        
        FixtureDef fd = new FixtureDef();
        fd.friction = 0.3f;
        fd.restitution = 0.5f;
        fd.density = 1.0f;
        
        if (bDynamic)
        {
            CircleShape cs = new CircleShape();
            cs.m_radius = box2d.scalarPixelsToWorld(nodeSize/2.0f);
            fd.shape = cs;
        }
        else
        {
            PolygonShape ps = new PolygonShape();
            ps.setAsBox(
                box2d.scalarPixelsToWorld(nodeSize/2.0f),
                box2d.scalarPixelsToWorld(nodeSize/2.0f)
            );
            fd.shape = ps;
        }
        createdBody.createFixture(fd);
        
        nodeBodys.add(createdBody);
    }
	
  	void display()
  	{
        stroke(80);
  
        int numOfLinks = nodeLinks.size();
        for (int i = 0; i < numOfLinks; ++i)
        {
  			    DistanceJoint curJoint = nodeLinks.get(i);
            Body bodyA = curJoint.getBodyA();
            Body bodyB = curJoint.getBodyB();
            
            Transform transA = bodyA.getTransform();
            Transform transB = bodyB.getTransform();
            
            Vec2 locA = box2d.coordWorldToPixels(transA.p);
            Vec2 locB = box2d.coordWorldToPixels(transB.p);
            
            line(locA.x, locA.y, locB.x, locB.y);
        }
        
        stroke(0);
        rectMode(CENTER);
        ellipseMode(CENTER);
    
        int numOfBodies = nodeBodys.size();
        for (int i = 0; i < numOfBodies; ++i)
        {
            Body curBody = nodeBodys.get(i);
            Transform trans = curBody.getTransform();
            Vec2 location = box2d.coordWorldToPixels(trans.p);
            float angle = trans.q.getAngle();
            
            pushMatrix();
            translate(location.x, location.y);
            rotate(-angle);
            
            if (curBody.m_type == BodyType.STATIC)
            {
                fill(0);
                rect(0.0, 0.0, nodeSize, nodeSize);
            }
            else if (curBody.m_type == BodyType.DYNAMIC)
            {
                fill(188);
                ellipse(0.0, 0.0, nodeSize, nodeSize);
            }
            
            popMatrix();
        }
  	}
};
