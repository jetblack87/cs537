#include "Angel.h"
#include "mat.h"
#include "vec.h"

typedef vec4 point4;
typedef vec4 color4;

//--------------------------------------------------------------------------
//----   CONSTANTS    ------------------------------------------------------
//--------------------------------------------------------------------------

const char *TITLE = "mwa29 - CS537 assignment 4";

const int NUM_POINTS   = 36;
const int NUM_VERTICES = 8;

const int Xaxis = 0;
const int Yaxis = 1;
const int Zaxis = 2;

const char KEY_XUP       = 'q';
const char KEY_XDOWN     = 'a';
const char KEY_YUP       = 'w';
const char KEY_YDOWN     = 's';
const char KEY_ZUP       = 'e';
const char KEY_ZDOWN     = 'd';
const char KEY_DELTAUP   = 'r';
const char KEY_DELTADOWN = 'f';
const char KEY_RESET     = 't';

const int TRANSFORM_SCALE     = 0;
const int TRANSFORM_ROTATE    = 1;
const int TRANSFORM_TRANSLATE = 2;

const double DELTA_DELTA = 0.01;

const double DEFAULT_DELTA = 0.1;

//--------------------------------------------------------------------------
//----   GLOBALS      ------------------------------------------------------
//--------------------------------------------------------------------------

int mainWindow;
int menu;
int w = 600, h = 600;

double scale_delta     = DEFAULT_DELTA;
double rotate_delta    = DEFAULT_DELTA;
double translate_delta = DEFAULT_DELTA;

vec3 scale_theta(1.0, 1.0, 1.0);
vec3 rotate_theta(0.0, 0.0, 0.0);
vec3 translate_theta(0.0, 0.0, 0.0);

GLint matrix_loc;
int current_transform = TRANSFORM_TRANSLATE;

//--------------------------------------------------------------------------

// Vertices of a unit cube centered at origin 
//  sides aligned with axes 
const point4 vertices[NUM_VERTICES] = { 
  point4( -0.5, -0.5,  0.5, 1.0 ), 
  point4( -0.5,  0.5,  0.5, 1.0 ), 
  point4(  0.5,  0.5,  0.5, 1.0 ), 
  point4(  0.5, -0.5,  0.5, 1.0 ), 
  point4( -0.5, -0.5, -0.5, 1.0 ), 
  point4( -0.5,  0.5, -0.5, 1.0 ), 
  point4(  0.5,  0.5, -0.5, 1.0 ), 
  point4(  0.5, -0.5, -0.5, 1.0 ) 
}; 

// RGBA colors 
const color4 vertex_colors[NUM_VERTICES] = { 
  color4( 0.0, 0.0, 0.0, 1.0 ),  // black 
  color4( 1.0, 0.0, 0.0, 1.0 ),  // red 
  color4( 1.0, 1.0, 0.0, 1.0 ),  // yellow 
  color4( 0.0, 1.0, 0.0, 1.0 ),  // green 
  color4( 0.0, 0.0, 1.0, 1.0 ),  // blue 
  color4( 1.0, 0.0, 1.0, 1.0 ),  // magenta 
  color4( 1.0, 1.0, 1.0, 1.0 ),  // white 
  color4( 0.0, 1.0, 1.0, 1.0 )   // cyan 
};

point4 points[NUM_POINTS];
color4 colors[NUM_POINTS];

// quad generates two triangles for each face and assigns colors 
//    to the vertices 
int Index = 0; 
void quad( int a, int b, int c, int d ) 
{ 
    colors[Index] = vertex_colors[a]; points[Index] = vertices[a]; Index++; 
    colors[Index] = vertex_colors[b]; points[Index] = vertices[b]; Index++; 
    colors[Index] = vertex_colors[c]; points[Index] = vertices[c]; Index++; 
    colors[Index] = vertex_colors[a]; points[Index] = vertices[a]; Index++; 
    colors[Index] = vertex_colors[c]; points[Index] = vertices[c]; Index++; 
    colors[Index] = vertex_colors[d]; points[Index] = vertices[d]; Index++; 
} 

// generate 12 triangles: 36 vertices and 36 colors 
void 
colorcube() 
{ 
    quad( 1, 0, 3, 2 ); 
    quad( 2, 3, 7, 6 ); 
    quad( 3, 0, 4, 7 ); 
    quad( 6, 5, 1, 2 ); 
    quad( 4, 5, 6, 7 ); 
    quad( 5, 4, 0, 1 ); 
} 

void
init( void )
{
  colorcube();

  // Create a vertex array object
  GLuint vao[1];
  glGenVertexArrays( 1, vao );
  glBindVertexArray( vao[0] );

  // Create and initialize a buffer object
  GLuint buffer;
  glGenBuffers( 1, &buffer );
  glBindBuffer( GL_ARRAY_BUFFER, buffer );
  glBufferData( GL_ARRAY_BUFFER, sizeof(points) + sizeof(colors),
		NULL, GL_STATIC_DRAW );

  //load data separately
  glBufferSubData(GL_ARRAY_BUFFER, 0,
		  sizeof(points), points);
  glBufferSubData(GL_ARRAY_BUFFER, sizeof(points),
		  sizeof(colors), colors);

  // Load shaders and use the resulting shader program
  GLuint program = InitShader( "vshdrcube.glsl", "fshdrcube.glsl" );
  glUseProgram( program );

  // Initialize the vertex position attribute from the vertex shader
  GLuint loc = glGetAttribLocation(program, "vPosition");
  glEnableVertexAttribArray(loc);
  glVertexAttribPointer(loc, 4, GL_FLOAT, GL_FALSE, 0,
			BUFFER_OFFSET(0));
  GLuint loc2 = glGetAttribLocation(program, "vColor");
  glEnableVertexAttribArray(loc2);
  glVertexAttribPointer(loc2, 4, GL_FLOAT, GL_FALSE, 0,
			BUFFER_OFFSET(sizeof(points)));

  matrix_loc = glGetUniformLocation( program, "matrix" );

  glClearColor( 1.0, 1.0, 1.0, 1.0 );
}

void
display( void )
{
  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

  mat4 m(1.0,0.0,0.0,0.0,
	 0.0,1.0,0.0,0.0,
	 0.0,0.0,1.0,0.0,
	 0.0,0.0,0.0,1.0);

  m = m*Translate(translate_theta[Xaxis],
		  translate_theta[Yaxis],
		  translate_theta[Zaxis]);

  m = m*RotateX(rotate_theta[Xaxis]);
  m = m*RotateY(rotate_theta[Yaxis]);
  m = m*RotateZ(rotate_theta[Zaxis]);

  m = m*Scale(scale_theta[Xaxis],
	      scale_theta[Yaxis],
	      scale_theta[Zaxis]);

  glUniformMatrix4fv(matrix_loc, 1, GL_TRUE, m); 
  glDrawArrays( GL_TRIANGLES, 0, NUM_POINTS );
  
  glFlush();
  glutSwapBuffers();
}

void
update_theta(int axis, int pos) {
  switch (current_transform) {
  case TRANSFORM_SCALE:     scale_theta[axis]     += pos * scale_delta; break;
  case TRANSFORM_ROTATE:    rotate_theta[axis]    += pos * rotate_delta; break;
  case TRANSFORM_TRANSLATE: translate_theta[axis] += pos * translate_delta; break;
  }
}

void
reset_theta ( void )
{
  scale_theta     = vec3(1.0, 1.0, 1.0);
  rotate_theta    = vec3(0.0, 0.0, 0.0);
  translate_theta = vec3(0.0, 0.0, 0.0);
  scale_delta     = DEFAULT_DELTA;
  rotate_delta    = DEFAULT_DELTA;
  translate_delta = DEFAULT_DELTA;
}

void
update_delta ( int pos )
{
  switch (current_transform) {
  case TRANSFORM_SCALE:     scale_delta     += pos * DELTA_DELTA; break;
  case TRANSFORM_ROTATE:    rotate_delta    += pos * DELTA_DELTA; break;
  case TRANSFORM_TRANSLATE: translate_delta += pos * DELTA_DELTA; break;
  }
}

void
keyboard( unsigned char key, int x, int y )
{
  switch (key) {
  case KEY_XUP:       update_theta(Xaxis,  1); break;
  case KEY_XDOWN:     update_theta(Xaxis, -1); break;
  case KEY_YUP:       update_theta(Yaxis,  1); break;
  case KEY_YDOWN:     update_theta(Yaxis, -1); break;
  case KEY_ZUP:       update_theta(Zaxis,  1); break;
  case KEY_ZDOWN:     update_theta(Zaxis, -1); break;
  case KEY_DELTAUP:   update_delta(1);   break;
  case KEY_DELTADOWN: update_delta(-1);  break;
  case KEY_RESET:     reset_theta(); break;
  }
  glutPostWindowRedisplay(mainWindow);
}

void
processMenuEvents(int menuChoice)
{
  current_transform = menuChoice;
}

void
setupMenu ( void )
{
  glutCreateMenu(processMenuEvents);
  glutAddMenuEntry("Scale",     TRANSFORM_SCALE);
  glutAddMenuEntry("Rotate",    TRANSFORM_ROTATE);
  glutAddMenuEntry("Translate", TRANSFORM_TRANSLATE);
  glutAttachMenu(GLUT_RIGHT_BUTTON);
}

void
printHelp ( void ) {
  printf("%s\n", TITLE);
  printf("Keyboard options:\n");
  printf("q - increase x\n");
  printf("a - decrease x\n");
  printf("w - increase y\n");
  printf("s - decrease y\n");
  printf("e - increase z\n");
  printf("d - decrease z\n");
  printf("r - increase delta of currently selected transform\n");
  printf("f - decrease delta of currently selected transform\n");
  printf("t - reset all transformations and deltas\n");
}

int
main( int argc, char **argv )
{
  glutInit( &argc, argv );
  glutInitDisplayMode( GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH );
  glutInitWindowPosition(100,100);
  glutInitWindowSize( w, h );

  mainWindow = glutCreateWindow( TITLE );

  glewExperimental=GL_TRUE;
  glewInit();

  init();
  setupMenu();
  glutDisplayFunc ( display  );
  glutKeyboardFunc( keyboard );

  printHelp();

  glEnable(GL_DEPTH_TEST); 
  glutMainLoop();
  return 0;
}
