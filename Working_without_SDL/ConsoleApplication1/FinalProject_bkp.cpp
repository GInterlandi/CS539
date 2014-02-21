#include "stdafx.h"
#include "Angel.h"
#include <assert.h>
#include <iostream>   
#include <fstream> 
#include <string>
#include <vector>

using namespace std;

typedef Angel::vec4 point4;
typedef Angel::vec4 color4;

#ifndef PI
#  define PI  3.14159265358979323846
#endif

//void m_glewInitAndVersion(void);
//void reshape(int width, int height);
//void specKey(int key, int x, int y);
//void keyboard( unsigned char key, int x, int y );
//
//
//Angel::vec4 v = vec4(0.0, 1.0, 0.0, 0.0);
//Angel::vec4 u = vec4(1.0, 0.0, 0.0, 0.0);
//Angel::vec4 eye = vec4(0.0, 1.0, 2.0, 1.0);
//Angel::vec4 n = Angel::normalize(vec4(0.0, 1.0, 2.0, 0.0));
//
//// Create buffer objects
//GLuint buffers[5];
//// Create a vertex array object
//GLuint vao;
////uniform variables locations
//GLuint program[2];
//
//GLuint vPosition;
//GLuint vColor;
//GLuint vShift;
//GLuint color_loc;
//GLuint proj_loc;
//GLuint model_view_loc;
////------------------------------------
// Draw variables
	vec4 *car_vertices = new vec4();
	vec3 *car_normals = new vec3();
	vec3 *car_mat= new vec3();
	int * car_mat_ind = new int();
	float * car_shinnines = new float();

//-----------------------------------------------
//// Necessary classes
//class MatrixStack {
//	int    _index;
//	int    _size;
//	mat4*  _matrices;
//
//public:
//	MatrixStack( int numMatrices = 32 ):_index(0), _size(numMatrices)
//	{ _matrices = new mat4[numMatrices]; }
//
//	~MatrixStack()
//	{ delete[]_matrices; }
//
//	void push( const mat4& m ) {
//		assert( _index + 1 < _size );
//		_matrices[_index++] = m;
//	}
//
//	mat4& pop( void ) {
//		assert( _index - 1 >= 0 );
//		_index--;
//		return _matrices[_index];
//	}
//};
//
//struct Node {
//	mat4  transform;
//	void  (*render)( void );
//	Node* sibling;
//	Node* child;
//
//	Node() :
//		render(NULL), sibling(NULL), child(NULL) {}
//
//	Node( mat4& m, void (*render)( void ), Node* sibling, Node* child ) :
//		transform(m), render(render), sibling(sibling), child(child) {}
//};
//--------------------------------------------------------------------------
//Voids definition

void read_obj(char *filename,vec4 *vertices,vec3 *normals,vec3 *lights,int *mat_ind,float* shinnines)
{
	ifstream file,file1;
	string line,mtlpath;
	vector<string> mat_names;
	int pos=string::npos,temp1[6];
	float temp[3]={0,0,0};
	int num_vertices=0,num_normals=0,num_faces=0,num_mat=0,cont_mat=0;
	float **materials = NULL;
	vec4 *temp_vertices = new vec4();
	vec3 *temp_normals = new vec3();
	mat_ind = (int *)malloc(sizeof(int));
	mat_ind[0]=0;

	file.open(filename);
	if(file.is_open())
	{
		//cout<<"opened geometry file\n";
		getline (file,line);
		getline (file,line);
		getline (file,mtlpath);
		pos = mtlpath.find("/");
		mtlpath = mtlpath.substr(pos+1,mtlpath.size()-pos);
		file1.open(mtlpath.c_str());
		if(file1.is_open())
		{
			while ( getline (file1,line))
			{
				if(line.compare(0,6,"newmtl")==0) // if equal return 0, thats why "!"
				{
					cont_mat++;
					mat_names = (string*)realloc(mat_names,cont_mat*sizeof(string));
					mat_names[cont_mat-1] = line.substr(7,line.size()-7);
					//cout<<line.substr(7,line.size()-7)<<"\n";
					materials = (float**)realloc(materials,cont_mat*sizeof(float*));
					materials[cont_mat-1] = (float*)malloc(10*sizeof(float));

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
			}
		}else
		{
			cout << "Unable to open mtl.";
			return;
		}
		while ( getline (file,line))
		{
			if(line.compare(0,1,"v")==0) // if equal return 0, thats why "!"
			{
				num_vertices++;
				temp_vertices = (vec4 *)realloc(temp_vertices,num_vertices*sizeof(vec4));
				sscanf(line.c_str(),"v %f %f %f",&temp[0],&temp[1],&temp[2]);
				temp_vertices[num_vertices-1] = vec4(temp[0],temp[1],temp[2],1);
			}
			if(line.compare(0,2,"vn")==0) // if equal return 0, thats why "!"
			{
				num_normals++;
				temp_normals = (vec3 *)realloc(temp_normals,num_normals*sizeof(vec3));
				sscanf(line.c_str(),"v %f %f %f",&temp[0],&temp[1],&temp[2]);
				temp_normals[num_normals-1] = vec3(temp[0],temp[1],temp[2]);
			}
			if(line.compare(0,6,"usemtl")==0) // if equal return 0, thats why "!"
			{
				num_mat++;
				mat_ind = (int *)realloc(mat_ind,(num_mat+1)*sizeof(int));
				mat_ind[num_mat]=num_faces;
				for(int i=0;i<cont_mat;i++)
				{
					if(line.compare(7,line.size()-7,mat_names[i]) == 0)
					{
						lights = (vec3*)realloc(lights,3*num_mat*sizeof(vec3));
						lights[num_mat-3] = vec3(materials[i][0],materials[i][1],materials[i][2]);
						lights[num_mat-2] = vec3(materials[i][3],materials[i][4],materials[i][5]);
						lights[num_mat-1] = vec3(materials[i][6],materials[i][7],materials[i][8]);
						shinnines = (float*)realloc(shinnines,num_faces*sizeof(float));
						shinnines[num_mat] = materials[i][9];
					}
				}
			}
			if(line.compare(0,1,"f")==0) // if equal return 0, thats why "!"
			{
				num_faces++;
				normals = (vec3 *)realloc(normals,3*num_faces*sizeof(vec3));
				vertices = (vec4 *)realloc(vertices,3*num_faces*sizeof(vec4));
				sscanf(line.c_str(),"f %f %*f %f %f %*f %f %f %*f %f",&temp[0],&temp[1],&temp[2]);
				vertices[num_faces-3] = temp_vertices[temp1[0]-1];
				vertices[num_faces-2] = temp_vertices[temp1[2]-1];
				vertices[num_faces-1] = temp_vertices[temp1[4]-1];
				normals[num_faces-3] = temp_normals[temp1[1]-1];
				normals[num_faces-2] = temp_normals[temp1[3]-1];
				normals[num_faces-1] = temp_normals[temp1[5]-1];
			}
		}
		mat_ind[0]=num_mat;
	}else
	{
		cout<< "Unable to open file";
	}

	delete[] temp_vertices;
	delete[] temp_normals;

	for( int i=0;i<cont_mat;i++)
	{
		delete [] materials[i];
	}
	delete [] materials;

	return;
}
//void traverse( Node* node )
//{
//	if ( node == NULL ) { return; }

//	mvstack.push( model_view );

//	model_view *= node->transform;
//	node->render();

//	if ( node->child ) { traverse( node->child ); }

//	model_view = mvstack.pop();

//	if ( node->sibling ) { traverse( node->sibling ); }
//}

//void display( void )
//{
//	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );


//	projmat = Perspective(90,1.0,0.2, 15.0); 

//	model_view = LookAt(eye, eye-n, v); 


//	traverse( &nodes[baseCyll]);

//	glutSwapBuffers();

//}

//void idle(void)
//{
//	glutPostRedisplay();
//}

//void keyboard( unsigned char key, int x, int y )
//{


//	GLfloat xt, yt, zt;
//	GLfloat cosine, sine;

//	// positive or negative rotation depending on upper or lower case letter
//	if(key > 96)  
//	{
//		cosine = cos(SCALE_ANGLE * PI/-180.0);
//		sine = sin(SCALE_ANGLE * PI/-180.0);
//	}
//	else
//	{
//		cosine = cos(SCALE_ANGLE * PI/180.0);
//		sine = sin(SCALE_ANGLE * PI/180.0);
//	}

//	switch(key)
//	{

//	case 'Z': // roll counterclockwise in the xy plane
//	case 'z': // roll clockwise in the xy plane
//		xt = u[X];
//		yt = u[Y];
//		zt = u[Z];
//		u[X] = xt*cosine - v[X]*sine;
//		u[Y] = yt*cosine - v[Y]*sine;
//		u[Z] = zt*cosine - v[Z]*sine;
//		v[X] = xt*sine + v[X]*cosine;
//		v[Y] = yt*sine + v[Y]*cosine;
//		v[Z] = zt*sine + v[Z]*cosine;
//		break;
//	case 'X': // pitch up
//	case 'x': // pitch down
//		xt = u[X];
//		yt = u[Y];
//		zt = u[Z];
//		u[X] = xt*cosine + n[X]*sine;
//		u[Y] = yt*cosine + n[Y]*sine;
//		u[Z] = zt*cosine + n[Z]*sine;
//		n[X] = -xt*sine + n[X]*cosine;
//		n[Y] = -yt*sine + n[Y]*cosine;
//		n[Z] = -zt*sine + n[Z]*cosine;
//		break;
//	case 'Y': // pitch up
//	case 'y': // pitch down
//		xt = v[X];
//		yt = v[Y];
//		zt = v[Z];
//		v[X] = xt*cosine - n[X]*sine;
//		v[Y] = yt*cosine - n[Y]*sine;
//		v[Z] = zt*cosine - n[Z]*sine;
//		n[X] = xt*sine + n[X]*cosine;
//		n[Y] = yt*sine + n[Y]*cosine;
//		n[Z] = zt*sine + n[Z]*cosine;
//		break;
//	case 'R': // pitch up
//	case 'r': // pitch down
//		v = vec4(0.0, 1.0, 0.0, 0.0);
//		u = vec4(1.0, 0.0, 0.0, 0.0);
//		eye = vec4(0.0, 1.0, 2.0, 1.0);
//		n = Angel::normalize(vec4(0.0, 1.0, 2.0, 0.0));
//		break;
//	case 033:  // Escape key
//	case 'q': case 'Q':
//		exit( EXIT_SUCCESS );
//	}

//	glutPostRedisplay();
//}

//void specKey(int key, int x, int y)
//{ 
//	switch( key ) {
//	case GLUT_KEY_UP: // MOVE FORWARD
//		eye[0] -= SCALE_VECTOR * n[0];
//		eye[1] -= SCALE_VECTOR * n[1];
//		eye[2] -= SCALE_VECTOR * n[2];
//		break;
//	case GLUT_KEY_DOWN: // MOVE BACKWARD
//		eye[0] += SCALE_VECTOR * n[0];
//		eye[1] += SCALE_VECTOR * n[1];
//		eye[2] += SCALE_VECTOR * n[2];
//		break;
//	case GLUT_KEY_LEFT: // MOVE FORWARD
//		eye[0] -= SCALE_VECTOR * u[0];
//		eye[1] -= SCALE_VECTOR * u[1];
//		eye[2] -= SCALE_VECTOR * u[2];
//		break;
//	case GLUT_KEY_RIGHT: // MOVE BACKWARD
//		eye[0] += SCALE_VECTOR * u[0];
//		eye[1] += SCALE_VECTOR * u[1];
//		eye[2] += SCALE_VECTOR * u[2];
//		break;
//	default:
//		break;
//	}
//	glutPostRedisplay();
//}


//void reshape( int width, int height )
//	//the same objects are shown (possibly scaled) w/o shape distortion 
//	//original viewport is a square
//{

//	glViewport( 0, 0, (GLsizei) width, (GLsizei) height );

//}

//void initNodes( void )
//{
//	mat4  m;

//	m = Scale(0.1,0.1,0.1) ;
//	nodes[cen_tt] = Node( m, Cen_tt, NULL, &nodes[treetop0] );//&nodes[treetop0]

//	m = Translate(0.0, 0.0, 1.0)*Scale(2.0,2.0,2.0) ;
//	nodes[treetop0] = Node( m, Treetop0, &nodes[walls1], NULL );
//}

//void init(){

//	buildCylinder( );
//	buildCone();
//	buildRoof();
//	colorcube();

//	glGenBuffers(5,buffers); 
//	glGenVertexArrays( 1, &vao );


//	glBindBuffer( GL_ARRAY_BUFFER, buffers[Cone] );
//	glBufferData( GL_ARRAY_BUFFER, sizeof(point4)*ConeVertices,  conedata, GL_STATIC_DRAW );

//	glBindBuffer( GL_ARRAY_BUFFER, buffers[Roof] );
//	glBufferData( GL_ARRAY_BUFFER, sizeof(roofdata),  roofdata, GL_STATIC_DRAW );

//	glBindBuffer( GL_ARRAY_BUFFER, buffers[Cyllinder] );
//	glBufferData( GL_ARRAY_BUFFER, sizeof(cylinderData),  cylinderData, GL_STATIC_DRAW );

//	glBindBuffer( GL_ARRAY_BUFFER, buffers[Cube] );
//	glBufferData( GL_ARRAY_BUFFER, sizeof(points),  points, GL_STATIC_DRAW );

//	for(int i=0;i<8;i++)
//	{
//		for(int j =0;j<8;j++)
//		{
//			shift[i*8+j] = vec2(5*i,5*j);
//		}
//	}

//	glBindBuffer( GL_ARRAY_BUFFER, buffers[Shift] );
//	glBufferData( GL_ARRAY_BUFFER, sizeof(shift),  shift, GL_STATIC_DRAW );

//	// Load shaders and use the resulting shader programs
//	program[baseCyll] = InitShader( "vshaderPerVertColor.glsl", "fshaderStock.glsl" );
//	program[TranslrotRyStretcedCyll] = InitShader( "vshaderUnifColor.glsl", "fshaderStock.glsl" );


//	glBindVertexArray( vao );

//	glBindBuffer( GL_ARRAY_BUFFER, buffers[0] );
//	vPosition = glGetAttribLocation( program[TranslrotRyStretcedCyll], "vPosition" );
//	glEnableVertexAttribArray( vPosition );
//	glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0) );

//	glBindVertexArray(0);

//	// Initialize tree
//	initNodes();

//	glClearColor( 0.0, 0.5, 0.0, 0.0 ); 
//	glClearDepth( 1.0 ); 
//	glEnable( GL_DEPTH_TEST );
//	glDepthFunc(GL_LEQUAL);
//	glPolygonMode(GL_FRONT, GL_FILL);

//}

//// Necessary instanciation
//MatrixStack  mvstack;
//mat4         model_view=mat4(1.0);
//mat4         projmat=mat4(1.0); 

int	main( int argc, char **argv )
{
	//glutInit( &argc, argv );
	//glutInitDisplayMode( GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH );
	//glutInitWindowSize( 512, 512 );
	//glutCreateWindow( "Scene Graph and a Flying Camera Stub" );

	//glutDisplayFunc(display); 
	//glutReshapeFunc(reshape);
	//glutKeyboardFunc( keyboard );
	//glutSpecialFunc( specKey );
	//glutIdleFunc(idle);

	//glewInit();

	//init();

	//glutMainLoop();

	read_obj("Peugeot_207.obj",car_vertices,car_normals,car_mat,car_mat_ind,car_shinnines);
	cout<< "verything ok!";
	return 0;
}