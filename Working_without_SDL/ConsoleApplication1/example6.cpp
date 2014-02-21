/*****

CS 537: Fragment shading of a sphere 
Author: George Kamberov


Code base: Angel 5th Edition Chapters 5.6-5.10
OGL Compatability: OGL3.1 GLSL 1.5 

Shaders: vshader56.glsl, fshader56.glsl

Lighting Scenario: distant light moving independently of the camera

Interaction: 

   Up/down arrow key     = Camera Slide forward/backward 
   Left/right arrow key  = Camera Slide left/right 



WARNING: The code supplement for Chapter 5 avialbale on Angel's Textbook  web site is buggy.  



***/
#include "stdafx.h"
#include "Angel.h"

const int NumTimesToSubdivide = 5;
const int NumTriangles        = 4096;  // (4 faces)^(NumTimesToSubdivide + 1)
const int NumVertices         = 3 * NumTriangles;

GLuint program;
GLuint vao;

typedef Angel::vec4 point4;
typedef Angel::vec4 color4;
float  material_shininess;
color4 ambient_product;
color4 diffuse_product;
color4 specular_product; 
point4 light_position;
GLuint vPosition;
GLuint vNormal;

//Spinning light: orbit parameterization
float spin =0.0, spin_step = 0.01; 

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

void
triangle( const point4& a, const point4& b, const point4& c )
{
    vec3  normal = normalize( cross(b - a, c - b) );

    normals[Index] = normal;  points[Index] = a;  Index++;
    normals[Index] = normal;  points[Index] = b;  Index++;
    normals[Index] = normal;  points[Index] = c;  Index++;
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
	
    // Create a vertex array object
    glGenVertexArrays( 1, &vao );
    glBindVertexArray( vao );


    // Create and initialize a buffer object
    GLuint buffer;
    glGenBuffers( 1, &buffer );
    glBindBuffer( GL_ARRAY_BUFFER, buffer );
    glBufferData( GL_ARRAY_BUFFER, sizeof(points) + sizeof(normals), NULL, GL_STATIC_DRAW );
    glBufferSubData( GL_ARRAY_BUFFER, 0, sizeof(points), points );
    glBufferSubData( GL_ARRAY_BUFFER, sizeof(points), sizeof(normals), normals );
	
    // Load shaders and use the resulting shader program
    program = InitShader( "vshader56.glsl", "fshader56.glsl" );
    glUseProgram( program );

	glBindVertexArray( vao );
    // set up vertex arrays

	vPosition = glGetAttribLocation( program, "vPosition" );
    glEnableVertexAttribArray( vPosition );
    glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0,
			   BUFFER_OFFSET(0) );

    vNormal = glGetAttribLocation( program, "vNormal" ); 
    glEnableVertexAttribArray( vNormal );
    glVertexAttribPointer( vNormal, 3, GL_FLOAT, GL_FALSE, 0,
			   BUFFER_OFFSET(sizeof(points)) );

    // Initialize shader lighting parameters
    light_position = vec4( 0.0, 0.0, 1.0, 0.0 );
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

    glUniform4fv( glGetUniformLocation(program, "AmbientProduct"),
		  1, ambient_product );
    glUniform4fv( glGetUniformLocation(program, "DiffuseProduct"),
		  1, diffuse_product );
    glUniform4fv( glGetUniformLocation(program, "SpecularProduct"),
		  1, specular_product );
	
    glUniform1f( glGetUniformLocation(program, "Shininess"),
		 material_shininess );
		 
    // Retrieve transformation uniform variable locations
    ModelView = glGetUniformLocation( program, "ModelView" );
    Projection = glGetUniformLocation( program, "Projection" );    
    
    glClearColor( 0.0, 0.0, 0.0, 1.0 ); /* black background */
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
    light_position = vec4(-3*sin(spin), 4, -3*cos(spin), 1.0);
	glUniform4fv( glGetUniformLocation(program, "LightPosition"), 1, light_position );

	glBindVertexArray( vao );
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glDrawArrays( GL_TRIANGLES, 0, NumVertices );
	
    glutSwapBuffers();
}

//----------------------------------------------------------------------------




void keyboard( unsigned char key, int x, int y )
{
    switch( key ) {
	case 033: // Escape Key
	case 'q': case 'Q':
	    exit( EXIT_SUCCESS );
	    break;
    }
}

//----------------------------------------------------------------------------

void
reshape( int width, int height )
{
	
	glViewport( 0, 0, (GLsizei) width, (GLsizei) height );

	projection = Perspective(30,1.0,1.0,10.0);
	glUniformMatrix4fv( Projection, 1, GL_TRUE, projection );
}

//----------------------------------------------------------------------------

int
main( int argc, char **argv )
{
    glutInit( &argc, argv );
    glutInitDisplayMode( GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH );
    glutInitWindowSize( 512, 512 );
    glutCreateWindow( "Shaded Sphere" );
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