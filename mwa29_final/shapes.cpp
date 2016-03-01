
#include "Angel.h"

typedef Angel::vec4 point4;
typedef Angel::vec4 color4;

const int NumVertices = 36; //(6 faces)(2 triangles/face)(3 vertices/triangle)

point4 points[NumVertices];
color4 colors[NumVertices];

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

// RGBA olors
color4 vertex_colors[8] = {
  color4( 0.0, 0.0, 0.0, 1.0 ),  // black
  color4( 1.0, 0.0, 0.0, 1.0 ),  // red
  color4( 1.0, 1.0, 0.0, 1.0 ),  // yellow
  color4( 0.0, 1.0, 0.0, 1.0 ),  // green
  color4( 0.0, 0.0, 1.0, 1.0 ),  // blue
  color4( 1.0, 0.0, 1.0, 1.0 ),  // magenta
  color4( 1.0, 1.0, 1.0, 1.0 ),  // white
  color4( 0.0, 1.0, 1.0, 1.0 )   // cyan
};


// Parameters controlling the size of the Robot's arm
const GLfloat BASE_HEIGHT      = 2.0;
const GLfloat BASE_WIDTH       = 5.0;
const GLfloat ARM_ONE_HEIGHT   = 5.0;
const GLfloat ARM_ONE_WIDTH    = 0.5;
const GLfloat ARM_TWO_HEIGHT   = 5.0;
const GLfloat ARM_TWO_WIDTH    = 0.5;
const GLfloat ARM_THREE_HEIGHT = 5.0;
const GLfloat ARM_THREE_WIDTH  = 0.5;
const GLfloat ARM_FOUR_HEIGHT = 5.0;
const GLfloat ARM_FOUR_WIDTH  = 0.5;

// Shader transformation matrices
mat4  model_view;
GLuint ModelView, Projection;

enum direction { INCREASE, DECREASE };

// Array of rotation angles (in degrees) for each rotation axis
enum { Joint1 = 0, Joint2 = 1, Joint3 = 2, NumAngles = 3 };
int      Axis = Joint1;
GLfloat  ThetaX[NumAngles] = { 0.0, 0.0, 0.0 };
GLfloat  ThetaY[NumAngles] = { 0.0, 0.0, 0.0 };
GLfloat  ThetaZ[NumAngles] = { 270.0, 00.0, 270.0 };
GLfloat  Joint3Adjust = 0;

// Menu option values
const int  Quit = 4;


//----------------------------------------------------------------------------

int Index = 0;

void
quad( int a, int b, int c, int d )
{
  colors[Index] = vertex_colors[a]; points[Index] = vertices[a]; Index++;
  colors[Index] = vertex_colors[a]; points[Index] = vertices[b]; Index++;
  colors[Index] = vertex_colors[a]; points[Index] = vertices[c]; Index++;
  colors[Index] = vertex_colors[a]; points[Index] = vertices[a]; Index++;
  colors[Index] = vertex_colors[a]; points[Index] = vertices[c]; Index++;
  colors[Index] = vertex_colors[a]; points[Index] = vertices[d]; Index++;
}

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

//----------------------------------------------------------------------------

/* Define the three parts */
/* Note use of push/pop to return modelview matrix
   to its state before functions were entered and use
   rotation, translation, and scaling to create instances
   of symbols (cube and cylinder */

void
base()
{
  mat4 instance = ( Translate( 0.0, 0.5 * BASE_HEIGHT, 0.0 ) *
		    Scale( BASE_WIDTH,
			   BASE_HEIGHT,
			   BASE_WIDTH ) );

  glUniformMatrix4fv( ModelView, 1, GL_TRUE, model_view * instance );

  glDrawArrays( GL_TRIANGLES, 0, NumVertices );
}

//----------------------------------------------------------------------------

void
arm_four()
{
  mat4 instance = ( Translate( 0.0, Joint3Adjust + (0.5 * ARM_FOUR_HEIGHT), 0.0 ) *
		    Scale( ARM_FOUR_WIDTH,
			   ARM_FOUR_HEIGHT,
			   ARM_FOUR_WIDTH ) );
    
  glUniformMatrix4fv( ModelView, 1, GL_TRUE, model_view * instance );
  glDrawArrays( GL_TRIANGLES, 0, NumVertices );
}

//----------------------------------------------------------------------------

void
arm_three()
{
  mat4 instance = ( Translate( 0.0, 0.5 * ARM_THREE_HEIGHT, 0.0 ) *
		    Scale( ARM_THREE_WIDTH,
			   ARM_THREE_HEIGHT,
			   ARM_THREE_WIDTH ) );
    
  glUniformMatrix4fv( ModelView, 1, GL_TRUE, model_view * instance );
  glDrawArrays( GL_TRIANGLES, 0, NumVertices );
}

//----------------------------------------------------------------------------

void
arm_two()
{
  mat4 instance = ( Translate( 0.0, 0.5 * ARM_TWO_HEIGHT, 0.0 ) *
		    Scale( ARM_TWO_WIDTH,
			   ARM_TWO_HEIGHT,
			   ARM_TWO_WIDTH ) );
    
  glUniformMatrix4fv( ModelView, 1, GL_TRUE, model_view * instance );
  glDrawArrays( GL_TRIANGLES, 0, NumVertices );
}

//----------------------------------------------------------------------------

void
arm_one()
{
  mat4 instance = ( Translate( 0.0, 0.5 * ARM_ONE_HEIGHT, 0.0 ) *
		    Scale( ARM_ONE_WIDTH,
			   ARM_ONE_HEIGHT,
			   ARM_ONE_WIDTH ) );
    
  glUniformMatrix4fv( ModelView, 1, GL_TRUE, model_view * instance );
  glDrawArrays( GL_TRIANGLES, 0, NumVertices );
}

//----------------------------------------------------------------------------

void
display( void )
{
  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

  // Accumulate ModelView Matrix as we traverse the tree
  model_view = mat4(1.0);
  base();

  model_view *= ( Translate(0.0, BASE_HEIGHT, 0.0) );
  arm_one();

  model_view *= ( Translate(0.0, ARM_ONE_HEIGHT, 0.0) *
		  RotateZ(ThetaZ[Joint1]) * RotateX(ThetaX[Joint1]) * RotateY(ThetaY[Joint1]) );
  arm_two();

  model_view *= ( Translate(0.0, ARM_TWO_HEIGHT, 0.0) *
  		  RotateZ(ThetaZ[Joint2]) * RotateX(ThetaX[Joint2]) * RotateY(ThetaY[Joint2]) );
  arm_three();

  model_view *= ( Translate(0.0, ARM_THREE_HEIGHT, 0.0) *
  		  RotateZ(ThetaZ[Joint3]) * RotateX(ThetaX[Joint3]) * RotateY(ThetaY[Joint3]) );

  arm_four();

  glutSwapBuffers();
}

//----------------------------------------------------------------------------

void
init( void )
{
  colorcube();
    
  // Create a vertex array object
  GLuint vao;
  glGenVertexArrays( 1, &vao );
  glBindVertexArray( vao );

  // Create and initialize a buffer object
  GLuint buffer;
  glGenBuffers( 1, &buffer );
  glBindBuffer( GL_ARRAY_BUFFER, buffer );
  glBufferData( GL_ARRAY_BUFFER, sizeof(points) + sizeof(colors),
		NULL, GL_DYNAMIC_DRAW );
  glBufferSubData( GL_ARRAY_BUFFER, 0, sizeof(points), points );
  glBufferSubData( GL_ARRAY_BUFFER, sizeof(points), sizeof(colors), colors );
    
  // Load shaders and use the resulting shader program
  GLuint program = InitShader( "vshader81.glsl", "fshader81.glsl" );
  glUseProgram( program );
    
  GLuint vPosition = glGetAttribLocation( program, "vPosition" );
  glEnableVertexAttribArray( vPosition );
  glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0,
			 BUFFER_OFFSET(0) );

  GLuint vColor = glGetAttribLocation( program, "vColor" );
  glEnableVertexAttribArray( vColor );
  glVertexAttribPointer( vColor, 4, GL_FLOAT, GL_FALSE, 0,
			 BUFFER_OFFSET(sizeof(points)) );

  ModelView = glGetUniformLocation( program, "ModelView" );
  Projection = glGetUniformLocation( program, "Projection" );

  glEnable( GL_DEPTH );
  glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );

  glClearColor( 1.0, 1.0, 1.0, 1.0 ); 
}

//----------------------------------------------------------------------------

void
mouse( int button, int state, int x, int y )
{
}

//----------------------------------------------------------------------------

void
menu( int option )
{
  if ( option == Quit ) {
    exit( EXIT_SUCCESS );
  }
  else {
    Axis = option;
  }
}

//----------------------------------------------------------------------------

void
reshape( int width, int height )
{
  glViewport( 0, 0, width, height );

  GLfloat  left = -10.0, right = 10.0;
  GLfloat  bottom = -5.0, top = 15.0;
  GLfloat  zNear = -10.0, zFar = 10.0;

  GLfloat aspect = GLfloat(width)/height;

  if ( aspect > 1.0 ) {
    left *= aspect;
    right *= aspect;
  }
  else {
    bottom /= aspect;
    top /= aspect;
  }

  mat4 projection = Ortho( left, right, bottom, top, zNear, zFar );
  glUniformMatrix4fv( Projection, 1, GL_TRUE, projection );

  model_view = mat4( 1.0 );  // An Identity matrix
}

//

void
move_joint ( direction d )
{
  GLfloat *Theta = ThetaX;
  if (Axis == Joint3) {
    if ( d == DECREASE ) {
      // Incrase the joint angle
      Joint3Adjust += 1.0;
      if (Joint3Adjust > 0) { Joint3Adjust = 0; }
    }

    if ( d == INCREASE ) {
      // Decrase the joint angle
      Joint3Adjust -= 1.0;
      if (Joint3Adjust < -ARM_THREE_HEIGHT) {Joint3Adjust = -ARM_THREE_HEIGHT;}
    }
  } else {
    if ( d == DECREASE ) {
      // Incrase the joint angle
      Theta[Axis] += 5.0;
      if ( Theta[Axis] > 360.0 ) { Theta[Axis] -= 360.0; }
      if (Axis == Joint2) {
	if ( Theta[Axis] < 200.0 && Theta[Axis] > 160 ) { Theta[Axis] = 160.0; }
      }
    }

    if ( d == INCREASE ) {
      // Decrase the joint angle
      Theta[Axis] -= 5.0;
      if ( Theta[Axis] < 0.0 ) { Theta[Axis] += 360.0; }
      if (Axis == Joint2) {
	if ( Theta[Axis] < 200.0 && Theta[Axis] > 160 ) { Theta[Axis] = 200.0; }
      }
    }
  }

  glutPostRedisplay();
}

//----------------------------------------------------------------------------

void
keyboard( unsigned char key, int x, int y )
{
  switch( key ) {
  case 033: // Escape Key
  case 'q': case 'Q':
    exit( EXIT_SUCCESS );
    break;
  case 'w':
    move_joint(DECREASE);
    break;
  case 'e': // Left arrow
    move_joint(INCREASE);
    break;
  }
}

//----------------------------------------------------------------------------

void
printHelp ( void )
{
  printf("Right-click menu can be used to select the joint\n");
  printf("%c/%c can be used to select the increase the joint\n", 'w', 'e');
  printf("%c can be used to quit.", 'q');
}

//----------------------------------------------------------------------------

int
main( int argc, char **argv )
{
  glutInit( &argc, argv );
  glutInitDisplayMode( GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH );
  glutInitWindowSize( 512, 512 );
  glutCreateWindow( "robot" );

  glewExperimental=GL_TRUE;
  glewInit();

  init();

  glutDisplayFunc( display );
  glutReshapeFunc( reshape );
  glutKeyboardFunc( keyboard );

  glutCreateMenu( menu );
  // Set the menu values to the relevant rotation axis values (or Quit)
  glutAddMenuEntry( "Joint1", Joint1 );
  glutAddMenuEntry( "Joint2", Joint2 );
  glutAddMenuEntry( "Joint3", Joint3 );
  glutAddMenuEntry( "quit", Quit );
  glutAttachMenu( GLUT_RIGHT_BUTTON );

  printHelp();
  glutMainLoop();
  return 0;
}
