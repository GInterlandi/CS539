#include "stdafx.h"
#include "Angel.h"
#include <assert.h>
#include "car.h"

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
#define SCALE_ANGLE     1.0

struct Node;

void m_glewInitAndVersion(void);
void reshape(int width, int height);
void specKey(int key, int x, int y);
void keyboard( unsigned char key, int x, int y );
void traverse( Node* node );
void init();

float spin =0.0, spin_step = 0.01; 

GLfloat d=0.5;
//
Angel::vec4 v = vec4(0.0, 1.0, 0.0, 0.0);
Angel::vec4 u = vec4(1.0, 0.0, 0.0, 0.0);
Angel::vec4 eye = vec4(0.0, 1.0, 2.0, 1.0);
Angel::vec4 n = Angel::normalize(vec4(0.0, 1.0, 2.0, 0.0));

point4 light_position;
//
//// Create buffer objects
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
GLuint model_view_loc;

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
//--------------------------------------------------------------------------
MatrixStack  mvstack;
mat4         model_view=mat4(1.0);
mat4         projmat=mat4(1.0); 
//Voids definition