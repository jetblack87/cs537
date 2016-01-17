#include "Angel.h"

typedef vec3 color3;

const color3 WHITE(1.0, 1.0, 1.0);
const color3 BLACK(0.0, 0.0, 0.0);
const color3 RED  (1.0, 0.0, 0.0);
const color3 GREEN(0.0, 1.0, 0.0);
const color3 BLUE (0.0, 0.0, 1.0);


const int TotalNumPoints = 6 + 8 + 10;

//--------------------------------------------------------------------------

double
getPolygonAngle(int point, int vertexCount) {
  return M_PI / vertexCount + (M_PI / (vertexCount / 2) * point);
}

float
generateAngleColor(double angle) {
  return 1.0 / (2 * M_PI) * angle;
}
void
generatePolygonPoints(vec2 vertices[], color3 colors[], int startVertexIndex, int vertexCount, double scale, vec2 center, color3 color) {  
  int vertexIndex = startVertexIndex;

  for (int j = 0; j < vertexCount; j++) {
    double currentAngle = getPolygonAngle(j, vertexCount);
    vertices[vertexIndex] = vec2( sin (currentAngle),  cos(currentAngle)) * scale + center;
    colors[vertexIndex] = color * generateAngleColor(currentAngle);
    vertexIndex++;
  }
}

void
init( void )
{
  vec2 vertices[ TotalNumPoints ];
  color3 colors[ TotalNumPoints ] = { color3( 0.0, 0.0, 0.0 ) }; 

  generatePolygonPoints(vertices, colors, 0,     6,  .30, vec2(0.0, -0.35),  RED);
  generatePolygonPoints(vertices, colors, 6,     8,  .30, vec2(0.35, 0.35),  GREEN);
  generatePolygonPoints(vertices, colors, 6 + 8, 10, .30, vec2(-0.35, 0.35), BLUE);

  // Create a vertex array object
  GLuint vao[1];
  glGenVertexArrays( 1, vao );
  glBindVertexArray( vao[0] );

  // Create and initialize a buffer object
  GLuint buffer;
  glGenBuffers( 1, &buffer );
  glBindBuffer( GL_ARRAY_BUFFER, buffer );
  glBufferData( GL_ARRAY_BUFFER, sizeof(vertices) + sizeof(colors),
		NULL, GL_STATIC_DRAW );

  //load data separately
  glBufferSubData(GL_ARRAY_BUFFER, 0,
		  sizeof(vertices), vertices);
  glBufferSubData(GL_ARRAY_BUFFER, sizeof(vertices),
		  sizeof(colors), colors);

  // Load shaders and use the resulting shader program
  GLuint program = InitShader( "vshader21.glsl", "fshader21.glsl" );
  glUseProgram( program );

  // Initialize the vertex position attribute from the vertex shader
  GLuint loc = glGetAttribLocation(program, "vPosition");
  glEnableVertexAttribArray(loc);
  glVertexAttribPointer(loc, 2, GL_FLOAT, GL_FALSE, 0,
			BUFFER_OFFSET(0));
  GLuint loc2 = glGetAttribLocation(program, "vColor");
  glEnableVertexAttribArray(loc2);
  glVertexAttribPointer(loc2, 3, GL_FLOAT, GL_FALSE, 0,
			BUFFER_OFFSET(sizeof(vertices)));

  glClearColor( 0.0, 0.0, 0.0, 1.0 );
}

//----------------------------------------------------------------------------

void
display( void )
{
  glClear( GL_COLOR_BUFFER_BIT );     // clear the window
  glDrawArrays( GL_TRIANGLE_FAN, 0,     6);
  glDrawArrays( GL_TRIANGLE_FAN, 6,     8);
  glDrawArrays( GL_TRIANGLE_FAN, 6 + 8, 10);
  glFlush();
}

//----------------------------------------------------------------------------

void
keyboard( unsigned char key, int x, int y )
{
  switch ( key ) {
  case 033:
    exit( EXIT_SUCCESS );
    break;
  }
}

//----------------------------------------------------------------------------

int
main( int argc, char **argv )
{
  glutInit( &argc, argv );
  glutInitDisplayMode( GLUT_RGBA );
  glutInitWindowSize( 500, 500 );

  glutCreateWindow( "mwa29 - CS537 assignment 2" );
  glewExperimental=GL_TRUE;
  glewInit();
  init();

  glutDisplayFunc( display );
  glutKeyboardFunc( keyboard );

  glutMainLoop();
  return 0;
}
