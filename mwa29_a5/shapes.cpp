#include "Angel.h"
#include "mat.h"
#include "vec.h"

#include <stdio.h>
#include <string>
#include <vector>

typedef vec4 point4;
typedef vec4 color4;
typedef vec3 color3;

//--------------------------------------------------------------------------
//----   CONSTANTS    ------------------------------------------------------
//--------------------------------------------------------------------------

const bool DEBUG = true;

const char *TITLE = "mwa29 - CS537 assignment 4";

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

const char SMF_TYPE_VERTEX = 'v';
const char SMF_TYPE_FACE   = 'f';

//--------------------------------------------------------------------------
//----   GLOBALS      ------------------------------------------------------
//--------------------------------------------------------------------------

int mainWindow;
int menu;
int w = 600, h = 600;

GLint AmbientProduct_loc, DiffuseProduct_loc, SpecularProduct_loc;
GLint ModelView_loc;
GLint Projection_loc;
GLint LightPosition_loc;
GLint Shininess_loc;

std::vector<vec3>   points;
std::vector<vec3>   vertices;
std::vector<vec3>   faces;
std::vector<vec3>   normals;
std::vector<color3> colors;

// Light0
vec4 diffuse0(1.0, 1.0, 1.0, 1.0);
vec4 ambient0(1.0, 1.0, 1.0, 1.0);
vec4 specular0(1.0, 1.0, 1.0, 1.0);
vec4 light0_pos(1.0, 2.0, 3.0);
float shininess = 100.0;

std::string smf_path("models/box.smf");

//--------------------------------------------------------------------------

void
parse_smf(std::string file_path, std::vector<vec3> &vertices, std::vector<vec3> &faces) {
  if (DEBUG) {
    printf("[DEBUG] Parsing smf file: %s\n", file_path.c_str());
  }

  FILE * file;

  file = fopen (file_path.c_str(),"r");
  if (NULL != file) {
    char line[1000];
    while (fgets ( line, sizeof line, file ) != NULL) {
      if (SMF_TYPE_VERTEX == line[0]) {
	double first;
	double second;
	double third;
	sscanf(line, "v %lf %lf %lf\n", &first, &second, &third);
	vertices.push_back(vec3(first, second, third));
      } else if (SMF_TYPE_FACE == line[0]) {
	int first;
	int second;
	int third;
	sscanf(line, "f %d %d %d\n", &first, &second, &third);
	faces.push_back(vec3(first, second, third));
      }
    }
    fclose (file);
  }

  if (DEBUG) {
    printf("[DEBUG] Parse complete.\n");
  }
}

std::vector<vec3>
calculate_normals(std::vector<vec3> vertices, std::vector<vec3> faces)
{
  // n = (p2 - p0) × (p1 - p0)
  std::vector<vec3> normals(vertices.size(), vec3(0.0,0.0,0.0));
  for (uint i = 0; i < faces.size(); i++) {
    GLint index_one   = faces.at(i).x - 1;
    GLint index_two   = faces.at(i).y - 1;
    GLint index_three = faces.at(i).z - 1;

    vec3 p0 = vertices.at(index_one);
    vec3 p1 = vertices.at(index_two);
    vec3 p2 = vertices.at(index_three);

    vec3 n = (p2 - p0) * (p1 - p0);

    normals.at(index_one)   += n;
    normals.at(index_two)   += n;
    normals.at(index_three) += n;
  }

  for (uint i = 0; i < normals.size(); i++) {
    normals.at(i) = normalize(normals.at(i));
  }

  return normals;
}

std::vector<vec3>
get_vertices(std::vector<vec3> points, std::vector<vec3> faces)
{
  if (DEBUG) {
    printf("[DEBUG] Getting vertices.\n");
  }

  std::vector<vec3> vertices;
  for (uint i = 0; i < faces.size(); i++) {
    vertices.push_back(points.at(faces.at(i).x - 1));
    vertices.push_back(points.at(faces.at(i).y - 1));
    vertices.push_back(points.at(faces.at(i).z - 1));
  }

  if (DEBUG) {
    printf("[DEBUG] Vertices gotten: %u.\n", (uint) vertices.size());
  }

  return vertices;
}

void
init( void )
{
  parse_smf(smf_path, points, faces);

  vertices = get_vertices(points, faces);

  normals = calculate_normals(points, faces);

  colors = std::vector<color3>(vertices.size(), color3(1.0,0.0,0.0));
  
  if (DEBUG) {
    printf("[DEBUG] printing points.\n");
    for(uint i = 0; i < points.size(); i++) {
      printf("%f, %f, %f\n", points.at(i).x, points.at(i).y, points.at(i).z);
    }
    printf("[DEBUG] printing faces.\n");
    for(uint i = 0; i < faces.size(); i++) {
      printf("%f, %f, %f\n", faces.at(i).x, faces.at(i).y, faces.at(i).z);
    }
    printf("[DEBUG] printing normals.\n");
    for(uint i = 0; i < normals.size(); i++) {
      printf("%f, %f, %f\n", normals.at(i).x, normals.at(i).y, normals.at(i).z);
    }
    printf("[DEBUG] printing vertices.\n");
    for(uint i = 0; i < vertices.size(); i++) {
      printf("%f, %f, %f\n", vertices.at(i).x, vertices.at(i).y, vertices.at(i).z);
    }
    printf("[DEBUG] printing colors.\n");
    for(uint i = 0; i < colors.size(); i++) {
      printf("%f, %f, %f\n", colors.at(i).x, colors.at(i).y, colors.at(i).z);
    }
  }

  // Create a vertex array object
  GLuint vao[1];
  glGenVertexArrays( 1, vao );
  glBindVertexArray( vao[0] );

  // Create and initialize a buffer object
  GLuint buffer;
  glGenBuffers( 1, &buffer );
  glBindBuffer( GL_ARRAY_BUFFER, buffer );
  glBufferData( GL_ARRAY_BUFFER, vertices.size() + colors.size() + normals.size(),
		NULL, GL_STATIC_DRAW );

  //load data separately
  glBufferSubData(GL_ARRAY_BUFFER, 0,
		  vertices.size(), &vertices[0]);
  glBufferSubData(GL_ARRAY_BUFFER, vertices.size(),
		  colors.size(), &colors[0]);
  glBufferSubData(GL_ARRAY_BUFFER, vertices.size() + colors.size(),
		  colors.size(), &normals[0]);

  // Load shaders and use the resulting shader program
  //GLuint program = InitShader( "vshdrcube.glsl", "fshdrcube.glsl" );
  GLuint program = InitShader( "vshdrgouraud.glsl", "fshdrgouraud.glsl" );
  glUseProgram( program );

  // Initialize the vertex position attribute from the vertex shader
  GLuint vPosition_loc = glGetAttribLocation(program, "vPosition");
  glEnableVertexAttribArray(vPosition_loc);
  glVertexAttribPointer(vPosition_loc, 3, GL_FLOAT, GL_FALSE, 0,
			BUFFER_OFFSET(0));
  GLuint vColor_loc = glGetAttribLocation(program, "vColor");
  glEnableVertexAttribArray(vColor_loc);
  glVertexAttribPointer(vColor_loc, 3, GL_FLOAT, GL_FALSE, 0,
			BUFFER_OFFSET(vertices.size()));
  GLuint vNormal_loc = glGetAttribLocation(program, "vNormal");
  glEnableVertexAttribArray(vNormal_loc);
  glVertexAttribPointer(vNormal_loc, 3, GL_FLOAT, GL_FALSE, 0,
			BUFFER_OFFSET(vertices.size() + colors.size()));

  AmbientProduct_loc  = glGetUniformLocation( program, "AmbientProduct" );
  DiffuseProduct_loc  = glGetUniformLocation( program, "DiffuseProduct" );
  SpecularProduct_loc = glGetUniformLocation( program, "SpecularProduct" );
  ModelView_loc       = glGetUniformLocation( program, "ModelView" );
  Projection_loc      = glGetUniformLocation( program, "Projection" );
  LightPosition_loc   = glGetUniformLocation( program, "LightPosition" );
  Shininess_loc       = glGetUniformLocation( program, "Shininess" );

  glClearColor( 1.0, 1.0, 1.0, 1.0 );
}

void
display( void )
{
  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

  // mat4 model_view(1.0,0.0,0.0,0.0,
  // 		  0.0,1.0,0.0,0.0,
  // 		  0.0,0.0,1.0,0.0,
  // 		  0.0,0.0,0.0,1.0);

  // LookAt(eye, at, up)
  mat4 model_view = LookAt(vec4(0.0,0.0,-1.0,1.0),
  			   vec4(0.0,0.0,0.0,1.0),
  			   vec4(1.0,1.0,1.0,1.0));

  //Ortho(left, right, bottom, top, near, far);
  mat4 projection = Ortho(-1.0, 1.0, -1.0, 1.0, 0.0, 1.0);
  //Perspective(fov, aspect, near, far);
  // projection = Perspective(1.0, 1.0, 0.0, 1.0);

  vec3 diffuse_product  = vec3(diffuse0.x,  diffuse0.y,  diffuse0.z);
  vec3 ambient_product  = vec3(ambient0.x,  ambient0.y,  ambient0.z);
  vec3 specular_product = vec3(specular0.x, specular0.y, specular0.z);

  glUniform3fv(DiffuseProduct_loc,  1, diffuse_product);
  glUniform3fv(AmbientProduct_loc,  1, ambient_product);
  glUniform3fv(SpecularProduct_loc, 1, specular_product);

  glUniformMatrix4fv(ModelView_loc, 1, GL_TRUE, model_view);

  glUniformMatrix4fv(Projection_loc, 1, GL_TRUE, projection);

  glUniform3fv(LightPosition_loc, 1, light0_pos);

  glUniform1f(Shininess_loc, shininess);

  glDrawArrays( GL_TRIANGLES, 0, vertices.size());

  glFlush();
  glutSwapBuffers();
}

void
keyboard( unsigned char key, int x, int y )
{
  glutPostWindowRedisplay(mainWindow);
}

void
processMenuEvents(int menuChoice)
{

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
myidle ( void ) {
  
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
  if (argc > 1) {
    smf_path = std::string(argv[1]);
  }
  printf("%s\n", smf_path.c_str());

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
  glutIdleFunc( myidle );

  printHelp();

  glEnable(GL_DEPTH_TEST);
  glutMainLoop();
  return 0;
}
