#include "Spring.h"

/*----------------------------*/
// CONSTRUCTORS AND DESTRUCTORS
/*----------------------------*/
Spring::Spring()
{
  stiffness = 10;
  dampening = 0.2;
  rest_len = 1.0;

  head_index = 0;
  tail_index = 0;
}

Spring::Spring(int n1, int n2)
{
  stiffness = 10;
  dampening = 0.2;
  rest_len = 1.0;

  head_index = n1;
  tail_index = n2;
}

Spring::Spring(float s, float d, float len)
{
  stiffness = s;
  dampening = d;
  rest_len = len;

  head_index = 0;
  tail_index = 0;
}

Spring::~Spring()
{

}

/*----------------------------*/
// SETTERS
/*----------------------------*/
void Spring::setConstants(float ks, float kd, float len)
{
  stiffness = ks;
  dampening = kd;
  rest_len = len;
}

void Spring::setNodeIndices(int n1, int n2)
{
  head_index = n1;
  tail_index = n2;
}

/*----------------------------*/
// GETTERS
/*----------------------------*/
float Spring::getStiffness()
{
  return stiffness;
}

float Spring::getDampening()
{
  return dampening;
}

float Spring::getRestLen()
{
  return rest_len;
}

int Spring::getHeadIndex()
{
  return head_index;
}

int Spring::getTailIndex()
{
  return tail_index;
}
