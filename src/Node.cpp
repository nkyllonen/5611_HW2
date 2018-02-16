#include "Node.h"

/*----------------------------*/
// CONSTRUCTORS AND DESTRUCTORS
/*----------------------------*/
Node::Node() : WorldObject()
{
  normal = Vec3D();
  isFixed = false;
}

Node::Node(Vec3D init_pos) : WorldObject(init_pos)
{
  normal = Vec3D();
  isFixed = false;
}

Node::~Node()
{

}

/*----------------------------*/
// SETTERS
/*----------------------------*/
void Node::setNormal(Vec3D n)
{
  normal = n;
}

void Node::setFixed(bool f)
{
  isFixed = f;
}

/*----------------------------*/
// GETTERS
/*----------------------------*/
Vec3D Node::getNormal()
{
  return normal;
}

bool Node::getIsFixed()
{
  return isFixed;
}

/*----------------------------*/
// VIRTUALS
/*----------------------------*/
int Node::getType()
{
	return NODE_WOBJ;
}
