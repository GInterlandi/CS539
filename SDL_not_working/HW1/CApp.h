
#ifndef _CAPP_H_
#define _CAPP_H_
#include "stdafx.h"
#include "Angel.h"
#include "glm.h"
#include <SDL.h>
#include <SDL_opengl.h>
#include <chrono>

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

typedef Angel::vec4 point4;
typedef Angel::vec4 color4;

#define NumTriangles 8
#define NumVertices 3 * NumTriangles

#define degrees2Radians 0.01745329251
#define SCALE_ANGLE     60.0 
#define SCALE_TRANS     1.0 

class CApp {
private:
	bool    Running;

	SDL_Window*		Main_window;
	SDL_GLContext   GLcontext;

	void	quad( int a, int b, int c, int d );
	void	cyllMesh(); 
	void GLinit(); // Initiate buffers, VAOs ans shaders

	int Index;

	point4 vertices[8];
point4  points[NumVertices];
vec3    quad_Normal[NumVertices];
vec2    tex_coords[NumVertices];

	GLuint buffer;
	GLuint vao; 
	GLuint program; 
	//Attributes 
	GLuint vPosition;
	GLuint vNormal;
	GLuint vTexCoord; 
	mat4   model_view;
	mat4   projmat;  
	GLuint  ModelViewCam; //Viewing portion of the ModelviewMatrix  uniform location
GLuint ModelViewObj;  // Object Modeling  portion of the ModelviewMatrix  uniform location
GLuint Projection;    //Pprojection matrices uniform location
GLuint NormalTransformation;  //Normal matrix uniform location
GLuint LightPosition;  //Light postition uniform location //GL variables

	GLubyte* image0; 
	GLuint texture;
	int  TextureWidth;
	int  TextureHeight;  //Texture Variables

	// Setup some lights
  vec4 light_ambient;
  vec4 light_diffuse;
  vec4 light_specular;
 //LIGHT POSITION
  vec4 light_position;

  vec4 material_ambient;
  vec4 material_diffuse;
  vec4 material_specular;
  float  material_shininess;

  vec4 ambient_product ;
  vec4 diffuse_product;
  vec4 specular_product ;

  vec4 u, v, n ,eye;
  mat4 bases;

  bool key_x,key_X,key_y,key_Y,key_z,key_Z;
  bool key_s,key_w,key_a,key_d;
  float roll,pitch,yaw;
  int dir;
std::chrono::time_point<std::chrono::system_clock> t_roll,t_pitch,t_yaw;
  std::chrono::time_point<std::chrono::system_clock> t_rot,t_trans;
public:
	CApp();

	int OnExecute();

	bool OnInit();

	void OnEvent(SDL_Event* Event);

	void OnLoop();

	void OnRender();

	void OnCleanup();
};

#endif