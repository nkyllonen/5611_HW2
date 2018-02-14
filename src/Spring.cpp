#include "Spring.h"

/*----------------------------*/
// CONSTRUCTORS AND DESTRUCTORS
/*----------------------------*/
Spring::Spring()
{
  stiffness = 10;
  dampening = 0.2;
  rest_len = 1.0;

  n1_index = 0;
  n2_index = 0;
}

Spring::Spring(int n1, int n2)
{
  stiffness = 10;
  dampening = 0.2;
  rest_len = 1.0;

  n1_index = n1;
  n2_index = n2;
}

Spring::Spring(float s, float d, float len)
{
  stiffness = s;
  dampening = d;
  rest_len = len;

  n1_index = 0;
  n2_index = 0;
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
  n1_index = n1;
  n2_index = n2;
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

int Spring::getN1Index()
{
  return n1_index;
}

int Spring::getN2Index()
{
  return n2_index;
}
