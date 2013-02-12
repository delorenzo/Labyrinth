#pragma once
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
// may need to change GLUT/glut.h to GL/glut.h on PC or Linux
#include "GLee.h";
#include <GL/glut.h>
#include <fcntl.h>
#include <errno.h>


struct vertex{
	GLfloat x;
	GLfloat y;
	GLfloat z;
	GLfloat w;
};
void my_display();
void my_reshape(int w, int h);
void my_keyboard( unsigned char key, int x, int y );
void my_setup();
void draw_quad(vertex vertices[51][51], int i, int j, int ic);
vertex matrix_multiply(vertex A[4], vertex B);
void make_sphere( double ray, int rs, int vs );
void normalize(GLfloat *p);
void special_keyboard(int key, int x, int y);
void make_cube_smart( double size );
void draw_cube();
void real_rotation(GLfloat deg, GLfloat x, GLfloat y, GLfloat z);
void floor_detect(float *rayStart, float *rayDirection, float result[3]);

void make_plane_smart( double size );
void draw_plane();
void real_translation(GLfloat x, GLfloat y, GLfloat z, int shape);
void draw_sphere();
void calc_points(vertex rayStart,  vertex rayDirection, float t, float p[3]);
void make_wall_smart(int i, double size, double length, double width);
void real_translation(GLfloat x, GLfloat y, GLfloat z, int shape, int wallindex);
void draw_walls();
void rotate_sphere(GLfloat deg, GLfloat x, GLfloat y, GLfloat z);
void set_up_cam();
void calc_ball_velocity ();
void my_TimeOut(int id) ;
void check_intersections();
void make_hole() ;
void draw_holes();
void make_holes();
void draw_triangle(vertex v, vertex w, vertex z, int ic);
void lighting_setup ();
void my_Intersect(int id) ;
void rest();
void bmp2rgb(GLubyte img[], int size) ;
void load_bmp(char *fname, GLubyte img[], int size, GLuint *ptname) ;
void normalize(vertex v);
vertex subtract(vertex a, vertex b);
void draw_quad(vertex vertices[], int iv1, int iv2, int iv3, int iv4, int ic);


void printINfoLog(GLhandleARB obj);
void setShaders();
char *textFileRead(char *fn);


void make_walls () ;

#define BLACK   0
#define RED     1
#define YELLOW  2
#define MAGENTA 3
#define GREEN   4
#define CYAN    5
#define BLUE    6
#define GREY    7
#define WHITE   8
#define BROWN   9
#define DBROWN 10

#define BOX 0
#define SPHERE 1
#define PLANE 2
#define WALL 3
#define HOLE 4


GLfloat colors [][3] = {
  {0.0, 0.0, 0.0},  /* black   */
  {1.0, 0.0, 0.0},  /* red     */
  {1.0, 1.0, 0.0},  /* yellow  */
  {1.0, 0.0, 1.0},  /* magenta */
  {0.0, 1.0, 0.0},  /* green   */
  {0.0, 1.0, 1.0},  /* cyan    */
  {0.0, 0.0, 1.0},  /* blue    */
  {0.5, 0.5, 0.5},  /* 50%grey */
  {1.0, 1.0, 1.0},   /* white   */
  {0.933333, 0.866667, 0.509804}, //light brown
  {0.721569, 0.52549, 0.0431373} //dark brown
};

GLfloat light_colors [][4] = {
  {1.0, 0.0, 0.0, 1},  
  {1.0, 1.0, 0.0, 1},
  {1.0, 0.0, 1.0, 1},
  {0.0, 1.0, 0.0, 1},
  {0.0, 1.0, 1.0, 1},
  {0.0, 0.0, 1.0, 1},
  {1.0, 1.0, 1.0, 1},
  {0.5, 0.5, 0.5, 1},
  {0.0, 0.0, 0.0, 1}
};

int   light1_theta = 0;


#define smallWidth      512
#define largeWidth     1024
static GLuint  tex_names[4];
static GLubyte wood_img[smallWidth*largeWidth] ;
static GLubyte wood2_img[largeWidth*largeWidth] ;
static GLubyte woodplane_img[smallWidth*smallWidth] ;
void texture_setup();

#define WOOD_TEX    0
#define WOOD2_TEX    1
#define PLANE_TEX   2


GLfloat tex_coords[][2] = {
  {1.0, 1.0}, {1.0, 0.0}, {0.0, 0.0}, {0.0, 1.0}
};