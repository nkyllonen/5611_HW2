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
#include <cmath>
#include <iostream>
#include <fstream>
#include <string>

#include "Vec3D.h"
#include "Camera.h"
#include "Util.h"

//what lives in the World
#include "WorldObject.h"
#include "Node.h"

enum cloth_type
{
	TEXTURED,
	SKELETON
};

enum pin_type
{
	PIN_TOP,
	PIN_CORNERS
};

class World
{
private:
	int width;
	int height;

	WorldObject* floor;
	Vec3D floor_normal = Vec3D(0,1,0);

	WorldObject* sphere;

	//VBO data
	float* modelData;
	float* lineData;
	float* texturedData;
	float* texturedCoords;
	ushort* texturedIndices;
	int CUBE_START = 0;
	int CUBE_VERTS = 0;
	int SPHERE_START = 0;
	int SPHERE_VERTS = 0;
	int total_model_verts = 0;
	int total_springs = 0;
	int total_triangles = 0;

	//VAO and VBO GLuints
	GLuint model_vao;
	GLuint model_vbo[1];
	GLuint line_vao;
	GLuint line_vbo[1];
	GLuint textured_vao;
	GLuint textured_vbos[2];
	GLuint textured_ibo[1];

	//Shader and Texture GLuints
	GLuint phongProgram;
	GLuint flatProgram;
	GLuint tex0;
	GLuint tex1;

	Material cloth_mat = Material();

	//updating constant(s)
	const int NUM_SUBSTEPS = 10;

	//masses and springs
	int num_nodes;
	Node** node_arr;
	float restlen = 1.0;
	const float ks = 1000.0;
	const float kd = 50.0;
	const float mass = 0.001;
	Vec3D gravity = Vec3D(0,-0.1,0);

	//PRIVATE FUNCTIONS
	void drawNodes();
	void drawSprings();
	void drawSkeleton(Camera* cam);
	void drawTextured(Camera* cam);
	void loadLineVertices();
	void checkForCollisions(Vec3D in_pos, Vec3D in_vel, double dt, Vec3D& out_pos, Vec3D& out_vel);
	void loadTexturedPosAndNorm();
	void loadTextureCoords();
	void loadTexturedIndices();

public:
	int cloth_state = SKELETON;
	int pin_state = PIN_CORNERS;

	//CONSTRUCTORS AND DESTRUCTORS
	World();
	World(int w, int h);
	~World();

	//SETTERS
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
	void moveClothBy(Vec3D v);
	void reset();
	void moveSphereBy(Vec3D v);

};

#endif
