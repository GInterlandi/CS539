//CS 537, TM and shading
//Slowly rotating object texture mapping and shading 
//   Geometry: cube mesh, with normal map from a cylinder
//   From most angles this simple mesh will have cylindrical appearance (in particular if yuo do not see the rectangual top/bottom rims)
//
//Single point light source, roatating independenly of teh camera snd teh object
//Fixed camera
//TM -- three choices 
//   Deafult = checkerboard
//   Switchable through with by pressing keys
//   1, 2, 3
//      -- image files: grass256by256.ppm", brks.ppm, checker.ppm
//   The images are loade with GLM (need glm.h and glm.cpp)
//
//   Shaders: "shadedTM_vert.glsl", "shadedTM_frag.glsl" 
//               
//
//G. Kamberov

#include "stdafx.h"
#include "Angel.h"
#include "glm.h"
#include <chrono>

#define degrees2Radians 0.01745329251
#define SCALE_ANGLE     60.0 
#define SCALE_TRANS     1.0 

GLfloat theta =0; 

bool key_x=0,key_X=0,key_y=0,key_Y=0,key_z=0,key_Z=0;
float roll=0,pitch=0,yaw=0;
std::chrono::time_point<std::chrono::system_clock> t_roll,t_pitch,t_yaw;
bool key_s=0,key_w=0,key_a=0,key_d=0;
std::chrono::time_point<std::chrono::system_clock> t_rot,t_trans;

const int  NumTriangles = 8; // (4 faces)(2 triangles/face)
const int  NumVertices  = 3 * NumTriangles;

int  TextureWidth[3];
int  TextureHeight[3];

typedef Angel::vec4 point4;

#include<iostream>
using namespace std;

void copperPolys(void);

//Attributes 
GLuint vPosition;
GLuint vNormal;
GLuint vTexCoord; 

// Texture objects and storage for texture image
GLuint textures[3];


GLubyte* image0; 
GLubyte* image1;
GLubyte* image2;

// Vertex data arrays
point4  points[NumVertices];
vec3    quad_Normal[NumVertices];
vec2    tex_coords[NumVertices];

// Create a vertex array object
GLuint vao;

// Global Transformation matrices 
mat4 modelViewCamera; 

//----------------------------------------------------------------------------

bool mouseDown = false;
float xrot = -20;
float yrot = 0;
float xdiff = 0;
float ydiff = 0;
GLfloat step = 5;


// Camera Coordianate System
vec4 u = vec4(1, 0, 0, 0);
vec4 v = vec4(0, 1, 0, 0);
vec4 n = vec4(0, 0, 1, 0);
vec4 eye = vec4(0,0,2,1);
mat4 bases = mat4(n,u,v,vec4(0.0,0.0,0.0,1.0)); 

GLuint program;


GLuint  ModelViewCam; //Viewing portion of the ModelviewMatrix  uniform location
GLuint ModelViewObj;  // Object Modeling  portion of the ModelviewMatrix  uniform location
GLuint Projection;    //Pprojection matrices uniform location
GLuint NormalTransformation;  //Normal matrix uniform location
GLuint LightPosition;  //Light postition uniform location


// Setup some lights
vec4 light_ambient( 0.5, 0.5, 0.0, 1.0 );
vec4 light_diffuse( 1.0, 1.0, 1.0, 1.0 );
vec4 light_specular( 1.0, 1.0, 1.0, 1.0 );
//LIGHT POSITION
vec4 light_position = vec4(0.0, 1.0, 3.0, 1.0);

vec4 material_ambient( 0.8, 0.36, 0.36, 1.0 );
vec4 material_diffuse( 0.88, 0.34, 0.2, 1.0 );
vec4 material_specular( 1.0, 0.84, 0.77, 1.0 );
float  material_shininess = 100.0;

vec4 productComponentwise(vec4 a, vec4 b)
{
	return vec4(a[0]*b[0], a[1]*b[1], a[2]*b[2],a[3]*b[3]);
}

vec4 ambient_product = productComponentwise(light_ambient, material_ambient);
vec4 diffuse_product = productComponentwise(light_diffuse, material_diffuse);
vec4 specular_product = productComponentwise(light_specular, material_specular);





int Index = 0;

void
	quad( int a, int b, int c, int d )
{
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

	points[Index] = vertices[a];
	quad_Normal[Index] = -normalize(cross(vertices[a]-vertices[b],vertices[c]-vertices[b]));
	tex_coords[Index] = vec2( 0.0, 0.0 );
	Index++;

	points[Index] = vertices[b];
	quad_Normal[Index] = -normalize(cross(vertices[a]-vertices[b],vertices[c]-vertices[b]));
	tex_coords[Index] = vec2( 0.0, 1.0 );
	Index++;

	points[Index] = vertices[c];
	quad_Normal[Index] = -normalize(cross(vertices[a]-vertices[b],vertices[c]-vertices[b]));
	tex_coords[Index] = vec2( 1.0, 1.0 );
	Index++;


	points[Index] = vertices[a];
	quad_Normal[Index] = -normalize(cross(vertices[a]-vertices[b],vertices[c]-vertices[b]));
	tex_coords[Index] = vec2( 0.0, 0.0 );
	Index++;


	points[Index] = vertices[c];
	quad_Normal[Index] = -normalize(cross(vertices[a]-vertices[b],vertices[c]-vertices[b]));
	tex_coords[Index] = vec2( 1.0, 1.0 );
	Index++;


	points[Index] = vertices[d];
	quad_Normal[Index] = -normalize(cross(vertices[a]-vertices[b],vertices[c]-vertices[b]));
	tex_coords[Index] = vec2( 1.0, 0.0 );
	Index++;
}

//----------------------------------------------------------------------------

void
	cyllMesh()
{
	quad( 1, 0, 3, 2 );
	quad( 2, 3, 7, 6 );
	quad( 4, 5, 6, 7 );
	quad( 5, 4, 0, 1 );
}

//----------------------------------------------------------------------------

void
	init()
{
	cyllMesh();


	image0 = glmReadPPM("./data/copper_texture256by256.ppm", &TextureWidth[0], &TextureHeight[0]);
	image1 = glmReadPPM("./data/brks.ppm", &TextureWidth[1], &TextureHeight[1]);
	image2 = glmReadPPM("./data/checker.ppm", &TextureWidth[2], &TextureHeight[2]);

	// Initialize texture objects

	glGenTextures( 3, textures );

	glActiveTexture( GL_TEXTURE0 ); //See p 469 Red Book 7th ed
	glBindTexture( GL_TEXTURE_2D, textures[0] );
	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, TextureWidth[0], TextureHeight[0], 0,
		GL_RGB, GL_UNSIGNED_BYTE, image0 );
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );

	glBindTexture( GL_TEXTURE_2D, textures[1] );
	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, TextureWidth[1], TextureHeight[1], 0,
		GL_RGB, GL_UNSIGNED_BYTE, image1 );
	glGenerateMipmap(GL_TEXTURE_2D);
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST_MIPMAP_NEAREST );
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST );

	glBindTexture( GL_TEXTURE_2D, textures[2] );
	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, TextureWidth[2], TextureHeight[2], 0,
		GL_RGB, GL_UNSIGNED_BYTE, image2 );
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );    

	glActiveTexture( GL_TEXTURE0 );

	glBindTexture( GL_TEXTURE_2D, textures[2] );

	if (!GLEW_ARB_vertex_array_object) 
		std::cerr << "ARB_vertex_array_object not available." << std::endl; 

	glGenVertexArrays( 1, &vao );

	// Create and initialize a buffer object
	GLuint buffer;
	glGenBuffers( 1, &buffer );
	glBindBuffer( GL_ARRAY_BUFFER, buffer );
	glBufferData( GL_ARRAY_BUFFER,
		sizeof(points) + sizeof(quad_Normal) + sizeof(tex_coords),
		NULL, GL_STATIC_DRAW );
	glBindVertexArray( vao );
	// Specify an offset to keep track of where we're placing data in our
	//   vertex array buffer.  We'll use the same technique when we
	//   associate the offsets with vertex attribute pointers.
	GLintptr offset = 0;
	glBufferSubData( GL_ARRAY_BUFFER, offset, sizeof(points), points );
	offset += sizeof(points);

	glBufferSubData( GL_ARRAY_BUFFER, offset,
		sizeof(quad_Normal), quad_Normal );
	offset += sizeof(quad_Normal);

	glBufferSubData( GL_ARRAY_BUFFER, offset, sizeof(tex_coords), tex_coords );

	// Load shaders and use the resulting shader program
	program = InitShader( "./shaders/shadedTM_vert.glsl", "./shaders/shadedTM_frag.glsl"  );
	glUseProgram( program );

	// set up vertex arrays
	offset = 0;
	vPosition = glGetAttribLocation( program, "vertex" );
	glEnableVertexAttribArray( vPosition );
	glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0,
		BUFFER_OFFSET(offset) );
	offset += sizeof(points);

	vNormal = glGetAttribLocation( program, "normal" ); 
	glEnableVertexAttribArray( vNormal );
	glVertexAttribPointer( vNormal, 3, GL_FLOAT, GL_FALSE, 0,
		BUFFER_OFFSET(offset) );
	offset += sizeof(quad_Normal);

	vTexCoord = glGetAttribLocation( program, "vTexCoord" );
	glEnableVertexAttribArray( vTexCoord );
	glVertexAttribPointer( vTexCoord, 2, GL_FLOAT, GL_FALSE, 0,
		BUFFER_OFFSET(offset) );

	// Set the value of the fragment shader texture sampler variable
	//   ("texture") to the the appropriate texture unit. In this case,
	//   zero, for GL_TEXTURE0 which was previously set by calling
	//   glActiveTexture().
	glUniform1i( glGetUniformLocation(program, "texture"), 0 );


	glEnable( GL_DEPTH_TEST );
	glClearDepth(1.0);
	glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );

	glClearColor( 1.0, 1.0, 1.0, 1.0 );
}

void
	display( void )
{
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	mat4 modelViewCamera = LookAt(eye, eye-n, v);

	copperPolys(); 


	glutSwapBuffers();
}

//----------------------------------------------------------------------------

void
	mouse( int button, int state, int x, int y )
{
	glutPostRedisplay();
}

//----------------------------------------------------------------------------

void
	idle( void )
{

	std::chrono::time_point<std::chrono::system_clock>  end;
	end = std::chrono::system_clock::now();
	std::chrono::duration<double> elapsed_seconds;
	int dir=1;

	if(key_z ) 
	{
		elapsed_seconds = end-t_roll;
		// positive or negative rotation depending on upper or lower case letter
		roll = roll + SCALE_ANGLE*elapsed_seconds.count();
	}
	if(key_Z) 
	{
		elapsed_seconds = end-t_roll;
		roll = roll - SCALE_ANGLE*elapsed_seconds.count();
	}
	if(key_y) 
	{
		elapsed_seconds = end-t_yaw;
		yaw = yaw + SCALE_ANGLE*elapsed_seconds.count();
	}
	if(key_Y ) 
	{
		elapsed_seconds = end-t_yaw;
		yaw = yaw - SCALE_ANGLE*elapsed_seconds.count();
	}
	if(key_x ) 
	{
		elapsed_seconds = end-t_pitch;
		pitch = pitch + SCALE_ANGLE*elapsed_seconds.count();
	}
	if(key_X) 
	{
		elapsed_seconds = end-t_pitch;
		pitch = pitch - SCALE_ANGLE*elapsed_seconds.count();
	}
	if(key_w) 
	{
		elapsed_seconds = end-t_trans;

		eye =Translate(0,0,-SCALE_TRANS*elapsed_seconds.count()) *eye; 
		n = bases[0];
		n.w=1;
		u = bases[1];
		u.w=1;
		v = bases[2];
		v.w=1;

	}
	if(key_s) 
	{
		elapsed_seconds = end-t_trans;
		eye =Translate(0,0,SCALE_TRANS*elapsed_seconds.count()) *eye; 
		n = bases[0];
		n.w=1;
		u = bases[1];
		u.w=1;
		v = bases[2];
		v.w=1;
	}
	if(key_a ) 
	{
		elapsed_seconds = end-t_rot;
		eye =Translate(-SCALE_TRANS*elapsed_seconds.count(),0,0) *eye;
		n = bases[0];
		n.w=1;
		u = bases[1];
		u.w=1;
		v = bases[2];
		v.w=1;
	}
	if(key_d ) 
	{	
		elapsed_seconds = end-t_rot;
		eye =Translate(SCALE_TRANS*elapsed_seconds.count(),0,0) *eye;
		n = bases[0];
		n.w=1;
		u = bases[1];
		u.w=1;
		v = bases[2];
		v.w=1;
	}

	t_roll = end;
	t_pitch = end;
	t_yaw=end;
	t_rot = end;
	t_trans = end;

	glutPostRedisplay();
}

//----------------------------------------------------------------------------

void
	keyboard( unsigned char key, int mousex, int mousey )
{
	switch( key ) {
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
	case 033: // Escape Key
	case 'q': case 'Q':
		exit( EXIT_SUCCESS );
		break;
	case '1':
		glActiveTexture( GL_TEXTURE0 );
		glBindTexture( GL_TEXTURE_2D, textures[0] );
		break;

	case '2':
		glActiveTexture( GL_TEXTURE0 );
		glBindTexture( GL_TEXTURE_2D, textures[1] );
		break;

	case '3':
		glActiveTexture( GL_TEXTURE0 );
		glBindTexture( GL_TEXTURE_2D, textures[2] );
		break;
	}

	glutPostRedisplay();
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
//----------------------------------------------------------------------------

// Get OpenGL version information
void getGLinfo()
{
	cout << "GL Vendor   : " << glGetString(GL_VENDOR) << endl; 
	cout << "GL Renderer : " << glGetString(GL_RENDERER) << endl;
	cout << "GL Version  : " << glGetString(GL_VERSION) << endl;
}
void reshape( int width, int height );

int
	main( int argc, char **argv )
{
	glutInit( &argc, argv );
	glutInitDisplayMode( GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH );
	glutInitWindowSize( 512, 512 );
	glutCreateWindow( "Textured and Shading, Cylinder Normal Mapping" );
	getGLinfo();
	glewInit();
	init();
	glutReshapeFunc(reshape);
	glutDisplayFunc( display );
	glutKeyboardFunc( keyboard );
	glutMouseFunc( mouse );
	glutIdleFunc( idle );
	glutKeyboardUpFunc(KeyboardUp);

	glutMainLoop();
	return 0;
}

void copperPolys(void)
{



	glUseProgram(program);



	// Bind the Ka/Kd/Ks to the shader
	glUniform4fv( glGetUniformLocation(program, "AmbientProduct"),1, ambient_product);
	glUniform4fv( glGetUniformLocation(program, "DiffuseProduct"),1, diffuse_product);
	glUniform4fv( glGetUniformLocation(program, "SpecularProduct"),1,specular_product);	
	glUniform1f( glGetUniformLocation(program, "mat_shininess"), material_shininess);
	glUniform4fv( glGetUniformLocation(program, "lightPosition"),1, light_position);


	// Retrieve transformation uniform variable locations
	ModelViewCam = glGetUniformLocation( program, "modelView" );
	ModelViewObj = glGetUniformLocation(program, "ModelViewObj");
	Projection = glGetUniformLocation( program, "projection" );
	NormalTransformation = glGetUniformLocation( program, "normalTransformation" );

	mat4 modelViewCamera = LookAt(eye, eye-n, v);
	glUniformMatrix4fv(ModelViewCam, 1, GL_TRUE, modelViewCamera);

	mat4 modelViewObject = RotateZ(roll)*RotateY(yaw)*RotateX(pitch)*Scale(1.0, 2.2, 1.0);
	mat4 normalMatrix = modelViewCamera*RotateZ(roll)*RotateY(yaw)*RotateX(pitch)*Scale(1.0, 1.0/2.2, 1.0);
	glUniformMatrix4fv( ModelViewObj, 1, GL_TRUE, modelViewObject );
	glUniformMatrix4fv(NormalTransformation , 1, GL_TRUE,  normalMatrix);

	glBindVertexArray( vao );
	glDrawArrays( GL_TRIANGLES, 0, NumVertices );


	glUseProgram(0);
	glBindVertexArray(0);

}

void reshape( int width, int height )
{ 
	glViewport( 0, 0, width, height );
	GLfloat aspect = GLfloat(width)/height;
	mat4  projection = Perspective( 80.0, aspect, 0.1, 10.0 );
	glUseProgram(program);
	Projection = glGetUniformLocation( program, "projection" );
	glUniformMatrix4fv( Projection, 1, GL_TRUE, projection );
}