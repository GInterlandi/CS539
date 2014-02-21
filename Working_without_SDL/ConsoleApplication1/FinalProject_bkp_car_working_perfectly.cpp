#include "stdafx.h"
#include "Angel.h"
#include <assert.h>
#include <fstream> 
#include <string>
#include <vector>
#include <chrono>
#include <ctime>
#include <iostream>

using namespace std;

// car variables na dfunctions
bool key_s=0,key_w=0,key_a=0,key_d=0;
vec4 *car_vertices = (vec4*)malloc(3*200000*sizeof(vec4));
int car_vert_sz=0,car_norm_sz=0,car_face_sz=0;
vec3 *car_normals = (vec3*)malloc(3*200000*sizeof(vec3));
vec4 *car_mat= (vec4*)malloc(3*200*sizeof(vec4));
int car_mat_ind[200+1];
float car_shinnines[200];
mat4 car_pos(1.0);
mat4 car_att(1.0);
mat4 repos(1.0);
#define CAR_SPEED 4
#define CAR_TURN_SPEED 30.0
void read_obj(char *filename,vec4 *vertices,vec3 *normals,vec4 *lights,int *mat_ind,float* shinnines,int* vertices_size,int* normals_size,int* faces_size);
std::chrono::time_point<std::chrono::system_clock> t_rot,t_trans;

using namespace std;

typedef Angel::vec4 point4;
typedef Angel::vec4 color4;
#ifndef PI
#  define PI  3.14159265358979323846
#endif

#define X               0
#define Y               1
#define Z               2
#define SCALE_VECTOR    0.2
#define SCALE_ANGLE     1.0 // Defined variables for camera moving with arrow keys and XYZ

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
void init(); // Functions headers

Angel::vec4 v = vec4(0.0, 1.0, 0.0, 0.0);
Angel::vec4 u = vec4(1.0, 0.0, 0.0, 0.0);
Angel::vec4 eye = vec4(0.0, 1.0, 2.0, 1.0);
Angel::vec4 n = Angel::normalize(vec4(0.0, 1.0, 2.0, 0.0)); // Camera matrices

point4 light_position;
vec4 light_ambient( 0.2, 0.2, 0.2, 1.0 );
vec4 light_diffuse( 1.0, 1.0, 1.0, 1.0 );
vec4 light_specular( 1.0, 1.0, 1.0, 1.0 ); // Light vecs and position

GLuint buffers;
// Create a vertex array object
GLuint vao;
//uniform variables locations
GLuint program;

GLuint vPosition;
GLuint vNormal;
GLuint vColor;
GLuint vShift;
GLuint color_loc;
GLuint proj_loc;
GLuint model_view_loc; // Shader variables

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
//----------------------------
//--------------------------------------------------------------------------
MatrixStack  mvstack;
mat4         model_view=mat4(1.0);
mat4         projmat=mat4(1.0); 

int	main( int argc, char **argv )
{
	glutInit( &argc, argv );
	glutInitDisplayMode( GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH );
	glutInitWindowSize( 512, 512 );
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
	float xmax=-99999,xmin=99999,ymax=-99999,ymin=99999,zmin=99999;

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
				sscanf(line.c_str(),"f %i/%*i/%i %i/%*i/%i %i/%*i/%i",&temp1[0],&temp1[1],&temp1[2],&temp1[3],&temp1[4],&temp1[5]);
				vertices[3*num_faces-3] = temp_vertices[temp1[0]-1];
				vertices[3*num_faces-2] = temp_vertices[temp1[2]-1];
				vertices[3*num_faces-1] = temp_vertices[temp1[4]-1];
				normals[3*num_faces-3] = temp_normals[temp1[1]-1];
				normals[3*num_faces-2] = temp_normals[temp1[3]-1];
				normals[3*num_faces-1] = temp_normals[temp1[5]-1];
			}
		}
		repos = Translate(-(xmax-xmin)/2000,-(ymax-ymin)/2000,-zmin/1000);
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

void init(){
	read_obj("Peugeot_207.obj",car_vertices,car_normals,car_mat,car_mat_ind,car_shinnines,&car_vert_sz,&car_norm_sz,&car_face_sz);
	glGenVertexArrays( 1, &vao );
	glBindVertexArray( vao );

	glGenBuffers(1,&buffers); 

	glBindBuffer( GL_ARRAY_BUFFER, buffers );
	glBufferData( GL_ARRAY_BUFFER, 3*car_face_sz*sizeof(vec4)+3* car_face_sz*sizeof(vec3),  NULL, GL_STATIC_DRAW );
	glBufferSubData( GL_ARRAY_BUFFER, 0, 3*car_face_sz*sizeof(vec4), car_vertices);
	glBufferSubData( GL_ARRAY_BUFFER, 3*car_face_sz*sizeof(vec4), 3*car_face_sz*sizeof(vec3), car_normals );

	// Load shaders and use the resulting shader programs
	program = InitShader( "vshader56.glsl", "fshader56.glsl" );//( "vshaderUnifColor.glsl", "fshaderStock.glsl" );
	glUseProgram( program );

	glBindVertexArray( vao );

	glBindBuffer( GL_ARRAY_BUFFER, buffers );
	vPosition = glGetAttribLocation( program, "vPosition" );
	glEnableVertexAttribArray( vPosition );
	glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0) );

	vNormal = glGetAttribLocation( program, "vNormal" ); 
	glEnableVertexAttribArray( vNormal );
	glVertexAttribPointer( vNormal, 3, GL_FLOAT, GL_FALSE, 0,
		BUFFER_OFFSET(3*car_face_sz*sizeof(vec4)) );

	model_view_loc = glGetUniformLocation( program, "ModelView" );
	proj_loc = glGetUniformLocation( program, "Projection" );  
	projmat = Perspective(90,1.0,0.2, 15.0); 
	glUniformMatrix4fv(proj_loc, 1, GL_TRUE, projmat);

	glBindVertexArray(0);

	// Initialize tree
	//	initNodes();

	glClearColor( 1.0, 1.0, 1.0, 0.0 ); 
	glClearDepth( 1.0 ); 
	glEnable( GL_DEPTH_TEST );
	glDepthFunc(GL_LEQUAL);
	glPolygonMode(GL_FRONT, GL_FILL);

}

void Car()
{
	mvstack.push( model_view );
	mat4 m = car_pos*repos*car_att*Scale(0.001,0.001,0.001);

	glUseProgram( program );
	glBindBuffer( GL_ARRAY_BUFFER, buffers );

	model_view_loc = glGetUniformLocation( program, "ModelView" );
	glUniformMatrix4fv( model_view_loc, 1, GL_TRUE, model_view*m );

	glBindVertexArray( vao );
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	//color_loc = glGetUniformLocation(program, "vColor");
	for(int i=0;i<car_mat_ind[0];i++)
	{
		//glUniform4fv(color_loc, 1, car_mat[3*i+2]);
		glUniform4fv( glGetUniformLocation(program, "AmbientProduct"), 1, light_ambient*car_mat[3*i]);
		glUniform4fv( glGetUniformLocation(program, "DiffuseProduct"), 1, light_diffuse*car_mat[3*i+1] );
		glUniform4fv( glGetUniformLocation(program, "SpecularProduct"), 1, light_specular*car_mat[3*i+2]  );
		glUniform1f( glGetUniformLocation(program, "Shininess"),car_shinnines[i] );
		if(i==0)
		{
			glDrawArrays( GL_TRIANGLES, 0, 3*car_mat_ind[i+1]-1 );
		}else
		{
			glDrawArrays( GL_TRIANGLES, 3*car_mat_ind[i], 3*car_mat_ind[i+1]-3*car_mat_ind[i] );
		}

	}
	model_view = mvstack.pop();
}

void display( void )
{
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	glUseProgram( program );
	glBindBuffer( GL_ARRAY_BUFFER, buffers );

	light_position = vec4( 0.0, 0.0, 1.5, 1.0 );
	//light_position = vec4(-1*sin(spin), 4, -1*cos(spin), 1.0);
	glUniform4fv( glGetUniformLocation(program, "LightPosition"), 1, light_position );

	model_view = LookAt(eye, eye-n, v); 



	Car();
	//traverse( &nodes[baseCyll]);

	glutSwapBuffers();

}

void idle(void)
{
	std::chrono::time_point<std::chrono::system_clock>  end;
	end = std::chrono::system_clock::now();
	std::chrono::duration<double> elapsed_seconds;

	if(key_w) 
	{
		elapsed_seconds = end-t_trans;
		car_pos = car_pos*Translate(car_att*vec4(0.0,0.0,-CAR_SPEED*elapsed_seconds.count(),1.0));
	}
	if(key_s) 
	{
		elapsed_seconds = end-t_trans;
		car_pos = car_pos*Translate(car_att*vec4(0.0,0.0,CAR_SPEED*elapsed_seconds.count(),1.0));
	}
	if(key_a) 
	{
		elapsed_seconds = end-t_rot;
		car_att = car_att*RotateY(CAR_TURN_SPEED*elapsed_seconds.count());
	}
	if(key_d) 
	{	
		elapsed_seconds = end-t_rot;
		car_att = car_att*RotateY(-CAR_TURN_SPEED*elapsed_seconds.count());
	}
	t_trans = end;
	t_rot = end;
	glutPostRedisplay();
}


void KeyboardUp	( unsigned char key, int x, int y) 	
{
	switch(key)
	{
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
	case 'Y': // pitch up
	case 'y': // pitch down
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


void reshape( int width, int height )
	//the same objects are shown (possibly scaled) w/o shape distortion 
	//original viewport is a square
{
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









*/