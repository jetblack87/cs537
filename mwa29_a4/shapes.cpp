#include "Angel.h"

typedef vec3 color3;

//--------------------------------------------------------------------------
//----   CONSTANTS    ------------------------------------------------------
//--------------------------------------------------------------------------

const char *TITLE = "mwa29 - CS537 assignment 3";

const int MENU_CHOICE_WHITE = 0;
const int MENU_CHOICE_BLACK = 1;
const int MENU_CHOICE_RED   = 2;
const int MENU_CHOICE_GREEN = 3;
const int MENU_CHOICE_BLUE  = 4;
const int MENU_CHOICE_START = 5;
const int MENU_CHOICE_STOP  = 6;


const color3 WHITE  (1.0, 1.0, 1.0);
const color3 BLACK  (0.0, 0.0, 0.0);
const color3 RED    (1.0, 0.0, 0.0);
const color3 GREEN  (0.0, 1.0, 0.0);
const color3 BLUE   (0.0, 0.0, 1.0);
const color3 YELLOW (1.0, 1.0, 0.0);
const color3 ORANGE (1.0, 0.65, 0.0);
const color3 PURPLE (0.8, 0.0, 0.8);

// Window 1
const int SquareNum         = 6;
const int SquareNumPoints   = 4 * SquareNum;

// Sub Window
const int EllipseNumPoints  = 360;

// Window 2
const int CircleNumPoints   = 360;
const int TriangleNumPoints = 3;

//--------------------------------------------------------------------------
//----   GLOBALS      ------------------------------------------------------
//--------------------------------------------------------------------------

int mainWindow, subWindow1;
int window2;
int menu_mainWindow, submenu_mainWindow, menu_subWindow1;
int border = 5;
int w = 600, h = 600;
double t  = 0;   // the time variable
double dt = .05; // the delta for time increment

color3 squareColor_mainWindow = WHITE;

color3 bg_subWindow1 = GREEN;

color3 objectColor_window2 = RED;

//--------------------------------------------------------------------------

void
myidle ( void )
{
  t += dt;
  glutPostWindowRedisplay(mainWindow);
  glutPostWindowRedisplay(subWindow1);
  glutPostWindowRedisplay(window2);
}

double
getVariableEllipseScale()
{
  const double MAX = 1.0;
  const double MIN = 0.25;

  const int DOWN = 0;
  const int UP   = 1;

  const double SCALE_DELTA = .01;

  static int direction = DOWN;
  static double scale  = MAX;

  if (DOWN == direction) {
    scale -= SCALE_DELTA;
    if (scale < MIN) {
      direction = UP;
    }
  } else {
    scale += SCALE_DELTA;
    if (scale > MAX) {
      direction = DOWN;
    }
  }
  return scale;
}

vec2
getEllipseVertex(vec2 center, double scale, double verticleScale, double angle)
{
  vec2 vertex (sin (angle), cos(angle));
  vertex   += center;
  vertex   *= scale;
  vertex.y *= verticleScale;
  return vertex;
}

double
generateAngleColor(double angle)
{
  return 1.0 / (2 * M_PI) * angle;
}

void
generateEllipsePoints(vec2 vertices[], color3 colors[], color3 color, int startVertexIndex, int numPoints,
		      vec2 center, double scale, double verticleScale)
{

  double angleIncrement = (2 * M_PI) / numPoints;
  double currentAngle = M_PI / 2;
  vec2 startVertex = getEllipseVertex(center, scale, verticleScale, currentAngle);
  vertices[startVertexIndex] = startVertex;

  if (1.0 != verticleScale) {
    colors[startVertexIndex] = color;
  }

  for(int i = startVertexIndex + 1; i < startVertexIndex + numPoints; i++) {
    vertices[i] = getEllipseVertex(center, scale, verticleScale, currentAngle);

    if (1.0 == verticleScale) {
      colors[i] = color * generateAngleColor(currentAngle);
    } else {
      colors[i] = color;
    }

    currentAngle += angleIncrement;
  }
}

double
getTriangleAngle(int point) {
  return (2 * M_PI / 3 * point) + t;
}

void
generateTrianglePoints(vec2 vertices[], color3 colors[], color3 color, int startVertexIndex, double scale, vec2 center) {

  for (int i = 0; i < 3; i++) {
    double currentAngle = getTriangleAngle(i);
    vertices[startVertexIndex + i] = vec2( sin (currentAngle),  cos(currentAngle)) * scale + center;
  }

  colors[startVertexIndex]     = color;
  colors[startVertexIndex + 1] = color;
  colors[startVertexIndex + 2] = color;
}

double
getSquareAngle(int point) {
  return (M_PI / 4 + (M_PI/2 * point)) - t;
}

void
generateSquarePoints(vec2 vertices[], color3 colors[], int squareNum, int startVertexIndex) {
  double scale = .90;
  double scaleAdjust = 0.15;//1.0 / squareNum;
  vec2 center (0.0, -0.25);

  int vertexIndex = startVertexIndex;

  for (int i = 0; i < squareNum; i++) {
    color3 currentColor = 0 == i % 2 ? squareColor_mainWindow : BLACK;

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
init_mainWindow( void )
{
  vec2 vertices[ SquareNum * 4 ];
  color3 colors[ SquareNum * 4 ] = { color3( 0.0, 0.0, 0.0 ) };

  generateSquarePoints(vertices, colors, SquareNum, 0);

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

void
processMenuEvents_mainWindow(int menuChoice)
{
  switch(menuChoice) {
  case MENU_CHOICE_STOP:
    glutIdleFunc( NULL );
    break;
  case MENU_CHOICE_START:
    glutIdleFunc( myidle );
    break;
  case MENU_CHOICE_WHITE:
    squareColor_mainWindow = WHITE;
    break;
  case MENU_CHOICE_RED:
    squareColor_mainWindow = RED;
    break;
  case MENU_CHOICE_GREEN:
    squareColor_mainWindow = GREEN;
    break;
  }
  glutPostWindowRedisplay(mainWindow);
}

void
setupMenu_mainWindow()
{
  submenu_mainWindow = glutCreateMenu(processMenuEvents_mainWindow);
  glutAddMenuEntry("White", MENU_CHOICE_WHITE);
  glutAddMenuEntry("Red",   MENU_CHOICE_RED);
  glutAddMenuEntry("Green", MENU_CHOICE_GREEN);

  menu_mainWindow = glutCreateMenu(processMenuEvents_mainWindow);
  glutAddMenuEntry("Stop Animation",  MENU_CHOICE_STOP);
  glutAddMenuEntry("Start Animation", MENU_CHOICE_START);
  glutAddSubMenu("Square Colors", submenu_mainWindow);
  glutAttachMenu(GLUT_RIGHT_BUTTON);
}

void
display_mainWindow( void )
{
  init_mainWindow();
  glClear( GL_COLOR_BUFFER_BIT );
  for (int i = 0; i < SquareNum; i++) {
    glDrawArrays( GL_TRIANGLE_FAN, (i * 4), 4);
  }
  glFlush();
  glutSwapBuffers();
}


void
keyboard_mainWindow( unsigned char key, int x, int y )
{
  switch ( key ) {
  case 033:
    exit( EXIT_SUCCESS );
    break;
  }
}

void
init_subWindow1( void )
{
  vec2 vertices[ EllipseNumPoints ];
  color3 colors[ EllipseNumPoints ] = { color3( 1.0, 0.0, 0.0 ) };

  generateEllipsePoints(vertices, colors, RED, 0, EllipseNumPoints,
			vec2( 0.0, 0.0 ), 0.75, 0.5);

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

  // TODO: Figure out swizzle for this so I can use .r, .g, .b?
  glClearColor( bg_subWindow1.x, bg_subWindow1.y, bg_subWindow1.z, 1.0 );
}

void
processMenuEvents_subWindow1(int menuChoice)
{
  switch(menuChoice) {
  case MENU_CHOICE_WHITE:
    bg_subWindow1 = WHITE;
    break;
  case MENU_CHOICE_BLACK:
    bg_subWindow1 = BLACK;
    break;
  case MENU_CHOICE_RED:
    bg_subWindow1 = RED;
    break;
  case MENU_CHOICE_BLUE:
    bg_subWindow1 = BLUE;
    break;
  case MENU_CHOICE_GREEN:
    bg_subWindow1 = GREEN;
    break;
  }
  glutPostWindowRedisplay(subWindow1);
}

void
setupMenu_subWindow1()
{
  menu_subWindow1 = glutCreateMenu(processMenuEvents_subWindow1);
  glutAddMenuEntry("White", MENU_CHOICE_WHITE);
  glutAddMenuEntry("Black", MENU_CHOICE_BLACK);
  glutAddMenuEntry("Red",   MENU_CHOICE_RED);
  glutAddMenuEntry("Green", MENU_CHOICE_GREEN);
  glutAddMenuEntry("Blue",  MENU_CHOICE_BLUE);
  glutAttachMenu(GLUT_RIGHT_BUTTON);
}

void
display_subWindow1( void )
{
  init_subWindow1();
  glClear( GL_COLOR_BUFFER_BIT );
  glDrawArrays( GL_TRIANGLE_FAN, 0, EllipseNumPoints );
  glFlush();
  glutSwapBuffers();
}


void
keyboard_subWindow1( unsigned char key, int x, int y )
{
  switch ( key ) {
  case 033:
    exit( EXIT_SUCCESS );
    break;
  }
}

void
init_window2( void )
{
  vec2 vertices[ TriangleNumPoints + CircleNumPoints ];
  color3 colors[ TriangleNumPoints + CircleNumPoints ] = { color3( 0.0, 0.0, 0.0 ) };

  // TODO: Fix movement during re-size
  double scaleSize = getVariableEllipseScale();
  generateTrianglePoints(vertices, colors, objectColor_window2, 0, 0.5, vec2 (-0.5, 0.0));
  generateEllipsePoints(vertices, colors, objectColor_window2,
			TriangleNumPoints, CircleNumPoints,
			vec2( 4.0, 1.0 ) - 2*scaleSize, 0.2 * scaleSize, 1.0);

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

void
setupMenu_window2()
{
}

void
display_window2( void )
{
  init_window2();
  glClear( GL_COLOR_BUFFER_BIT );
  glDrawArrays( GL_TRIANGLES, 0, TriangleNumPoints );
  glDrawArrays( GL_TRIANGLE_FAN, TriangleNumPoints, CircleNumPoints );
  glFlush();
  glutSwapBuffers();
}


void
keyboard_window2( unsigned char key, int x, int y )
{
  switch ( key ) {
  case 'r': objectColor_window2 = RED; break;
  case 'g': objectColor_window2 = GREEN; break;
  case 'b': objectColor_window2 = BLUE; break;
  case 'y': objectColor_window2 = YELLOW; break;
  case 'o': objectColor_window2 = ORANGE; break;
  case 'p': objectColor_window2 = PURPLE; break;
  case 'w': objectColor_window2 = WHITE; break;
  }
  glutPostWindowRedisplay(window2);
}

void
printHelp ( void ) {
  printf("%s\n", TITLE);
  printf("Keyboard options:\n");
  printf("Within 'window 2' you can use the keyboard keys to update the object colors:\n");
  printf("\t'r' - red\n\t'g' - green\n\t'b' - blue\n\t'y' - yellow\n\t'o' - orange\n\t'p' - purple\n\t'w' - white\n");
}

int
main( int argc, char **argv )
{
  glutInit( &argc, argv );
  glutInitDisplayMode( GLUT_RGBA | GLUT_DOUBLE);
  glutInitWindowPosition(100,100);
  glutInitWindowSize( w, h );

  mainWindow = glutCreateWindow( TITLE );

  glewExperimental=GL_TRUE;
  glewInit();

  init_mainWindow();
  setupMenu_mainWindow();
  glutDisplayFunc ( display_mainWindow  );
  glutKeyboardFunc( keyboard_mainWindow );

  subWindow1  = glutCreateSubWindow(mainWindow,
				    0, 0,
				    w / 3, h / 3);
  init_subWindow1();
  setupMenu_subWindow1();
  glutDisplayFunc ( display_subWindow1  );
  glutKeyboardFunc( keyboard_subWindow1 );

  glutInitWindowPosition(100 + w,100);
  glutInitWindowSize( w, h / 3 );
  window2 = glutCreateWindow( "window 2" );
  init_window2();
  glutDisplayFunc ( display_window2  );
  glutKeyboardFunc( keyboard_window2 );

  glutIdleFunc( myidle );

  printHelp();

  glutMainLoop();
  return 0;
}
