#include "stdafx.h"
#include "Angel.h"
#include <assert.h>
#include <fstream> 
#include <string>
#include <vector>
#include <chrono>
#include <ctime>
#include <iostream>
#include "glm.h"
using namespace std;

// car variables na dfunctions
bool key_s=0,key_w=0,key_a=0,key_d=0;

#define CAR_SPEED 5
#define CAR_TURN_SPEED 30.0
GLMmodel* model_car;
GLuint  ModelViewCam, ModelViewObj, Projection, LightPosition, NormalTransformation;
std::chrono::time_point<std::chrono::system_clock> t_rot,t_trans;

typedef Angel::vec4 point4;
typedef Angel::vec4 color4;
#ifndef PI
#  define PI  3.14159265358979323846
#endif

#define X               0
#define Y               1
#define Z               2
#define SCALE_VECTOR    0.05
#define SCALE_ANGLE     30.0 

struct Node;

void m_glewInitAndVersion(void);
void reshape(int width, int height);
void specKey(int key, int x, int y);
void keyboard( unsigned char key, int x, int y );
void traverse( Node* node );
void display( void );
void KeyboardUp	( unsigned char key, int x, int y) ;
void idle(void);
void initNodes( void );
void	quad( int a, int b, int c, int d );
void	gen_cube( void );
void Cube();
void init(); // Functions headers

Angel::vec4 v = vec4(0.0, 0.0, 1.0, 1.0);
Angel::vec4 u = vec4(0.0, 1.0, 0.0, 1.0);
Angel::vec4 eye = vec4(-4.0, 0.0, 3.0, 1.0);
Angel::vec4 eye0 = eye;
Angel::vec4 n = Angel::normalize(vec4(-1.0, 0.0, 0.0, 1.0)); 
mat4 bases(n,u,v,vec4(0.0,0.0,0.0,1.0));// Camera vectors

point4 light_position;
vec4 light_ambient( 0.2, 0.2, 0.2, 1.0 );
vec4 light_diffuse( 1.0, 1.0, 1.0, 1.0 );
vec4 light_specular( 1.0, 1.0, 1.0, 1.0 ); // Light vecs and position

GLuint buffers[4];
// Create a vertex array object
GLuint vao[3];
//uniform variables locations
GLuint program[3];

GLuint vPosition;
GLuint vNormal;
GLuint vColor;
GLuint vShift;
GLuint color_loc;
GLuint proj_loc;
GLuint model_view_loc; // Shader variables

// Cube variables
const int NumVertices = 36; //(6 faces)(2 triangles/face)(3 vertices/triangle)

point4 cube_points[NumVertices];
vec3 cube_normals[NumVertices];

point4 vertices[8] = {
	point4( -0.5, -0.5, 0.5, 1.0 ),
	point4( -0.5, 0.5, 0.5, 1.0 ),
	point4( 0.5, 0.5, 0.5, 1.0 ),
	point4( 0.5, -0.5, 0.5, 1.0 ),
	point4( -0.5, -0.5, -0.5, 1.0 ),
	point4( -0.5, 0.5, -0.5, 1.0 ),
	point4( 0.5, 0.5, -0.5, 1.0 ),
	point4( 0.5, -0.5, -0.5, 1.0 )};
int Index = 0;
void	quad( int a, int b, int c, int d )
{
	vec3 temp_norm = normalize(cross( vertices[b]-vertices[a],vertices[b]-vertices[c]));
	cube_normals[Index] = temp_norm; cube_points[Index] = vertices[a]; Index++;
	cube_normals[Index] = temp_norm; cube_points[Index] = vertices[b]; Index++;
	cube_normals[Index] = temp_norm; cube_points[Index] = vertices[c]; Index++;
	cube_normals[Index] = temp_norm; cube_points[Index] = vertices[a]; Index++;
	cube_normals[Index] = temp_norm; cube_points[Index] = vertices[c]; Index++;
	cube_normals[Index] = temp_norm; cube_points[Index] = vertices[d]; Index++;
}

void	gen_cube( void )
{
	quad( 1, 0, 3, 2 );
	quad( 2, 3, 7, 6 );
	quad( 3, 0, 4, 7 );
	quad( 6, 5, 1, 2 );
	quad( 4, 5, 6, 7 );
	quad( 5, 4, 0, 1 );
}


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
//----------------------------
//--------------------------------------------------------------------------
MatrixStack  mvstack;
mat4         model_view=mat4(1.0);
mat4         projmat=mat4(1.0); 

int	main( int argc, char **argv )
{
	glutInit( &argc, argv );
	glutInitDisplayMode( GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH );
	glutInitWindowSize( 1280, 720 );
	glutCreateWindow( "Final Project." );

	glutDisplayFunc(display); 
	glutReshapeFunc(reshape);
	glutKeyboardFunc( keyboard );
	glutSpecialFunc( specKey );
	glutIdleFunc(idle);
	glutKeyboardUpFunc(KeyboardUp);
	glewInit();

	init();

	glutMainLoop();

	return 0;
}

void traverse( Node* node )
{
	if ( node == NULL ) { return; }

	mvstack.push( model_view );

	model_view *= node->transform;
	node->render();

	if ( node->child ) { traverse( node->child ); }

	model_view = mvstack.pop();

	if ( node->sibling ) { traverse( node->sibling ); }
}

void init(){
			model_car = glmReadOBJ("data/porsche.obj");
	if (!model_car) exit(0);
	glmUnitize(model_car);
	glmFacetNormals(model_car);
	glmVertexNormals(model_car, 90.0);  
	glmLoadInVBO(model_car);
	gen_cube();
	glGenVertexArrays( 3, vao );
	glBindVertexArray( vao[0] );

	glGenBuffers(4,buffers); 

	//	glBindVertexArray( vao[0] );
	//glBindBuffer( GL_ARRAY_BUFFER, buffers[0] );
	//glBufferData( GL_ARRAY_BUFFER, 3*car_face_sz*sizeof(vec4)+3* car_face_sz*sizeof(vec3),  NULL, GL_STATIC_DRAW );
	//glBufferSubData( GL_ARRAY_BUFFER, 0, 3*car_face_sz*sizeof(vec4), car_vertices);
	//glBufferSubData( GL_ARRAY_BUFFER, 3*car_face_sz*sizeof(vec4), 3*car_face_sz*sizeof(vec3), car_normals );

	glBindVertexArray( vao[1] );
	glBindBuffer( GL_ARRAY_BUFFER, buffers[1] );
	glBufferData( GL_ARRAY_BUFFER, sizeof(cube_points)+sizeof(cube_normals), NULL, GL_STATIC_DRAW );
	glBufferSubData( GL_ARRAY_BUFFER, 0, sizeof(cube_points), cube_points);
	glBufferSubData( GL_ARRAY_BUFFER, sizeof(cube_points), sizeof(cube_normals), cube_normals );

	// Load shaders and use the resulting shader programs
	program[0] = InitShader( "./shaders/vshader56.glsl", "./shaders/fshader56.glsl" );//( "vshaderUnifColor.glsl", "fshaderStock.glsl" );
	program[1] = InitShader( "./shaders/skyboxvertex.glsl", "./shaders/skyboxfragment.glsl" );
	program[2] = InitShader( "./shaders/shader_vert.glsl", "./shaders/shader_frag.glsl" );

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
	

	glBindVertexArray( vao[1] );

	glBindBuffer( GL_ARRAY_BUFFER, buffers[1] );
	vPosition = glGetAttribLocation( program[0], "vPosition" );
	glEnableVertexAttribArray( vPosition );
	glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0) );

	vNormal = glGetAttribLocation( program[0], "vNormal" ); 
	glEnableVertexAttribArray( vNormal );
	glVertexAttribPointer( vNormal, 3, GL_FLOAT, GL_FALSE, 0,
		BUFFER_OFFSET(sizeof(cube_points)) );

	model_view_loc = glGetUniformLocation( program[0], "ModelView" );
	proj_loc = glGetUniformLocation( program[0], "Projection" );  
	projmat = Perspective(160,16/9,0.02, 15.0); 
	glUniformMatrix4fv(proj_loc, 1, GL_TRUE, projmat);


	// Initialize tree
	//	initNodes();


	glClearColor( 1.0, 1.0, 1.0, 0.0 ); 
	glClearDepth( 1.0 ); 
	glEnable( GL_DEPTH_TEST );
	glDepthFunc(GL_LEQUAL);
	//glPolygonMode(GL_FRONT, GL_FILL);

}

//void Car()
//{
//	mvstack.push( model_view );
//	mat4 m = car_pos*car_att*RotateZ(-90)*RotateX(90)*/*Translate(0.0,0.0,-0.28)*/Scale(0.001,0.001,0.001);
//
//	//glUseProgram( program );
//	glBindBuffer( GL_ARRAY_BUFFER, buffers[0] );
//
//	model_view_loc = glGetUniformLocation( program[0], "ModelView" );
//	glUniformMatrix4fv( model_view_loc, 1, GL_TRUE, model_view*m );
//
//	glBindVertexArray( vao[0] );
//	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
//	//color_loc = glGetUniformLocation(program, "vColor");
//	for(int i=0;i<car_mat_ind[0];i++)
//	{
//		//glUniformMatrix4fv( model_view_loc, 1, GL_TRUE, model_view*m );
//		//glUniform4fv(color_loc, 1, car_mat[3*i+2]);
//		glUniform4fv( glGetUniformLocation(program[0], "AmbientProduct"), 1, light_ambient*car_mat[3*i]);
//		glUniform4fv( glGetUniformLocation(program[0], "DiffuseProduct"), 1, light_diffuse*car_mat[3*i+1] );
//		glUniform4fv( glGetUniformLocation(program[0], "SpecularProduct"), 1, light_specular*car_mat[3*i+2]  );
//		glUniform1f( glGetUniformLocation(program[0], "Shininess"),car_shinnines[i] );
//		if(i==0)
//		{
//			glDrawArrays( GL_TRIANGLES, 0, 3*car_mat_ind[i+1]-1 );
//		}else
//		{
//			glDrawArrays( GL_TRIANGLES, 3*car_mat_ind[i], 3*car_mat_ind[i+1]-3*car_mat_ind[i] );
//		}
//	}
//	model_view = mvstack.pop();
//}

void Cube()
{
	mvstack.push( model_view );
	glBindBuffer( GL_ARRAY_BUFFER, buffers[1] );
	mat4 m = Scale(50, 1.0, 0.05);
	model_view_loc = glGetUniformLocation( program[0], "ModelView" );
	glUniformMatrix4fv( model_view_loc, 1, GL_TRUE, model_view*m );

	color4 material_ambient( 1.0, 1.0, 1.0, 1.0 );
	color4 material_diffuse( 1.0, 0.8, 0.0, 1.0 );
	color4 material_specular( 1.0, 0.0, 1.0, 1.0 );
	int material_shininess = 5.0;

	glBindVertexArray( vao[1] );
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glUniform4fv( glGetUniformLocation(program[0], "AmbientProduct"), 1, light_ambient*material_ambient);
	glUniform4fv( glGetUniformLocation(program[0], "DiffuseProduct"), 1, light_diffuse*material_diffuse );
	glUniform4fv( glGetUniformLocation(program[0], "SpecularProduct"), 1, light_specular*material_specular  );
	glUniform1f( glGetUniformLocation(program[0], "Shininess"),material_shininess );

	glDrawArrays( GL_TRIANGLES, 0, sizeof(cube_points) );

	model_view = mvstack.pop();
}

void display( void )
{
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	mat4 modelViewCamera = model_view;
	glUniformMatrix4fv(ModelViewCam, 1, GL_TRUE, modelViewCamera);

	mat4 normalMatrix = mat4(1.0);
	mat4 modelViewObject = mat4(1.0);
	glUniformMatrix4fv( ModelViewObj, 1, GL_TRUE, modelViewObject );
	glUniformMatrix4fv(NormalTransformation , 1, GL_TRUE,  normalMatrix);
	glmDrawVBO(model_car, program[2]);


	Cube();
	//traverse( &nodes[baseCyll]);
	glutSwapBuffers();

}

void idle(void)
{
	//std::chrono::time_point<std::chrono::system_clock>  end;
	//end = std::chrono::system_clock::now();
	//std::chrono::duration<double> elapsed_seconds;

	//GLfloat xt, yt, zt;
	//GLfloat cosine, sine;

	//if(key_z) 
	//{
	//	elapsed_seconds = end-t_roll;
	//	if(roll<60 )
	//	{
	//		// positive or negative rotation depending on upper or lower case letter

	//		cosine = cos(SCALE_ANGLE*elapsed_seconds.count() * PI/180.0);
	//		sine = sin(SCALE_ANGLE*elapsed_seconds.count() * PI/180.0);

	//		roll = roll + SCALE_ANGLE*elapsed_seconds.count();

	//		xt = u[X];
	//		yt = u[Y];
	//		zt = u[Z];
	//		u[X] = xt*cosine + n[X]*sine;
	//		u[Y] = yt*cosine + n[Y]*sine;
	//		u[Z] = zt*cosine + n[Z]*sine;
	//		n[X] = -xt*sine + n[X]*cosine;
	//		n[Y] = -yt*sine + n[Y]*cosine;
	//		n[Z] = -zt*sine + n[Z]*cosine;
	//	}
	//}
	//if(key_Z) 
	//{
	//	elapsed_seconds = end-t_roll;
	//	if( roll > -60)
	//	{
	//		cosine = cos(-SCALE_ANGLE*elapsed_seconds.count() * PI/180.0);
	//		sine = sin(-SCALE_ANGLE*elapsed_seconds.count() * PI/180.0);
	//		roll = roll - SCALE_ANGLE*elapsed_seconds.count();
	//		xt = u[X];
	//		yt = u[Y];
	//		zt = u[Z];
	//		u[X] = xt*cosine + n[X]*sine;
	//		u[Y] = yt*cosine + n[Y]*sine;
	//		u[Z] = zt*cosine + n[Z]*sine;
	//		n[X] = -xt*sine + n[X]*cosine;
	//		n[Y] = -yt*sine + n[Y]*cosine;
	//		n[Z] = -zt*sine + n[Z]*cosine;
	//	}
	//}
	//if(key_y) 
	//{
	//	elapsed_seconds = end-t_yaw;
	//	if( yaw < 60)
	//	{
	//		yaw = yaw + SCALE_ANGLE*elapsed_seconds.count();
	//		bases = RotateY(SCALE_ANGLE*elapsed_seconds.count())*bases;
	//		n = bases[0];
	//		u = bases[1];
	//		v = bases[2];
	//	}
	//}
	//if(key_Y) 
	//{
	//	elapsed_seconds = end-t_yaw;
	//	if( yaw > -60)
	//	{
	//		yaw = yaw - SCALE_ANGLE*elapsed_seconds.count();
	//		bases = RotateY(-SCALE_ANGLE*elapsed_seconds.count())*bases;
	//		n = bases[0];
	//		u = bases[1];
	//		v = bases[2];
	//	}
	//}
	//if(key_x) 
	//{
	//	elapsed_seconds = end-t_pitch;
	//	if( pitch < 60)
	//	{
	//		pitch = pitch + SCALE_ANGLE*elapsed_seconds.count();
	//		bases = RotateX(SCALE_ANGLE*elapsed_seconds.count())*bases;
	//		n = bases[0];
	//		u = bases[1];
	//		v = bases[2];
	//	}
	//}
	//if(key_X) 
	//{
	//	elapsed_seconds = end-t_pitch;
	//	if( pitch > -60)
	//	{
	//		pitch = pitch - SCALE_ANGLE*elapsed_seconds.count();
	//		bases = RotateX(-SCALE_ANGLE*elapsed_seconds.count())*bases;
	//		n = bases[0];
	//		u = bases[1];
	//		v = bases[2];
	//	}
	//}
	//if(key_w) 
	//{
	//	elapsed_seconds = end-t_trans;
	//	car_pos = car_pos*Translate(car_att*vec4(CAR_SPEED*elapsed_seconds.count(),0.0,0.0,1.0));
	//	eye =  car_att*eye0 + car_pos*vec4(0.0,0.0,0.0,1.0); // car_pos*vec4(0.0,0.0,0.0,1.0);
	//}
	//if(key_s) 
	//{
	//	elapsed_seconds = end-t_trans;
	//	car_pos = car_pos*Translate(car_att*vec4(-CAR_SPEED*elapsed_seconds.count(),0.0,0.0,1.0));
	//	eye =  car_att*eye0 + car_pos*vec4(0.0,0.0,0.0,1.0);
	//}
	//if(key_a) 
	//{

	//	elapsed_seconds = end-t_rot;
	//	cosine = cos(SCALE_ANGLE*elapsed_seconds.count() * PI/180.0);
	//	sine = sin(SCALE_ANGLE*elapsed_seconds.count() * PI/180.0);
	//	car_att = RotateZ(CAR_TURN_SPEED*elapsed_seconds.count())*car_att;
	//	eye =  car_att*eye0 + car_pos*vec4(0.0,0.0,0.0,1.0);
	//	//RotateX(CAR_TURN_SPEED*elapsed_seconds.count())*Translate(car_pos*vec4(0.0,0.0,0.0,-1.0)*vec4(1.0,1.0,1.0,-1.0))*eye;
	//	//bases = RotateZ(CAR_TURN_SPEED*elapsed_seconds.count())*bases;
	//	//n = bases[0];
	//	//n = normalize(n);
	//	//u = bases[1];
	//	//v = bases[2];
	//	xt = u[X];
	//	yt = u[Y];
	//	zt = u[Z];
	//	u[X] = xt*cosine + n[X]*sine;
	//	u[Y] = yt*cosine + n[Y]*sine;
	//	u[Z] = zt*cosine + n[Z]*sine;
	//	n[X] = -xt*sine + n[X]*cosine;
	//	n[Y] = -yt*sine + n[Y]*cosine;
	//	n[Z] = -zt*sine + n[Z]*cosine;
	//}
	//if(key_d) 
	//{	

	//	elapsed_seconds = end-t_rot;
	//	cosine = cos(-SCALE_ANGLE*elapsed_seconds.count() * PI/180.0);
	//	sine = sin(-SCALE_ANGLE*elapsed_seconds.count() * PI/180.0);
	//	car_att = car_att*RotateZ(-CAR_TURN_SPEED*elapsed_seconds.count());
	//	eye =  car_att*eye0 + car_pos*vec4(0.0,0.0,0.0,1.0);
	//	//bases = RotateZ(-CAR_TURN_SPEED*elapsed_seconds.count())*bases;
	//	//n = bases[0];
	//	//n = normalize(n);
	//	//u = bases[1];
	//	//v = bases[2];
	//	xt = u[X];
	//	yt = u[Y];
	//	zt = u[Z];
	//	u[X] = xt*cosine + n[X]*sine;
	//	u[Y] = yt*cosine + n[Y]*sine;
	//	u[Z] = zt*cosine + n[Z]*sine;
	//	n[X] = -xt*sine + n[X]*cosine;
	//	n[Y] = -yt*sine + n[Y]*cosine;
	//	n[Z] = -zt*sine + n[Z]*cosine;
	//}

	//t_roll = end;
	//t_pitch = end;
	//t_yaw=end;
	//t_rot = end;
	//t_trans = end;

	glutPostRedisplay();
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

void keyboard( unsigned char key, int x, int y )
{
	GLfloat xt, yt, zt;
	GLfloat cosine, sine;

	// positive or negative rotation depending on upper or lower case letter
	//if(key > 96)  
	//{
	//	cosine = cos(SCALE_ANGLE * PI/-180.0);
	//	sine = sin(SCALE_ANGLE * PI/-180.0);
	//}
	//else
	//{
	//	cosine = cos(SCALE_ANGLE * PI/180.0);
	//	sine = sin(SCALE_ANGLE * PI/180.0);
	//}

	switch(key)
	{
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
	case 033:  // Escape key
	case 'q': case 'Q':
		exit( EXIT_SUCCESS );
	}

	//glutPostRedisplay();
}

void specKey(int key, int x, int y)
{ 
	//switch( key ) {
	//case GLUT_KEY_UP: // MOVE FORWARD
	//	eye[0] -= SCALE_VECTOR * n[0];
	//	eye[1] -= SCALE_VECTOR * n[1];
	//	eye[2] -= SCALE_VECTOR * n[2];
	//	break;
	//case GLUT_KEY_DOWN: // MOVE BACKWARD
	//	eye[0] += SCALE_VECTOR * n[0];
	//	eye[1] += SCALE_VECTOR * n[1];
	//	eye[2] += SCALE_VECTOR * n[2];
	//	break;
	//case GLUT_KEY_LEFT: // MOVE FORWARD
	//	eye[0] -= SCALE_VECTOR * u[0];
	//	eye[1] -= SCALE_VECTOR * u[1];
	//	eye[2] -= SCALE_VECTOR * u[2];
	//	break;
	//case GLUT_KEY_RIGHT: // MOVE BACKWARD
	//	eye[0] += SCALE_VECTOR * u[0];
	//	eye[1] += SCALE_VECTOR * u[1];
	//	eye[2] += SCALE_VECTOR * u[2];
	//	break;
	//default:
	//	break;
	//}
	glutPostRedisplay();
}

void reshape( int width, int height )
	//the same objects are shown (possibly scaled) w/o shape distortion 
	//original viewport is a square
{
	//if(height<=width)	glViewport( 0, 0, (GLsizei) height, (GLsizei) height );
	//else glViewport( 0, 0, (GLsizei) width, (GLsizei) width );
	glViewport( 0, 0, (GLsizei) width, (GLsizei) height );
	projmat = Perspective(90,width/height,0.2, 15.0); 
	glUniformMatrix4fv(proj_loc, 1, GL_TRUE, projmat);


}

//void initNodes( void )
//{
//	mat4  m;

//	m = Scale(0.1,0.1,0.1) ;
//	nodes[cen_tt] = Node( m, Cen_tt, NULL, &nodes[treetop0] );//&nodes[treetop0]

//	m = Translate(0.0, 0.0, 1.0)*Scale(2.0,2.0,2.0) ;
//	nodes[treetop0] = Node( m, Treetop0, &nodes[walls1], NULL );
//}


/*
case 'W': // 
case 'w': // go in front
car_pos = car_pos*Translate(car_att*vec4(0.0,0.0,-CAR_SPEED,1.0));
break;
case 'S': // 
case 's': // go in front
car_pos = car_pos*Translate(car_att*vec4(0.0,0.0,CAR_SPEED,1.0));
break;
case 'A': // 
case 'a': // go in front
car_att = car_att*RotateY(CAR_TURN_SPEED);
break;
case 'D': // 
case 'd': // go in front
car_att = car_att*RotateY(-CAR_TURN_SPEED);
break;

//glUniform4fv( glGetUniformLocation(program, "AmbientProduct"), 1, light_ambient);
//glUniform4fv( glGetUniformLocation(program, "DiffuseProduct"), 1, light_diffuse );
//glUniform4fv( glGetUniformLocation(program, "SpecularProduct"), 1, light_specular);
//glUniform1f( glGetUniformLocation(program, "Shininess"),5.0 );
//glDrawArrays( GL_TRIANGLES, 0, 3*car_face_sz );


case 'Z': // roll counterclockwise in the xy plane
case 'z': // roll clockwise in the xy plane
xt = u[X];
yt = u[Y];
zt = u[Z];
u[X] = xt*cosine - v[X]*sine;
u[Y] = yt*cosine - v[Y]*sine;
u[Z] = zt*cosine - v[Z]*sine;
v[X] = xt*sine + v[X]*cosine;
v[Y] = yt*sine + v[Y]*cosine;
v[Z] = zt*sine + v[Z]*cosine;
break;
case 'X': // pitch up
case 'x': // pitch down
xt = u[X];
yt = u[Y];
zt = u[Z];
u[X] = xt*cosine + n[X]*sine;
u[Y] = yt*cosine + n[Y]*sine;
u[Z] = zt*cosine + n[Z]*sine;
n[X] = -xt*sine + n[X]*cosine;
n[Y] = -yt*sine + n[Y]*cosine;
n[Z] = -zt*sine + n[Z]*cosine;
break;
case 'Y': // yaw up
case 'y': // yaw down
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
case 'R': // reset
case 'r': // reset
v = vec4(0.0, 1.0, 0.0, 0.0);
u = vec4(1.0, 0.0, 0.0, 0.0);
eye = vec4(0.0, 1.0, 2.0, 1.0);
n = Angel::normalize(vec4(0.0, 1.0, 2.0, 0.0));
break;






*/