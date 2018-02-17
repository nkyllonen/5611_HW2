#include "World.h"

using namespace std;


/*----------------------------*/
// CONSTRUCTORS AND DESTRUCTORS
/*----------------------------*/
World::World()
{
	width = 0;
	height = 0;
}

World::World(int w, int h)
{
	width = w;
	height = h;
}

World::~World()
{
	delete floor;
	delete[] modelData;

	for (int i = 0; i < num_nodes; i++)
	{
		delete node_arr[i];
	}

	delete [] node_arr;
}

/*----------------------------*/
// SETTERS
/*----------------------------*/
void World::setCubeIndices(int start, int tris)
{
	CUBE_START = start;
	CUBE_VERTS = tris;
}

void World::setSphereIndices(int start, int tris)
{
	SPHERE_START = start;
	SPHERE_VERTS = tris;
}

void World::adjustRestLen(float x)
{
	restlen += x;
	printf("---rest length: %f\n", restlen);
}

/*----------------------------*/
// GETTERS
/*----------------------------*/
int World::getWidth()
{
	return width;
}

int World::getHeight()
{
	return height;
}

/*----------------------------*/
// OTHERS
/*----------------------------*/
//load in all models and store data into the modelData array
bool World::loadModelData()
{
	/////////////////////////////////
	//LOAD IN MODELS
	/////////////////////////////////
	//CUBE
	int CUBE_VERTS = 0;
	float* cubeData = util::loadModel("models/cube.txt", CUBE_VERTS);
	cout << "\nNumber of vertices in cube model : " << CUBE_VERTS << endl;
	total_verts += CUBE_VERTS;
	setCubeIndices(0, CUBE_VERTS);

	//SPHERE
	/*int SPHERE_VERTS = 0;
	float* sphereData = util::loadModel("models/sphere.txt", SPHERE_VERTS);
	cout << "\nNumber of vertices in sphere model : " << SPHERE_VERTS << endl;
	total_verts += SPHERE_VERTS;
	setSphereIndices(CUBE_VERTS, SPHERE_VERTS);

	/////////////////////////////////
	//BUILD MODELDATA ARRAY
	/////////////////////////////////
	if (!(cubeData != nullptr && sphereData != nullptr))
	{
		delete[] cubeData;
		delete[] sphereData;
		return false;
	}*/
	if (cubeData == nullptr)
	{
		delete[] cubeData;
		return false;
	}
	modelData = new float[total_verts * 8];
	//copy data into modelData array
	copy(cubeData, cubeData + CUBE_VERTS * 8, modelData);
	//copy(sphereData, sphereData + SPHERE_VERTS * 8, modelData + (CUBE_VERTS * 8));
	delete[] cubeData;
	//delete[] sphereData;
	return true;
}

//
bool World::setupGraphics()
{
	/////////////////////////////////
	//BUILD VERTEX ARRAY OBJECT
	/////////////////////////////////
	//This stores the VBO and attribute mappings in one object
	glGenVertexArrays(1, &vao); //Create a VAO
	glBindVertexArray(vao); //Bind the above created VAO to the current context

	/////////////////////////////////
	//BUILD VERTEX BUFFER OBJECT
	/////////////////////////////////
	//Allocate memory on the graphics card to store geometry (vertex buffer object)
	glGenBuffers(1, vbo);  //Create 1 buffer called vbo
	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]); //Set the vbo as the active array buffer (Only one buffer can be active at a time)
	glBufferData(GL_ARRAY_BUFFER, total_verts * 8 * sizeof(float), modelData, GL_STATIC_DRAW); //upload vertices to vbo

	/////////////////////////////////
	//SETUP SHADERS
	/////////////////////////////////
	shaderProgram = util::LoadShader("Shaders/phongTex.vert", "Shaders/phongTex.frag");

	//load in textures
	tex0 = util::LoadTexture("textures/wood.bmp");
	tex1 = util::LoadTexture("textures/grey_stones.bmp");

	if (tex0 == -1 || tex1 == -1 || shaderProgram == -1)
	{
		cout << "\nCan't load texture(s)" << endl;
		printf(strerror(errno));
		return false;
	}

	//Tell OpenGL how to set fragment shader input
	GLint posAttrib = glGetAttribLocation(shaderProgram, "position");
	glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), 0);
	//Attribute, vals/attrib., type, normalized?, stride, offset
	//Binds to VBO current GL_ARRAY_BUFFER
	glEnableVertexAttribArray(posAttrib);

	GLint normAttrib = glGetAttribLocation(shaderProgram, "inNormal");
	glVertexAttribPointer(normAttrib, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(5 * sizeof(float)));
	glEnableVertexAttribArray(normAttrib);

	GLint texAttrib = glGetAttribLocation(shaderProgram, "inTexcoord");
	glEnableVertexAttribArray(texAttrib);
	glVertexAttribPointer(texAttrib, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));

	glBindVertexArray(0); //Unbind the VAO in case we want to create a new one

	glEnable(GL_DEPTH_TEST);
	return true;
}

//loops through WObj array and draws each
//also draws floor
void World::draw(Camera * cam)
{
	glClearColor(.2f, 0.4f, 0.8f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(shaderProgram); //Set the active shader (only one can be used at a time)

	//vertex shader uniforms
	GLint uniView = glGetUniformLocation(shaderProgram, "view");
	GLint uniProj = glGetUniformLocation(shaderProgram, "proj");
	GLint uniTexID = glGetUniformLocation(shaderProgram, "texID");

	//build view matrix from Camera
	glm::mat4 view = glm::lookAt(
		util::vec3DtoGLM(cam->getPos()),
		util::vec3DtoGLM(cam->getPos() + cam->getDir()),  //Look at point
		util::vec3DtoGLM(cam->getUp()));

	glUniformMatrix4fv(uniView, 1, GL_FALSE, glm::value_ptr(view));

	glm::mat4 proj = glm::perspective(3.14f / 4, 800.0f / 600.0f, 0.1f, 100.0f); //FOV, aspect, near, far
	glUniformMatrix4fv(uniProj, 1, GL_FALSE, glm::value_ptr(proj));

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tex0);
	glUniform1i(glGetUniformLocation(shaderProgram, "tex0"), 0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, tex1);
	glUniform1i(glGetUniformLocation(shaderProgram, "tex1"), 1);

	glBindVertexArray(vao);

	glUniform1i(uniTexID, -1); //Set texture ID to use for floor (-1 = no texture)

	for (int i = 0; i < num_nodes; i++)
	{
			node_arr[i]->draw(shaderProgram);
	}//END for loop

	glUniform1i(uniTexID, 1); //Set texture ID to use for floor (1 = metal floor)
	floor->draw(shaderProgram);
}

//loops through and updates attributes of springs and masses
void World::update(double dt)
{
	for (int step = 0; step < NUM_SUBSTEPS; step++)
	{
		//1. reset all accelerations [0, n]
		for (int i = 0; i < num_nodes; i++)
		{
			node_arr[i]->setAcc(Vec3D(0,0,0));
		}

		//2. calculate net forces on i and i+1 [0, n-1]
		float force_i = 0.0, len = 1.0, vi, vii;
		Vec3D e = Vec3D(1,1,1);

		//2.1 calculate horizontal forces
		for (int i = 0; i < width*height-1; i++)
		{
			if ((i+1) % width != 0) //don't apply to right column
			{
				//position vector from i to i+1
				e = node_arr[i+1]->getPos() - node_arr[i]->getPos();
				len = e.getMagnitude();

				//components of velocities along e
				vi = dotProduct(node_arr[i]->getVel(), e);
				vii = dotProduct(node_arr[i+1]->getVel(), e);

				force_i = -ks*(restlen - len) - kd*(vi-vii);

				//apply force to i and i+1 -- update velocities first
				node_arr[i]->setVel(node_arr[i]->getVel() + force_i*dt*e);
				node_arr[i+1]->setVel(node_arr[i+1]->getVel() - force_i*dt*e);
			}
		}//FOR - horizontal

		//2.2 calculate vertical forces
		for (int i = 0; i < width*(height-1); i++) //don't apply to bottom row
		{
			//position vector from i to i+width
			e = node_arr[i+width]->getPos() - node_arr[i]->getPos();
			len = e.getMagnitude();

			//components of velocities along e
			vi = dotProduct(node_arr[i]->getVel(), e);
			vii = dotProduct(node_arr[i+width]->getVel(), e);

			force_i = -ks*(restlen - len) - kd*(vi-vii);

			//apply force to i and i+1 -- update velocities first
			node_arr[i]->setVel(node_arr[i]->getVel() + force_i*dt*e);
			node_arr[i+width]->setVel(node_arr[i+width]->getVel() - force_i*dt*e);
		}//FOR - vertical

		//2.3 add gravity to each velocity
		for (int i = 0; i < num_nodes; i++)
		{
			node_arr[i]->setVel(node_arr[i]->getVel() + gravity);
		}

		//2.4 fix top row so it doesn't move
		for (int i = 0; i < width; i++)
		{
			node_arr[i]->setVel(Vec3D(0,0,0));
		}

		//3. set positions accordingly
		for (int i = 0; i < num_nodes; i++)
		{
			node_arr[i]->setPos(node_arr[i]->getPos() + dt*node_arr[i]->getVel());
		}
	}//FOR - substeps
}//END update()

//init masses and springs using the width and height parameters
void World::init()
{
	num_nodes = width * height;
	Vec3D pos;
	Node* n;

	//center cloth on (0,0)
	//flattened 2D -> 1D arrays
	node_arr = new Node*[num_nodes];
	printf("Allocated array of %i Nodes.\n", num_nodes);

	for (int i = 0; i < width; i++)
	{
		for (int j = 0; j < height; j++)
		{
			pos.setX(i - (width/2));
			pos.setY((height/2) - j);
			n = new Node(pos);

			//green cube
			Material mat = Material();
			mat.setAmbient(glm::vec3(0, 1, 0));
			mat.setDiffuse(glm::vec3(0, 1, 0));
			mat.setSpecular(glm::vec3(0.75, 0.75, 0.75));
			n->setMaterial(mat);
			n->setVertexInfo(CUBE_START, CUBE_VERTS);
			n->setSize(Vec3D(0.25,0.25,0.1));

			node_arr[i + j*width] = n;

			//printf("[%i] at %i , %i :", i + j*width, i , j);
			//pos.print();
		}
	}

	//initialize floor to be below cloth
	floor = new WorldObject(Vec3D(0,-0.5*height - 1, 0));
	floor->setVertexInfo(CUBE_START, CUBE_VERTS);

	Material mat = Material();
	mat.setAmbient(glm::vec3(0.7, 0.7, 0.7));
	mat.setDiffuse(glm::vec3(0.7, 0.7, 0.7));
	mat.setSpecular(glm::vec3(0, 0, 0));

	floor->setMaterial(mat);
	floor->setSize(Vec3D(10, 0.1, 10)); //xz plane
}//END init()

//add velocity v to top row of Nodes
void World::moveBy(Vec3D v)
{
	for (int i = 0; i < width; i++)
	{
		node_arr[i]->setPos(node_arr[i]->getPos() + v);
	}
}

//set back to original state
void World::reset()
{
	for (int i = 0; i < num_nodes; i++)
	{
		delete node_arr[i];
	}

	delete [] node_arr;
	init();
}

/*----------------------------*/
// PRIVATE FUNCTIONS
/*----------------------------*/
