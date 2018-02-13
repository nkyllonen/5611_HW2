#ifndef SPRING_INCLUDED
#define SPRING_INCLUDED

#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <fstream>
#include <string>

class Spring
{
private:
  //parameters
  float stiffness;
  float dampening;
  float rest_len;

  //Node indices - into World's array
  int head_index;
  int tail_index;

public:
  //CONSTRUCTORS AND DESTRUCTORS
  Spring();
  Spring(int n1, int n2);
  Spring(float s, float d, float len);
  ~Spring();

  //SETTERS
  void setConstants(float ks, float kd, float len);
  void setNodeIndices(int n1, int n2);

  //GETTERS
  float getStiffness();
  float getDampening();
  float getRestLen();
  int getHeadIndex();
  int getTailIndex();

};

#endif
