#include "Node.h"

/*----------------------------*/
// CONSTRUCTORS AND DESTRUCTORS
/*----------------------------*/
Node::Node() : WorldObject()
{
  normal = Vec3D();
}

Node::Node(Vec3D init_pos) : WorldObject(init_pos)
{
  normal = Vec3D();
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

/*----------------------------*/
// GETTERS
/*----------------------------*/
Vec3D Node::getNormal()
{
  return normal;
}

/*----------------------------*/
// VIRTUALS
/*----------------------------*/
int Node::getType()
{
	return NODE_WOBJ;
}
