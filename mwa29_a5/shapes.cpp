#include "Angel.h"
#include "mat.h"
#include "vec.h"

#include <stdio.h>
#include <string>
#include <vector>

typedef vec3 color3;
typedef vec4 color4;

//--------------------------------------------------------------------------
//----   CONSTANTS    ------------------------------------------------------
//--------------------------------------------------------------------------

const bool DEBUG = true;

const char *TITLE = "mwa29 - CS537 assignment 5";

const char KEY_EYE_UP    = 'q';
const char KEY_EYE_DOWN  = 'a';
const char KEY_EYE_CLOSE = 'w';
const char KEY_EYE_FAR   = 's';
const char KEY_DELTAUP   = 'e';
const char KEY_DELTADOWN = 'd';
const char KEY_STOP      = 'r';
const char KEY_START     = 'f';

const double DELTA_DELTA = 0.001;

const double DEFAULT_DELTA = 0.001;

const char SMF_TYPE_VERTEX = 'v';
const char SMF_TYPE_FACE   = 'f';

const int BOUNDING_BOX_INDEX_LEFT   = 0;
const int BOUNDING_BOX_INDEX_RIGHT  = 1;
const int BOUNDING_BOX_INDEX_BOTTOM = 2;
const int BOUNDING_BOX_INDEX_TOP    = 3;
const int BOUNDING_BOX_INDEX_NEAR   = 4;
const int BOUNDING_BOX_INDEX_FAR    = 5;
const int BOUNDING_BOX_SIZE         = 6;

const color4 DEFAULT_COLOR = color4(1.0, 1.0, 1.0, 1.0);

const int PARALLEL_PROJECTION    = 0;
const int PERSPECTIVE_PROJECTION = 1;

const double EYE_DELTA = 0.25;

//--------------------------------------------------------------------------
//----   GLOBALS      ------------------------------------------------------
//--------------------------------------------------------------------------

int mainWindow;
int menu;
int w = 600, h = 600;

double t  = 0;   // the time variable
double dt = DEFAULT_DELTA; // the delta for time increment

double eye_y = 0.0;
double eye_z = 0.0;

GLint ModelView_loc;
GLint Projection_loc;

std::vector<vec3>   points;
std::vector<vec4>   vertices;
std::vector<vec3>   faces;
std::vector<color4> colors;

// Light0
vec4 diffuse0(1.0, 1.0, 1.0, 1.0);
vec4 ambient0(1.0, 1.0, 1.0, 1.0);
vec4 specular0(1.0, 1.0, 1.0, 1.0);
vec4 light0_pos(1.0, 01.0, 0.0, 1.0);

// Light1
vec4 diffuse1(1.0, 1.0, 1.0, 1.0);
vec4 ambient1(1.0, 1.0, 1.0, 1.0);
vec4 specular1(1.0, 1.0, 1.0, 1.0);
vec4 light1_pos(-1.0, 1.0, 0.0, 1.0);

std::string smf_path("models/frog.smf");

double bounding_box[BOUNDING_BOX_SIZE] = {-1.0, 1.0, -1.0, 1.0, -1.0, 1.0};

int current_projection = PARALLEL_PROJECTION;

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

std::vector<vec4>
get_vertices(std::vector<vec3> points, std::vector<vec3> faces)
{
  if (DEBUG) {
    printf("[DEBUG] Getting vertices.\n");
  }

  std::vector<vec4> vertices;
  for (uint i = 0; i < faces.size(); i++) {
    vertices.push_back(vec4(points.at(faces.at(i).x - 1), 1.0));
    vertices.push_back(vec4(points.at(faces.at(i).y - 1), 1.0));
    vertices.push_back(vec4(points.at(faces.at(i).z - 1), 1.0));
  }

  if (DEBUG) {
    printf("[DEBUG] Vertices gotten: %u.\n", (uint) vertices.size());
  }

  return vertices;
}

double
min_double(double one, double two)
{
  if (one < two) {
    return one;
  } else {
    return two;
  }
}

double
max_double(double one, double two)
{
  if (one > two) {
    return one;
  } else {
    return two;
  }
}

void
calculate_bounding_box(std::vector<vec3> points)
{
  if(DEBUG) {
    printf("[DEBUG] Calculating bounding box.\n");
  }
  double min_num = 5000.0;
  double max_num = -5000.0;
  for (uint i = 0; i < points.size(); i++) {
    vec3 p = points.at(i);
    min_num = min_double(p.x, min_num);
    max_num = max_double(p.x, max_num);
    min_num = min_double(p.y, min_num);
    max_num = max_double(p.y, max_num);
    min_num = min_double(p.z, min_num);
    max_num = max_double(p.z, max_num);
  }
  
  bounding_box[BOUNDING_BOX_INDEX_LEFT]   = min_num;
  bounding_box[BOUNDING_BOX_INDEX_RIGHT]  = max_num;
  bounding_box[BOUNDING_BOX_INDEX_BOTTOM] = min_num;
  bounding_box[BOUNDING_BOX_INDEX_TOP]    = max_num;
  bounding_box[BOUNDING_BOX_INDEX_NEAR]   = min_num;
  bounding_box[BOUNDING_BOX_INDEX_FAR]    = max_num;

  if (DEBUG) {
    printf("[DEBUG] Bounding box calculated\n");
    for (int i = 0; i < BOUNDING_BOX_SIZE; i++) {
      printf("[DEBUG] %f\n", bounding_box[i]);
    }
  }
}

void
calculate_phong_shading_model(std::vector<color4> &colors, std::vector<vec3> points, std::vector<vec3> faces)
{
  if (DEBUG) {
    printf("[DEBUG] Calculating Phong shading model.\n");
  }

  for (uint i = 0; i < faces.size(); i++) {
    GLint index_one   = faces.at(i).x - 1;
    GLint index_two   = faces.at(i).y - 1;
    GLint index_three = faces.at(i).z - 1;
    vec3 p0 = points.at(index_one);
    vec3 p1 = points.at(index_two);
    vec3 p2 = points.at(index_three);

    vec3 n = (p1 - p0) * (p2 - p0);

    color4 diffuse = diffuse0 * max_double(dot(n, normalize(light0_pos)),0.0);
    diffuse += diffuse1 * max_double(dot(n, normalize(light1_pos)),0.0);

    if (DEBUG) {
      printf("[DEBUG] diffuse %f, %f, %f\n", diffuse.x, diffuse.y, diffuse.z);
    }
    colors.push_back(DEFAULT_COLOR * diffuse);
    colors.push_back(DEFAULT_COLOR * diffuse);
    colors.push_back(DEFAULT_COLOR * diffuse);    
  }

  if (DEBUG) {
    printf("[DEBUG] Phong shading model calculated.\n");
  }
}


// Taken from http://stackoverflow.com/questions/5294955/how-to-scale-down-a-range-of-numbers-with-a-known-min-and-max-value
double scale(double valueIn, double baseMin, double baseMax, double limitMin, double limitMax) {
  return ((limitMax - limitMin) * (valueIn - baseMin) / (baseMax - baseMin)) + limitMin;
}

void
scale_colors(std::vector<color4> &colors)
{
  if (DEBUG) {
    printf("[DEBUG] Scaling colors.\n");
  }
  double max_color = -500.0;
  double min_color = 500.0;
  if (DEBUG) {
    printf("[DEBUG] printing colors.\n");
    for(uint i = 0; i < colors.size(); i++) {
      printf("[DEBUG] %f, %f, %f\n", colors.at(i).x, colors.at(i).y, colors.at(i).z);
    }
  }
  for (uint i = 0; i < colors.size(); i++) {
    max_color = max_double(max_color, colors.at(i).x);
    max_color = max_double(max_color, colors.at(i).y);
    max_color = max_double(max_color, colors.at(i).z);
    min_color = min_double(min_color, colors.at(i).x);
    min_color = min_double(min_color, colors.at(i).y);
    min_color = min_double(min_color, colors.at(i).z);
  }

  if (DEBUG) {
    printf("[DEBUG] min_color=%f, max_color=%f\n", min_color, max_color);
  }

  for (uint i = 0; i < colors.size(); i++) {
    colors.at(i).x = scale(colors.at(i).x, min_color, max_color, 0, 1.0);
    colors.at(i).y = scale(colors.at(i).y, min_color, max_color, 0, 1.0);
    colors.at(i).z = scale(colors.at(i).z, min_color, max_color, 0, 1.0);
  }
  if (DEBUG) {
    printf("[DEBUG] Colors scaled.\n");
  }
}

void
init( void )
{

  parse_smf(smf_path, points, faces);

  vertices = get_vertices(points, faces);

  calculate_bounding_box(points);

  std::vector<color4> colors;
  calculate_phong_shading_model(colors, points, faces);

  scale_colors(colors);

  if (DEBUG) {
    printf("[DEBUG] printing points.\n");
    for(uint i = 0; i < points.size(); i++) {
      printf("[DEBUG] %f, %f, %f\n", points.at(i).x, points.at(i).y, points.at(i).z);
    }
    printf("[DEBUG] printing faces.\n");
    for(uint i = 0; i < faces.size(); i++) {
      printf("[DEBUG] %f, %f, %f\n", faces.at(i).x, faces.at(i).y, faces.at(i).z);
    }
    printf("[DEBUG] printing vertices.\n");
    for(uint i = 0; i < vertices.size(); i++) {
      printf("[DEBUG] %f, %f, %f\n", vertices.at(i).x, vertices.at(i).y, vertices.at(i).z);
      if (2 == i % 3) {
	printf("\n");
      }
    }
    printf("[DEBUG] printing colors.\n");
    for(uint i = 0; i < colors.size(); i++) {
      printf("[DEBUG] %f, %f, %f\n", colors.at(i).x, colors.at(i).y, colors.at(i).z);
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
  glBufferData( GL_ARRAY_BUFFER,
		(vertices.size()*sizeof(vec4))
		+ (colors.size()*sizeof(color4)),
		NULL, GL_STATIC_DRAW );

  //load data separately
  glBufferSubData(GL_ARRAY_BUFFER, 0,
		  vertices.size()*sizeof(vec4),
		  &vertices[0]);
  glBufferSubData(GL_ARRAY_BUFFER, vertices.size()*sizeof(vec4),
		  colors.size()*sizeof(color4),
		  &colors[0]);

  // Load shaders and use the resulting shader program
  GLuint program = InitShader( "vshdrcube.glsl", "fshdrcube.glsl" );
  glUseProgram( program );

  // Initialize the vertex position attribute from the vertex shader
  GLuint vPosition_loc = glGetAttribLocation(program, "vPosition");
  glEnableVertexAttribArray(vPosition_loc);
  glVertexAttribPointer(vPosition_loc, 4, GL_FLOAT, GL_FALSE, 0,
			BUFFER_OFFSET(0));
  GLuint vColor_loc = glGetAttribLocation(program, "vColor");
  glEnableVertexAttribArray(vColor_loc);
  glVertexAttribPointer(vColor_loc, 4, GL_FLOAT, GL_FALSE, 0,
			BUFFER_OFFSET(vertices.size()*sizeof(vec4)));

  ModelView_loc = glGetUniformLocation( program, "matrix" );

  Projection_loc = glGetUniformLocation( program, "Projection" );

  glClearColor( 0.75, 0.75, 0.75, 1.0 );
}

vec4
get_eye( void )
{
  double angle = t;
  return vec4(cos(angle),
  	      eye_y,
  	      eye_z+sin(angle),
  	      1.0);
}

vec4
get_eye_at( void )
{
  return vec4((bounding_box[BOUNDING_BOX_INDEX_LEFT]
  	       + bounding_box[BOUNDING_BOX_INDEX_RIGHT]) / 2,
  	      (bounding_box[BOUNDING_BOX_INDEX_BOTTOM]
  	       + bounding_box[BOUNDING_BOX_INDEX_TOP]) / 2,
  	      (bounding_box[BOUNDING_BOX_INDEX_NEAR]
  	       + bounding_box[BOUNDING_BOX_INDEX_FAR]) / 2,
  	      1.0);
}

vec4
get_up( vec4 eye )
{
  return vec4(eye.x, 1.0, eye.z, 1.0);
}

void
display( void )
{
  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

  // LookAt(eye, at, up)
  vec4 eye    = get_eye();
  vec4 eye_at = get_eye_at();
  vec4 eye_up = get_up(eye);
  mat4 model_view = LookAt(eye, eye_at, eye_up);

  mat4 projection;
  if (PARALLEL_PROJECTION == current_projection) {
    //Ortho(left, right, bottom, top, near, far);
    projection = Ortho(bounding_box[BOUNDING_BOX_INDEX_LEFT],
		       bounding_box[BOUNDING_BOX_INDEX_RIGHT],
		       bounding_box[BOUNDING_BOX_INDEX_BOTTOM],
		       bounding_box[BOUNDING_BOX_INDEX_TOP],
		       bounding_box[BOUNDING_BOX_INDEX_NEAR],
		       bounding_box[BOUNDING_BOX_INDEX_FAR]);
  } else {
    projection = Frustum(bounding_box[BOUNDING_BOX_INDEX_LEFT],
			 bounding_box[BOUNDING_BOX_INDEX_RIGHT],
			 bounding_box[BOUNDING_BOX_INDEX_BOTTOM],
			 bounding_box[BOUNDING_BOX_INDEX_TOP],
			 bounding_box[BOUNDING_BOX_INDEX_NEAR],
			 bounding_box[BOUNDING_BOX_INDEX_FAR]);
  }

  glUniformMatrix4fv(ModelView_loc, 1, GL_TRUE, model_view);

  glUniformMatrix4fv(Projection_loc, 1, GL_TRUE, projection);

  glDrawArrays( GL_TRIANGLES, 0, vertices.size() );

  glFlush();
  glutSwapBuffers();
}

void
myidle ( void )
{
  t += dt;
  glutPostRedisplay();
}

void
keyboard( unsigned char key, int x, int y )
{
  switch (key) {
  case KEY_EYE_UP:    eye_y += EYE_DELTA;   break;
  case KEY_EYE_DOWN:  eye_y -= EYE_DELTA;   break;
  case KEY_EYE_CLOSE: eye_z += EYE_DELTA;   break;
  case KEY_EYE_FAR:   eye_z -= EYE_DELTA;   break;
  case KEY_DELTAUP:   dt    += DELTA_DELTA; break;
  case KEY_DELTADOWN: dt    -= DELTA_DELTA; break;
  case KEY_STOP:      glutIdleFunc(NULL);   break;
  case KEY_START:     glutIdleFunc(myidle); break;
  }
  glutPostWindowRedisplay(mainWindow);
}

void
processMenuEvents(int menuChoice)
{
  current_projection = menuChoice;
}

void
setupMenu ( void )
{
  glutCreateMenu(processMenuEvents);
  glutAddMenuEntry("Parallel projection",    PARALLEL_PROJECTION);
  glutAddMenuEntry("Perspective projection", PERSPECTIVE_PROJECTION);
  glutAttachMenu(GLUT_RIGHT_BUTTON);
}

void
printHelp ( void ) {
  printf("%s\n", TITLE);
  printf("Keyboard options:\n");
  printf("q - move eye up\n");
  printf("a - move eye down\n");
  printf("w - move eye closer\n");
  printf("s - move eye farther\n");
  printf("e - spin eye faster\n");
  printf("d - spin eye slower\n");
  printf("r - stop movement\n");
  printf("f - start movement\n");
  printf("t - reset all transformations and deltas\n");
}

int
main( int argc, char **argv )
{
  if (argc > 1) {
    smf_path = std::string(argv[1]);
  }
  if (DEBUG) {
    printf("[DEBUG] Filepath: %s\n", smf_path.c_str());
  }

  printHelp();

  glutInit( &argc, argv );
  glutInitDisplayMode( GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH );
  glutInitWindowPosition(100,100);
  glutInitWindowSize( w, h );

  mainWindow = glutCreateWindow( (std::string(TITLE) + " " + smf_path).c_str() );

  glewExperimental=GL_TRUE;
  glewInit();

  init();
  setupMenu();
  glutDisplayFunc ( display  );
  glutKeyboardFunc( keyboard );
  glutIdleFunc    ( myidle );

  glEnable(GL_DEPTH_TEST);
  glutMainLoop();
  return 0;
}
