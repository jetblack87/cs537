#include "Angel.h"

typedef vec3 color3;

const color3 WHITE(1.0, 1.0, 1.0);
const color3 BLACK(0.0, 0.0, 0.0);
const color3 RED  (1.0, 0.0, 0.0);
const color3 GREEN(0.0, 1.0, 0.0);
const color3 BLUE (0.0, 0.0, 1.0);


const int CircleNumPoints   = 1080;
const int TriangleNumPoints = 3;
const int EllipseNumPoints  = 1080;
const int SquareNum         = 6;
const int SquareNumPoints   = 4 * SquareNum;
const int TotalNumPoints = CircleNumPoints + TriangleNumPoints + EllipseNumPoints + SquareNumPoints;

//--------------------------------------------------------------------------


vec2 getEllipseVertex(vec2 center, double scale, double verticleScale, double angle) {
  vec2 vertex (sin (angle), cos(angle));
  vertex   += center;
  vertex   *= scale;
  vertex.y *= verticleScale;
  return vertex;
}

float
generateAngleColor(double angle) {
  return 1.0 / (2 * M_PI) * angle;
}

void
generateEllipsePoints(vec2 vertices[], color3 colors[], int startVertexIndex, int numPoints,
		      vec2 center, double scale, double verticleScale) {

  double angleIncrement = (2 * M_PI) / numPoints;
  double currentAngle = M_PI / 2;
  vec2 startVertex = getEllipseVertex(center, scale, verticleScale, currentAngle);
  vertices[startVertexIndex] = startVertex;

  if (1.0 != verticleScale) {
    colors[startVertexIndex] = color3(1.0, 0.0, 0.0);
  }

  for(int i = startVertexIndex + 1; i < startVertexIndex + numPoints; i++) {
    vertices[i] = getEllipseVertex(center, scale, verticleScale, currentAngle);

    if (1.0 == verticleScale) {
      colors[i] = color3(generateAngleColor(currentAngle), 0.0, 0.0);
    } else {
      colors[i] = color3(1.0, 0.0, 0.0);
    }

    currentAngle += angleIncrement;
  }
}

double
getTriangleAngle(int point) {
  return 2 * M_PI / 3 * point;
}

void
generateTrianglePoints(vec2 vertices[], color3 colors[], int startVertexIndex) {
  double scale = 0.25;
  vec2 center (0.0, 0.70);

  for (int i = 0; i < 3; i++) {
    double currentAngle = getTriangleAngle(i);
    vertices[startVertexIndex + i] = vec2( sin (currentAngle),  cos(currentAngle)) * scale + center;
  }

  colors[startVertexIndex]     = RED;;
  colors[startVertexIndex + 1] = GREEN;
  colors[startVertexIndex + 2] = BLUE;
}

double
getSquareAngle(int point) {
  return M_PI / 4 + (M_PI/2 * point);
}

void
generateSquarePoints(vec2 vertices[], color3 colors[], int squareNum, int startVertexIndex) {  
  double scale = .90;
  double scaleAdjust = 0.15;//1.0 / squareNum;
  vec2 center (0.0, -0.25);

  int vertexIndex = startVertexIndex;

  for (int i = 0; i < squareNum; i++) {
    color3 currentColor = 0 == i % 2 ? WHITE : BLACK;

    for (int j = 0; j < 4; j++) {
      double currentAngle = getSquareAngle(j);
      vertices[vertexIndex] = vec2( sin (currentAngle),  cos(currentAngle)) * scale + center;
      colors[vertexIndex] = currentColor;

      vertexIndex++;
    }

    scale -= scaleAdjust;
  }
}

void
init( void )
{
  vec2 vertices[ TotalNumPoints ];
  color3 colors[ TotalNumPoints ] = { color3( 0.0, 0.0, 0.0 ) };

  generateEllipsePoints(vertices, colors, 0, CircleNumPoints, vec2( 2.75, 3.75 ), 0.2, 1.0);
  generateTrianglePoints(vertices, colors, CircleNumPoints);
  generateEllipsePoints(vertices, colors, CircleNumPoints + TriangleNumPoints, EllipseNumPoints, vec2( -2.75, 7.5 ), 0.2, 0.5);
  generateSquarePoints(vertices, colors, SquareNum, CircleNumPoints + TriangleNumPoints + EllipseNumPoints);

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
  glDrawArrays( GL_TRIANGLE_FAN, 0, CircleNumPoints );
  glDrawArrays( GL_TRIANGLES, CircleNumPoints, TriangleNumPoints );
  glDrawArrays( GL_TRIANGLE_FAN, CircleNumPoints + TriangleNumPoints, EllipseNumPoints );
  for (int i = 0; i < SquareNum; i++) {
    glDrawArrays( GL_TRIANGLE_FAN, CircleNumPoints + TriangleNumPoints + EllipseNumPoints + (i * 4), 4);
  }
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
