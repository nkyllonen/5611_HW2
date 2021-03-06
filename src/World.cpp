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
	num_nodes = width * height;
	total_springs = (width - 1) * height + (height - 1) * width;
	total_triangles = (width - 1) * (height - 1) * 2;
}

World::~World()
{
	delete floor;
	delete sphere;
	delete[] modelData;
	delete[] lineData;
	delete[] texturedData;
	delete[] texturedCoords;
	delete[] texturedIndices;

	for (int i = 0; i < num_nodes; i++)
	{
		delete node_arr[i];
	}

	delete [] node_arr;
}

/*----------------------------*/
// SETTERS
/*----------------------------*/
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
/*--------------------------------------------------------------*/
// loadModelData : load in all models and store data into the cubeData array
/*--------------------------------------------------------------*/
bool World::loadModelData()
{
	/////////////////////////////////
	//LOAD IN MODELS
	/////////////////////////////////
	//CUBE
	CUBE_VERTS = 0;
	float* cubeData = util::loadModel("models/cube.txt", CUBE_VERTS);
	cout << "Number of vertices in cube model : " << CUBE_VERTS << endl;
	total_model_verts += CUBE_VERTS;

	//SPHERE
	SPHERE_START = CUBE_VERTS;
	float* sphereData = util::loadModel("models/sphere.txt", SPHERE_VERTS);
	cout << "Number of vertices in sphere model : " << SPHERE_VERTS << endl << endl;
	total_model_verts += SPHERE_VERTS;

	/////////////////////////////////
	//BUILD MODELDATA ARRAY
	/////////////////////////////////
	if (!(cubeData != nullptr && sphereData != nullptr))
	{
		cout << "ERROR. Failed to load model(s)" << endl;
		delete[] cubeData;
		delete[] sphereData;
		return false;
	}

	modelData = new float[total_model_verts * 8];
	cout << "Allocated modelData : " << total_model_verts * 8 << endl;
	copy(cubeData, cubeData + CUBE_VERTS * 8, modelData);
	copy(sphereData, sphereData + SPHERE_VERTS * 8, modelData + (CUBE_VERTS * 8));
	delete[] cubeData;
	delete[] sphereData;

	/////////////////////////////////
	//BUILD LINE + TEXTURED ARRAYS
	/////////////////////////////////
	lineData = new float[total_springs * 6]; //3 coords per endpts of each spring (3 x 2)
	cout << "Allocated lineData : " << total_springs * 6 << endl;
	texturedData = new float[num_nodes * 6]; //each "node" has a pos + norm (3 + 3 floats)
	cout << "Allocated texturedData : " << num_nodes * 6 << endl;

	//load textures coords now since they're static
	texturedCoords = new float[num_nodes * 2]; //2 coords per vertex
	cout << "Allocated texturedCoords : " << num_nodes * 2 << endl;
	loadTextureCoords();

	//load indices now since they're static
	texturedIndices = new unsigned int[total_triangles * 3];
	cout << "Allocated texturedIndices : " << total_triangles * 3 << endl;
	loadTexturedIndices();

	return true;
}

/*--------------------------------------------------------------*/
// setupGraphics : load shaders, textures, set model_vao + model_vbo
/*--------------------------------------------------------------*/
bool World::setupGraphics()
{
	/////////////////////////////////
	//BUILD CUBE&SPHERE VAO + VBOs
	/////////////////////////////////
	//This stores the VBO and attribute mappings in one object
	glGenVertexArrays(1, &model_vao); //Create a VAO
	glBindVertexArray(model_vao); //Bind the model_vao to the current context

	//Allocate memory on the graphics card to store geometry (vertex buffer object)
	glGenBuffers(1, model_vbo);  //Create 1 buffer called model_vbo
	glBindBuffer(GL_ARRAY_BUFFER, model_vbo[0]); //Set the model_vbo as the active array buffer (Only one buffer can be active at a time)
	glBufferData(GL_ARRAY_BUFFER, total_model_verts * 8 * sizeof(float), modelData, GL_STATIC_DRAW); //upload vertices to model_vbo

	/////////////////////////////////
	//SETUP CUBE SHADERS (model_vao attributes --> bound to model_vbo[0])
	/////////////////////////////////
	phongProgram = util::LoadShader("Shaders/phongTex.vert", "Shaders/phongTex.frag");

	//load in textures
	tex0 = util::LoadTexture("textures/fabric1.bmp");
	tex1 = util::LoadTexture("textures/grey_stones.bmp");

	if (tex0 == -1 || tex1 == -1 || phongProgram == -1)
	{
		cout << "\nCan't load texture(s)" << endl;
		printf(strerror(errno));
		return false;
	}

	//Tell OpenGL how to set fragment shader input
	GLint posAttrib = glGetAttribLocation(phongProgram, "position");
	glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), 0); //positions first
	//Attribute, vals/attrib., type, normalized?, stride, offset
	//Binds to VBO current GL_ARRAY_BUFFER
	glEnableVertexAttribArray(posAttrib);

	GLint texAttrib = glGetAttribLocation(phongProgram, "inTexcoord");
	glVertexAttribPointer(texAttrib, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float))); //texcoords second
	glEnableVertexAttribArray(texAttrib);

	GLint normAttrib = glGetAttribLocation(phongProgram, "inNormal");
	glVertexAttribPointer(normAttrib, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(5 * sizeof(float))); //normals last
	glEnableVertexAttribArray(normAttrib);

	/////////////////////////////////
	//BUILD LINE VAO + VBO
	/////////////////////////////////
	glGenVertexArrays(1, &line_vao);
	glBindVertexArray(line_vao); //Bind the line_vao to the current context
	glGenBuffers(1, line_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, line_vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, total_springs * 6 * sizeof(float), lineData, GL_STREAM_DRAW);

	/////////////////////////////////
	//SETUP LINE SHADERS
	/////////////////////////////////
	flatProgram = util::LoadShader("Shaders/flat.vert", "Shaders/flat.frag");

	if (flatProgram == -1)
	{
		cout << "\nCan't load flat shaders(s)" << endl;
		printf(strerror(errno));
		return false;
	}

	//only passing in a position vec3 value into shader
	GLint line_posAttrib = glGetAttribLocation(flatProgram, "position");
	glVertexAttribPointer(line_posAttrib, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
	glEnableVertexAttribArray(line_posAttrib);

	/////////////////////////////////
	//BUILD TEXTURED VAO + VBO
	/////////////////////////////////
	glGenVertexArrays(1, &textured_vao);
	glBindVertexArray(textured_vao);
	glGenBuffers(2, textured_vbos); //get 2 ids for the 2 VBOs

	//1.1 POSITIONS + NORMALS --> textured_vbos[0]
	glBindBuffer(GL_ARRAY_BUFFER, textured_vbos[0]);
	glBufferData(GL_ARRAY_BUFFER, num_nodes * 6 * sizeof(float), texturedData, GL_STREAM_DRAW); //dynamic

	//1.2 setup position and normal attributes --> need to set now while textured_vbos[0] is bound
	GLint tex_posAttrib = glGetAttribLocation(phongProgram, "position");
	glVertexAttribPointer(tex_posAttrib, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), 0); //positions first
	glEnableVertexAttribArray(tex_posAttrib);

	GLint tex_normAttrib = glGetAttribLocation(phongProgram, "inNormal");
	glVertexAttribPointer(tex_normAttrib, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float))); //normals second
	glEnableVertexAttribArray(tex_normAttrib);

	//2.1 TEX COORDS --> textured_vbos[1]
	glBindBuffer(GL_ARRAY_BUFFER, textured_vbos[1]);
	glBufferData(GL_ARRAY_BUFFER, num_nodes * 2 * sizeof(float), texturedCoords, GL_STATIC_DRAW);

	//2.2 setup texture coord attributes --> need to set now while textured_vbos[1] is bound
	GLint tex_texAttrib = glGetAttribLocation(phongProgram, "inTexcoord");
	glVertexAttribPointer(tex_texAttrib, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0);
	glEnableVertexAttribArray(tex_texAttrib);

	//3. INDICES --> textured_ibo
	glGenBuffers(1, textured_ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, textured_ibo[0]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, total_triangles * 3 * sizeof(unsigned int), texturedIndices, GL_STATIC_DRAW);

	glBindVertexArray(0);

	glEnable(GL_DEPTH_TEST);
	return true;
}

/*--------------------------------------------------------------*/
// draw : loops through node array and draws each + floor
/*--------------------------------------------------------------*/
void World::draw(Camera * cam)
{
	glClearColor(.2f, 0.4f, 0.8f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if (cloth_state == SKELETON)
	{
		drawSkeleton(cam);
	}
	else if (cloth_state == TEXTURED)
	{
		drawTextured(cam);
	}
}

/*--------------------------------------------------------------*/
// update : loops through and updates attributes of springs and masses
/*--------------------------------------------------------------*/
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
		#pragma omp parallel for schedule(dynamic)
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
		#pragma omp parallel for schedule(dynamic)
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
		Vec3D vel_i;
		for (int i = 0; i < num_nodes; i++)
		{
			if (node_arr[i]->getIsFixed())
			{
				node_arr[i]->setVel(Vec3D(0,0,0));
			}
			else
			{
				vel_i = node_arr[i]->getVel();
				node_arr[i]->setVel(vel_i + gravity + windV);
			}
		}

		//3. set positions accordingly
		Vec3D temp_pos, new_pos, new_vel;

		for (int i = 0; i < num_nodes; i++)
		{
			if (node_arr[i]->getIsFixed() == false) //nly bother if they're not fixed
			{
				temp_pos = node_arr[i]->getPos() + dt*node_arr[i]->getVel();
				checkForCollisions(temp_pos, node_arr[i]->getVel(), dt, new_pos, new_vel);
				//check for collisions!
				node_arr[i]->setVel(new_vel);
				node_arr[i]->setPos(new_pos);
			}
		}
	}//FOR - substeps
}//END update()

/*--------------------------------------------------------------*/
// init : initializes masses and springs using the width and height parameters
/*--------------------------------------------------------------*/
void World::init()
{
	Vec3D pos;
	Node* n;

	//center cloth on (0,0)
	//flattened 2D -> 1D arrays
	node_arr = new Node*[num_nodes];
	printf("\nAllocated array of %i Nodes.\n", num_nodes);

	#pragma omp parallel for schedule(dynamic)
	for (int i = 0; i < width; i++)
	{
		for (int j = 0; j < height; j++)
		{
			pos.setX((i - (width/2))*restlen); //draw left to right
			pos.setY(((height/2) - j)*restlen); //draw top to bottom
			n = new Node(pos);

			//green cube
			Material mat = Material();
			mat.setAmbient(glm::vec3(0, 1, 0));
			mat.setDiffuse(glm::vec3(0, 1, 0));
			mat.setSpecular(glm::vec3(0.75, 0.75, 0.75));
			n->setMaterial(mat);
			n->setVertexInfo(CUBE_START, CUBE_VERTS);
			n->setSize(Vec3D(0.25,0.25,0.05));

			node_arr[i + j*width] = n;
		}
	}

	if (pin_state == PIN_TOP)
	{
		for (int i = 0; i < width; i++)
		{
			node_arr[i]->setFixed(true);
		}
	}
	else if(pin_state == PIN_CORNERS)
	{
		node_arr[0]->setFixed(true);
		node_arr[width-1]->setFixed(true);
	}

	loadLineVertices();

	//initialize floor to be below cloth
	floor = new WorldObject(Vec3D(0,-0.5*height - 2, 0));
	floor->setVertexInfo(CUBE_START, CUBE_VERTS);

	Material mat = Material();
	mat.setAmbient(glm::vec3(0.7, 0.7, 0.7));
	mat.setDiffuse(glm::vec3(0.7, 0.7, 0.7));
	mat.setSpecular(glm::vec3(0, 0, 0));

	floor->setMaterial(mat);
	floor->setSize(Vec3D(width*5, 0.1, width)); //xz plane

	//initialize a sphere
	sphere = new WorldObject(Vec3D(0, -0.5*height, 0));
	sphere->setVertexInfo(SPHERE_START, SPHERE_VERTS);

	Material mat2 = Material();
	mat2.setAmbient(glm::vec3(0.5, 0, 1));
	mat2.setDiffuse(glm::vec3(0.5, 0, 1));
	mat2.setSpecular(glm::vec3(0.5, 0.5, 0.5));

	sphere->setMaterial(mat2);
	sphere->setSize(Vec3D(3,3,3));

	//initialize cloth material
	cloth_mat.setAmbient(glm::vec3(0.5,0.5,0.5));
	cloth_mat.setDiffuse(glm::vec3(0.5,0.5,0.5));
	cloth_mat.setSpecular(glm::vec3(0, 0, 0));
}//END init

/*--------------------------------------------------------------*/
// moveClothby : add velocity v to top row of Nodes
/*--------------------------------------------------------------*/
void World::moveClothBy(Vec3D v)
{
	for (int i = 0; i < width; i++)
	{
		node_arr[i]->setPos(node_arr[i]->getPos() + v);
	}
}

/*--------------------------------------------------------------*/
// reset : set back to initial state
/*--------------------------------------------------------------*/
void World::reset()
{
	//restlen = 1.0;
	for (int i = 0; i < num_nodes; i++)
	{
		delete node_arr[i];
	}

	delete [] node_arr;
	delete floor;
	delete sphere;
	init();
}//END reset

/*--------------------------------------------------------------*/
// moveSphereby : add velocity v to sphere to displace
/*--------------------------------------------------------------*/
void World::moveSphereBy(Vec3D v)
{
	sphere->setPos(sphere->getPos() + v);
}

/*----------------------------*/
// PRIVATE FUNCTIONS
/*----------------------------*/
void World::drawNodes()
{
	for (int i = 0; i < num_nodes; i++)
	{
			node_arr[i]->draw(phongProgram);
	}
}//END drawNodes

void World::drawSprings()
{
	loadLineVertices();
	glBufferData(GL_ARRAY_BUFFER, total_springs * 6 * sizeof(float), lineData, GL_STREAM_DRAW);
	glLineWidth(2);
	glDrawArrays(GL_LINES, 0, total_springs * 2);
}//END drawSprings

/*--------------------------------------------------------------*/
// drawSkeleton : draws Nodes and springs
/*--------------------------------------------------------------*/
void World::drawSkeleton(Camera* cam)
{
	glUseProgram(phongProgram); //Set the active shader (only one can be used at a time)

	//vertex shader uniforms
	GLint uniView = glGetUniformLocation(phongProgram, "view");
	GLint uniProj = glGetUniformLocation(phongProgram, "proj");
	GLint uniTexID = glGetUniformLocation(phongProgram, "texID");

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
	glUniform1i(glGetUniformLocation(phongProgram, "tex0"), 0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, tex1);
	glUniform1i(glGetUniformLocation(phongProgram, "tex1"), 1);

	glBindVertexArray(model_vao);
	glBindBuffer(GL_ARRAY_BUFFER, model_vbo[0]); //Set the model_vbo as the active VBO

	glUniform1i(uniTexID, -1); //Set texture ID to use for nodes and springs (-1 = no texture)
	drawNodes();

	glUniform1i(uniTexID, 1); //Set texture ID to use for floor (1 = stone)
	floor->draw(phongProgram);

	glUniform1i(uniTexID, -1);
	sphere->draw(phongProgram);

	glUseProgram(flatProgram);
	glBindVertexArray(line_vao);
	glBindBuffer(GL_ARRAY_BUFFER, line_vbo[0]); //Set the line_vbo as the active

	//new uniforms for the flat shading program
	GLint uniLineModel = glGetUniformLocation(flatProgram, "model");
	GLint uniLineView = glGetUniformLocation(flatProgram, "view");
	GLint uniLineProj = glGetUniformLocation(flatProgram, "proj");
	glm::mat4 model;
	glUniformMatrix4fv(uniLineModel, 1, GL_FALSE, glm::value_ptr(model));
	glUniformMatrix4fv(uniLineView, 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(uniLineProj, 1, GL_FALSE, glm::value_ptr(proj));

	drawSprings();
}

/*--------------------------------------------------------------*/
// drawTextured : draws textured cloth
/*--------------------------------------------------------------*/
void World::drawTextured(Camera* cam)
{
	glUseProgram(phongProgram); //Set the active shader (only one can be used at a time)

	//vertex shader uniforms
	GLint uniView = glGetUniformLocation(phongProgram, "view");
	GLint uniProj = glGetUniformLocation(phongProgram, "proj");
	GLint uniTexID = glGetUniformLocation(phongProgram, "texID");

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
	glUniform1i(glGetUniformLocation(phongProgram, "tex0"), 0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, tex1);
	glUniform1i(glGetUniformLocation(phongProgram, "tex1"), 1);

	glBindVertexArray(textured_vao);
	glBindBuffer(GL_ARRAY_BUFFER, textured_vbos[0]); //bind pos + norm VBO

	//load new positions and normals
	loadTexturedPosAndNorm();
	glBufferData(GL_ARRAY_BUFFER, num_nodes * 6 * sizeof(float), texturedData, GL_STREAM_DRAW); //dynamic

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, textured_ibo[0]);
	glUniform1i(uniTexID, 0);

	//set up uniforms for shader
	GLint uniModel = glGetUniformLocation(phongProgram, "model");

	glm::mat4 model;
	glUniformMatrix4fv(uniModel, 1, GL_FALSE, glm::value_ptr(model));

	//fragment shader uniforms (from Material)
	GLint uniform_ka = glGetUniformLocation(phongProgram, "ka");
	GLint uniform_kd = glGetUniformLocation(phongProgram, "kd");
	GLint uniform_ks = glGetUniformLocation(phongProgram, "ks");
	GLint uniform_s = glGetUniformLocation(phongProgram, "s");

	glm::vec3 mat_AMB = cloth_mat.getAmbient();
	glUniform3f(uniform_ka, mat_AMB[0], mat_AMB[1], mat_AMB[2]);

	glm::vec3 mat_DIF = cloth_mat.getDiffuse();
	glUniform3f(uniform_kd, mat_DIF[0], mat_DIF[1], mat_DIF[2]);

	glm::vec3 mat_SPEC = cloth_mat.getSpecular();
	glUniform3f(uniform_ks, mat_SPEC[0], mat_SPEC[1], mat_SPEC[2]);

	glUniform1f(uniform_s, cloth_mat.getNS());

	glDrawElements(GL_TRIANGLES, total_triangles * 3, GL_UNSIGNED_INT, 0);
	//glDrawArrays(GL_TRIANGLES, 0, total_triangles);

	//switch to model_vao to render floor and sphere
	glBindVertexArray(model_vao);
	glBindBuffer(GL_ARRAY_BUFFER, model_vbo[0]); //Set the model_vbo as the active VBO

	glUniform1i(uniTexID, 1); //Set texture ID to use for floor (1 = stone)
	floor->draw(phongProgram);

	glUniform1i(uniTexID, -1);
	sphere->draw(phongProgram);
}

/*--------------------------------------------------------------*/
// loadLineVertices : loop through nodes array and plug positions into lineData
/*--------------------------------------------------------------*/
void World::loadLineVertices()
{
	int count = 0;
	Vec3D pi;
	Vec3D pii;

	//horizontal first
	for (int i = 0; i < num_nodes; i++)
	{
		if ((i+1) % width != 0) //don't apply to right column
		{
			pi = node_arr[i]->getPos();
			pii = node_arr[i+1]->getPos();

			util::loadVecValues(lineData, pi, count);
			util::loadVecValues(lineData, pii, count);
		}
	}

	//vertical
	for (int i = 0; i < width*(height-1); i++) //don't apply to bottom row
	{
		pi = node_arr[i]->getPos();
		pii = node_arr[i+width]->getPos();

		util::loadVecValues(lineData, pi, count);
		util::loadVecValues(lineData, pii, count);
	}
}//END loadLineVertices

/*--------------------------------------------------------------*/
// checkForCollisions : takes possible position and current velocity
//											calculates new velocity after collision
/*--------------------------------------------------------------*/
void World::checkForCollisions(Vec3D in_pos, Vec3D in_vel, double dt, Vec3D& out_pos, Vec3D& out_vel)
{
	//1. check with floor
	Vec3D f_size = floor->getSize();
	Vec3D f_pos = floor->getPos();
	float COR = 0.2;

	Vec3D f_dist = in_pos - f_pos;
	float distalong_n = dotProduct(f_dist, floor_normal);
	float sizealong_n = dotProduct(f_size, floor_normal);

	if (distalong_n < sizealong_n + 0.1
				&& fabs(f_dist.getX()) < f_size.getX()/2
	 			&& fabs(f_dist.getZ()) < f_size.getZ()/2)
	{
		out_vel = util::calcCollisionVel(in_vel, floor_normal, COR);
		out_pos = in_pos + (dt+0.01)*out_vel;
		return;
	}

	//2. check with sphere
	Vec3D s_size = sphere->getSize();
	Vec3D s_pos = sphere->getPos();

	Vec3D s_dist = in_pos - s_pos;
	float dist_sq = dotProduct(s_dist, s_dist);
	//float r_sq = dotProduct(s_size, s_size);
	float r_sq = s_size.getX();

	if (dist_sq < r_sq)
	{
		s_dist.normalize();//radial vector normalized
		out_vel = util::calcCollisionVel(in_vel, s_dist, COR);

		//move outside of sphere along radial vector
		out_pos = s_pos + (sqrt(r_sq) + 0.0001)*s_dist;
		return;
	}

	//no collisions
	out_vel = in_vel;
	out_pos = in_pos;
}

/*--------------------------------------------------------------*/
// loadTexturedPosAndNorm : calculates & stores vertex position
//													and normal data in texturedData
/*--------------------------------------------------------------*/
void World::loadTexturedPosAndNorm()
{
	int cur_index = 0;
	Vec3D vec1, vec2, pos_i, temp_norm;

	for (int i = 0; i < num_nodes; i++)
	{
		pos_i = node_arr[i]->getPos();

		//1. load position
		util::loadVecValues(texturedData, pos_i, cur_index);

		//2. calc and load normal
		if ((i >= width * (height - 1)) && ((i+1)%width == 0)) //bottom right corner
		{
			vec1 = node_arr[i-width]->getPos() - pos_i;
			vec2 = node_arr[i-1]->getPos() - pos_i;
		}
		else if (i >= width * (height - 1)) //bottom
		{
			vec2 = node_arr[i-width]->getPos() - pos_i;
			vec1 = node_arr[i+1]->getPos() - pos_i;
		}
		else if ((i+1)%width == 0) //right side
		{
			vec1 = node_arr[i-1]->getPos() - pos_i;
			vec2 = node_arr[i+width]->getPos() - pos_i;
		}
		else //all not special cases
		{
			vec1 = node_arr[i+width]->getPos() - pos_i;
			vec2 = node_arr[i+1]->getPos() - pos_i;
		}

		temp_norm = cross(vec1, vec2);
		temp_norm.normalize();

		util::loadVecValues(texturedData, temp_norm, cur_index);
	}//END for
}//END loadTexturedPosAndNorm

/*--------------------------------------------------------------*/
// loadTextureCoords :
/*--------------------------------------------------------------*/
void World::loadTextureCoords()
{
	int cur_index = 0;
	float u = 0, v = 0;

	for (int row = 0; row < height; row++)
	{
		//1. v is constant along each row
		v = util::interp(1, 0, (float)row/(height - 1));

		for (int col = 0; col < width; col++)
		{
			//2. calc u
			u = util::interp(0, 1, (float)col/(width - 1));

			//3. store in texturedCoords (u,v)
			texturedCoords[cur_index++] = u;
			texturedCoords[cur_index++] = v;

		}//END col - for
	}//END row - for

	cout << "**" << cur_index << " texture coordinates loaded for texturing**" << endl;
}//END loadTextureCoords

/*--------------------------------------------------------------*/
// loadTexturedIndices :
/*--------------------------------------------------------------*/
void World::loadTexturedIndices()
{
	int cur_index = 0;

	for (int i = 0; i < width*(height-1); i++) //don't apply to bottom row
	{
		if ((i+1) % width != 0) //not the right side
		{
			//lower triangle
			texturedIndices[cur_index++] = i;
			texturedIndices[cur_index++] = i + width;
			texturedIndices[cur_index++] = i + width + 1;

			//upper triangle
			texturedIndices[cur_index++] = i;
			texturedIndices[cur_index++] = i + width + 1;
			texturedIndices[cur_index++] = i + 1;
		}
	}

	cout << "**" << cur_index << " indices loaded for texturing**" << endl;
}//END loadTexturedIndices
