/*****

CS 537: OGL Instancing 
Author: George Kamberov


Shaders: vshader56_OGL2_GLSL110_InstArraysGroundedFlyStub.glsl, fshader56_v110.glsl

An example using Instancing to draw NumInstances instances  of a spherical mesh. 
The spheres are laid out on a planar grid. The sphere radii are random. The spheres are all graunded on 
a plane at y = -1( in camera coordinates). 

Lighting Scenario: distant light moving independently of the camera

Interaction: 

Up/down arrow key     = Camera Slide forward/backward 
Left/right arrow key  = Camera Slide left/right 

z/Z = roll the camera -- you must implement
c/C yaw the camera  -- you must implement
x/X pitch the camera --  implemented

***/

#include <stdlib.h>
#include <time.h>
#include "Angel.h"

#ifndef PI
#  define PI  3.14159265358979323846
#endif

#define X               0
#define Y               1
#define Z               2
#define SCALE_VECTOR    1.0
#define SCALE_ANGLE     1.0

const float scale_bound =0.0; 
void keyboard( unsigned char key, int x, int y );

//Pre: 0\leq float low_limit \leq 1 
//Post:a roandom flat in the interval [low_limit, 1]
float randScale(float low_limit){
	float range = 1 - low_limit; 
	return (low_limit + range*((float) rand()/RAND_MAX));
}

const int NumTimesToSubdivide = 5;
const int NumTriangles        = 4096;  // (4 faces)^(NumTimesToSubdivide + 1)
const int NumVertices         = 3 * NumTriangles;

const int plantGrid = 20; 
const int NumInstances = plantGrid*plantGrid; 

vec3 shiftATTR[NumInstances];

GLuint program;

typedef Angel::vec4 point4;
typedef Angel::vec4 color4;
float  material_shininess;
color4 ambient_product;
color4 diffuse_product;
color4 specular_product; 
point4 light_position;
GLuint vPosition;
GLuint vNormal;
GLuint vShift;

//Spinning light: orbit parameterization
float spin =0.0, spin_step = 0.1; 

GLfloat d=0.5;

point4 points[NumVertices];
vec3   normals[NumVertices];







void specKey(int key, int x, int y); 

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
void idle(void){

	//Spinning light orbit: azimuth update 
	spin += spin_step; 

	glutPostRedisplay();
}

mat4 projection = mat4(1.0);
/////Camera unv basis///////

Angel::vec4 n = Angel::normalize( vec4(0.0, 1.0, 3.0, 0.0));
Angel::vec4 v = Angel::normalize( vec4(0.0, 3.0, -1.0, 0.0));
Angel::vec4 u = vec4(1.0, 0.0, 0.0, 0.0);
Angel::vec4 eye = vec4(0.0, 1.0, 3.0, 1.0);


// Model-view and projection matrices uniform location
GLuint  ModelView, Projection;

//----------------------------------------------------------------------------

int Index = 0;


//Sphere triangle. The normal Component must be changed for other surfaces
void
	triangle( const point4& a, const point4& b, const point4& c )
{


	normals[Index] = Angel::normalize(vec3(a[0], a[1], a[2]));  points[Index] = a;  Index++;
	normals[Index] = Angel::normalize(vec3(b[0], b[1], b[2]));  points[Index] = b;  Index++;
	normals[Index] = Angel::normalize(vec3(c[0], c[1], c[2]));  points[Index] = c;  Index++;
}

//----------------------------------------------------------------------------

point4
	unit( const point4& p )
{
	float len = p.x*p.x + p.y*p.y + p.z*p.z;

	point4 t;
	if ( len > DivideByZeroTolerance ) {
		t = p / sqrt(len);
		t.w = 1.0;
	}

	return t;
}

void
	divide_triangle( const point4& a, const point4& b,
	const point4& c, int count )
{
	if ( count > 0 ) {
		point4 v1 = unit( a + b );
		point4 v2 = unit( a + c );
		point4 v3 = unit( b + c );
		divide_triangle(  a, v1, v2, count - 1 );
		divide_triangle(  c, v2, v3, count - 1 );
		divide_triangle(  b, v3, v1, count - 1 );
		divide_triangle( v1, v3, v2, count - 1 );
	}
	else {
		triangle( a, b, c );
	}
}

void
	tetrahedron( int count )
{
	point4 v[4] = {
		vec4( 0.0, 0.0, 1.0, 1.0 ),
		vec4( 0.0, 0.942809, -0.333333, 1.0 ),
		vec4( -0.816497, -0.471405, -0.333333, 1.0 ),
		vec4( 0.816497, -0.471405, -0.333333, 1.0 )
	};

	divide_triangle( v[0], v[1], v[2], count );
	divide_triangle( v[3], v[2], v[1], count );
	divide_triangle( v[0], v[3], v[1], count );
	divide_triangle( v[0], v[2], v[3], count );
}

//----------------------------------------------------------------------------

// OpenGL initialization
void
	init()
{
	// Subdivide a tetrahedron into a sphere
	tetrahedron( NumTimesToSubdivide );

	// Instanced Arrays
	srand((unsigned)time(0));
	for (int i=0; i < plantGrid; i++)
	{
		shiftATTR[i*plantGrid+j]=vec3(2.1*i,-2.1*j, randScale(scale_bound)); 
	}

	// Create and initialize a buffer object

	GLuint buffer;
	glGenBuffers( 1, &buffer );

	glBindBuffer( GL_ARRAY_BUFFER, buffer );
	glBufferData( GL_ARRAY_BUFFER, sizeof(points) + sizeof(normals)+sizeof(shiftATTR), NULL, GL_STATIC_DRAW );
	glBufferSubData( GL_ARRAY_BUFFER, 0, sizeof(points), points );
	glBufferSubData( GL_ARRAY_BUFFER, sizeof(points), sizeof(normals), normals );
	glBufferSubData( GL_ARRAY_BUFFER, sizeof(points)+sizeof(normals), sizeof(shiftATTR), shiftATTR );




	// Load shaders and use the resulting shader program
	program = InitShader( "vshader56_OGL2_GLSL110_InstArraysGroundedFlyStub.glsl", "fshader56_v110_Inst.glsl" );
	glUseProgram( program );


	// Set up vertex attrbute arrays:
	// vPosition  for the positions of the vertices (4D vectors)
	// vNormal    for the normals of the vertices (3D vectors)

	vPosition = glGetAttribLocation( program, "vPosition" );
	glEnableVertexAttribArray( vPosition );
	glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0,
		BUFFER_OFFSET(0) );

	vNormal = glGetAttribLocation( program, "vNormal" ); 
	glEnableVertexAttribArray( vNormal );
	glVertexAttribPointer( vNormal, 3, GL_FLOAT, GL_FALSE, 0,
		BUFFER_OFFSET(sizeof(points)) );

	//The Instanced attribute
	vShift = glGetAttribLocation( program, "vShift" ); 
	glEnableVertexAttribArray( vShift );
	glVertexAttribPointer( vShift, 3, GL_FLOAT, GL_FALSE, 0,
		BUFFER_OFFSET(sizeof(points)+sizeof(normals)) );
	glVertexAttribDivisor(vShift, 1);

	// Initialize shader lighting parameters
	light_position = vec4( -1.0, 0.0, 1.0, 0.0 );
	color4 light_ambient( 0.2, 0.2, 0.2, 1.0 );
	color4 light_diffuse( 1.0, 1.0, 1.0, 1.0 );
	color4 light_specular( 1.0, 1.0, 1.0, 1.0 );

	color4 material_ambient( 1.0, 1.0, 1.0, 1.0 );
	color4 material_diffuse( 1.0, 0.8, 0.0, 1.0 );
	color4 material_specular( 1.0, 0.0, 1.0, 1.0 );
	material_shininess = 5.0;

	ambient_product = light_ambient * material_ambient;
	diffuse_product = light_diffuse * material_diffuse;
	specular_product = light_specular * material_specular;

	glUniform4fv( glGetUniformLocationARB(program, "AmbientProduct"),
		1, ambient_product );
	glUniform4fv( glGetUniformLocationARB(program, "DiffuseProduct"),
		1, diffuse_product );
	glUniform4fv( glGetUniformLocationARB(program, "SpecularProduct"),
		1, specular_product );

	glUniform1f( glGetUniformLocationARB(program, "Shininess"),
		material_shininess );

	// Retrieve transformation uniform variable locations
	ModelView = glGetUniformLocation( program, "ModelView" );
	Projection = glGetUniformLocation( program, "Projection" );

	glClearColor( 1.0, 1.0, 1.0, 1.0 ); /* white background */
	glClearDepth( 1.0 ); 
	glEnable( GL_DEPTH_TEST );
	glDepthFunc(GL_LEQUAL);
}

//----------------------------------------------------------------------------

void
	display( void )
{
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );


	mat4 model_view = LookAt( eye, eye-n, v );


	glUniformMatrix4fv( ModelView, 1, GL_TRUE, model_view );


	//spinning distant light
	light_position = vec4(-3*sin(spin), 4, -3*cos(spin), 0.0);
	glUniform4fv( glGetUniformLocation(program, "LightPosition"), 1, light_position );

	glDrawArraysInstanced( GL_TRIANGLES, 0, NumVertices, NumInstances);

	glutSwapBuffers();
}

void keyboard( unsigned char key, int x, int y )
{


	GLfloat xt, yt, zt;
	GLfloat cosine, sine;

	// positive or negative rotation depending on upper or lower case letter
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
		/****** You must write the rest of teh code yourself
		case 'X': // pitch up
		case 'x': // pitch down
		// complete
		****/
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
		xt = u[X];
		yt = u[Y];
		zt = u[Z];
		u[X] = xt*cosine - n[X]*sine;
		u[Y] = yt*cosine - n[Y]*sine;
		u[Z] = zt*cosine - n[Z]*sine;
		n[X] = xt*sine + n[X]*cosine;
		n[Y] = yt*sine + n[Y]*cosine;
		n[Z] = zt*sine + n[Z]*cosine;
		break;
	case 033:  // Escape key
	case 'q': case 'Q':
		exit( EXIT_SUCCESS );
	}

	glutPostRedisplay();
}

//----------------------------------------------------------------------------

void
	reshape( int width, int height )
{

	glViewport( 0, 0, (GLsizei) width, (GLsizei) height );

	projection = Perspective(90,1.0,1.0,20.0);
	glUniformMatrix4fv( Projection, 1, GL_TRUE, projection );
}

//----------------------------------------------------------------------------

int
	main( int argc, char **argv )
{
	glutInit( &argc, argv );
	glutInitDisplayMode( GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH );
	glutInitWindowSize( 512, 512 );
	glutCreateWindow( "Instanced Arrays" );
	m_glewInitAndVersion();

	init();

	glutDisplayFunc( display );
	glutReshapeFunc( reshape );
	glutKeyboardFunc( keyboard );
	glutSpecialFunc( specKey );
	glutIdleFunc(idle);

	glutMainLoop();
	return 0;
}

void specKey(int key, int x, int y)
{ 
	switch( key ) {
	case GLUT_KEY_UP : // MOVE FORWARD
		eye -=0.5*n;
		break;
	case GLUT_KEY_DOWN :  // MOVE WARD
		eye +=0.5*n;
		break;
	case GLUT_KEY_LEFT: // MOVE LEFT
		eye -=0.5*u;
		break;
	case GLUT_KEY_RIGHT: // MOVE RIGHT
		eye +=0.5*u;
		break;
	}

}