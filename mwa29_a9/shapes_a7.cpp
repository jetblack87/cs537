#include "Angel.h"
#include <vector>

typedef Angel::vec4 point4;
typedef Angel::vec4 color4;

typedef GLfloat     point3[3];

bool debug = false;

const char *TITLE = "mwa29 - CS537 assignment 9";

const int  TextureSize  = 64;
// Texture objects and storage for texture image
GLuint textures[1];
GLubyte image[TextureSize][TextureSize][3];

const bool just_lines = false;

int mainWindow;
int menu;

enum {FLAT_SHADING, SMOOTH_SHADING};

const int NumAxesPoints = 6;
point4 axes_points[NumAxesPoints] = {
  point4( 10.0,0.0,0.0,1.0),
  point4(-10.0,0.0,0.0,1.0),
  point4(0.0, 10.0,0.0,1.0),
  point4(0.0,-10.0,0.0,1.0),
  point4(0.0,0.0, 10.0,1.0),
  point4(0.0,0.0,-10.0,1.0),
};

const int NumControlVertices = 16;
int selected_control_vertex = 10;

point3 vertices[NumControlVertices];

int NumPatches = 1;
int NumTimesToSubdivide = 4;
int NumVertices;
int NumNormals;

std::vector<point4> points;
std::vector<vec3> normals;
std::vector<vec2> tex_coords;

GLuint  Projection;
GLuint  ModelView;

enum { X = 0, Y = 1, Z = 2 };

double t  = 0;
double dt = 0.01;
double control_point_dt = 0.1;

double translate_dt = 1.0;
double translate_x  = 0.0;
double translate_y  = 0.0;
double translate_z  = -13.0;
double rotate_dt    = 1.0;
double rotate_x     = 0.0;
double rotate_y     = 0.0;
double rotate_z     = -180.0;

int FlatShading = 0;

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
  points.push_back(p[0][0]);
  points.push_back(p[3][0]);
  points.push_back(p[3][3]);
  points.push_back(p[0][0]);
  points.push_back(p[3][3]);
  points.push_back(p[0][3]);
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

bool
double_equals(double a, double b)
{
  double epsilon = 0.001;
  return fabs(a - b) < epsilon;
}

bool
point_equals(point4 one, point4 two)
{
  return double_equals(one.x, two.x)
    && double_equals(one.y, two.y)
    && double_equals(one.z, two.z);
}

void
calculate_normals( void )
{
  int point_index = 0;
  for (int normal_index = NumAxesPoints + NumControlVertices;
       normal_index < NumNormals;
       normal_index+=3) {
    point4 p0 = points[point_index++];
    point4 p1 = points[point_index++];
    point4 p2 = points[point_index++];

    vec4 u = p1 - p0;
    vec4 v = p2 - p1;

    vec3 normal = normalize( cross(u, v) );

    normals.push_back(normal);
    normals.push_back(normal);
    normals.push_back(normal);
  }
}

void
init( void )
{
  NumVertices = NumPatches*6*pow(4, NumTimesToSubdivide);
  NumNormals = NumAxesPoints + NumControlVertices + NumVertices;
  points.clear();
  normals.clear();

  for ( int n = 0; n < NumPatches; n++ ) {
    point4  patch[4][4];

    // Initialize each patch's control point data
    int index = 0;
    for ( int i = 0; i < 4; ++i ) {
      for ( int j = 0; j < 4; ++j ) {
	point3& v = vertices[index++];
	patch[i][j] = point4( v[X], v[Y], v[Z], 1.0 );
      }
    }

    // Subdivide the patch
    divide_patch( patch, NumTimesToSubdivide );
  }

  calculate_normals();

  for (int i = 0; i < NumAxesPoints; i++) {
    tex_coords.push_back(vec2(axes_points[i].x, axes_points[i].y));
  }
  for (uint i = 0; i < points.size(); i++) {
    tex_coords.push_back(vec2(points.at(i).x, points.at(i).y));
  }

  // Need vertices of size of vec4
  point4 control_points[NumControlVertices];
  for (int i = 0; i < NumControlVertices; i++) {
    int w = 1.0;
    if (i == selected_control_vertex) {
      w = 1.2345;
    }
    control_points[i] = vec4(vertices[i][X],
			     vertices[i][Y],
			     vertices[i][Z], w);
    tex_coords.push_back(vec2(vertices[i][X], vertices[i][Y]));
  }

  // Create a checkerboard pattern
  for ( int i = 0; i < TextureSize; i++ ) {
    for ( int j = 0; j < TextureSize; j++ ) {
      GLubyte c = (((i & 0x8) == 0) ^ ((j & 0x8)  == 0)) * 255;
      image[i][j][0]  = c;
      image[i][j][1]  = c;
      image[i][j][2]  = c;
    }
  }

  // Initialize texture objects
  glGenTextures( 1, textures );

  glBindTexture( GL_TEXTURE_2D, textures[0] );
  glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, TextureSize, TextureSize, 0,
		GL_RGB, GL_UNSIGNED_BYTE, image );
  glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
  glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
  glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
  glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );

  glActiveTexture( GL_TEXTURE0 );
  glBindTexture( GL_TEXTURE_2D, textures[0] );

  // Create a vertex array object
  GLuint vao[1];
  glGenVertexArrays( 1, vao );
  glBindVertexArray( vao[0] );

  // Create and initialize a buffer object
  GLuint buffer;
  glGenBuffers( 1, &buffer );
  glBindBuffer( GL_ARRAY_BUFFER, buffer );
  glBufferData( GL_ARRAY_BUFFER,
		sizeof(axes_points) +
		sizeof(control_points) +
		(points.size()*sizeof(point4)) +
		(normals.size()*sizeof(vec3)) +
		(tex_coords.size()*sizeof(vec2)),
		NULL, GL_STATIC_DRAW );

  GLintptr offset = 0;
  glBufferSubData(GL_ARRAY_BUFFER,
		  offset,
		  sizeof(axes_points), axes_points);
  offset += sizeof(axes_points);
  glBufferSubData(GL_ARRAY_BUFFER,
		  offset,
		  sizeof(control_points), control_points);
  offset += sizeof(control_points);
  glBufferSubData(GL_ARRAY_BUFFER,
		  offset,
		  (points.size()*sizeof(point4)), &points[0]);
  offset += (points.size()*sizeof(point4));
  glBufferSubData(GL_ARRAY_BUFFER,
		  offset,
		  (normals.size()*sizeof(vec3)), &normals[0]);
  offset += (normals.size()*sizeof(vec3));
  glBufferSubData( GL_ARRAY_BUFFER,
		   offset,
		   (tex_coords.size()*sizeof(vec2)), &tex_coords[0] );

  // Load shaders and use the resulting shader program
  GLuint program = InitShader( "vshader56_a7.glsl", "fshader56_a7.glsl" );
  glUseProgram( program );

  // set up vertex arrays
  GLuint vPosition = glGetAttribLocation( program, "vPosition" );
  glEnableVertexAttribArray( vPosition );
  offset = 0;
  glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0,
			 BUFFER_OFFSET(offset) );
  offset += sizeof(axes_points) + sizeof(control_points) + (points.size()*sizeof(point4));

  GLuint vNormal = glGetAttribLocation( program, "vNormal" );
  glEnableVertexAttribArray( vNormal );
  glVertexAttribPointer( vNormal, 3, GL_FLOAT, GL_FALSE, 0,
			 BUFFER_OFFSET(offset));
  offset += (normals.size()*sizeof(vec3));

  GLuint vTexCoord = glGetAttribLocation( program, "vTexCoord" );
  glEnableVertexAttribArray( vTexCoord );
  glVertexAttribPointer( vTexCoord, 2, GL_FLOAT, GL_FALSE, 0,
			 BUFFER_OFFSET(offset));

  glUniform1i( glGetUniformLocation(program, "texture"), 0 );

  // Initialize shader lighting parameters
  point4 light_position( 0.0, 0.0, -2.0, 0.0 );
  color4 light_ambient( 0.2, 0.2, 0.2, 1.0 );
  color4 light_diffuse( 1.0, 1.0, 1.0, 1.0 );
  color4 light_specular( 1.0, 1.0, 1.0, 1.0 );

  color4 material_ambient( 1.0, 0.8, 1.0, 1.0 );
  color4 material_diffuse( 1.0, 0.8, 0.0, 1.0 );
  color4 material_specular( 1.0, 0.8, 1.0, 1.0 );
  float  material_shininess = 5.0;

  color4 ambient_product = light_ambient * material_ambient;
  color4 diffuse_product = light_diffuse * material_diffuse;
  color4 specular_product = light_specular * material_specular;

  glUniform4fv( glGetUniformLocation(program, "AmbientProduct"),
		1, ambient_product );
  glUniform4fv( glGetUniformLocation(program, "DiffuseProduct"),
		1, diffuse_product );
  glUniform4fv( glGetUniformLocation(program, "SpecularProduct"),
		1, specular_product );

  glUniform4fv( glGetUniformLocation(program, "LightPosition"),
		1, light_position );

  glUniform4fv( glGetUniformLocation(program, "MaterialDiffuse"),
		1, material_diffuse );
  glUniform4fv( glGetUniformLocation(program, "LightDiffuse"),
		1, light_diffuse );

  glUniform1f( glGetUniformLocation(program, "Shininess"),
	       material_shininess );

  glUniform1iv( glGetUniformLocation(program, "FlatShading"),
		1, &FlatShading );


  ModelView = glGetUniformLocation( program, "ModelView" );
  Projection = glGetUniformLocation( program, "Projection" );

  if (just_lines) {
    glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
  } else {
    glEnable( GL_DEPTH_TEST );
    glShadeModel(GL_FLAT);
  }

  glClearColor( 0.5, 0.5, 0.5, 1.0 );
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

  mat4 projection = Frustum( left, right, bottom, top, zNear, zFar );
  glUniformMatrix4fv( Projection, 1, GL_TRUE, projection );
}

//----------------------------------------------------------------------------

void
display( void )
{
  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  mat4 model_view = mat4(1.0);
  model_view = model_view * Translate(translate_x,
				      translate_y,
				      translate_z);
  model_view = model_view * RotateX(rotate_x);
  model_view = model_view * RotateY(rotate_y);
  model_view = model_view * RotateZ(rotate_z);
  glUniformMatrix4fv(ModelView, 1, GL_TRUE, model_view);

  reshape(glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT));

  glDrawArrays( GL_LINES, 0, NumAxesPoints );
  glDrawArrays( GL_POINTS, NumAxesPoints, NumControlVertices );
  glDrawArrays( GL_TRIANGLES, NumAxesPoints + NumControlVertices, NumVertices );
  glFlush();
  glutSwapBuffers();
}

void
alter_sampling(int delta) {
  NumTimesToSubdivide += delta;
  if (NumTimesToSubdivide == 0) {
    NumTimesToSubdivide = 1;
  }
  init();
  glutPostWindowRedisplay(mainWindow);
}

//----------------------------------------------------------------------------

void
keyboard( unsigned char key, int x, int y )
{
  switch ( key ) {
  case 'q':
    translate_x -= translate_dt;
    glutPostWindowRedisplay(mainWindow);
    break;
  case 'Q':
    translate_x += translate_dt;
    glutPostWindowRedisplay(mainWindow);
    break;
  case 'w':
    translate_y -= translate_dt;
    glutPostWindowRedisplay(mainWindow);
    break;
  case 'W':
    translate_y += translate_dt;
    glutPostWindowRedisplay(mainWindow);
    break;
  case 'e':
    translate_z -= translate_dt;
    glutPostWindowRedisplay(mainWindow);
    break;
  case 'E':
    translate_z += translate_dt;
    glutPostWindowRedisplay(mainWindow);
    break;
  case 'a':
    rotate_x -= translate_dt;
    glutPostWindowRedisplay(mainWindow);
    break;
  case 'A':
    rotate_x += translate_dt;
    glutPostWindowRedisplay(mainWindow);
    break;
  case 's':
    rotate_y -= translate_dt;
    glutPostWindowRedisplay(mainWindow);
    break;
  case 'S':
    rotate_y += translate_dt;
    glutPostWindowRedisplay(mainWindow);
    break;
  case 'd':
    rotate_z -= translate_dt;
    glutPostWindowRedisplay(mainWindow);
    break;
  case 'D':
    rotate_z += translate_dt;
    glutPostWindowRedisplay(mainWindow);
    break;
  case 'X':
    vertices[selected_control_vertex][X] += control_point_dt;
    init();
    glutPostWindowRedisplay(mainWindow);
    break;
  case 'x':
    vertices[selected_control_vertex][X] -= control_point_dt;
    init();
    glutPostWindowRedisplay(mainWindow);
    break;
  case 'Y':
    vertices[selected_control_vertex][Y] += control_point_dt;
    init();
    glutPostWindowRedisplay(mainWindow);
    break;
  case 'y':
    vertices[selected_control_vertex][Y] -= control_point_dt;
    init();
    glutPostWindowRedisplay(mainWindow);
    break;
  case 'Z':
    vertices[selected_control_vertex][Z] += control_point_dt;
    init();
    glutPostWindowRedisplay(mainWindow);
    break;
  case 'z':
    vertices[selected_control_vertex][Z] -= control_point_dt;
    init();
    glutPostWindowRedisplay(mainWindow);
    break;
  case 'V':
    selected_control_vertex++;
    if (selected_control_vertex > NumControlVertices)
      {selected_control_vertex = NumControlVertices;}
    init();
    glutPostWindowRedisplay(mainWindow);
    break;
  case 'v':
    selected_control_vertex--;
    if (selected_control_vertex < 0)
      {selected_control_vertex = 0;}
    init();
    glutPostWindowRedisplay(mainWindow);
    break;
  case '+':
    alter_sampling(1);
    break;
  case '-':
    alter_sampling(-1);
    break;
  }
}

void
myidle ( void )
{
  t += dt;
  glutPostWindowRedisplay(mainWindow);
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

void
processMenuEvents(int menuChoice)
{
  switch (menuChoice) {
  case FLAT_SHADING: FlatShading = true; init(); glutPostWindowRedisplay(mainWindow); break;
  case SMOOTH_SHADING: FlatShading = false; init(); glutPostWindowRedisplay(mainWindow); break;
  }
}

void
setupMenu ( void )
{
  glutCreateMenu(processMenuEvents);
  glutAddMenuEntry("Flat Shading",    FLAT_SHADING);
  glutAddMenuEntry("Smooth Shading",  SMOOTH_SHADING);
  glutAttachMenu(GLUT_RIGHT_BUTTON);
}

void printHelp ( void )
{
  printf("%s\n", TITLE);
  printf("Use right-click menu to change shading.\n");
  printf("Keyboard options:\n");
  printf("%c/%c - translate model in x direction\n", 'q', 'Q');
  printf("%c/%c - translate model in y direction\n", 'w', 'W');
  printf("%c/%c - translate model in z direction\n", 'e', 'E');
  printf("%c/%c - rotate around the x axes\n", 'a', 'A');
  printf("%c/%c - rotate around the y axes\n", 's', 'S');
  printf("%c/%c - rotate around the z axes\n", 'd', 'D');
  printf("%c/%c - move control vertex in the x direction\n", 'x', 'X');
  printf("%c/%c - move control vertex in the y direction\n", 'y', 'Y');
  printf("%c/%c - move control vertex in the z direction\n", 'z', 'Z');
  printf("%c/%c - change selected control vertex\n", 'v', 'V');
  printf("%c/%c - increase/decrease sample size\n", '+', '-');
}

int
main( int argc, char *argv[] )
{
  glutInit( &argc, argv );
  glutInitDisplayMode( GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH );
  glutInitWindowSize( 512, 512 );
  mainWindow = glutCreateWindow( TITLE );

  glewExperimental=GL_TRUE;
  glewInit();

  if (argc > 1) {
    read_patchfile(argv[1]);
  } else {
    read_patchfile("models/patchPoints.txt");
  }
  init();

  setupMenu();

  glutIdleFunc( myidle );
  glutDisplayFunc( display );
  glutReshapeFunc( reshape );
  glutKeyboardFunc( keyboard );

  glEnable(GL_DEPTH_TEST);
  glEnable(GL_PROGRAM_POINT_SIZE);

  printHelp();

  glutMainLoop();
  return 0;
}
