import org.jbox2d.dynamics.joints.MouseJoint;
import org.jbox2d.dynamics.joints.MouseJointDef;

class MouseDragger
{
    MouseJoint mouseJoint;  
    Shape draggedShape;
    
    MouseDragger()
    {
        mouseJoint = null;
        draggedShape = null;
    }
    
    void createJoint(ArrayList<Shape> allShapes)
    {
        if (mouseJoint != null)
            return;
        
        draggedShape = findDraggerBody(allShapes);
        if (draggedShape == null)
            return;
        
        draggedShape.bIsDragged = true;
        MouseJointDef mjd = new MouseJointDef();
        mjd.bodyA = box2d.getGroundBody();
        mjd.bodyB = draggedShape.shapeBody;
        
        Vec2 mouseWorldPos = box2d.coordPixelsToWorld(new Vec2(mouseX, mouseY));
        mjd.target.set(mouseWorldPos);
        
        mjd.maxForce = 1000.0;
        mjd.frequencyHz = 6.0;
        mjd.dampingRatio = 0.0;
        
        mouseJoint = (MouseJoint) box2d.createJoint(mjd);
    }
    
    void destroyJoint()
    {
        if (mouseJoint == null)
            return;
        
        box2d.world.destroyJoint(mouseJoint);
        draggedShape.bIsDragged = false;
        mouseJoint = null;
        draggedShape = null;
    }
    
    void display()
    {
        if(mouseJoint != null)
        {
            Vec2 mouseWorldPos = box2d.coordPixelsToWorld(new Vec2(mouseX, mouseY));
            mouseJoint.setTarget(mouseWorldPos);
        }
    }
    
    private Shape findDraggerBody(ArrayList<Shape> allShapes)
    {
        Shape target = null;
        for (Shape one : allShapes)
        {
            if (one.checkDrag(mouseX, mouseY))
            {
                target = one;
                break;
            }
        }
        return target;
    }
};
