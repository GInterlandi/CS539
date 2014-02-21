//CS 537
//A stub using GLM08 to load and Render .obj models 
// The models polygons are organized in groups based on their materials
//G. Kamberov
#include "stdafx.h"
#include <iostream>
#include "Angel.h"
#include "glm.h"

using namespace std;

typedef Angel::vec4  color4;
typedef Angel::vec4  point4;

void specKey(int key, int x, int y);

#define X               0
#define Y               1
#define Z               2
#define SCALE_VECTOR    1.0
#define SCALE_ANGLE     1.0s

GLMmodel* models[3];
int VBO_num_vert;
int VBO_num_norm;
int num_vert;

#define SCALE_VECTOR    0.5

GLuint vPosition;
GLuint vNormal;
int winHeight = 480;
int winWidth = 640;
bool mouseDown = false;
float xrot = -20;
float yrot = 0;
float xdiff = 0;
float ydiff = 0;
GLfloat step = 5; // misc
// Camera Coordianate System
vec4 u = vec4(1, 0, 0, 0);
vec4 v = vec4(0, 1, 0, 0);
vec4 n = vec4(0, 0, 1, 0);
vec4 eye = vec4(0,2,10,1);

GLuint program[2];
GLuint vao;
GLuint buffers[2];
GLuint color_loc;
GLuint proj_loc;
GLuint model_view_loc;// shader variables // shader variables

// Model-view and projection matrices uniform location
GLuint  ModelViewCam, ModelViewObj, Projection, LightPosition, NormalTransformation;

point4 cube_vertices_positions[24] = {
	//x_positive
	point4( 1.0f, -1.0f, -1.0f, 1.0f),
	point4( 1.0f, -1.0f,  1.0f, 1.0f),
	point4( 1.0f,  1.0f,  1.0f, 1.0f),
	point4( 1.0f,  1.0f, -1.0f, 1.0f),
	//x-negative
	point4(-1.0f, -1.0f,  1.0f, 1.0f),
	point4(-1.0f, -1.0f, -1.0f, 1.0f),
	point4(-1.0f,  1.0f, -1.0f, 1.0f),
	point4(-1.0f,  1.0f,  1.0f, 1.0f),
	//y-positive
	point4(-1.0f,  1.0f, -1.0f, 1.0f),
	point4( 1.0f,  1.0f, -1.0f, 1.0f),
	point4( 1.0f,  1.0f,  1.0f, 1.0f),
	point4(-1.0f,  1.0f,  1.0f, 1.0f),
	//y-negatve
	point4(-1.0f, -1.0f,  1.0f, 1.0f),
	point4( 1.0f, -1.0f,  1.0f, 1.0f),
	point4( 1.0f, -1.0f, -1.0f, 1.0f),
	point4(-1.0f, -1.0f, -1.0f, 1.0f),
	//z-positive
	point4( 1.0f, -1.0f,  1.0f, 1.0f),
	point4(-1.0f, -1.0f,  1.0f, 1.0f),
	point4(-1.0f,  1.0f,  1.0f, 1.0f),
	point4( 1.0f,  1.0f,  1.0f, 1.0f),
	//z-negative
	point4(-1.0f, -1.0f, -1.0f, 1.0f),
	point4( 1.0f, -1.0f, -1.0f, 1.0f),
	point4( 1.0f,  1.0f, -1.0f, 1.0f),
	point4(-1.0f,  1.0f, -1.0f, 1.0f)
};

//Skybox struc
GLuint texture;
typedef struct
{

	int topWidth;
	int topHeight;
	GLubyte *top;

	int bottomWidth;
	int bottomHeight;
	GLubyte *bottom;

	int leftHeight;
	int leftWidth;
	GLubyte *left;


	int rightWidth;
	int rightHeight;
	GLubyte *right;

	int frontWidth;
	int frontHeight;
	GLubyte *front;

	int backWidth;
	int backHeight;
	GLubyte *back;

} CubeMap;

CubeMap skybox;
// A generate 12 flat shaded triangles using pointers to vertices (from 8 // vertices)

GLubyte indices[]= {
	1, 0, 3, 
	1, 3, 2, 
	2, 3, 7, 
	2, 7, 6, 
	3, 0, 4, 
	3, 4, 7, 
	6, 5, 1, 
	6, 1, 2, 
	4, 5, 6, 
	4, 6, 7, 
	5, 4, 0, 
	5, 0, 1
};   // Skybox vertices and indices

// OpenGL initialization
void init()
{
	glGenBuffers( 2, buffers );
	glBindBuffer( GL_ARRAY_BUFFER, buffers[0] ); 
	glBufferData( GL_ARRAY_BUFFER, sizeof(cube_vertices_positions), cube_vertices_positions, GL_STATIC_DRAW );

	std::cout<< "sizeof(cube_vertices_positions)" << sizeof(cube_vertices_positions) << std::endl;
	

	//Elements buffer for the pointers
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, buffers[1] ); 
	glBufferData( GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW); 
	glGenVertexArrays(1, &vao);  
	// Load the OBJ models from file
  models[0] = glmReadOBJ("data/Peugeot_207.obj");
  if (!models[0]) exit(0);
  // Normilize vertices
  glmUnitize(models[0]);
  // Compute facet normals
  glmFacetNormals(models[0]);
  // Comput vertex normals
  glmVertexNormals(models[0], 90.0);  
  // Load the model (vertices and normals) into a vertex buffer
  glmLoadInVBO(models[0]);
  // Setup some sample materials
  color4 light_ambient( 0.2, 0.2, 0.2, 1.0 );
  color4 light_diffuse( 1.0, 1.0, 1.0, 1.0 );
  color4 light_specular( 1.0, 1.0, 1.0, 1.0 );
  color4 material_ambient( 1.0, 0.0, 1.0, 1.0 );
  color4 material_diffuse( 1.0, 0.8, 0.0, 1.0 );
  color4 material_specular( 1.0, 0.8, 0.0, 1.0 );
  float  material_shininess = 100.0;
  color4 ambient_product = light_ambient * material_ambient;
  color4 diffuse_product = light_diffuse * material_diffuse;
  color4 specular_product = light_specular * material_specular;
  //LIGHT POSITION
  vec4 light_position_distant = vec4(1.0, -1.0, -1.0, 0.0);

  // Load shaders and use the resulting shader program
  program[0] = InitShader( "./shaders/shader_vert.glsl", "./shaders/shader_frag.glsl" );
  	program[1] = InitShader( "./shaders/skyboxvertex.glsl", "./shaders/skyboxfragment.glsl" ); 

  glUseProgram( program[0] );
  // set up vertex arrays
  glBindVertexArray( models[0]->vao );

  glUseProgram( program[0] );

  glUniform4fv( glGetUniformLocation(program[0], "light_ambient"),1, light_ambient);
  glUniform4fv( glGetUniformLocation(program[0], "light_diffuse"),1, light_diffuse);
  glUniform4fv( glGetUniformLocation(program[0], "light_specular"),1, light_specular);	
  glUniform4fv( glGetUniformLocation(program[0], "lightPosition"),1, light_position_distant );
  // Retrieve transformation uniform variable locations
  ModelViewCam = glGetUniformLocation( program[0], "modelView" );
  ModelViewObj = glGetUniformLocation(program[0], "ModelViewObj");
  Projection = glGetUniformLocation( program[0], "projection" );
  NormalTransformation = glGetUniformLocation( program[0], "normalTransformation" );

  glUseProgram( program[1] );
	glBindVertexArray(vao);
	glBindBuffer( GL_ARRAY_BUFFER, buffers[0] ); 
	vPosition = glGetAttribLocation( program[1], "vPosition" );
	glEnableVertexAttribArray( vPosition );
	glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0) );
	//done with this packet
	//glBindVertexArray(0); 
	//glUseProgram(0);

  glClearDepth( 1.0 ); 
  glEnable( GL_DEPTH_TEST );
  glDepthFunc(GL_LESS);
  glClearColor( 0.0, 0.0, 0.0, 0.0 ); 
}

//----------------------------------------------------------------------------

void display( void )
{
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram( program[0] );
  	glBindVertexArray(models[0]->vao);
  // Setup camera
  mat4 modelViewCamera = LookAt(eye, eye-n, v);
  glUniformMatrix4fv(ModelViewCam, 1, GL_TRUE, modelViewCamera);

 
  float scale_x, scale_y, scale_z;
  mat4 scaleTransformation;
  mat4 invScaleTranformation;
  mat4 normalMatrix;
  mat4 modelViewObject;

  // _________________________________Load the first model
  scale_x = 2;
  scale_y = 2;
  scale_z =2;
  // Scale Transformation Matrix
  scaleTransformation = Scale(scale_x, scale_y, scale_z);
  // Inverse Scale Transformation Matrix 
  invScaleTranformation = Scale(1/scale_x, 1/scale_y, 1/scale_z);
  normalMatrix =  RotateX( xrot ) * RotateY( yrot ) * invScaleTranformation;
  modelViewObject = Translate(0.0, 0.0, 2.0)*RotateX( xrot ) * RotateY( yrot )*scaleTransformation;
  glUniformMatrix4fv( ModelViewObj, 1, GL_TRUE, modelViewObject );
  glUniformMatrix4fv(NormalTransformation , 1, GL_TRUE,  normalMatrix);
  glmDrawVBO(models[0], program[0]);



  glutSwapBuffers();
}

//----------------------------------------------------------------------------



//----------------------------------------------------------------------------

void reshape( int width, int height )
{
 glViewport( 0, 0, width, height );
  GLfloat aspect = GLfloat(width)/height;
  mat4  projection = Perspective( 45.0, aspect, 0.0001, 300.0 );
  glUniformMatrix4fv( Projection, 1, GL_TRUE, projection );
}


// Get OpenGL version information
void getGLinfo()
{
  cout << "GL Vendor   : " << glGetString(GL_VENDOR) << endl;
  cout << "GL Renderer : " << glGetString(GL_RENDERER) << endl;
  cout << "GL Version  : " << glGetString(GL_VERSION) << endl;
}

int main( int argc, char **argv )
{
  glutInit( &argc, argv );
  glutInitDisplayMode( GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH );
  glutInitWindowSize( winWidth, winHeight );
  glutCreateWindow( "Loading OBJ Models into VBO" );
  glewInit();
  getGLinfo();
  init();
  glutDisplayFunc( display );
  glutReshapeFunc( reshape );
  	glutSpecialFunc( specKey );
  glutMainLoop();
  return 0;
}

void specKey(int key, int x, int y)
{ 
	switch( key ) {
	case GLUT_KEY_UP: // MOVE FORWARD
		eye[0] -= SCALE_VECTOR * n[0];
		eye[1] -= SCALE_VECTOR * n[1];
		eye[2] -= SCALE_VECTOR * n[2];
		break;
	case GLUT_KEY_DOWN: // MOVE BACKWARD
		eye[0] += SCALE_VECTOR * n[0];
		eye[1] += SCALE_VECTOR * n[1];
		eye[2] += SCALE_VECTOR * n[2];
		break;
	case GLUT_KEY_LEFT: // MOVE FORWARD
		eye[0] -= SCALE_VECTOR * u[0];
		eye[1] -= SCALE_VECTOR * u[1];
		eye[2] -= SCALE_VECTOR * u[2];
		break;
	case GLUT_KEY_RIGHT: // MOVE BACKWARD
		eye[0] += SCALE_VECTOR * u[0];
		eye[1] += SCALE_VECTOR * u[1];
		eye[2] += SCALE_VECTOR * u[2];
		break;
	default:
		break;
	}
	glutPostRedisplay();
}