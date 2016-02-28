// fragment shading of sphere model

#include "Angel.h"

const char *TITLE = "mwa29 - CS537 assignment 8";

const int WIDTH  = 512;
const int HEIGHT = 512;

const int NumShapes           = 3;
const int NumTimesToSubdivide = 5;
const int NumTriangles        = 4096;  // (4 faces)^(NumTimesToSubdivide + 1)
const int NumVertices         = 3 * NumTriangles;

typedef Angel::vec4 point4;
typedef Angel::vec4 color4;

enum buffer_type {BACK_BUFFER, FRAMEBUFFER};
enum material {BRASS, RED_PLASTIC, GREEN_RUBBER};

enum object {OBJECT_ONE, OBJECT_TWO, OBJECT_THREE};
const GLubyte OBJECT_COLOR_ONE[3]   = {255,0,0};
const color4  OBJECT_COLOR_ONE_VEC(1.0,0.0,0.0,1.0);
const GLubyte OBJECT_COLOR_TWO[3]   = {0,255,0};
const color4  OBJECT_COLOR_TWO_VEC(0.0,1.0,0.0,1.0);
const GLubyte OBJECT_COLOR_THREE[3] = {0,0,255};
const color4  OBJECT_COLOR_THREE_VEC(0.0,0.0,1.0,1.0);

const color4 BRASS_AMBIENT(0.329412, 0.223529, 0.027451, 1.0);
const color4 BRASS_DIFFUSE(0.780392, 0.568627, 0.113725, 1.0);
const color4 BRASS_SPECULAR(0.992157, 0.941176, 0.807843, 1.0);
const float  BRASS_SHININESS = 0.21794872;

const color4 RED_PLASTIC_AMBIENT(0.0, 0.0, 0.0, 1.0);
const color4 RED_PLASTIC_DIFFUSE(0.5, 0.0, 0.0, 1.0);
const color4 RED_PLASTIC_SPECULAR(0.7, 0.6, 0.6, 1.0);
const float  RED_PLASTIC_SHININESS = 0.25;

const color4 GREEN_RUBBER_AMBIENT(0.0, 0.05, 0.0, 1.0);
const color4 GREEN_RUBBER_DIFFUSE(0.4, 0.5, 0.4, 1.0);
const color4 GREEN_RUBBER_SPECULAR(0.04, 0.7, 0.04, 1.0);
const float  GREEN_RUBBER_SHININESS = 0.078125;

GLuint program;

int mainWindow;
int menu;

GLuint buffer;
GLuint fb;
GLuint color_rb;
GLuint depth_rb;

point4 points[NumVertices];
vec3   normals[NumVertices];

double rotate_dt    = 1.0;
double rotate_x     = 0.0;
double rotate_y     = 0.0;
double rotate_z     = 0.0;

// Model-view and projection matrices uniform location
GLuint  ModelView, Projection;

material current_material[3] = {BRASS, RED_PLASTIC, GREEN_RUBBER};

//----------------------------------------------------------------------------

int Index = 0;

void
triangle( const point4& a, const point4& b, const point4& c )
{
  vec3  normal = normalize( cross(b - a, c - b) );

  normals[Index] = normal;  points[Index] = a;  Index++;
  normals[Index] = normal;  points[Index] = b;  Index++;
  normals[Index] = normal;  points[Index] = c;  Index++;
}

//----------------------------------------------------------------------------

point4
unit( const point4& p )
{
  float len = p.x*p.x + p.y*p.y + p.z*p.z;
    
  point4 t;
  if ( len > DivideByZeroTolerance ) {
    t = p / sqrt(len);
    t.w = 1.0;
  }

  return t;
}

void
divide_triangle( const point4& a, const point4& b,
		 const point4& c, int count )
{
  if ( count > 0 ) {
    point4 v1 = unit( a + b );
    point4 v2 = unit( a + c );
    point4 v3 = unit( b + c );
    divide_triangle(  a, v1, v2, count - 1 );
    divide_triangle(  c, v2, v3, count - 1 );
    divide_triangle(  b, v3, v1, count - 1 );
    divide_triangle( v1, v3, v2, count - 1 );
  }
  else {
    triangle( a, b, c );
  }
}

void
tetrahedron( int count )
{
  point4 v[4] = {
    vec4( 0.0, 0.0, 1.0, 1.0 ),
    vec4( 0.0, 0.942809, -0.333333, 1.0 ),
    vec4( -0.816497, -0.471405, -0.333333, 1.0 ),
    vec4( 0.816497, -0.471405, -0.333333, 1.0 )
  };

  divide_triangle( v[0], v[1], v[2], count );
  divide_triangle( v[3], v[2], v[1], count );
  divide_triangle( v[0], v[3], v[1], count );
  divide_triangle( v[0], v[2], v[3], count );
}

//----------------------------------------------------------------------------

// OpenGL initialization
void
init_renderbuffer( void )
{
  int width  = glutGet(GLUT_WINDOW_WIDTH);
  int height = glutGet(GLUT_WINDOW_HEIGHT);
  //RGBA8 RenderBuffer, 24 bit depth RenderBuffer, 256x256
  glGenFramebuffers(1, &fb);
  glBindFramebuffer(GL_FRAMEBUFFER, fb);

  //Create and attach a color buffer
  glGenRenderbuffers(1, &color_rb);

  //We must bind color_rb before we call glRenderbufferStorage
  glBindRenderbuffer(GL_RENDERBUFFER, color_rb);

  //The storage format is RGBA8
  glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, width, height);

  //Attach color buffer to FBO
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
			    GL_RENDERBUFFER, color_rb);

  //-------------------------
  glGenRenderbuffers(1, &depth_rb);
  glBindRenderbuffer(GL_RENDERBUFFER, depth_rb);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);

  //-------------------------
  //Attach depth buffer to FBO
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, 
			    GL_RENDERBUFFER, depth_rb);
}

void
init()
{
  // Subdivide a tetrahedron into a sphere
  tetrahedron( NumTimesToSubdivide );

  // Create a vertex array object
  GLuint vao;
  glGenVertexArrays( 1, &vao );
  glBindVertexArray( vao );

  // Create and initialize a buffer object
  glGenBuffers( 1, &buffer );
  glBindBuffer( GL_ARRAY_BUFFER, buffer );
  glBufferData( GL_ARRAY_BUFFER, NumShapes*(sizeof(points) + sizeof(normals)),
		NULL, GL_STATIC_DRAW );
  glBufferSubData( GL_ARRAY_BUFFER, 0, sizeof(points), points );
  glBufferSubData( GL_ARRAY_BUFFER, sizeof(points),
		   sizeof(normals), normals );
  glBufferSubData( GL_ARRAY_BUFFER, 0, sizeof(points), points );
  glBufferSubData( GL_ARRAY_BUFFER, sizeof(points),
		   sizeof(normals), normals );

  // Load shaders and use the resulting shader program
  glUseProgram( program );
	
  // set up vertex arrays
  GLuint vPosition = glGetAttribLocation( program, "vPosition" );
  glEnableVertexAttribArray( vPosition );
  glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0,
			 BUFFER_OFFSET(0) );

  GLuint vNormal = glGetAttribLocation( program, "vNormal" ); 
  glEnableVertexAttribArray( vNormal );
  glVertexAttribPointer( vNormal, 3, GL_FLOAT, GL_FALSE, 0,
			 BUFFER_OFFSET(sizeof(points)) );


  // Retrieve transformation uniform variable locations
  ModelView = glGetUniformLocation( program, "ModelView" );
  Projection = glGetUniformLocation( program, "Projection" );
    
  glEnable( GL_DEPTH_TEST );
    
  glClearColor( 0.25, 0.25, 0.25, 1.0 );

  init_renderbuffer();
}

//----------------------------------------------------------------------------

void
set_material (material mat)
{
  color4 light_ambient( 0.2, 0.2, 0.2, 1.0 );
  color4 light_diffuse( 1.0, 1.0, 1.0, 1.0 );
  color4 light_specular( 1.0, 1.0, 1.0, 1.0 );
  color4 material_ambient;
  color4 material_diffuse;
  color4 material_specular;
  float  material_shininess;
  switch (mat) {
  case BRASS:
    material_ambient   = BRASS_AMBIENT;
    material_diffuse   = BRASS_DIFFUSE;
    material_specular  = BRASS_SPECULAR;
    material_shininess = BRASS_SHININESS;
    break;
  case RED_PLASTIC:
    material_ambient   = RED_PLASTIC_AMBIENT;
    material_diffuse   = RED_PLASTIC_DIFFUSE;
    material_specular  = RED_PLASTIC_SPECULAR;
    material_shininess = RED_PLASTIC_SHININESS;
    break;
  case GREEN_RUBBER:
    material_ambient   = GREEN_RUBBER_AMBIENT;
    material_diffuse   = GREEN_RUBBER_DIFFUSE;
    material_specular  = GREEN_RUBBER_SPECULAR;
    material_shininess = GREEN_RUBBER_SHININESS;
    break;
  }

  color4 ambient_product = light_ambient * material_ambient;
  color4 diffuse_product = light_diffuse * material_diffuse;
  color4 specular_product = light_specular * material_specular;

  glUniform4fv( glGetUniformLocation(program, "AmbientProduct"),
		1, ambient_product );
  glUniform4fv( glGetUniformLocation(program, "DiffuseProduct"),
		1, diffuse_product );
  glUniform4fv( glGetUniformLocation(program, "SpecularProduct"),
		1, specular_product );
	
  glUniform1f( glGetUniformLocation(program, "Shininess"),
	       material_shininess );
}

void
reshape( int width, int height )
{
  glViewport( 0, 0, width, height );

  GLfloat left = -2.0, right = 2.0;
  GLfloat top = 2.0, bottom = -2.0;
  GLfloat zNear = 1.0, zFar = 20.0;

  GLfloat aspect = GLfloat(width)/height;

  if ( aspect > 1.0 ) {
    left *= aspect;
    right *= aspect;
  }
  else {
    top /= aspect;
    bottom /= aspect;
  }

  mat4 projection = Frustum( left, right, bottom, top, zNear, zFar );
  glUniformMatrix4fv( Projection, 1, GL_TRUE, projection );
}

void
update_buffer( buffer_type b_type )
{
  if (BACK_BUFFER == b_type) {
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
  } else {
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fb);
  }
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  reshape(glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT));

  mat4 rotation = mat4(1);
  rotation = rotation * RotateX(rotate_x);
  rotation = rotation * RotateY(rotate_y);
  rotation = rotation * RotateZ(rotate_z);

  point4 at( 0.0, 0.0, 0.0, 1.0 );
  point4 eye( 0.0, 0.0, 2.0, 1.0 );
  vec4   up( 0.0, 1.0, 0.0, 0.0 );

  // Initialize shader lighting parameters
  point4 light_position( 0.0, 0.0, 2.0, 0.0 );

  glUniform4fv( glGetUniformLocation(program, "LightPosition"),
		1, light_position );

  if (FRAMEBUFFER == b_type) {
    glUniform4fv( glGetUniformLocation(program, "FrameBufferColor"),
		  1,  OBJECT_COLOR_ONE_VEC);
  } else {
    // Set FrameBufferColor to 'off' color
    glUniform4fv( glGetUniformLocation(program, "FrameBufferColor"),
		  1,  color4(0.0,0.0,0.0,1.0));
  }
  set_material(current_material[OBJECT_ONE]);
  mat4 model_view = LookAt( eye, at, up );
  model_view = model_view * Translate(0,0,-1);
  model_view = model_view * rotation;
  model_view = model_view * Translate(0,1,0);
  glUniformMatrix4fv( ModelView, 1, GL_TRUE, model_view );
  glDrawArrays( GL_TRIANGLES, 0, NumVertices );

  if (FRAMEBUFFER == b_type) {
    glUniform4fv( glGetUniformLocation(program, "FrameBufferColor"),
		  1,  OBJECT_COLOR_TWO_VEC);
  }
  set_material(current_material[OBJECT_TWO]);
  model_view = LookAt( eye, at, up );
  model_view = model_view * Translate(0,0,-1);
  model_view = model_view * rotation;
  model_view = model_view * Translate(-1,-1,0);
  glUniformMatrix4fv( ModelView, 1, GL_TRUE, model_view );
  glDrawArrays( GL_TRIANGLES, 0, NumVertices );

  if (FRAMEBUFFER == b_type) {
    glUniform4fv( glGetUniformLocation(program, "FrameBufferColor"),
		  1,  OBJECT_COLOR_THREE_VEC);
  }
  set_material(current_material[OBJECT_THREE]);
  model_view = LookAt( eye, at, up );
  model_view = model_view * Translate(0,0,-1);
  model_view = model_view * rotation;
  model_view = model_view * Translate(1,-1,0);
  glUniformMatrix4fv( ModelView, 1, GL_TRUE, model_view );
  glDrawArrays( GL_TRIANGLES, 0, NumVertices );

  if (BACK_BUFFER == b_type) {
    glutSwapBuffers();
  }
}

void
display( void )
{
  update_buffer(BACK_BUFFER);
  update_buffer(FRAMEBUFFER);
}

bool
object_colors_match( const GLubyte* one, const GLubyte* two )
{
  return one[0] == two[0] && one[1] == two[1] && one[2] == two[2];
}

void
change_object_color ( object o )
{
  if (BRASS == current_material[o]) {
    current_material[o] = RED_PLASTIC;
  } else if (RED_PLASTIC == current_material[o]) {
    current_material[o] = GREEN_RUBBER;
  } else if (GREEN_RUBBER == current_material[o]) {
    current_material[o] = BRASS;
  }
  glutPostWindowRedisplay(mainWindow);
}

void
process_mouse_click(int x, int y)
{
  int width  = glutGet(GLUT_WINDOW_WIDTH);
  int height = glutGet(GLUT_WINDOW_HEIGHT);

  int buffer_x = x;
  int buffer_y = height - y;

  glBindFramebuffer(GL_READ_FRAMEBUFFER, fb);
  GLubyte selected_color[3];
  glReadPixels(buffer_x,buffer_y, 1, 1, GL_RGB,  
	       GL_UNSIGNED_BYTE, selected_color);

  if (object_colors_match(OBJECT_COLOR_ONE, selected_color)) {
    change_object_color(OBJECT_ONE);
  } else if (object_colors_match(OBJECT_COLOR_TWO, selected_color)) {
    change_object_color(OBJECT_TWO);
  } else if (object_colors_match(OBJECT_COLOR_THREE, selected_color)) {
    change_object_color(OBJECT_THREE);
  }
}

void
reset ( void )
{
  rotate_x     = 0.0;
  rotate_y     = 0.0;
  rotate_z     = 0.0;
  current_material[OBJECT_ONE]   = BRASS;
  current_material[OBJECT_TWO]   = RED_PLASTIC;
  current_material[OBJECT_THREE] = GREEN_RUBBER;
}

void
mouse( int button, int state, int x, int y )
{
  if ( state == GLUT_DOWN ) {
    switch( button ) {
    case GLUT_LEFT_BUTTON: process_mouse_click(x, y); break;
    }
  }
}

void
keyboard( unsigned char key, int x, int y )
{
  switch( key ) {
  case 'a':
    rotate_x -= rotate_dt;
    glutPostWindowRedisplay(mainWindow);
    break;
  case 'A':
    rotate_x += rotate_dt;
    glutPostWindowRedisplay(mainWindow);
    break;
  case 's':
    rotate_y -= rotate_dt;
    glutPostWindowRedisplay(mainWindow);
    break;
  case 'S':
    rotate_y += rotate_dt;
    glutPostWindowRedisplay(mainWindow);
    break;
  case 'd':
    rotate_z -= rotate_dt;
    glutPostWindowRedisplay(mainWindow);
    break;
  case 'D':
    rotate_z += rotate_dt;
    glutPostWindowRedisplay(mainWindow);
    break;
  case 'r':
    reset();
    glutPostWindowRedisplay(mainWindow);
    break;
  }
}

void printHelp ( void )
{
  printf("%s\n", TITLE);
  printf("Left-click on the shapes to change their color.\n");
  printf("Keyboard options:\n");
  printf("%c/%c - rotate around the x axes\n", 'a', 'A');
  printf("%c/%c - rotate around the y axes\n", 's', 'S');
  printf("%c/%c - rotate around the z axes\n", 'd', 'D');
  printf("%c    - Reset shape positions and colors\n", 'r');
}

int
main( int argc, char **argv )
{
  glutInit( &argc, argv );
  glutInitDisplayMode( GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH );
  glutInitWindowSize( WIDTH, HEIGHT );
  mainWindow = glutCreateWindow( TITLE );

  glewExperimental=GL_TRUE;
  glewInit();

  program = InitShader( "vshader56.glsl", "fshader56.glsl" );
  init();

  glutDisplayFunc( display );
  glutReshapeFunc( reshape );
  glutKeyboardFunc( keyboard );
  glutMouseFunc( mouse );

  printHelp();

  glutMainLoop();
  return 0;
}
