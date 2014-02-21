#include "stdafx.h"
#include "CApp.h"

bool CApp::OnInit() {
	if(SDL_Init(SDL_INIT_EVERYTHING) < 0) {
		return false;
	}

	Main_window = SDL_CreateWindow( "Homework 1", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN /*| SDL_GL_DOUBLEBUFFER*/ | SDL_WINDOW_OPENGL );

	if(Main_window == NULL) {
		return false;
	}
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

	GLcontext = SDL_GL_CreateContext( Main_window );
	if( GLcontext == NULL )
	{
		printf( "OpenGL context could not be created! SDL Error: %s\n", SDL_GetError() );
		return false;
	}

	//Initialize GLEW
	glewExperimental = GL_TRUE; 
	GLenum glewError = glewInit();
	if( glewError != GLEW_OK )
	{
		printf( "Error initializing GLEW! %s\n", glewGetErrorString( glewError ) );
		return false;
	}

	//Use Vsync
	//if( SDL_GL_SetSwapInterval( 1 ) < 0 )
	//{
	//	printf( "Warning: Unable to set VSync! SDL Error: %s\n", SDL_GetError() );
	//	return false;
	//}

	GLinit();

	return true;
}

void CApp::GLinit()
{
	cyllMesh();

	image0 = glmReadPPM("./data/copper_texture256by256.ppm", &TextureWidth, &TextureHeight);

	glGenTextures( 1, &texture );

	glActiveTexture( GL_TEXTURE0 ); //See p 469 Red Book 7th ed
	glBindTexture( GL_TEXTURE_2D, texture );
	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, TextureWidth, TextureHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, image0 );
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );

	glActiveTexture( GL_TEXTURE0 );

	glBindTexture( GL_TEXTURE_2D, texture );

	if (!GLEW_ARB_vertex_array_object) 
		std::cerr << "ARB_vertex_array_object not available." << std::endl; 

	glGenVertexArrays( 1, &vao );
	glGenBuffers(1,&buffer); 

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

	glBindVertexArray( vao );

	// Load shaders and use the resulting shader programs
	program = InitShader( "./shaders/shadedTM_vert.glsl", "./shaders/shadedTM_frag.glsl"  );
	glUseProgram( program );

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

	glClearColor( 1.0, 1.0, 0.0, 1.0 );

}

void CApp::quad( int a, int b, int c, int d )
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
	quad_Normal[Index] = normalize(vec3(points[Index][0],0.0 , points[Index][2]));
	tex_coords[Index] = vec2( 0.0, 0.0 );
	Index++;

	points[Index] = vertices[b];
	quad_Normal[Index] = normalize(vec3(points[Index][0],0.0 , points[Index][2]));
	tex_coords[Index] = vec2( 0.0, 1.0 );
	Index++;

	points[Index] = vertices[c];
	quad_Normal[Index] = normalize(vec3(points[Index][0],0.0 , points[Index][2]));
	tex_coords[Index] = vec2( 1.0, 1.0 );
	Index++;


	points[Index] = vertices[a];
	quad_Normal[Index] = normalize(vec3(points[Index][0],0.0 , points[Index][2]));
	tex_coords[Index] = vec2( 0.0, 0.0 );
	Index++;


	points[Index] = vertices[c];
	quad_Normal[Index] = normalize(vec3(points[Index][0],0.0 , points[Index][2]));
	tex_coords[Index] = vec2( 1.0, 1.0 );
	Index++;


	points[Index] = vertices[d];
	quad_Normal[Index] = normalize(vec3(points[Index][0],0.0 , points[Index][2]));
	tex_coords[Index] = vec2( 1.0, 0.0 );
	Index++;
}

void CApp::cyllMesh()
{
	quad( 1, 0, 3, 2 );
	quad( 2, 3, 7, 6 );
	quad( 4, 5, 6, 7 );
	quad( 5, 4, 0, 1 );
}