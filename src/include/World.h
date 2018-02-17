#ifndef WORLD_INCLUDED
#define WORLD_INCLUDED

#include "glad.h"  //Include order can matter here

#ifdef __APPLE__
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#elif __linux__
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#else
#include <SDL.h>
#include <SDL_opengl.h>
#endif

#define GLM_FORCE_RADIANS
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <fstream>
#include <string>

#include "Vec3D.h"
#include "Camera.h"
#include "Util.h"

//what lives in the World
#include "WorldObject.h"
#include "Node.h"

class World
{
private:
	int width;
	int height;

	int total_verts = 0;
	float* modelData;

	WorldObject* floor;

	//modelData indices
	int CUBE_START = 0;
	int CUBE_VERTS = 0;
	int SPHERE_START = 0;
	int SPHERE_VERTS = 0;

	//VAO and VBO GLuints
	GLuint vao;
	GLuint vbo[1];

	//Shader and Texture GLuints
	GLuint shaderProgram;
	GLuint tex0;
	GLuint tex1;

	//updating constant(s)
	const int NUM_SUBSTEPS = 10;

	//masses and springs
	int num_nodes;
	Node** node_arr;
	float restlen = 1.0;
	const float ks = 500.0;
	const float kd = 50.0;
	const float mass = 1.0;
	Vec3D gravity = Vec3D(0,0,0);

	//PRIVATE FUNCTIONS

public:
	//CONSTRUCTORS AND DESTRUCTORS
	World();
	World(int w, int h);
	~World();

	//SETTERS
	void setCubeIndices(int start, int tris);
	void setSphereIndices(int start, int tris);
	void adjustRestLen(float x);

	//GETTERS
	int getWidth();
	int getHeight();

	//OTHERS
	bool loadModelData();
	bool setupGraphics();
	void draw(Camera * cam);
	void update(double dt);
	void init();
	void moveBy(Vec3D v);

};

#endif
