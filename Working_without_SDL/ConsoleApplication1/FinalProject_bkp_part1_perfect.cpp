//CS 537
//A stub using GLM08 to load and Render .obj models 
// The models polygons are organized in groups based on their materials
//G. Kamberov
#include "stdafx.h"
#include <iostream>
#include "Angel.h"
#include "glm.h"
#include <chrono>
#include <ctime>

using namespace std;

typedef Angel::vec4  color4;
typedef Angel::vec4  point4;

#define SCALE_ANGLE     1.0 
#define SCALE_VECTOR    0.5
#define CAR_SPEED		5
#define CAR_TURN_SPEED  30.0
#ifndef PI
#  define PI  3.14159265358979323846
#endif
#define X               0
#define Y               1
#define Z               2// Defines

void specKey(int key, int x, int y);
void display( void );
void getGLinfo();
void keyboard( unsigned char key, int x, int y );
void KeyboardUp	( unsigned char key, int x, int y);
void reshape( int width, int height );
void idle(void); // Functions headers

GLMmodel* models[3];
int VBO_num_vert;
int VBO_num_norm;
int num_vert; // Model import variables

int winHeight = 480;
int winWidth = 640;
bool mouseDown = false;
float xrot = 90;
float yrot = 0;
float zrot = -90;
float xdiff = 0;
float ydiff = 0;
GLfloat step = 5; // Miscelaneous

Angel::vec4 v = vec4(0.0, 0.0, 1.0, 1.0);
Angel::vec4 u = vec4(0.0, 1.0, 0.0, 1.0);
Angel::vec4 eye = vec4(-12.5, 0.0, 0.5, 1.0);
Angel::vec4 eye0 = eye;
Angel::vec4 n = Angel::normalize(vec4(-1.0, 0.0, 0.0, 1.0)); 
mat4 bases = mat4(n,u,v,vec4(0.0,0.0,0.0,1.0)); 
mat4 bases2 = bases;// Camera Coordianate System

GLuint program;
GLuint vPosition;
GLuint vNormal;
GLuint vColor;
GLuint vShift;
GLuint color_loc;
GLuint proj_loc;
GLuint model_view_loc;
// Model-view and projection matrices uniform location
GLuint  ModelViewCam, ModelViewObj, Projection, LightPosition, NormalTransformation; // Shader variables and matrices

bool key_x=0,key_X=0,key_y=0,key_Y=0,key_z=0,key_Z=0;
float roll=0,pitch=0,yaw=0;
std::chrono::time_point<std::chrono::system_clock> t_roll,t_pitch,t_yaw; // Variables for camera moving with arrow keys and XYZ

bool key_s=0,key_w=0,key_a=0,key_d=0,key_t=0;
std::chrono::time_point<std::chrono::system_clock> t_rot,t_trans;
vec4 car_init = vec4(-10.0,0.0,0.0,1.0);
mat4 car_pos(1.0);
mat4 car_att(1.0); // Variables for moving the car

//// Necessary classes
class MatrixStack {
	int    _index;
	int    _size;
	mat4*  _matrices;

public:
	MatrixStack( int numMatrices = 32 ):_index(0), _size(numMatrices)
	{ _matrices = new mat4[numMatrices]; }

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

struct Node {
	mat4  transform;
	void  (*render)( void );
	Node* sibling;
	Node* child;

	Node() :
		render(NULL), sibling(NULL), child(NULL) {}

	Node( mat4& m, void (*render)( void ), Node* sibling, Node* child ) :
		transform(m), render(render), sibling(sibling), child(child) {}
};

MatrixStack  mvstack;
mat4         model_view=mat4(1.0);
mat4         projmat=mat4(1.0);  // matrix stack, model view and projection matrix

void init()
{
	// Load the OBJ models from file
	models[0] = glmReadOBJ("data/Harley-Davidson.obj");
	models[1] = glmReadOBJ("data/CasaSimples.obj"); 
	models[2] = glmReadOBJ("data/quakingAspen.obj"); 

	if (!models[0]) exit(0);
	if (!models[1]) exit(0);
	if (!models[1]) exit(0);

	// Normilize vertices
	glmUnitize(models[0]);
	glmUnitize(models[1]);
	glmUnitize(models[2]);
	// Compute facet normals
	glmFacetNormals(models[0]);
	glmFacetNormals(models[1]);
	glmFacetNormals(models[2]);
	// Comput vertex normals
	glmVertexNormals(models[0], 90.0);  
	glmVertexNormals(models[1], 90.0);
	glmVertexNormals(models[2], 90.0);
	// Load the model (vertices and normals) into a vertex buffer
	glmLoadInVBO(models[0]);
	glmLoadInVBO(models[1]);
	glmLoadInVBO(models[2]);


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
	program = InitShader( "./shaders/shader_vert.glsl", "./shaders/shader_frag.glsl" );
	glUseProgram( program );
	// set up vertex arrays
	glBindVertexArray( models[0]->vao );

	glUseProgram( program );

	glUniform4fv( glGetUniformLocation(program, "light_ambient"),1, light_ambient);
	glUniform4fv( glGetUniformLocation(program, "light_diffuse"),1, light_diffuse);
	glUniform4fv( glGetUniformLocation(program, "light_specular"),1, light_specular);	
	glUniform4fv( glGetUniformLocation(program, "lightPosition"),1, light_position_distant );
	// Retrieve transformation uniform variable locations
	ModelViewCam = glGetUniformLocation( program, "modelView" );
	ModelViewObj = glGetUniformLocation(program, "ModelViewObj");
	Projection = glGetUniformLocation( program, "projection" );
	NormalTransformation = glGetUniformLocation( program, "normalTransformation" );

	glClearDepth( 1.0 ); 
	glEnable( GL_DEPTH_TEST );
	glDepthFunc(GL_LESS);
	glClearColor( 1.0, 1.0, 1.0, 1.0 ); 
} // OpenGL initializations

int main( int argc, char **argv )
{
	glutInit( &argc, argv );
	glutInitDisplayMode( GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH );
	glutInitWindowSize( winWidth, winHeight );
	glutCreateWindow( "Loading OBJ Models into VBO" );
	glewInit();
	getGLinfo();
	init();
	glutDisplayFunc(display); 
	glutReshapeFunc(reshape);
	glutKeyboardFunc( keyboard );
	glutSpecialFunc( specKey );
	glutIdleFunc(idle);
	glutKeyboardUpFunc(KeyboardUp);
	glutMainLoop();
	return 0;
}

void idle(void)
{
	std::chrono::time_point<std::chrono::system_clock>  end;
	end = std::chrono::system_clock::now();
	std::chrono::duration<double> elapsed_seconds;
	int dir=1;
	GLfloat xt, yt, zt;
	GLfloat cosine=0, sine=0;

	if(key_z) 
	{
		elapsed_seconds = end-t_roll;
		if(roll<60 )
		{
			// positive or negative rotation depending on upper or lower case letter
			roll = roll + SCALE_ANGLE*elapsed_seconds.count();
			bases = RotateZ(SCALE_ANGLE*elapsed_seconds.count())*bases;
			n = bases[0];
			n.w=1;
			u = bases[1];
			u.w=1;
			v = bases[2];
			v.w=1;
		}
	}
	if(key_Z) 
	{
		elapsed_seconds = end-t_roll;
		if( roll > -60)
		{
			roll = roll - SCALE_ANGLE*elapsed_seconds.count();
			bases = RotateZ(-SCALE_ANGLE*elapsed_seconds.count())*bases;
			n = bases[0];
			n.w=1;
			u = bases[1];
			u.w=1;
			v = bases[2];
			v.w=1;
		}
	}
	if(key_y) 
	{
		elapsed_seconds = end-t_yaw;
		if( yaw < 60)
		{
			yaw = yaw + SCALE_ANGLE*elapsed_seconds.count();
			bases = RotateY(SCALE_ANGLE*elapsed_seconds.count())*bases;
			n = bases[0];
			n.w=1;
			u = bases[1];
			u.w=1;
			v = bases[2];
			v.w=1;
		}
	}
	if(key_Y) 
	{
		elapsed_seconds = end-t_yaw;
		if( yaw > -60)
		{
			yaw = yaw - SCALE_ANGLE*elapsed_seconds.count();
			bases = RotateY(-SCALE_ANGLE*elapsed_seconds.count())*bases;
			n = bases[0];
			n.w=1;
			u = bases[1];
			u.w=1;
			v = bases[2];
			v.w=1;
		}
	}
	if(key_x) 
	{
		elapsed_seconds = end-t_pitch;
		if( pitch < 60)
		{
			pitch = pitch + SCALE_ANGLE*elapsed_seconds.count();
			bases = RotateX(SCALE_ANGLE*elapsed_seconds.count())*bases;
			n = bases[0];
			n.w=1;
			u = bases[1];
			u.w=1;
			v = bases[2];
			v.w=1;
		}
	}
	if(key_X) 
	{
		elapsed_seconds = end-t_pitch;
		if( pitch > -60)
		{
			pitch = pitch - SCALE_ANGLE*elapsed_seconds.count();
			bases = RotateX(-SCALE_ANGLE*elapsed_seconds.count())*bases;
			n = bases[0];
			n.w=1;
			u = bases[1];
			u.w=1;
			v = bases[2];
			v.w=1;
		}
	}
	if(key_w) 
	{
		elapsed_seconds = end-t_trans;
		car_pos = car_pos*Translate(car_att*vec4(CAR_SPEED*elapsed_seconds.count(),0.0,0.0,1.0));
		eye = Translate(vec4(0.0,0.0,key_t*15.0,1.0))*(-vec4(0.0,0.0,0.0,-1.0)+car_init +  car_att*(eye0-car_init) + car_pos*vec4(0.0,0.0,0.0,1.0)); //eye = car_init +  car_att*(eye0-car_init) + car_pos*vec4(0.0,0.0,0.0,1.0);
		if(!key_t)
		{
			n = bases[0];
			n.w=1;
			u = bases[1];
			u.w=1;
			v = bases[2];
			v.w=1;
		}
	}
	if(key_s) 
	{
		elapsed_seconds = end-t_trans;
		car_pos = car_pos*Translate(car_att*vec4(-CAR_SPEED*elapsed_seconds.count(),0.0,0.0,1.0));
		if(!key_w)
		{
			dir=-1;
		}
		eye = Translate(vec4(0.0,0.0,key_t*15.0,1.0))*(-vec4(0.0,0.0,0.0,-1.0)+car_init +  car_att*(eye0-car_init) + car_pos*vec4(0.0,0.0,0.0,1.0));
		if(!key_t)
		{
			n = bases[0];
			n.w=1;
			u = bases[1];
			u.w=1;
			v = bases[2];
			v.w=1;
		}
	}
	if(key_a && (key_w || key_s) ) 
	{

		elapsed_seconds = end-t_rot;
		cosine = cos(dir*CAR_TURN_SPEED*elapsed_seconds.count() * PI/180.0);
		sine = sin(dir*CAR_TURN_SPEED*elapsed_seconds.count() * PI/180.0);
		car_att = RotateZ(dir*CAR_TURN_SPEED*elapsed_seconds.count())*car_att;
		eye = Translate(vec4(0.0,0.0,key_t*15.0,1.0))*(-vec4(0.0,0.0,0.0,-1.0)+car_init +  car_att*(eye0-car_init) + car_pos*vec4(0.0,0.0,0.0,1.0));
		bases = RotateZ(dir*CAR_TURN_SPEED*elapsed_seconds.count())*bases;
		if(!key_t)
		{
			n = bases[0];
			n.w=1;
			u = bases[1];
			u.w=1;
			v = bases[2];
			v.w=1;
		}
		//xt = u[X];
		//yt = u[Y];
		//zt = u[Z];
		//u[X] = xt*cosine + n[X]*sine;
		//u[Y] = yt*cosine + n[Y]*sine;
		//u[Z] = zt*cosine + n[Z]*sine;
		//n[X] = -xt*sine + n[X]*cosine;
		//n[Y] = -yt*sine + n[Y]*cosine;
		//n[Z] = -zt*sine + n[Z]*cosine;
	}
	if(key_d && (key_w || key_s) ) 
	{	
		elapsed_seconds = end-t_rot;
		cosine = cos(-dir*CAR_TURN_SPEED*elapsed_seconds.count() * PI/180.0);
		sine = sin(-dir*CAR_TURN_SPEED*elapsed_seconds.count() * PI/180.0);
		car_att = car_att*RotateZ(-dir*CAR_TURN_SPEED*elapsed_seconds.count());
		eye = Translate(vec4(0.0,0.0,key_t*15.0,1.0))*(-vec4(0.0,0.0,0.0,-1.0)+car_init +  car_att*(eye0-car_init) + car_pos*vec4(0.0,0.0,0.0,1.0));
		bases = RotateZ(-dir*CAR_TURN_SPEED*elapsed_seconds.count())*bases;
		if(!key_t)
		{
			n = bases[0];
			n.w=1;
			u = bases[1];
			u.w=1;
			v = bases[2];
			v.w=1;
		}
		//xt = u[X];
		//yt = u[Y];
		//zt = u[Z];
		//u[X] = xt*cosine + n[X]*sine;
		//u[Y] = yt*cosine + n[Y]*sine;
		//u[Z] = zt*cosine + n[Z]*sine;
		//n[X] = -xt*sine + n[X]*cosine;
		//n[Y] = -yt*sine + n[Y]*cosine;
		//n[Z] = -zt*sine + n[Z]*cosine;
	}

	//t_roll = end;
	//t_pitch = end;
	//t_yaw=end;
	t_rot = end;
	t_trans = end;

	glutPostRedisplay();
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
		eye[0] += SCALE_VECTOR * u[0];
		eye[1] += SCALE_VECTOR * u[1];
		eye[2] += SCALE_VECTOR * u[2];
		break;
	case GLUT_KEY_RIGHT: // MOVE BACKWARD
		eye[0] -= SCALE_VECTOR * u[0];
		eye[1] -= SCALE_VECTOR * u[1];
		eye[2] -= SCALE_VECTOR * u[2];
		break;
	default:
		break;
	}
	glutPostRedisplay();
}

void display( void )
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Setup camera
	mat4 modelViewCamera = LookAt(eye, eye-n, v);
	glUniformMatrix4fv(ModelViewCam, 1, GL_TRUE, modelViewCamera);


	float scale_x, scale_y, scale_z;
	mat4 scaleTransformation;
	mat4 invScaleTranformation;
	mat4 normalMatrix;
	mat4 modelViewObject;
	xrot = 90;
	yrot = 0;
	zrot = 90;
	// _________________________________Load the first model
	scale_x = 1;
	scale_y = 1;
	scale_z = 1;
	// Scale Transformation Matrix
	scaleTransformation = Scale(scale_x, scale_y, scale_z);
	// Inverse Scale Transformation Matrix 
	invScaleTranformation = Scale(1/scale_x, 1/scale_y, 1/scale_z);
	normalMatrix =  car_att*RotateZ( zrot ) * RotateY( yrot )*RotateX( xrot ) * invScaleTranformation;
	modelViewObject = car_pos*Translate(car_init)*car_att*RotateZ( zrot ) * RotateY( yrot )*RotateX( xrot ) *scaleTransformation;
	glUniformMatrix4fv( ModelViewObj, 1, GL_TRUE, modelViewObject );
	glUniformMatrix4fv(NormalTransformation , 1, GL_TRUE,  normalMatrix);
	glmDrawVBO(models[0], program);
	xrot = 90;
	yrot = 0;
	zrot = -90;
	// _________________________________Load the second model
	scale_x = 2;
	scale_y = 2;
	scale_z = 2;
	// Scale Transformation Matrix
	scaleTransformation = Scale(scale_x, scale_y, scale_z);
	// Inverse Scale Transformation Matrix 
	invScaleTranformation = Scale(1/scale_x, 1/scale_y, 1/scale_z);
	normalMatrix = RotateZ( zrot ) * RotateY( yrot )*RotateX( xrot ) *invScaleTranformation;
	modelViewObject = Translate(0.0, -5, 1.5)*RotateZ( zrot ) * RotateY( yrot )*RotateX( xrot ) *scaleTransformation;
	glUniformMatrix4fv( ModelViewObj, 1, GL_TRUE, modelViewObject );
	glUniformMatrix4fv(NormalTransformation , 1, GL_TRUE,  normalMatrix);
	glmDrawVBO(models[1], program);

	// _________________________________Load the third model multiple times
	scale_x = 1;
	scale_y = 1;
	scale_z =1;
	// Scale Transformation Matrix
	scaleTransformation = Scale(scale_x, scale_y, scale_z);
	// Inverse Scale Transformation Matrix 
	invScaleTranformation = Scale(1/scale_x, 1/scale_y, 1/scale_z);

	normalMatrix = RotateZ( zrot ) * RotateY( yrot )*RotateX( xrot ) *invScaleTranformation;
	glUniformMatrix4fv(NormalTransformation , 1, GL_TRUE,  normalMatrix);
	modelViewObject = Translate(-5, 0, 0.5)*RotateZ( zrot ) * RotateY( yrot )*RotateX( xrot ) *scaleTransformation; 
	glUniformMatrix4fv( ModelViewObj, 1, GL_TRUE, modelViewObject );
	glmDrawVBO(models[2], program);

	modelViewObject = Translate(-3, 0, 0.5)*RotateZ( zrot ) * RotateY( yrot )*RotateX( xrot ) *scaleTransformation;
	glUniformMatrix4fv( ModelViewObj, 1, GL_TRUE, modelViewObject );
	glmDrawVBO(models[2], program);

	modelViewObject = Translate(-2, 0, 0.5)*RotateZ( zrot ) * RotateY( yrot )*RotateX( xrot ) *scaleTransformation;
	glUniformMatrix4fv( ModelViewObj, 1, GL_TRUE, modelViewObject );
	glmDrawVBO(models[2], program);


	glutSwapBuffers();
}

void reshape( int width, int height )
{
	glViewport( 0, 0, width, height );
	GLfloat aspect = GLfloat(width)/height;
	mat4  projection = Perspective( 45.0, aspect, 0.0001, 300.0 );
	glUniformMatrix4fv( Projection, 1, GL_TRUE, projection );
}

void getGLinfo()
{
	cout << "GL Vendor   : " << glGetString(GL_VENDOR) << endl;
	cout << "GL Renderer : " << glGetString(GL_RENDERER) << endl;
	cout << "GL Version  : " << glGetString(GL_VERSION) << endl;
} // Get OpenGL version information

void keyboard( unsigned char key, int x, int y )
{
	GLfloat xt, yt, zt;
	GLfloat cosine=0, sine=0;
	if(key > 96)  
	{
		cosine = cos(SCALE_ANGLE * PI/-180.0);
		sine = sin(SCALE_ANGLE * PI/-180.0);
	}
	else
	{
		cosine = cos(SCALE_ANGLE * PI/180.0);
		sine = sin(SCALE_ANGLE * PI/180.0);
	}

	switch(key)
	{
		//case 'Z': // roll counterclockwise in the xy plane
		//case 'z': // roll clockwise in the xy plane
		//	xt = u[X];
		//	yt = u[Y];
		//	zt = u[Z];
		//	u[X] = xt*cosine - v[X]*sine;
		//	u[Y] = yt*cosine - v[Y]*sine;
		//	u[Z] = zt*cosine - v[Z]*sine;
		//	v[X] = xt*sine + v[X]*cosine;
		//	v[Y] = yt*sine + v[Y]*cosine;
		//	v[Z] = zt*sine + v[Z]*cosine;
		//	break;
		//case 'X': // pitch up
		//case 'x': // pitch down
		//	xt = u[X];
		//	yt = u[Y];
		//	zt = u[Z];
		//	u[X] = xt*cosine + n[X]*sine;
		//	u[Y] = yt*cosine + n[Y]*sine;
		//	u[Z] = zt*cosine + n[Z]*sine;
		//	n[X] = -xt*sine + n[X]*cosine;
		//	n[Y] = -yt*sine + n[Y]*cosine;
		//	n[Z] = -zt*sine + n[Z]*cosine;
		//	break;
		//case 'Y': // yaw up
		//case 'y': // yaw down
		//	xt = v[X];
		//	yt = v[Y];
		//	zt = v[Z];
		//	v[X] = xt*cosine - n[X]*sine;
		//	v[Y] = yt*cosine - n[Y]*sine;
		//	v[Z] = zt*cosine - n[Z]*sine;
		//	n[X] = xt*sine + n[X]*cosine;
		//	n[Y] = yt*sine + n[Y]*cosine;
		//	n[Z] = zt*sine + n[Z]*cosine;
		//	break;
	case 'Z': // roll counterclockwise in the xy plane
		if(!key_Z) t_roll = std::chrono::system_clock::now();
		key_Z=1;
		break;
	case 'z': // roll clockwise in the xy plane
		if(!key_z) t_roll = std::chrono::system_clock::now();
		key_z=1;
		break;
	case 'X': // pitch up
		if(!key_X) t_pitch = std::chrono::system_clock::now();
		key_X=1;
		break;
	case 'x': // pitch down
		if(!key_x) t_pitch = std::chrono::system_clock::now();
		key_x=1;
		break;
	case 'Y': // yaw up
		if(!key_Y) t_yaw = std::chrono::system_clock::now();
		key_Y=1;
		break;
	case 'y': // yaw down
		if(!key_y) t_yaw = std::chrono::system_clock::now();
		key_y=1;
		break;
	case 'W': // 
	case 'w': // go in front
		if(!key_w) t_trans = std::chrono::system_clock::now();
		key_w=1;
		break;
	case 'S': // 
	case 's': // go in front
		if(!key_s) t_trans = std::chrono::system_clock::now();
		key_s=1;
		break;
	case 'A': // 
	case 'a': // go in front
		if(!key_a) t_rot = std::chrono::system_clock::now();
		key_a=1;
		break;
	case 'D': // 
	case 'd': // go in front
		if(!key_d) t_rot = std::chrono::system_clock::now();
		key_d=1;
		break;
	case 'T':
	case 't':
		if(!key_t)
		{
			bases2=RotateY(75)*bases;
			eye = Translate(0.0,0.0,15.0)*eye;
			n = bases2[0];
			n.w=1;
			u = bases2[1];
			u.w=1;
			v = bases2[2];
			v.w=1;
			key_t=!key_t;
		}else
		{
			n = bases[0];
			n.w=1;
			u = bases[1];
			u.w=1;
			v = bases[2];
			v.w=1;
			eye = Translate(0.0,0.0,-15.0)*eye;
			key_t=!key_t;
		}
		break;
	case 033:  // Escape key
	case 'q': case 'Q':
		exit( EXIT_SUCCESS );
	}
}

void KeyboardUp	( unsigned char key, int x, int y) 	
{
	switch(key)
	{
	case 'Z': // roll counterclockwise in the xy plane
		key_Z=0;
		break;
	case 'z': // roll clockwise in the xy plane
		key_z=0;
		break;
	case 'X': // pitch up
		key_X=0;
		break;
	case 'x': // pitch down
		key_x=0;
		break;
	case 'Y': // yaw up
		key_Y=0;
		break;
	case 'y': // yaw down
		key_y=0;
		break;
	case 'W': // 
	case 'w': // go in front
		key_w=0;
		break;
	case 'S': // 
	case 's': // go in front
		key_s=0;
		break;
	case 'A': // 
	case 'a': // go in front
		key_a=0;
		break;
	case 'D': // 
	case 'd': // go in front
		key_d=0;
		break;
	}
}