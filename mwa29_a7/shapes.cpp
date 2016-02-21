
#include "Angel.h"

typedef Angel::vec4 point4;

// Define a convenient type for referencing patch control points, which
//   is used in the declaration of the vertices' array (used in "vertices.h")
typedef GLfloat     point3[3];

const char *TITLE = "mwa29 - CS537 assignment 7";

const int NumTeapotVertices = 16;

point3 vertices[NumTeapotVertices];

const int NumTeapotPatches = 32;

int indices[NumTeapotPatches][4][4] = {
    {
	{0, 1, 2, 3},
	{4, 5, 6, 7},
	{8, 9, 10, 11},
	{12, 13, 14, 15}
    }
};

const int NumTimesToSubdivide = 3;
const int PatchesPerSubdivision = 4;
const int NumQuadsPerPatch = 64;
// const int NumQuadsPerPatch = (int) pow( PatchesPerSubdivision, NumTimesToSubdivide );
const int NumTriangles = 32*64*2;
// const int NumTriangles = ( NumTeapotPatches * NumQuadsPerPatch * 2 /* triangles / quad */ );
const int NumVertices = 32*64*2*3;
//     ( NumTriangles * 3 /* vertices / triangle */ );

int     Index = 0;
point4  points[NumVertices];

GLuint  Projection;
GLuint  ModelView;

enum { X = 0, Y = 1, Z = 2 };

double t  = 0;
double dt = 0.01;

//----------------------------------------------------------------------------

void
divide_curve( point4 c[4], point4 r[4], point4 l[4] )
{
    // Subdivide a Bezier curve into two equaivalent Bezier curves:
    //   left (l) and right (r) sharing the midpoint of the middle
    //   control point
    point4  t, mid = ( c[1] + c[2] ) / 2;

    l[0] = c[0];
    l[1] = ( c[0] + c[1] ) / 2;
    l[2] = ( l[1] + mid ) / 2;

    r[3] = c[3];
    r[2] = ( c[2] + c[3] ) / 2;
    r[1] = ( mid + r[2] ) / 2;

    l[3] = r[0] = ( l[2] + r[1] ) / 2;
    
    for ( int i = 0; i < 4; ++i ) {
	l[i].w = 1.0;
	r[i].w = 1.0;
    }
}

//----------------------------------------------------------------------------

void
draw_patch( point4 p[4][4] )
{
    // Draw the quad (as two triangles) bounded by the corners of the
    //   Bezier patch.
    points[Index++] = p[0][0];
    points[Index++] = p[3][0];
    points[Index++] = p[3][3];
    points[Index++] = p[0][0];
    points[Index++] = p[3][3];
    points[Index++] = p[0][3];
}

//----------------------------------------------------------------------------

inline void
transpose( point4 a[4][4] )
{
    for ( int i = 0; i < 4; i++ ) {
        for ( int j = i; j < 4; j++ ) {
	    point4 t = a[i][j];
	    a[i][j] = a[j][i];
	    a[j][i] = t;
	}
    }
}

void
divide_patch( point4 p[4][4], int count )
{
    if ( count > 0 ) {
	point4 q[4][4], r[4][4], s[4][4], t[4][4];
	point4 a[4][4], b[4][4];

	// subdivide curves in u direction, transpose results, divide
	// in u direction again (equivalent to subdivision in v)
        for ( int k = 0; k < 4; ++k ) {
            divide_curve( p[k], a[k], b[k] );
	}

        transpose( a );
        transpose( b );

        for ( int k = 0; k < 4; ++k ) {
            divide_curve( a[k], q[k], r[k] );
            divide_curve( b[k], s[k], t[k] );
        }

	// recursive division of 4 resulting patches
        divide_patch( q, count - 1 );
        divide_patch( r, count - 1 );
        divide_patch( s, count - 1 );
        divide_patch( t, count - 1 );
    }
    else {
        draw_patch( p );
    }
}

//----------------------------------------------------------------------------

void
init( void )
{
    for ( int n = 0; n < NumTeapotPatches; n++ ) {
	point4  patch[4][4];

	// Initialize each patch's control point data
	for ( int i = 0; i < 4; ++i ) {
	    for ( int j = 0; j < 4; ++j ) {
		point3& v = vertices[indices[n][i][j]];
		patch[i][j] = point4( v[X], v[Y], v[Z], 1.0 );
	    }
	}

	// Subdivide the patch
        divide_patch( patch, NumTimesToSubdivide );
    }

    // Create a vertex array object
    GLuint vao;
    glGenVertexArraysAPPLE( 1, &vao );
    glBindVertexArrayAPPLE( vao );

    // Create and initialize a buffer object
    GLuint buffer;
    glGenBuffers( 1, &buffer );
    glBindBuffer( GL_ARRAY_BUFFER, buffer );
    glBufferData( GL_ARRAY_BUFFER, sizeof(points), points, GL_STATIC_DRAW );

    // Load shaders and use the resulting shader program
    GLuint program = InitShader( "vshader101.glsl", "fshader101.glsl" );
    glUseProgram( program );

    // set up vertex arrays
    GLuint vPosition = glGetAttribLocation( program, "vPosition" );
    glEnableVertexAttribArray( vPosition );
    glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0,
			   BUFFER_OFFSET(0) );

    Projection = glGetUniformLocation( program, "Projection" );
    ModelView = glGetUniformLocation( program, "ModelView" );

    glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );

    glClearColor( 1.0, 1.0, 1.0, 1.0 );
}

//----------------------------------------------------------------------------

void
display( void )
{
    glClear( GL_COLOR_BUFFER_BIT );
    mat4 model_view = mat4(1.0);
    model_view = model_view * RotateY(t);
    glUniformMatrix4fv(ModelView, 1, GL_TRUE, model_view);
    glDrawArrays( GL_TRIANGLES, 0, NumVertices );
    glutSwapBuffers();
}

//----------------------------------------------------------------------------

void
reshape( int width, int height )
{
    glViewport( 0, 0, width, height );

    GLfloat  left = -4.0, right = 4.0;
    GLfloat  bottom = -3.0, top = 5.0;
    GLfloat  zNear = -10.0, zFar = 10.0;

    GLfloat  aspect = GLfloat(width)/height;

    if ( aspect > 0 ) {
	left *= aspect;
	right *= aspect;
    }
    else {
	bottom /= aspect;
	top /= aspect;
    }

    mat4 projection = Ortho( left, right, bottom, top, zNear, zFar );
    glUniformMatrix4fv( Projection, 1, GL_TRUE, projection );
}

//----------------------------------------------------------------------------

void
keyboard( unsigned char key, int x, int y )
{
    switch ( key ) {
	case 'q': case 'Q': case 033 /* Escape key */:
	    exit( EXIT_SUCCESS );
	    break;
    }
}

void
myidle ( void )
{
  t += dt;
  glutPostRedisplay();
}

//----------------------------------------------------------------------------

void
read_patchfile(const char *file_path)
{
  FILE * file;

  int i = 0;
  file = fopen (file_path,"r");
  if (NULL != file) {
    char line[1000];
    while (fgets ( line, sizeof line, file ) != NULL) {
      double first;
      double second;
      double third;
      sscanf(line, "%lf %lf %lf\n", &first, &second, &third);
      vertices[i][X] = first;
      vertices[i][Y] = second;
      vertices[i][Z] = third;
      i++;
    }
    fclose (file);
  }
}

int
main( int argc, char *argv[] )
{
    glutInit( &argc, argv );
    glutInitDisplayMode( GLUT_RGBA | GLUT_DOUBLE );
    glutInitWindowSize( 512, 512 );
    glutCreateWindow( TITLE );

    glewExperimental=GL_TRUE;
    glewInit();

    if (argc > 1) {
      read_patchfile(argv[1]);
    } else {
      read_patchfile("models/patchPoints.txt");
    }
    init();

    glutIdleFunc( myidle );
    glutDisplayFunc( display );
    glutReshapeFunc( reshape );
    glutKeyboardFunc( keyboard );

    glutMainLoop();
    return 0;
}
