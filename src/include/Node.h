#ifndef NODE_INCLUDED
#define NODE_INCLUDED

#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <fstream>
#include <string>

#include "WorldObject.h"
#include "Vec3D.h"

class Node : public WorldObject
{
private:
  Vec3D normal;
  bool isFixed;

public:
  //CONSTRUCTORS AND DESTRUCTORS
  Node();
  Node(Vec3D init_pos);
  ~Node();

  //SETTERS
  void setNormal(Vec3D n);
  void setFixed(bool f);

  //GETTERS
  Vec3D getNormal();
  bool getIsFixed();

  //VIRTUALS
  int getType();

};

#endif
