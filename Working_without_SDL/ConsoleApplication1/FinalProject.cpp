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
#include <fstream> 
#include <string>
#include <vector>

using namespace std;

typedef Angel::vec4  color4;
typedef Angel::vec4  point4;

//// Necessary classes
struct Node;

#define SCALE_ANGLE     20.0 
#define SCALE_VECTOR    0.5
#define CAR_SPEED		5
#define CAR_TURN_SPEED  30.0
#define AIR_VIEW		20
#define MAX_ANGLE		40
#ifndef PI
#  define PI  3.14159265358979323846
#endif
#define X               0
#define Y               1
#define Z               2  // Defines

void specKey(int key, int x, int y);
void display( void );
void traverse( Node* node );
void initNodes();
void getGLinfo();
void keyboard( unsigned char key, int x, int y );
void KeyboardUp	( unsigned char key, int x, int y);
void reshape( int width, int height );
void read_obj(char *filename,vec4 *vertices,vec3 *normals,vec4 *lights,int *mat_ind,float* shinnines,int* vertices_size,int* normals_size,int* faces_size);
void Player();
void Road();
void House();
void Trees();
void Forest();
void idle(void); // Functions headers

GLMmodel* models[3];
int VBO_num_vert;
int VBO_num_norm;
int num_vert; // Model import variables

int winHeight = 720;
int winWidth = 720*16/9;
bool mouseDown = false;
float xrot = 90;
float yrot = 0;
float zrot = -90;
float scale_x, scale_y, scale_z;
mat4 scaleTransformation;
mat4 invScaleTranformation;
mat4 normalMatrix;
mat4 modelViewObject;
float xdiff = 0;
float ydiff = 0;
GLfloat step = 5;  // Miscelaneous

Angel::vec4 v = vec4(0.0, 0.0, 1.0, 1.0);
Angel::vec4 u = vec4(0.0, 1.0, 0.0, 1.0);
Angel::vec4 eye = vec4(0.0, 0.0, 0.5, 1.0);
Angel::vec4 eye0 = eye;
Angel::vec4 n = Angel::normalize(vec4(-1.0, 0.0, 0.0, 1.0)); 
mat4 bases = mat4(n,u,v,vec4(0.0,0.0,0.0,1.0)); 
mat4 bases2 = bases; // Camera Coordianate System

GLuint program,cube_shader;
GLuint instanced;
GLuint vPosition;
GLuint vNormal;
GLuint vColor;
GLuint vShift;
GLuint color_loc;
GLuint proj_loc;
GLuint model_view_loc;
GLuint buffers[2];
// Create a vertex array object
GLuint vao[2];
// Model-view and projection matrices uniform location
GLuint  ModelViewCam, ModelViewObj, Projection, LightPosition, NormalTransformation;  // Shader variables and matrices

bool key_x=0,key_X=0,key_y=0,key_Y=0,key_z=0,key_Z=0;
float roll=0,pitch=0,yaw=0;
std::chrono::time_point<std::chrono::system_clock> t_roll,t_pitch,t_yaw; // Variables for camera moving with arrow keys and XYZ

bool key_s=0,key_w=0,key_a=0,key_d=0,key_t=0;
std::chrono::time_point<std::chrono::system_clock> t_rot,t_trans;
mat4 car_pos= Translate(0.0,0.0,0.0);
mat4 car_att(1.0);  // Variables for moving the car

vec4 *tree_vertices = (vec4*)malloc(3*200000*sizeof(vec4));
int tree_vert_sz=0,tree_norm_sz=0,tree_face_sz=0;
vec3 *tree_normals = (vec3*)malloc(3*200000*sizeof(vec3));
vec4 *tree_mat= (vec4*)malloc(3*200*sizeof(vec4));
int tree_mat_ind[200+1];
float tree_shinnines[200]; // tree variables

color4 light_ambient( 0.2, 0.2, 0.2, 1.0 );
color4 light_diffuse( 1.0, 1.0, 1.0, 1.0 );
color4 light_specular( 1.0, 1.0, 1.0, 1.0 ); // illumination vectors

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

enum{
	player,//moto
	house,
	trees,
	road,
	forest,
	num_nodes
}; //enum for nodes

Node  nodes[num_nodes];

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

	glGenVertexArrays( 2, vao );
	glBindVertexArray( vao[0] );
	glGenBuffers(2,buffers); 

	glBindBuffer( GL_ARRAY_BUFFER, buffers[0] );
	glBufferData( GL_ARRAY_BUFFER, sizeof(cube_points)+sizeof(cube_normals), NULL, GL_STATIC_DRAW );
	glBufferSubData( GL_ARRAY_BUFFER, 0, sizeof(cube_points), cube_points);
	glBufferSubData( GL_ARRAY_BUFFER, sizeof(cube_points), sizeof(cube_normals), cube_normals );

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
	cube_shader = InitShader( "./shaders/vshader56.glsl", "./shaders/fshader56.glsl" );
	glUseProgram( program );
	glBindVertexArray( vao[0] );

	glBindBuffer( GL_ARRAY_BUFFER, buffers[0] );
	vPosition = glGetAttribLocation( program, "vPosition" );
	glEnableVertexAttribArray( vPosition );
	glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0) );

	vNormal = glGetAttribLocation( program, "vNormal" ); 
	glEnableVertexAttribArray( vNormal );
	glVertexAttribPointer( vNormal, 3, GL_FLOAT, GL_FALSE, 0,
		BUFFER_OFFSET(sizeof(cube_points)) );

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
	projmat = Perspective(120,16/9,0.002, 150.0); 
	glUniformMatrix4fv(Projection, 1, GL_TRUE, projmat);
	NormalTransformation = glGetUniformLocation( program, "normalTransformation" );

	// Initialize tree
	initNodes();


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

	if(key_z && !key_t) 
	{
		elapsed_seconds = end-t_roll;
		if(roll<MAX_ANGLE )
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
	if(key_Z && !key_t) 
	{
		elapsed_seconds = end-t_roll;
		if( roll > -MAX_ANGLE)
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
	if(key_y && !key_t) 
	{
		elapsed_seconds = end-t_yaw;
		if( yaw < MAX_ANGLE)
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
	if(key_Y && !key_t) 
	{
		elapsed_seconds = end-t_yaw;
		if( yaw > -MAX_ANGLE)
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
	if(key_x && !key_t) 
	{
		elapsed_seconds = end-t_pitch;
		if( pitch < MAX_ANGLE)
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
	if(key_X && !key_t) 
	{
		elapsed_seconds = end-t_pitch;
		if( pitch > -MAX_ANGLE)
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
		eye = Translate(vec4(0.0,0.0,key_t*AIR_VIEW,1.0))*( car_att*eye0-vec4(0.0,0.0,0.0,1.0) + car_pos*vec4(0.0,0.0,0.0,1.0)); //eye = car_init +  car_att*(eye0-car_init) + car_pos*vec4(0.0,0.0,0.0,1.0);
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
		eye = Translate(vec4(0.0,0.0,key_t*AIR_VIEW,1.0))*(car_att*eye0-vec4(0.0,0.0,0.0,1.0) + car_pos*vec4(0.0,0.0,0.0,1.0));
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
		car_att = RotateZ(dir*CAR_TURN_SPEED*elapsed_seconds.count())*car_att;
		eye = Translate(vec4(0.0,0.0,key_t*AIR_VIEW,1.0))*( car_att*eye0-vec4(0.0,0.0,0.0,1.0) + car_pos*vec4(0.0,0.0,0.0,1.0));
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
	}
	if(key_d && (key_w || key_s) ) 
	{	
		elapsed_seconds = end-t_rot;
		car_att = car_att*RotateZ(-dir*CAR_TURN_SPEED*elapsed_seconds.count());
		eye = Translate(vec4(0.0,0.0,key_t*AIR_VIEW,1.0))*(  car_att*eye0 -vec4(0.0,0.0,0.0,1.0)+ car_pos*vec4(0.0,0.0,0.0,1.0));
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
	}

	t_roll = end;
	t_pitch = end;
	t_yaw=end;
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

	traverse(&nodes[player]);

	glutSwapBuffers();
}

void reshape( int width, int height )
{
	glViewport( 0, 0, width, height );
	GLfloat aspect = GLfloat(width)/height;
	mat4  projection = Perspective( 45.0, aspect, 0.0001, 300.0 );
	glUniformMatrix4fv( Projection, 1, GL_TRUE, projection );
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

void getGLinfo()
{
	cout << "GL Vendor   : " << glGetString(GL_VENDOR) << endl;
	cout << "GL Renderer : " << glGetString(GL_RENDERER) << endl;
	cout << "GL Version  : " << glGetString(GL_VERSION) << endl;
} // Get OpenGL version information

void keyboard( unsigned char key, int x, int y )
{
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
	case 'T':
	case 't':
		if(!key_t)
		{
			bases2=RotateY(75)*bases;
			eye = Translate(0.0,0.0,AIR_VIEW)*eye;
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
			eye = Translate(0.0,0.0,-AIR_VIEW)*eye;
			key_t=!key_t;
		}
		break;
	case 'R':
	case 'r':
		bases = RotateZ(-roll)*RotateY(-yaw)*RotateX(-pitch)*bases;
		n = bases[0];
		n.w=1;
		u = bases[1];
		u.w=1;
		v = bases[2];
		v.w=1;
		roll=0;
		pitch=0;
		yaw=0;
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
		key_z=0;
		break;
	case 'z': // roll clockwise in the xy plane
		key_Z=0;
		key_z=0;
		break;
	case 'X': // pitch up
		key_X=0;
		key_x=0;
		break;
	case 'x': // pitch down
		key_X=0;
		key_x=0;
		break;
	case 'Y': // yaw up
		key_Y=0;
		key_y=0;
		break;
	case 'y': // yaw down
		key_y=0;
		key_Y=0;
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

void read_obj(char *filename,vec4 *vertices,vec3 *normals,vec4 *lights,int *mat_ind,float* shinnines,int* vertices_size,int* normals_size,int* faces_size)
{
	ifstream file,file1;
	string line,mtlpath;
	vector<string> mat_names;
	int pos=string::npos,temp1[6];
	float temp[3]={0,0,0};
	int num_vertices=0,num_normals=0,num_faces=0,num_mat=0,cont_mat=0;
	float materials[150][11];
	vec4 *temp_vertices = (vec4*)malloc(250000*sizeof(vec4));
	vec3 *temp_normals = (vec3*)malloc(250000*sizeof(vec3));
	vec4 mean,scale;
	vec3 scale2;
	float xmax=-99999,xmin=99999,ymax=-99999,ymin=99999,zmin=99999,zmax=-999999,xscale=1,yscale=1,zscale=1;

	file.open(filename);
	if(file.is_open())
	{
		//cout<<"opened geometry file\n";
		getline (file,line);
		getline (file,mtlpath);
		pos = mtlpath.find("/");
		if(pos==string::npos)
		{
			pos = mtlpath.find(" ");
		}
		mtlpath = mtlpath.substr(pos+1,mtlpath.size()-pos);
		file1.open(mtlpath.c_str());
		if(file1.is_open())
		{
			while ( getline (file1,line))
			{
				if(line.compare(0,6,"newmtl")==0) // if equal return 0, thats why "!"
				{
					cont_mat++;					
					mat_names.push_back(line.substr(7,line.size()-7));
					//materials = (float**)realloc(materials,cont_mat*sizeof(float*));
					//materials[cont_mat-1] = (float*)malloc(11*sizeof(float));
				}
				if(line.compare(0,2,"Ka")==0) // if equal return 0, thats why "!"
				{
					sscanf(line.c_str(),"Ka  %f %f %f",&temp[0],&temp[1],&temp[2]);
					materials[cont_mat-1][0] = temp[0];
					materials[cont_mat-1][1] = temp[1];
					materials[cont_mat-1][2] = temp[2];
				}
				if(line.compare(0,2,"Kd")==0) // if equal return 0, thats why "!"
				{
					sscanf(line.c_str(),"Kd  %f %f %f",&temp[0],&temp[1],&temp[2]);
					materials[cont_mat-1][3] = temp[0];
					materials[cont_mat-1][4] = temp[1];
					materials[cont_mat-1][5] = temp[2];
				}
				if(line.compare(0,2,"Ks")==0) // if equal return 0, thats why "!"
				{
					sscanf(line.c_str(),"Ks  %f %f %f",&temp[0],&temp[1],&temp[2]);
					materials[cont_mat-1][6] = temp[0];
					materials[cont_mat-1][7] = temp[1];
					materials[cont_mat-1][8] = temp[2];
				}
				if(line.compare(0,2,"Ns")==0) // if equal return 0, thats why "!"
				{
					sscanf(line.c_str(),"Ns %f",&temp[0]);
					materials[cont_mat-1][9] = temp[0];
				}
				if(line.compare(0,2,"d ")==0) // if equal return 0, thats why "!"
				{
					sscanf(line.c_str(),"d %f",&temp[0]);
					materials[cont_mat-1][10] = temp[0];
				}
			}
			file1.close();

		}else
		{
			cout << "Unable to open mtl.";
			return;
		}
		while ( getline (file,line))
		{
			if(line.compare(0,2,"v ")==0) // if equal return 0, thats why "!"
			{
				num_vertices++;
				sscanf(line.c_str(),"v %f %f %f",&temp[0],&temp[1],&temp[2]);
				if(temp[0]<xmin) xmin=temp[0];
				if(temp[0]>xmax) xmax=temp[0];
				if(temp[1]<ymin) ymin=temp[1];
				if(temp[1]>ymax) ymax=temp[1];
				if(temp[2]<zmin) zmin=temp[2];
				if(temp[2]>zmax) zmax=temp[2];
				temp_vertices[num_vertices-1] = vec4(temp[0],temp[1],temp[2],1);
			}
			if(line.compare(0,2,"vn")==0) // if equal return 0, thats why "!"
			{
				num_normals++;
				sscanf(line.c_str(),"v %f %f %f",&temp[0],&temp[1],&temp[2]);
				temp_normals[num_normals-1] = vec3(temp[0],temp[1],temp[2]);
			}
			if(line.compare(0,6,"usemtl")==0) // if equal return 0, thats why "!"
			{
				num_mat++;
				mat_ind[num_mat-1]=num_faces;				
				for(int i=0;i<cont_mat;i++)
				{
					if(line.compare(7,line.size()-7,mat_names[i]) == 0)
					{
						lights[num_mat-3] = vec4(materials[i][0],materials[i][1],materials[i][2],materials[i][10]);
						lights[num_mat-2] = vec4(materials[i][3],materials[i][4],materials[i][5],materials[i][10]);
						lights[num_mat-1] = vec4(materials[i][6],materials[i][7],materials[i][8],materials[i][10]);
						shinnines[num_mat-1] = materials[i][9];
						break;
					}
				}
			}
			if(line.compare(0,1,"f")==0) // if equal return 0, thats why "!"
			{
				num_faces++;
				xscale=xmax-xmin;
				yscale =ymax-ymin;
				zscale=zmax-zmin;
				mean = vec4(-(xmax-xmin)/2,-(ymax-ymin)/2,-(zmax-zmin)/2,1);
				scale = vec4(1/(xmax-xmin),1/(ymax-ymin),1/(zmax-zmin),1);
				scale2 = vec3((xmax-xmin),(ymax-ymin),(zmax-zmin));
				sscanf(line.c_str(),"f %i %i %i %i",&temp1[0],&temp1[1],&temp1[2],&temp1[3]);
				vertices[3*num_faces-3] = (temp_vertices[temp1[0]-1]-mean)*scale;
				vertices[3*num_faces-2] = (temp_vertices[temp1[1]-1]-mean)*scale;
				vertices[3*num_faces-1] = (temp_vertices[temp1[2]-1]-mean)*scale;
				normals[3*num_faces-3] = (temp_normals[temp1[0]-1])*scale2;
				normals[3*num_faces-2] = (temp_normals[temp1[1]-1])*scale2;
				normals[3*num_faces-1] = (temp_normals[temp1[2]-1])*scale2;
				num_faces++;
				vertices[3*num_faces-3] = (temp_vertices[temp1[1]-1]-mean)*scale;
				vertices[3*num_faces-2] = (temp_vertices[temp1[2]-1]-mean)*scale;
				vertices[3*num_faces-1] = (temp_vertices[temp1[3]-1]-mean)*scale;
				normals[3*num_faces-3] = (temp_normals[temp1[1]-1])*scale2;
				normals[3*num_faces-2] = (temp_normals[temp1[2]-1])*scale2;
				normals[3*num_faces-1] = (temp_normals[temp1[3]-1])*scale2;

			}
		}
		mat_ind[0]=num_mat;
		num_mat++;
		mat_ind[num_mat]=num_faces;	
		*vertices_size = num_vertices;
		*normals_size = num_normals;
		*faces_size = num_faces;
	}else
	{
		cout<< "Unable to open file";
	}

	delete[] temp_vertices;
	delete[] temp_normals;

	file.close();

	return;
}

void initNodes()
{
	mat4  m;

	m = mat4(1.0) ;
	nodes[player] = Node( m, Player, NULL, &nodes[house] );

	m = Translate(5.0, -5.0, 0.0)*RotateZ(90) ;
	nodes[house] = Node( m, House, &nodes[trees], &nodes[road]);

	m = Translate(10.0, -1.5, 0.0) ;
	nodes[trees] = Node( m, Trees, NULL, NULL);

	m = mat4(1.0); ;
	nodes[road] = Node( m, Road, &nodes[forest], NULL);

	//m = Translate(0.0, 0.0, 1.0)*Scale(2.0,2.0,2.0) ;
	nodes[forest] = Node( m, Forest, NULL, NULL);
}

void Road()
{
	//mvstack.push( model_view );
	//glUseProgram(cube_shader);
	//glBindBuffer( GL_ARRAY_BUFFER, buffers[0] );
	//glBindVertexArray( vao[0] );
	//xrot = 0;
	//yrot = 0;
	//zrot = 0;

	//scale_x = 50;
	//scale_y = 1;
	//scale_z =0.05;

	//	model_view_loc = glGetUniformLocation( cube_shader, "ModelView" );
	//proj_loc = glGetUniformLocation( cube_shader, "Projection" );  
	//projmat = Perspective(160,16/9,0.02, 15.0); 
	//glUniformMatrix4fv(proj_loc, 1, GL_TRUE, projmat);

	//scaleTransformation = Scale(scale_x, scale_y, scale_z);
	//invScaleTranformation = Scale(1/scale_x, 1/scale_y, 1/scale_z);

	////normalMatrix = RotateZ( zrot ) * RotateY( yrot )*RotateX( xrot ) *invScaleTranformation;
	////glUniformMatrix4fv(NormalTransformation , 1, GL_TRUE,  normalMatrix);

	//color4 material_ambient( 0.5, 0.5, 0.5, 1.0 );
	//color4 material_diffuse( 1.0, 0.8, 0.0, 1.0 );
	//color4 material_specular( 1.0, 0.0, 1.0, 1.0 );
	//int material_shininess = 5.0;


	//glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	//glUniform4fv( glGetUniformLocation(program, "AmbientProduct"), 1, light_ambient*material_ambient);
	//glUniform4fv( glGetUniformLocation(program, "DiffuseProduct"), 1, light_diffuse*material_diffuse );
	//glUniform4fv( glGetUniformLocation(program, "SpecularProduct"), 1, light_specular*material_specular  );
	//glUniform1f( glGetUniformLocation(program, "Shininess"),material_shininess );

	//modelViewObject = RotateZ( zrot ) * RotateY( yrot )*RotateX( xrot ) *scaleTransformation; 
	//	model_view_loc = glGetUniformLocation( program, "ModelView" );
	//glUniformMatrix4fv( model_view_loc, 1, GL_TRUE, model_view*modelViewObject );
	//glDrawArrays( GL_TRIANGLES, 0, sizeof(cube_points) );

	//glUseProgram(0);
	//glBindVertexArray(0);

	//model_view = mvstack.pop();
}

void Forest()
{

}

void Trees()
{
	mvstack.push( model_view );
	xrot = 90;
	yrot = 0;
	zrot = -90;

	scale_x = 1;
	scale_y = 1;
	scale_z =1;

	scaleTransformation = Scale(scale_x, scale_y, scale_z);
	invScaleTranformation = Scale(1/scale_x, 1/scale_y, 1/scale_z);

	normalMatrix = RotateZ( zrot ) * RotateY( yrot )*RotateX( xrot ) *invScaleTranformation;
	glUniformMatrix4fv(NormalTransformation , 1, GL_TRUE,  normalMatrix);

	modelViewObject = Translate(0, 0, 0.5)*RotateZ( zrot ) * RotateY( yrot )*RotateX( xrot ) *scaleTransformation; 
	glUniformMatrix4fv( ModelViewObj, 1, GL_TRUE,model_view* modelViewObject );
	glmDrawVBO(models[2], program);

	modelViewObject = Translate(1, 0, 0.5)*RotateZ( zrot ) * RotateY( yrot )*RotateX( xrot ) *scaleTransformation;
	glUniformMatrix4fv( ModelViewObj, 1, GL_TRUE, model_view*modelViewObject );
	glmDrawVBO(models[2], program);

	modelViewObject = Translate(2, 0, 0.5)*RotateZ( zrot ) * RotateY( yrot )*RotateX( xrot ) *scaleTransformation;
	glUniformMatrix4fv( ModelViewObj, 1, GL_TRUE,model_view* modelViewObject );
	glmDrawVBO(models[2], program);
	model_view = mvstack.pop();
}

void House()
{
	mvstack.push( model_view );
	xrot = 90;
	yrot = 0;
	zrot = 90;

	scale_x = 2;
	scale_y = 2;
	scale_z = 2;

	scaleTransformation = Scale(scale_x, scale_y, scale_z);
	invScaleTranformation = Scale(1/scale_x, 1/scale_y, 1/scale_z);

	normalMatrix = RotateZ( zrot ) * RotateY( yrot )*RotateX( xrot ) *invScaleTranformation;
	modelViewObject = Translate(0.0, 0.0, 1.5)*RotateZ( zrot ) * RotateY( yrot )*RotateX( xrot ) *scaleTransformation;

	glUniformMatrix4fv( ModelViewObj, 1, GL_TRUE,model_view* modelViewObject );
	glUniformMatrix4fv(NormalTransformation , 1, GL_TRUE,  normalMatrix);

	glmDrawVBO(models[1], program);
	model_view = mvstack.pop();
}

void Player()
{
	mvstack.push( model_view );
	xrot = 90;
	yrot = 0;
	zrot = 90;

	scale_x = 1;
	scale_y = 1;
	scale_z = 1;

	scaleTransformation = Scale(scale_x, scale_y, scale_z);
	invScaleTranformation = Scale(1/scale_x, 1/scale_y, 1/scale_z);

	normalMatrix =  car_att*RotateZ( zrot ) * RotateY( yrot )*RotateX( xrot ) * invScaleTranformation;
	modelViewObject = car_pos*car_att*RotateZ( zrot ) * RotateY( yrot )*RotateX( xrot ) *scaleTransformation;

	glUniformMatrix4fv( ModelViewObj, 1, GL_TRUE, model_view*modelViewObject );
	glUniformMatrix4fv(NormalTransformation , 1, GL_TRUE,  normalMatrix);

	glmDrawVBO(models[0], program);
	model_view = mvstack.pop();
}

/*
mvstack.push( model_view );
mat4 m = Translate(2.0,0.0,0.0);

glUseProgram( program );
glBindBuffer( GL_ARRAY_BUFFER, buffers[0] );
glBindVertexArray( vao[0] );
//model_view_loc = glGetUniformLocation( program, "ModelView" );
//glUniformMatrix4fv( model_view_loc, 1, GL_TRUE, model_view*m );
glUniformMatrix4fv( ModelViewObj, 1, GL_TRUE, model_view*m );
glUniformMatrix4fv(NormalTransformation , 1, GL_TRUE, mat4(1.0));


glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

vPosition = glGetAttribLocation( program, "vertex" );
glEnableVertexAttribArray( vPosition );
glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0,BUFFER_OFFSET(0) );

vNormal = glGetAttribLocation( program, "normal" );
glEnableVertexAttribArray( vNormal );
glVertexAttribPointer( vNormal, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(3*tree_face_sz*sizeof(vec4)));

for(int i=0;i<tree_mat_ind[0];i++)
{
//glUniformMatrix4fv( model_view_loc, 1, GL_TRUE, model_view*m );
//glUniform4fv(color_loc, 1, car_mat[3*i+2]);
glUniform4fv( glGetUniformLocation(program, "mat_ambient"), 1, tree_mat[3*i]);
glUniform4fv( glGetUniformLocation(program, "mat_diffuse"), 1, tree_mat[3*i+1] );
glUniform4fv( glGetUniformLocation(program, "mat_specular"), 1, tree_mat[3*i+2]  );
glUniform1f( glGetUniformLocation(program, "mat_shininess"),tree_shinnines[i] );
if(i==0)
{
glDrawArrays( GL_TRIANGLES, 0, 3*tree_mat_ind[i+1]-1 );
}else
{
glDrawArrays( GL_TRIANGLES, 3*tree_mat_ind[i], 3*tree_mat_ind[i+1]-3*tree_mat_ind[i] );
}
}
model_view = mvstack.pop();
*/