#include "Angel.h"
#include "mat.h"
#include "vec.h"

#include <stdio.h>
#include <string>
#include <vector>

typedef vec3 color3;
typedef vec4 color4;
typedef vec4 point4;

//--------------------------------------------------------------------------
//----   CONSTANTS    ------------------------------------------------------
//--------------------------------------------------------------------------

const char *TITLE = "mwa29 - CS537 assignment 6";

const int  DEBUG_MAX_FACES = 1500;

const char KEY_EYE_UP    = 'q';
const char KEY_EYE_DOWN  = 'a';
const char KEY_EYE_CLOSE = 'w';
const char KEY_EYE_FAR   = 's';
const char KEY_DELTAUP   = 'e';
const char KEY_DELTADOWN = 'd';
const char KEY_STOP      = 'r';
const char KEY_START     = 'f';

const double DELTA_DELTA = 0.001;

const double DEFAULT_DELTA = 0.1;

const char SMF_TYPE_VERTEX = 'v';
const char SMF_TYPE_FACE   = 'f';

const int BOUNDING_BOX_INDEX_LEFT   = 0;
const int BOUNDING_BOX_INDEX_RIGHT  = 1;
const int BOUNDING_BOX_INDEX_BOTTOM = 2;
const int BOUNDING_BOX_INDEX_TOP    = 3;
const int BOUNDING_BOX_INDEX_NEAR   = 4;
const int BOUNDING_BOX_INDEX_FAR    = 5;
const int BOUNDING_BOX_SIZE         = 6;

const color4 DEFAULT_COLOR = color4(0.0, 1.0, 0.0, 1.0);

const int PARALLEL_PROJECTION    = 0;
const int PERSPECTIVE_PROJECTION = 1;
const int PHONG_SHADING          = 2;
const int GOURAUD_SHADING        = 3;

const double EYE_DELTA = 0.25;

const double MAX_EYE_THETA = 360;
const double MIN_EYE_THETA = 0;
const double MAX_EYE_PHI   = 180;
const double MIN_EYE_PHI   = -180;

//--------------------------------------------------------------------------
//----   GLOBALS      ------------------------------------------------------
//--------------------------------------------------------------------------

bool debug = false;

int mainWindow;
int menu;
int w = 600, h = 600;

double t  = 0;   // the time variable
double dt = DEFAULT_DELTA; // the delta for time increment

double eye_radius = 0.0;
double eye_theta  = 0.0;
double eye_phi    = 0.0;

GLint ModelView_loc;
GLint Projection_loc;

std::vector<vec3>   points;
std::vector<vec4>   vertices;
std::vector<vec4>   normals;
std::vector<vec3>   faces;
std::vector<color4> colors;

std::string smf_path("models/cube.smf");

double bounding_box[BOUNDING_BOX_SIZE] = {-1.0, 1.0, -1.0, 1.0, -1.0, 1.0};

int current_projection = PARALLEL_PROJECTION;
int current_shading    = PHONG_SHADING;

//--------------------------------------------------------------------------

void
parse_smf(std::string file_path, std::vector<vec3> &vertices, std::vector<vec3> &faces) {
  if (debug) {
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

  if (debug) {
    printf("[DEBUG] Parse complete.\n");
  }
}

std::vector<vec4>
get_vertices(std::vector<vec3> points, std::vector<vec3> faces)
{
  if (debug) {
    printf("[DEBUG] Getting vertices.\n");
  }

  std::vector<vec4> vertices;
  for (uint i = 0; i < faces.size(); i++) {
    vertices.push_back(vec4(points.at(faces.at(i).x - 1), 1.0));
    vertices.push_back(vec4(points.at(faces.at(i).y - 1), 1.0));
    vertices.push_back(vec4(points.at(faces.at(i).z - 1), 1.0));
  }

  if (debug) {
    printf("[DEBUG] Vertices gotten: %u.\n", (uint) vertices.size());
  }

  return vertices;
}

std::vector<vec4>
get_normals(std::vector<vec3> points, std::vector<vec3> faces)
{
  std::vector<vec4>   normals;
  for (uint i = 0; i < faces.size(); i++) {
    vec4 p0 = points.at(faces.at(i).x - 1);
    vec4 p1 = points.at(faces.at(i).y - 1);
    vec4 p2 = points.at(faces.at(i).z - 1);

    vec4 u = p1 - p0;
    vec4 v = p2 - p0;

    vec3 normal = normalize( cross(u, v) );

    normals.push_back(normal);
  }
  return normals;
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
  if(debug) {
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

  min_num = min_double(-1.0, min_num); // don't go lower than -1
  max_num = max_double( 1.0, max_num); // don't go higher than 1

  bounding_box[BOUNDING_BOX_INDEX_LEFT]   = min_num;
  bounding_box[BOUNDING_BOX_INDEX_RIGHT]  = max_num;
  bounding_box[BOUNDING_BOX_INDEX_BOTTOM] = min_num;
  bounding_box[BOUNDING_BOX_INDEX_TOP]    = max_num;
  bounding_box[BOUNDING_BOX_INDEX_NEAR]   = min_num;
  bounding_box[BOUNDING_BOX_INDEX_FAR]    = max_num;

  if (debug) {
    printf("[DEBUG] Bounding box calculated\n");
    for (int i = 0; i < BOUNDING_BOX_SIZE; i++) {
      printf("[DEBUG] %f\n", bounding_box[i]);
    }
  }
}

void
read_smf ( void )
{
  parse_smf(smf_path, points, faces);

  if (debug && faces.size() > DEBUG_MAX_FACES) {
    printf("[DEBUG] Number of faces (%u) are greater than '%d', disabling DEBUG \n", (uint) faces.size(), DEBUG_MAX_FACES);
    debug = false;
  }

  vertices = get_vertices(points, faces);

  normals = get_normals(points, faces);

  calculate_bounding_box(points);

  if (debug) {
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
}

void
init( void )
{
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
		+ (colors.size()*sizeof(color4))
		+ (normals.size()*sizeof(vec4)),
		NULL, GL_STATIC_DRAW );

  //load data separately
  glBufferSubData(GL_ARRAY_BUFFER, 0,
		  vertices.size()*sizeof(vec4),
		  &vertices[0]);
  glBufferSubData(GL_ARRAY_BUFFER, vertices.size()*sizeof(vec4),
		  colors.size()*sizeof(color4),
		  &colors[0]);
  glBufferSubData(GL_ARRAY_BUFFER, vertices.size()*sizeof(vec4)
		  + colors.size()*sizeof(color4),
		  normals.size()*sizeof(vec4),
		  &normals[0]);

  // Load shaders and use the resulting shader program
  //  GLuint program = InitShader( "vshdrcube.glsl", "fshdrcube.glsl" );
  GLuint program;
  if (current_shading == PHONG_SHADING) {
    program = InitShader( "vshader53.glsl", "fshader53.glsl" );
  } else {
    program = InitShader( "vshader56.glsl", "fshader56.glsl" );
  }
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
  GLuint vNormal_loc = glGetAttribLocation(program, "vNormal");
  glEnableVertexAttribArray(vNormal_loc);
  glVertexAttribPointer(vNormal_loc, 4, GL_FLOAT, GL_FALSE, 0,
			BUFFER_OFFSET(normals.size()*sizeof(vec4)));

  // Initialize shader lighting parameters
  point4 light_position( 0.0, 0.0, -1.0, 0.0 );
  color4 light_ambient( 0.2, 0.2, 0.2, 1.0 );
  color4 light_diffuse( 1.0, 1.0, 1.0, 1.0 );
  color4 light_specular( 1.0, 1.0, 1.0, 1.0 );

  color4 material_ambient( 1.0, 0.0, 1.0, 1.0 );
  color4 material_diffuse( 1.0, 0.8, 0.0, 1.0 );
  color4 material_specular( 1.0, 0.8, 0.0, 1.0 );
  float  material_shininess = 100.0;

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

  glUniform1f( glGetUniformLocation(program, "Shininess"),
	       material_shininess );
		 
  // Retrieve transformation uniform variable locations
  ModelView_loc = glGetUniformLocation( program, "ModelView" );
  Projection_loc = glGetUniformLocation( program, "Projection" );

  glEnable( GL_DEPTH_TEST );

  glShadeModel(GL_FLAT);

  glClearColor( 0.75, 0.75, 0.75, 1.0 );
}

vec4
get_eye( void )
{
  vec4 eye(0.0,
	   0.0,
	   1.0,
	   1.0);

  mat4 eye_transform(1.0);

  vec4 centroid((bounding_box[BOUNDING_BOX_INDEX_LEFT]
		 + bounding_box[BOUNDING_BOX_INDEX_RIGHT]) / 2,
		(bounding_box[BOUNDING_BOX_INDEX_BOTTOM]
		 + bounding_box[BOUNDING_BOX_INDEX_TOP]) / 2,
		(bounding_box[BOUNDING_BOX_INDEX_NEAR]
		 + bounding_box[BOUNDING_BOX_INDEX_FAR]) / 2);

  eye_transform = Translate(-centroid) * eye_transform;

  eye_transform = Translate(0.0,0.0,eye_radius) * eye_transform;
  eye_transform = RotateX(eye_phi) * eye_transform;
  eye_transform = RotateY(eye_theta) * eye_transform;

  eye_transform = Translate(centroid) * eye_transform;

  return eye_transform * eye;
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
  if (PARALLEL_PROJECTION == current_projection) {
    return vec4(eye.x, 1.0, eye.z, 1.0);
  } else {
    return vec4(eye.x, -1.0, eye.z, 1.0);
  }
}

void
display( void )
{
  init();
  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

  // LookAt(eye, at, up)
  vec4 eye    = get_eye();
  vec4 eye_at = get_eye_at();
  vec4 eye_up = get_up(eye);
  mat4 model_view = LookAt(eye, eye_at, eye_up);

  mat4 projection;
  if (PARALLEL_PROJECTION == current_projection) {
    //Ortho(left, right, bottom, top, near, far);
    projection = Ortho(bounding_box[BOUNDING_BOX_INDEX_LEFT]
		       - bounding_box[BOUNDING_BOX_INDEX_RIGHT],
		       bounding_box[BOUNDING_BOX_INDEX_RIGHT],
		       bounding_box[BOUNDING_BOX_INDEX_BOTTOM]
		       - bounding_box[BOUNDING_BOX_INDEX_TOP],
		       bounding_box[BOUNDING_BOX_INDEX_TOP],
		       bounding_box[BOUNDING_BOX_INDEX_NEAR]
		       - bounding_box[BOUNDING_BOX_INDEX_FAR],
		       bounding_box[BOUNDING_BOX_INDEX_FAR]);
  } else {
    projection = Frustum(bounding_box[BOUNDING_BOX_INDEX_LEFT]
			 - bounding_box[BOUNDING_BOX_INDEX_RIGHT],
			 bounding_box[BOUNDING_BOX_INDEX_RIGHT],
			 bounding_box[BOUNDING_BOX_INDEX_BOTTOM]
			 - bounding_box[BOUNDING_BOX_INDEX_TOP],
			 bounding_box[BOUNDING_BOX_INDEX_TOP],
			 bounding_box[BOUNDING_BOX_INDEX_NEAR]
			 - bounding_box[BOUNDING_BOX_INDEX_FAR],
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
  eye_theta += dt;
  if (eye_theta > MAX_EYE_THETA) {
    eye_theta = MIN_EYE_THETA;
  }
  glutPostRedisplay();
}

void
keyboard( unsigned char key, int x, int y )
{
  switch (key) {
  case KEY_EYE_UP:    eye_phi += EYE_DELTA;   break;
  case KEY_EYE_DOWN:  eye_phi -= EYE_DELTA;   break;
  case KEY_EYE_CLOSE: eye_radius += EYE_DELTA;   break;
  case KEY_EYE_FAR:   eye_radius -= EYE_DELTA;   break;
  case KEY_DELTAUP:   dt    += DELTA_DELTA; break;
  case KEY_DELTADOWN: dt    -= DELTA_DELTA; break;
  case KEY_STOP:      glutIdleFunc(NULL);   break;
  case KEY_START:     glutIdleFunc(myidle); break;
  }

  if (eye_phi < MIN_EYE_PHI) {
    eye_phi  = MIN_EYE_PHI;
  }
  if (eye_phi > MAX_EYE_PHI) {
    eye_phi  = MAX_EYE_PHI;
  }

  glutPostWindowRedisplay(mainWindow);
}

void
processMenuEvents(int menuChoice)
{
  switch (menuChoice) {
  case PARALLEL_PROJECTION: current_projection = PARALLEL_PROJECTION; break;
  case PERSPECTIVE_PROJECTION: current_projection = PERSPECTIVE_PROJECTION; break;
  case PHONG_SHADING: current_shading = PHONG_SHADING; break;
  case GOURAUD_SHADING: current_shading = GOURAUD_SHADING; break;
  }
}

void
setupMenu ( void )
{
  glutCreateMenu(processMenuEvents);
  glutAddMenuEntry("Parallel projection",    PARALLEL_PROJECTION);
  glutAddMenuEntry("Perspective projection", PERSPECTIVE_PROJECTION);
  glutAddMenuEntry("Phong shading",          PHONG_SHADING);
  glutAddMenuEntry("Gouraud shading",        GOURAUD_SHADING);
  glutAttachMenu(GLUT_RIGHT_BUTTON);
}

void
printHelp ( void ) {
  printf("%s\n", TITLE);
  printf("Use right-click menu to change perspective.\n");
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
  if (debug) {
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

  read_smf();
  init();
  setupMenu();
  glutDisplayFunc ( display  );
  glutKeyboardFunc( keyboard );
  glutIdleFunc    ( myidle );

  glEnable(GL_DEPTH_TEST);
  glutMainLoop();
  return 0;
}
