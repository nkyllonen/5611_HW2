//////////////////////////////////
//CSCI 5611 - HW2 - Physical Simulation
//Nikki Kyllonen
//--------------------------------
//draws a blue background SDL window
//////////////////////////////////

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

#include <cerrno>
#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <fstream>
#include <string>

//MY CLASSES
#include "Util.h"
#include "World.h"

using namespace std;

/*=============================*/
// Global Default Parameters
/*=============================*/
bool fullscreen = false;
int screen_width = 800;
int screen_height = 600;

//shader globals
string vertFile = "Shaders/phong.vert";
string fragFile = "Shaders/phong.frag";

//other globals
const float mouse_speed = 0.05f;
const float step_size = 0.15f;

enum state
{
	MOVE_CLOTH,
	MOVE_SPHERE
};

int cur_state = MOVE_CLOTH;

/*=============================*/
// Helper Functions
/*=============================*/
void onKeyDown(SDL_KeyboardEvent & event, Camera* cam, World* myWorld);
void mouseMove(SDL_MouseMotionEvent & event, Camera * player, float horizontal_angle, float vertical_angle);

/*==============================================================*/
//							  MAIN
/*==============================================================*/
int main(int argc, char *argv[]) {
	srand(time(0));

	//CHECK FOR WIDTH AND HEIGHT VALUES
	if (argc != 3)
	{
		cout << "\nERROR: Incorrect usage. Expected ./a.out WIDTH HEIGHT\n";
		exit(0);
	}

	int w = atoi(argv[1]);
	int h = atoi(argv[2]);
	printf("Cloth width: %i by height: %i\n", w, h);

	/////////////////////////////////
	//INITIALIZE SDL WINDOW
	/////////////////////////////////
	SDL_GLContext context;
	SDL_Window* window = util::initSDL(context, screen_width, screen_height);

	if (window == NULL)
	{
		cout << "ERROR: initSDL() failed." << endl;
		SDL_GL_DeleteContext(context);
		SDL_Quit();
		exit(0);
	}

	World* myWorld = new World(w, h);

	/////////////////////////////////
	//LOAD MODEL DATA INTO WORLD
	/////////////////////////////////
	if (!myWorld->loadModelData())
	{
		cout << "ERROR. Unable to load model data." << endl;
		//Clean up
		myWorld->~World();
		SDL_GL_DeleteContext(context);
		SDL_Quit();
		exit(0);
	}

	myWorld->init();

	/////////////////////////////////
	//SETUP CAMERA
	/////////////////////////////////
	Camera* cam = new Camera();
	cam->setDir(Vec3D(0, 0, 1));					//look along +z
	cam->setPos(Vec3D(0,0,-10));						//start
	cam->setUp(Vec3D(0, 1, 0));						//map is in xz plane
	cam->setRight(Vec3D(1, 0, 0));				//look along +z

	/////////////////////////////////
	//SETUP MOUSE INITIAL STATE
	/////////////////////////////////
	float horizontal_angle = 0.0f;
	float vertical_angle = 0.0f;

	/////////////////////////////////
	//BUILD VAO + VBO + SHADERS + TEXTURES
	/////////////////////////////////
	if (!myWorld->setupGraphics())
	{
		//Clean Up
		SDL_GL_DeleteContext(context);
		SDL_Quit();
		myWorld->~World();
		cam->~Camera();
	}

	/*===========================================================================================
	* EVENT LOOP (Loop forever processing each event as fast as possible)
	* List of keycodes: https://wiki.libsdl.org/SDL_Keycode - You can catch many special keys
	* Scancode referes to a keyboard position, keycode referes to the letter (e.g., EU keyboards)
	===========================================================================================*/
	SDL_Event windowEvent;
	bool quit = false;
	bool mouse_active = false;
	bool recentering = true;
	float mouse_x, mouse_y;

	//FPS calculations
	float new_time = 0;
	float framecount = 0;
	float fps = 0, last_fps_print = 0.0;

	while (!quit)
	{
		if (SDL_PollEvent(&windowEvent)) {
			switch (windowEvent.type) //event type -- key up or down
			{
				case SDL_QUIT:
					quit = true; //Exit event loop
					break;
				case SDL_KEYDOWN:
					//check for escape or fullscreen before checking other commands
					if (windowEvent.key.keysym.sym == SDLK_ESCAPE) quit = true; //Exit event loop
					else if (windowEvent.key.keysym.sym == SDLK_f) fullscreen = !fullscreen;
					onKeyDown(windowEvent.key, cam, myWorld);
					break;
				case SDL_MOUSEMOTION:
					if (recentering)
					{
						SDL_WarpMouseInWindow(window, screen_width / 2, screen_height / 2);
						mouse_active = true;
					}
					else if (mouse_active && !recentering)
					{
						mouse_x = windowEvent.motion.x;
						mouse_y = windowEvent.motion.y;
						mouseMove(windowEvent.motion, cam, horizontal_angle, vertical_angle);
						// recentering = true;
					}
				default:
					break;
				}//END polling switch

			SDL_SetWindowFullscreen(window, fullscreen ? SDL_WINDOW_FULLSCREEN : 0); //Set to full screen
		}//END polling If

		if (mouse_active)
		{
			recentering = false;
		}

		//draw all WObjs
		myWorld->draw(cam);

		new_time = SDL_GetTicks();

		if ((new_time - last_fps_print) / 1000.0 >= 1.0) //only print every 1+ seconds
		{
			fps = framecount;
			last_fps_print = new_time;
			printf("FPS: %f\n", fps);
			framecount = 0;
		}

		myWorld->update(1.0e-3);
		SDL_GL_SwapWindow(window);
		framecount++;

	}//END looping While

	//Clean Up
	SDL_GL_DeleteContext(context);
	SDL_Quit();
	myWorld->~World();
	cam->~Camera();

	return 0;
}//END MAIN

/*--------------------------------------------------------------*/
// onKeyDown : determine which key was pressed and how to edit
//				current translation or rotation parameters
/*--------------------------------------------------------------*/
void onKeyDown(SDL_KeyboardEvent & event, Camera* cam, World* myWorld)
{
	Vec3D pos = cam->getPos();
	Vec3D dir = cam->getDir();
	Vec3D right = cam->getRight();
	Vec3D up = cam->getUp();

	//temps to be modified in switch
	Vec3D temp_pos = pos;
	Vec3D temp_dir = dir;
	Vec3D temp_right = right;

	switch (event.keysym.sym)
	{
	/////////////////////////////////
	//CAMERA TRANSLATION WITH WASD
	/////////////////////////////////
	case SDLK_w:
		temp_pos = pos + (step_size*dir);
		break;
	case SDLK_s:
		temp_pos = pos - (step_size*dir);		//for some reason opposite to what I thought?
		break;
	case SDLK_d:
		temp_pos = pos - (step_size*right);	//for some reason opposite to what I thought?
		break;
	case SDLK_a:
		temp_pos = pos + (step_size*right);
		break;
	/////////////////////////////////
	//CLOTH TRANSLATION WITH ARROWS
	/////////////////////////////////
	case SDLK_UP:
		if (cur_state == MOVE_CLOTH)
		{
			//add up velocity to top row
			myWorld->moveClothBy(Vec3D(0,step_size,0));
		}
		else if (cur_state == MOVE_SPHERE)
		{
			myWorld->moveSphereBy(Vec3D(0,step_size,0));
		}

		break;
	case SDLK_DOWN:
		if (cur_state == MOVE_CLOTH)
		{
			//add down velocity to top row
			myWorld->moveClothBy(Vec3D(0,-step_size,0));
		}
		else if (cur_state == MOVE_SPHERE)
		{
			myWorld->moveSphereBy(Vec3D(0,-step_size,0));
		}
		break;
	case SDLK_LEFT:
		if (cur_state == MOVE_CLOTH)
		{
			//add left velocity to top row
			myWorld->moveClothBy(Vec3D(step_size,0,0));
		}
		else if (cur_state == MOVE_SPHERE)
		{
			myWorld->moveSphereBy(Vec3D(step_size,0,0));
		}
		break;
	case SDLK_RIGHT:
		if (cur_state == MOVE_CLOTH)
		{
			//add right velocity to top row
			myWorld->moveClothBy(Vec3D(-step_size,0,0));
		}
		else if (cur_state == MOVE_SPHERE)
		{
			myWorld->moveSphereBy(Vec3D(-step_size,0,0));
		}
		break;
	/////////////////////////////////
	//ADJUST SPRING REST LEN WITH +/-
	/////////////////////////////////
	case SDLK_PLUS:
	case SDLK_KP_PLUS:
		myWorld->adjustRestLen(0.05);
		break;
	case SDLK_MINUS:
	case SDLK_KP_MINUS:
		myWorld->adjustRestLen(-0.05);
		break;
	/////////////////////////////////
	//RESET TO ORIGINAL POSITIONS ON 0
	/////////////////////////////////
	case SDLK_0:
	case SDLK_KP_0:
		myWorld->reset();
		break;
	/////////////////////////////////
	//CHANGE STATE WITH 1/2
	/////////////////////////////////
	case SDLK_1:
	case SDLK_KP_1:
		cur_state = MOVE_CLOTH;
		cout << "--State changed to MOVE_CLOTH--" << endl;
		break;
	case SDLK_2:
	case SDLK_KP_2:
		cur_state = MOVE_SPHERE;
		cout << "--State changed to MOVE_SPHERE--" << endl;
		break;
	/////////////////////////////////
	//CHANGE CLOTH STATE WITH T
	/////////////////////////////////
	case SDLK_t:
		cout << "--Cloth state changed--" << endl;
		(myWorld->cloth_state == SKELETON) ? myWorld->cloth_state = TEXTURED : myWorld->cloth_state = SKELETON;
		break;
	/////////////////////////////////
	//CHANGE PIN STATE WITH P
	/////////////////////////////////
	case SDLK_p:
		cout << "--Pin state changed--" << endl;
		(myWorld->pin_state == PIN_TOP) ? myWorld->pin_state = PIN_CORNERS : myWorld->pin_state = PIN_TOP;
		break;
	/////////////////////////////////
	//TURN WIND ON/OFF WITH 3
	/////////////////////////////////
	case SDLK_3:
	case SDLK_KP_3:
	{
		Vec3D w = myWorld->windV;
		if (w.z != 0)
		{
			cout << "--Turning wind off" << endl;
			w.z = 0;
		}
		else
		{
			cout << "--Turning wind on" << endl;
			w.z = 0.001;
		}
		myWorld->windV = w;
		break;
	}
	default:
		printf("ERROR: Invalid key pressed (%s)\n", SDL_GetKeyName(event.keysym.sym));
		break;
	}//END switch key press

	cam->setDir(temp_dir);
	cam->setRight(temp_right);
	cam->setPos(temp_pos);
}//END onKeyUp

/*--------------------------------------------------------------*/
// mouseMove : change the view accordingly when the mouse moves!
/*--------------------------------------------------------------*/
void mouseMove(SDL_MouseMotionEvent & event, Camera * cam, float horizontal_angle, float vertical_angle)
{
	Vec3D dir = cam->getDir();
	Vec3D right = cam->getRight();
	Vec3D up = cam->getUp();

	//temps to be modified
	Vec3D temp_dir = dir;
	Vec3D temp_right = right;
	Vec3D temp_up = up;

	horizontal_angle += mouse_speed * step_size * float(screen_width / 2 - event.x);
	vertical_angle += mouse_speed * step_size * float(screen_height / 2 - event.y);

	temp_dir = dir + (Vec3D(cos(vertical_angle) * sin(horizontal_angle), sin(vertical_angle), cos(vertical_angle) * cos(horizontal_angle)));
	temp_right = right + (Vec3D(sin(horizontal_angle - 3.14f / 2.0f), 0, cos(horizontal_angle - 3.14f / 2.0f)));
	temp_up = cross(temp_dir, -1 * temp_right);

	cam->setDir(temp_dir);
	cam->setRight(temp_right);
	cam->setUp(temp_up);
}//END mouseMove
