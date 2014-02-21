//CS 537 Exmaple Sky Box
//George Kamberov
//
//A simple 3D scene:
//			A skybox using a CubeMap texture and a cube at infinity
//			
//			
//
//
// Interaction: 
//     
//      
//      Use the up/down direction (arrow)  keys to Move the camera back/forward  
//      (along the fixed viewing axis). 
//
// We use perspective projection.
//
// Shaders used: 
//  vshader30_TwoCubes_FullPipe.glsl
//  vshader6_2.glsl
//  fshader30_TwoCubes.glsl
//  fshader6_2.glsl
//
#include "stdafx.h"
#include "Angel.h"
#include "glm.h"
#include<assert.h>
#include <iostream>

///////CONSTANT/////////////////////
#ifndef M_PI
#define M_PI            3.14159265358979323846
#endif



void m_glewInitAndVersion(void);
void reshape(int width, int height);

GLuint program[3]; 
GLuint VAO[3];
GLuint buffers[5];
GLuint vPosition;
//uniform variables locations
GLuint color_loc;
GLuint proj_loc;
GLuint model_view_loc;// shader variables

enum {cube_vertices, cube_indeces, wire_indeces, cube_vertices_explicit};

typedef Angel::vec4  color4;
typedef Angel::vec4  point4;  //enum and typedef


/////Camera unv basis///////

Angel::vec4 n = vec4(0.0, 0.0, 1.0, 0.0);
Angel::vec4 v = vec4(0.0, 1.0, 0.0, 00);
Angel::vec4 u = vec4(1.0, 0.0, 0.0, 0.0);
Angel::vec4 eye = vec4(0.0, 0.0, 2.0, 1.0);

// Car load variables
GLMmodel* model_car;
int VBO_num_vert;
int VBO_num_norm;
int num_vert;
// Model-view and projection matrices uniform location
GLuint  ModelViewCam, ModelViewObj, Projection, LightPosition, NormalTransformation;


GLfloat step = 20;
GLfloat theta_y = 0;
int spinToggle = 0; 
GLfloat d=0.5;
int w; int h;    //misc variables


// Vertices of a unit cube centered at origin, sides aligned with axes
point4 vertices[8] = {
	point4( -0.5, -0.5,  0.5, 1.0 ),
	point4( -0.5,  0.5,  0.5, 1.0 ),
	point4(  0.5,  0.5,  0.5, 1.0 ),
	point4(  0.5, -0.5,  0.5, 1.0 ),
	point4( -0.5, -0.5, -0.5, 1.0 ),
	point4( -0.5,  0.5, -0.5, 1.0 ),
	point4(  0.5,  0.5, -0.5, 1.0 ),
	point4(  0.5, -0.5, -0.5, 1.0 )
};




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
};



// RGBA olors
color4 green_transparent = color4( 0.0, 1.0, 0.0, 0.1 ); 




class MatrixStack {
	int    _index;
	int    _size;
	mat4*  _matrices;

public:
	MatrixStack( int numMatrices = 32):_index(0), _size(numMatrices)
	{	_matrices = new mat4[numMatrices]; }

	~MatrixStack()
	{ delete[]_matrices; }

	void push( const mat4& m ) {
		assert( _index + 1 < _size );
		_matrices[_index++] = m;
	}

	mat4& pop( void ) {
		assert( _index - 1 >= 0 );
		_index--;
		return _matrices[_index];
	}
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


//make sure you start with the default coordinate system
mat4 projmat = mat4(1.0);

MatrixStack mvstack;

mat4 model_view = Angel::mat4(1.0);

mat4 modelviewStackTop = mat4(1.0);
mat4 modelviewStackBottom=  mat4(1.0);


//----------------------------------------------------------------------------

// OpenGL initialization
void
	init()
{

	/**
	***  Create and initialize buffer objects
	**/


	glGenBuffers( 4, buffers );
	//Vertex buffer for the vertex coordinates
	glBindBuffer( GL_ARRAY_BUFFER, buffers[cube_vertices] ); 
	glBufferData( GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW );

	glBindBuffer( GL_ARRAY_BUFFER, buffers[cube_vertices_explicit] ); 
	glBufferData( GL_ARRAY_BUFFER, sizeof(cube_vertices_positions), cube_vertices_positions, GL_STATIC_DRAW );

	std::cout<< "sizeof(cube_vertices_positions)" << sizeof(cube_vertices_positions) << std::endl;
	

	//Elements buffer for the pointers
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, buffers[cube_indeces] ); 
	glBufferData( GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW); 


	glGenVertexArrays(2, VAO);
		model_car = glmReadOBJ("data/porsche.obj");
	if (!model_car) exit(0);
	glmUnitize(model_car);
	glmFacetNormals(model_car);
	glmVertexNormals(model_car, 90.0);  
	glmLoadInVBO(model_car);
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

	// Load shaders and use the resulting shader programs
	program[0] = InitShader( "./shaders/vshader30_TwoCubes_FullPipe.glsl", "./shaders/fshader30_TwoCubes.glsl" ); 
	program[1] = InitShader( "./shaders/skyboxvertex.glsl", "./shaders/skyboxfragment.glsl" ); 
	program[2] = InitShader( "./shaders/shader_vert.glsl", "./shaders/shader_frag.glsl" );

	glUseProgram( program[2] );
	// set up vertex arrays
	glBindVertexArray( model_car->vao );

	glUniform4fv( glGetUniformLocation(program[2], "light_ambient"),1, light_ambient);
	glUniform4fv( glGetUniformLocation(program[2], "light_diffuse"),1, light_diffuse);
	glUniform4fv( glGetUniformLocation(program[2], "light_specular"),1, light_specular);	
	glUniform4fv( glGetUniformLocation(program[2], "lightPosition"),1, light_position_distant );
	// Retrieve transformation uniform variable locations
	ModelViewCam = glGetUniformLocation( program[2], "modelView" );
	ModelViewObj = glGetUniformLocation(program[2], "ModelViewObj");
	Projection = glGetUniformLocation( program[2], "projection" );
	NormalTransformation = glGetUniformLocation( program[2], "normalTransformation" );

	//VAO[1] the skybox
	glUseProgram( program[1] );
	glBindVertexArray(VAO[1]);
	glBindBuffer( GL_ARRAY_BUFFER, buffers[cube_vertices_explicit] ); 
	vPosition = glGetAttribLocation( program[1], "vPosition" );
	glEnableVertexAttribArray( vPosition );
	glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0) );
	//done with this packet
	glBindVertexArray(0); 
	glUseProgram(0);

	//VAO[0] the Cube 
	//glUseProgram( program[0] );

	//glBindVertexArray(VAO[0]);
	//glBindBuffer( GL_ARRAY_BUFFER, buffers[cube_vertices] ); 
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers[cube_indeces]);
	//vPosition = glGetAttribLocation( program[0], "vPosition" );
	//glEnableVertexAttribArray( vPosition );
	//glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0) );
	////done with this packet
	glBindVertexArray(0); 

	glUseProgram(0);

	//Load Skybox Images. 6 images to represent the 6 angles of view. Inside it's own structured Cubemap
	skybox.top = glmReadPPM("skybox\\sky-top.ppm", &skybox.topWidth, &skybox.topHeight);
	skybox.bottom = glmReadPPM("skybox\\sky-bottom.ppm", &skybox.bottomWidth, &skybox.bottomHeight);
	skybox.right = glmReadPPM("skybox\\sky-right.ppm", &skybox.rightWidth, &skybox.rightHeight);
	skybox.left = glmReadPPM("skybox\\sky-left.ppm", &skybox.leftWidth, &skybox.leftHeight);
	skybox.front = glmReadPPM("skybox\\sky-front.ppm", &skybox.frontWidth, &skybox.frontHeight);
	skybox.back = glmReadPPM("skybox\\sky-back.ppm", &skybox.backWidth, &skybox.backHeight);

	glActiveTexture(GL_TEXTURE0);

	glGenTextures(1, &texture);

	int isEnabled=0; 

	if (glIsEnabled(GL_TEXTURE_CUBE_MAP) == GL_TRUE) {isEnabled = 1;} else {isEnabled = 0;};


	std::cout << isEnabled << std::endl;

	glEnable(GL_TEXTURE_CUBE_MAP);


	if (glIsEnabled(GL_TEXTURE_CUBE_MAP) == GL_TRUE) {isEnabled = 1;} else {isEnabled = 0;};

	std::cout << isEnabled << std::endl;


	glBindTexture(GL_TEXTURE_CUBE_MAP, texture);
	glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_GENERATE_MIPMAP, GL_TRUE);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, GL_RGB, skybox.topWidth, skybox.topHeight, 0,
		GL_RGB, GL_UNSIGNED_BYTE,skybox.top);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, GL_RGB, skybox.bottomWidth, skybox.bottomHeight, 0,
		GL_RGB, GL_UNSIGNED_BYTE,skybox.bottom);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_RGB, skybox.rightWidth, skybox.rightHeight, 0,
		GL_RGB, GL_UNSIGNED_BYTE,skybox.right);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, GL_RGB, skybox.leftWidth, skybox.rightWidth, 0,
		GL_RGB, GL_UNSIGNED_BYTE,skybox.left);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, GL_RGB, skybox.frontWidth, skybox.frontHeight, 0,
		GL_RGB, GL_UNSIGNED_BYTE,skybox.front);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, GL_RGB, skybox.backWidth, skybox.backHeight, 0,
		GL_RGB, GL_UNSIGNED_BYTE,skybox.back);
	glGenerateMipmap(GL_TEXTURE_CUBE_MAP);


	glClearColor( 1.0, 1.0, 1.0, 0.0 ); 
	glClearDepth( 1.0 ); 
	glEnable( GL_DEPTH_TEST );
  glDepthFunc(GL_LESS);
}

//----------------------------------------------------------------------------

void
	display( void )
{
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );



	//Make sure you start with the Default Coordinate Systems
	projmat=mat4(1.0);
	model_view = mat4(1.0);

	//Set up the camera optics
	projmat = projmat*Perspective(90,1.0,0.1,20.0); 

	//Sky Box @  infinity
	vec4 skyboxEye = vec4(0.0, 0.0, 0.0, 1.0);
	model_view = LookAt(skyboxEye, skyboxEye-n, v);

	glUseProgram(program[1]);
	proj_loc       = glGetUniformLocation(program[1], "Projection");
	model_view_loc = glGetUniformLocation(program[1], "ModelView");
	glUniformMatrix4fv(proj_loc, 1, GL_TRUE, projmat);
	glUniformMatrix4fv(model_view_loc, 1, GL_TRUE, model_view);
	glUniform1i(glGetUniformLocation(program[1], "CubeMap"), 0);
	
	glBindVertexArray(VAO[1]);

	glCullFace(GL_BACK);
	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);
	glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
	glDrawArrays(GL_QUADS, 0, 24);

	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	  glDepthFunc(GL_LESS);

	glUseProgram(0);


	model_view = Angel::mat4(1.0);

	//Render the  rest of the scene

	//Position and orient the camera
	model_view = LookAt(eye, eye-n, v)*model_view;
	//Render surface mesh

	/*glUseProgram(program[0]);
	color_loc = glGetUniformLocation(program[0], "color");
	proj_loc       = glGetUniformLocation(program[0], "projection");
	model_view_loc = glGetUniformLocation(program[0], "modelview");
	glUniformMatrix4fv(proj_loc, 1, GL_TRUE, projmat);
	glUniformMatrix4fv(model_view_loc, 1, GL_TRUE, model_view);
	glUniform4fv(color_loc, 1, green_transparent);

	glPolygonOffset(1.0, 1.0);
	glBindVertexArray(VAO[0]);
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_BYTE, BUFFER_OFFSET(0));*/

	//glUseProgram( program[2] );
	//glBindVertexArray( model_car->vao );
	mat4 modelViewCamera = LookAt(eye, eye-n, v);
	glUniformMatrix4fv(ModelViewCam, 1, GL_TRUE, modelViewCamera);

	mat4 normalMatrix = mat4(1.0);
	mat4 modelViewObject = mat4(1.0);
	glUniformMatrix4fv( ModelViewObj, 1, GL_TRUE, modelViewObject );
	glUniformMatrix4fv(NormalTransformation , 1, GL_TRUE,  normalMatrix);
	glmDrawVBO(model_car, program[2]);

	glutSwapBuffers();

}

//----------------------------------------------------------------------------

void
	keyboard( unsigned char key, int x, int y )
{
	GLfloat xt, yt, zt;
	GLfloat cosine, sine;

	/*
	switch(key)
	{

	GLfloat xt, yt, zt;
	GLfloat cosine, sine;
	*/  

	// positive or negative rotation depending on upper or lower case letter
	if(key > 96)  
	{
		cosine = cos(SCALE_ANGLE * M_PI/-180.0);
		sine = sin(SCALE_ANGLE * M_PI/-180.0);
	}
	else
	{
		cosine = cos(SCALE_ANGLE * M_PI/180.0);
		sine = sin(SCALE_ANGLE * M_PI/180.0);
	}

	switch(key)
	{
	case 'Z': // roll counterclockwise in the xy plane
	case 'z': // roll clockwise in the xy plane
		std::cout << "Implement Roll" << std::endl; 
		break;
	case 'X': // pitch up
	case 'x': // pitch down
		xt = v[X];
		yt = v[Y];
		zt = v[Z];
		v[X] = xt*cosine - n[X]*sine;
		v[Y] = yt*cosine - n[Y]*sine;
		v[Z] = zt*cosine - n[Z]*sine;
		n[X] = xt*sine + n[X]*cosine;
		n[Y] = yt*sine + n[Y]*cosine;
		n[Z] = zt*sine + n[Z]*cosine;
		break;
	case 'C': // yaw counterclockwise in the xz plane
	case 'c': // yaw clockwise in the xz plane
		std::cout << "Implement Yaw" << std::endl; 
		break;
	case 033:  // Escape key
		break; 
	case 'q': case 'Q':
		exit( EXIT_SUCCESS );
		break;
	}
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
	case GLUT_KEY_LEFT: // MOVE LEFT
		eye[0] -= SCALE_VECTOR * u[0];
		eye[1] -= SCALE_VECTOR * u[1];
		eye[2] -= SCALE_VECTOR * u[2];
		break;
	case GLUT_KEY_RIGHT: // MOVE RIGHT
		eye[0] += SCALE_VECTOR * u[0];
		eye[1] += SCALE_VECTOR * u[1];
		eye[2] += SCALE_VECTOR * u[2];
		break;
	}
}

void idle(void){
	glutPostRedisplay();
}
//----------------------------------------------------------------------------

int main( int argc, char **argv )
{
	glutInit( &argc, argv );
	glutInitDisplayMode( GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH );
	glutInitWindowSize( 512, 512 );
	glutCreateWindow( " Cube  Map: skybox" );
	m_glewInitAndVersion();
	
	init();

	glutReshapeFunc(reshape);
	glutDisplayFunc( display );
	glutKeyboardFunc( keyboard );
	glutSpecialFunc( specKey );
	glutIdleFunc(idle);

	glutMainLoop();
	return 0;
}

void m_glewInitAndVersion(void)
{
	fprintf(stdout, "OpenGL Version: %s\n", glGetString(GL_VERSION));
	fprintf(stdout, "GLSL Version: %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));
	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
	}
	fprintf(stdout, "Status: Using GLEW %s\n", glewGetString(GLEW_VERSION));
}

void reshape( int width, int height )
	//the same objects are shown (possibly scaled) w/o shape distortion 
	//original viewport is a square
{

	glViewport( 0, 0, (GLsizei) width, (GLsizei) height );
	glutPostRedisplay();

}
