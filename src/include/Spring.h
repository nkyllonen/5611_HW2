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
  int n1_index;
  int n2_index;

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
  int getN1Index();
  int getN2Index();

};

#endif
