
/****************************
FILE:  Labyrinth.cpp
DATE: 11/26/2012
Author:  Julie De Lorenzo, based on example code
from homework assignments
****************************/

#include "Labyrinth.h"
#include <GL/glut.h>
#include <iostream>	
using namespace std;
bool ignoreRepeats = false;
int crt_render_mode;
int rs = 10;
int vs = 10;
double PI = 3.14159265359;

GLhandleARB v,f,f2,p;

struct _WALL {
	vertex vertices[8];
} walls[56];

struct _HOLE {
	vertex vertices[20][20];
	vertex pos;
} holes[40];
int hole_rs = 20;
int hole_vs = 20;
int inhole = 0;
int intersect[4];

struct _SPHERE {	
	vertex vertices[51][51];
	vertex normals[51][51];
	float velocity[3];
	int rs;
	int vs;
	vertex pos;
	float radius;
};
_SPHERE ball;

vertex verts_cube[8];
vertex verts_plane[8];
int rot_limit[2] = {0, 0};

typedef struct _CAM{
  GLfloat pos[4];
  GLfloat look[4];
  GLfloat up[4];

  GLfloat dir[4];

  GLfloat u[3];
  GLfloat v[3];
  GLfloat w[3];
}CAM;
CAM my_cam;


typedef struct _LITE{
  GLfloat amb[4];
  GLfloat diff[4];
  GLfloat spec[4];
  GLfloat pos[4];
  GLfloat dir[3];
  GLfloat angle;
}LITE;
LITE my_lights[3];
int  num_lights;

void glut_setup (){
	glutInitDisplayMode(GLUT_DOUBLE|GLUT_RGB|GLUT_DEPTH);
  
	glutInitWindowSize(700,700);
	glutInitWindowPosition(20,20);
	glutCreateWindow("Labyrinth");

	 /* set up callback functions */
	glutDisplayFunc(my_display);
	glutReshapeFunc(my_reshape);
	glutKeyboardFunc(my_keyboard);  //this keyboard handles the regular keys like Q for quit

	glutSpecialFunc(special_keyboard); //this keyboard handles the arrow keys

	glutTimerFunc( 20, my_TimeOut, 0); //schedule a my_TimeOut call with the ID 0 in 20 ms
	glutTimerFunc(1, my_Intersect, 1);
	//glEnable(GL_COLOR_MATERIAL);
	 return;
}

void gl_setup(void) {

  // enable depth handling (z-buffer)
  glEnable(GL_DEPTH_TEST);

  // enable auto normalize
  glEnable(GL_NORMALIZE);

  // define the background color 
  glClearColor(0,0,0,1);

  glMatrixMode(GL_PROJECTION) ;
  glLoadIdentity() ;
  gluPerspective( 40, 1.0, 1, 200.0);
  glMatrixMode(GL_MODELVIEW) ;
  glLoadIdentity() ;  // init modelview to identity

  // toggle to smooth shading (instead of flat)
  glShadeModel(GL_SMOOTH); 
  lighting_setup();
  
  
  //set up camera
  
  set_up_cam();
  //texture_setup();

  return ;
}

/*******************************************************
FUNCTION: main
ARGS: argc, argv
RETURN: 0
DOES: main function; starts GL, GLU, GLUT, then loops 
********************************************************/
int main(int argc, char** argv)

{	
  glutInit(&argc, argv);
  glut_setup();
  gl_setup();
  my_setup();
  //setShaders();
  glutMainLoop();
  return(0);
}

/*******************************************************
FUNCTION: my_setup
ARGS: 
RETURN:
DOES: pre-computes stuff and presets some values
********************************************************/
void my_setup(){
  //set render mode
  crt_render_mode = GL_POLYGON;
  //create objects
  
  make_cube_smart(1.1);
  make_plane_smart(1.0); //plane is at origin

  //make walls
  make_walls();

  //make holes
  make_holes();

  //make sphere
  make_sphere(0.05, 20, 20);

  //position objects
  real_translation(0, -1, 0, BOX, 0);

  //reset inhole
  inhole = 0;

  
  //plane is at origin, so it's highest surface is origin (X,0,X) + height (0.1) = (X, 0.1, X)
  //so all objects sitting on the plane should be transformed by Ty(0.1 + heightobj)
  real_translation(0.12, 0.15, -0.82, SPHERE, 0);
  for (int i = 0; i < 56; i++) {
	real_translation(0, 0.15, 0, WALL, i);
  }
  for (int i = 0; i < 40; i++) {
	  real_translation(0, 0.1, 0, HOLE, i);
  }

  
  return;
}

void reset() {
	inhole = 0;
	//rotate everything back
	real_rotation(-rot_limit[0] * 0.02, 1, 0, 0);
	real_rotation(-rot_limit[1] * 0.02, 0, 0, 1);
	rot_limit[0] = 0;
	rot_limit[1] = 0;

	//set velocity to 0
	ball.velocity[0] = 0;
	ball.velocity[1] = 0;
	ball.velocity[2] = 0;
	//put sphere at origin
	real_translation(-ball.pos.x, -ball.pos.y, -ball.pos.z, SPHERE, 0);
	//then put back at start
	real_translation(0.12, 0.15, -0.82, SPHERE, 0);
}

/*******************************************************
FUNCTION: set_up_cam
ARGS: none
RETURN:
DOES: sets up camera
********************************************************/
void set_up_cam() {
  CAM *pc = &my_cam;
  pc->pos[0] = 0;
  pc->pos[1] = 3.5;
  pc->pos[2] = -0.1;
  pc->pos[3] = 1;

  pc->look[0] = 0;
  pc->look[1] = -1;
  pc->look[2] = 0;
  pc->look[3] = 1;

  pc->up[0] = 0;
  pc->up[1] = 0;
  pc->up[2] = -1;
  pc->up[3] = 1;

  pc->look[0] += pc->pos[0];
  pc->look[1] += pc->pos[1];
  pc->look[2] += pc->pos[2];

  pc->dir[0] = pc->look[0] - pc->pos[0];
  pc->dir[1] = pc->look[1] - pc->pos[1];
  pc->dir[2] = pc->look[2] - pc->pos[2];
  normalize(pc->dir);
}

/*******************************************************
FUNCTION: normalize
ARGS: GLfloat *p
RETURN:
DOES: normalizes vector/vertex
********************************************************/
void normalize(GLfloat *p) { 
  double d=0.0;
  int i;
  for(i=0; i<3; i++) d+=p[i]*p[i];
  d=sqrt(d);
  if(d > 0.0) for(i=0; i<3; i++) p[i]/=d;
}

void normalize(vertex v) {
	double d = 0.0 ;
	d = (v.x * v.x) + (v.y*v.y) + (v.z*v.z);
	d = sqrt(d);
	if (d > 0.0) {
		v.x/=d;
		v.y /=d;
		v.z/=d;
		
	}
}


/*******************************************************
FUNCTION: my_reshape
ARGS: new window width and height 
RETURN:
DOES: remaps viewport to the Window Manager's window
********************************************************/
void my_reshape(int w, int h) {
  // ensure a square view port
  glViewport(0,0,min(w,h),min(w,h)) ;
  return ;

}
/***********************************
  FUNCTION: my_display
  ARGS: none
  RETURN: none
  DOES: main drawing function
************************************/
/*TODO add on*/
void my_display() {
	//float cur_mv[16];
	// clear all pixels, reset depth 
	glClear(GL_COLOR_BUFFER_BIT |GL_DEPTH_BUFFER_BIT );
    //glGetFloatv(GL_MODELVIEW_MATRIX, cur_mv);
	// init to identity 
	//glPushMatrix();
	//{
	glLoadIdentity() ;

	gluLookAt(my_cam.pos[0],my_cam.pos[1], my_cam.pos[2], //xyz coord of camera
	    my_cam.look[0],my_cam.look[1],my_cam.look[2],  //xyz lookAt
	    my_cam.up[0], my_cam.up[1], my_cam.up[2]);  //directon of up (default y axis = y = 1.0)
	//glMultMatrixf(cur_mv);
	
	
	glEnable(GL_TEXTURE_2D);
	//glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL );
	draw_cube();
	draw_plane();   
	
	//the walls
	draw_walls();   
	//the holes
	draw_holes();
	
	glDisable(GL_TEXTURE_2D);
	
	//finally draw the sphere
	//glUseProgramObjectARB(p);
	draw_sphere();
	//glUseProgramObjectARB(0);
	//display text?
	if (inhole == 1) {
	  char str[10]  = "Game Over";
	  glColor3f(1.0f, 0.f, 0.f);
	  glRasterPos3f(-0.05f, 2.4f,-0.05f);
	  for (int i = 0; i < strlen(str); i++) {
	  glutBitmapCharacter(GLUT_BITMAP_8_BY_13,str[i]);
	  }
	}
	//}
	//glPopMatrix();
	//this buffer is ready
	glutSwapBuffers();


}

void special_keyboard(int key, int x, int y) {
	switch(key) {
		case GLUT_KEY_UP: {
			//cout << "Up arrow key pressed." << endl;
			if (rot_limit[0] > -5) {
				//plane should rotate
				real_rotation(-0.02,1,0,0);
				
				rot_limit[0]--;
			}
			//the ball should move 
			glutPostRedisplay();
						  }; break;
		case GLUT_KEY_RIGHT: {
			if (rot_limit[1] > -5) {
				//cout << "Right arrow key pressed. \n";
				real_rotation(-0.02,0,0,1);
				
				rot_limit[1] --;
			}
			glutPostRedisplay();
							 }; break;
		case GLUT_KEY_DOWN: {
			if (rot_limit[0] < 5) {
				//cout << "Down arrow key pressed. \n";
				real_rotation(0.02,1,0,0);
				
				rot_limit[0]++;
			}
			glutPostRedisplay();
							}; break;
		case GLUT_KEY_LEFT: {
			//cout << "Left arrow key pressed. \n";
			if (rot_limit[1] <5) {
				real_rotation(0.02,0,0,1);
				
				rot_limit[1] ++;
			}
			glutPostRedisplay();
							}; break;
		default:
			break;
	}
	return;
}

void my_keyboard( unsigned char key, int x, int y ) {
	switch(key) {
		case 'Z': {
			my_cam.pos[2] += 0.5;
			//calc_cam_locals();
			glutPostRedisplay(); 
			}; break;

		case 'z': {
			my_cam.pos[2] -= 0.5;
			//calc_cam_locals();
			glutPostRedisplay(); 
			}; break;
		 case 'X': {
			 my_cam.pos[0] += 0.5;
			//calc_cam_locals();
			glutPostRedisplay(); 
			}; break;
		 case 'x': {
			my_cam.pos[0] -= 0.5;
			//calc_cam_locals();
			glutPostRedisplay(); 
			}; break;
		case 'y': {
			my_cam.pos[1] -= 0.5;
			glutPostRedisplay(); 
			}; break;
		case 'Y': {
			my_cam.pos[1] += 0.5;
			glutPostRedisplay(); 
		}; break;

		case 'r': 
		case 'R':{
			reset();
				 }; break;
		case 'P':
		case 'p': {
			printf("Current position:  <%f, %f, %f> \n", my_cam.pos[0], my_cam.pos[1], my_cam.pos[2]);
			printf("Current eye point:  <%f, %f, %f> \n", my_cam.look[0], my_cam.look[1], my_cam.look[2]);
			printf("Current look vector:  <%f, %f, %f> \n", my_cam.dir[0], my_cam.dir[1], my_cam.dir[2]);
			printf("Current up vector:  <%f, %f, %f> \n", my_cam.up[0], my_cam.up[1], my_cam.up[2]);
			printf("Plane 0:  <%f, %f, %f> \n", verts_plane[0].x, verts_plane[0].y, verts_plane[0].z);
			printf("Plane 1:  <%f, %f, %f> \n", verts_plane[1].x, verts_plane[1].y, verts_plane[1].z);
			printf("Plane 2:  <%f, %f, %f> \n", verts_plane[2].x, verts_plane[2].y, verts_plane[2].z);
			printf("Plane 3:  <%f, %f, %f> \n", verts_plane[3].x, verts_plane[3].y, verts_plane[3].z);
			printf("Ball:  <%f, %f, %f> \n", ball.pos.x, ball.pos.y, ball.pos.z);
			printf("Ball velocity:  <%f,%f,%f> \n", ball.velocity[0], ball.velocity[1], ball.velocity[2]);
			printf("Hole:  <%f, %f, %f> \n", holes[8].pos.x, holes[8].pos.y, holes[8].pos.z);
			//height angle
			//aspect ratio
			//world to film matrix
			}; break;
		case 'q': 
		case 'Q':
			exit(0) ;
			break;	
		default:
			break;
	}
	return;
}

void draw_quad(vertex vertices[51][51], int i, int j, int ic) {
  glBegin(crt_render_mode); 
  {
    glColor3fv(colors[ic]);
	
	glNormal3f(ball.normals[i][j].x,ball.normals[i][j].y,ball.normals[i][j].z);
    glVertex4f(vertices[i][j].x,vertices[i][j].y,vertices[i][j].z,vertices[i][j].w);
	
	glNormal3f(ball.normals[i+1][j].x,ball.normals[i+1][j].y,ball.normals[i+1][j].z);
	glVertex4f(vertices[i+1][j].x,vertices[i+1][j].y,vertices[i+1][j].z,vertices[i+1][j].w);
	
	glNormal3f(ball.normals[i+1][j+1].x,ball.normals[i+1][j+1].y,ball.normals[i+1][j+1].z);
	glVertex4f(vertices[i+1][j+1].x,vertices[i+1][j+1].y,vertices[i+1][j+1].z,vertices[i+1][j+1].w);
	
	glNormal3f(ball.normals[i][j+1].x,ball.normals[i][j+1].y,ball.normals[i][j+1].z);
	glVertex4f(vertices[i][j+1].x,vertices[i][j+1].y,vertices[i][j+1].z,vertices[i][j+1].w);
  }
  glEnd();
}


void draw_quad(vertex vertices[], int iv1, int iv2, int iv3, int iv4, int ic) {
  glBegin(crt_render_mode); 
  {
    glColor3fv(colors[ic]);
	glTexCoord2fv(tex_coords[0]);
    glVertex4f(vertices[iv1].x, vertices[iv1].y, vertices[iv1].z, vertices[iv1].w);
	glTexCoord2fv(tex_coords[1]);
	glVertex4f(vertices[iv2].x, vertices[iv2].y, vertices[iv2].z, vertices[iv2].w);
	glTexCoord2fv(tex_coords[2]);
	glVertex4f(vertices[iv3].x, vertices[iv3].y, vertices[iv3].z, vertices[iv3].w);
	glTexCoord2fv(tex_coords[3]);
	glVertex4f(vertices[iv4].x, vertices[iv4].y, vertices[iv4].z, vertices[iv4].w);
  }
  glEnd();
}

void draw_triangle(GLfloat vertices[][4], int iv1, int iv2, int iv3, int ic) {
  glBegin(crt_render_mode); 
  {
    glColor3fv(colors[ic]);
    /*note the explicit use of homogeneous coords below: glVertex4f*/
    glVertex4fv(vertices[iv1]);
    glVertex4fv(vertices[iv2]);
    glVertex4fv(vertices[iv3]);
  }
  glEnd();
}


void rotate_sphere(GLfloat deg, GLfloat x, GLfloat y, GLfloat z) {
	//for rotating sphere in place
	//translate to origin

	//find cosine and sine for deg
	double c = cos(deg);
	double s = sin(deg);

	//if x == 1 do x rotation
	if (x == 1) {
		//define x rotation matrix
		vertex real_rx[4] = {
			{1, 0, 0, 0},
			{0, c, -s, 0},
			{0, s, c, 0},
			{0, 0, 0, 1}
		};

		for (int i = 0; i < ball.rs; i++) {
			for (int j = 0; j < ball.vs; j++) {
				ball.vertices[i][j] = matrix_multiply(real_rx, ball.vertices[i][j]);
			}
		}
		ball.pos = matrix_multiply(real_rx, ball.pos);
	}
		

	//if y == 1 do y rotation
	else if (y == 1) {
		//define y rotation matrix
		vertex real_ry[4] = {
			{c, 0, s, 0},
			{0, 1, 0, 0},
			{-s, 0, c, 0},
			{0, 0, 0, 1}
		};

		for (int i = 0; i < ball.rs; i++) {
			for (int j = 0; j < ball.vs; j++) {
				ball.vertices[i][j] = matrix_multiply(real_ry, ball.vertices[i][j]);
			}
		}
		ball.pos = matrix_multiply(real_ry, ball.pos);
	}

	//if z == 1 do z rotation 
	else if (z == 1) {
		//define z rotation matrix
		vertex real_rz[4] = {
			{c, -s, 0, 0},
			{s, c, 0, 0},
			{0, 0, 1, 0},
			{0, 0, 0, 1}
		};

		for (int i = 0; i < ball.rs; i++) {
				for (int j = 0; j < ball.vs; j++) {
					ball.vertices[i][j] = matrix_multiply(real_rz, ball.vertices[i][j]);
				}
		}
		ball.pos = matrix_multiply(real_rz, ball.pos);
	}
}


///Matrix Multiply
vertex matrix_multiply(vertex A[4], vertex B) {
	//rows of A by columns of B
	vertex answer;
	answer.x = A[0].x * B.x + A[0].y * B.y + A[0].z * B.z + A[0].w * B.w;
	answer.y = A[1].x * B.x + A[1].y * B.y + A[1].z * B.z + A[1].w * B.w;
	answer.z = A[2].x * B.x + A[2].y * B.y + A[2].z * B.z + A[2].w * B.w;
	answer.w = A[3].x * B.x + A[3].y * B.y + A[3].z * B.z + A[3].w * B.w;
	return answer;
}

void make_sphere( double ray, int rs, int vs ) {
	ball.radius = ray;
	ball.rs = 20;
	ball.vs = 20;
	vertex pos = {0, -0.05, 0, 1};
	ball.pos = pos;
	vertex vstart = {ray, 0, 0, 1};
	double theta = 0;
	double phi = 0;
	for (int i = 0; i < ball.vs; theta += 2*PI/ball.vs, i++) {
		for (int j = 0; j < ball.rs; phi += 2*PI/ball.rs, j++) {
			double c = cos(theta);
			double s = sin(theta);
			double cp = cos(phi);
			double sp = sin(phi);
			vertex RZ[4] = {
				{c, -s, 0, 0},
				{s, c, 0, 0},
				{0,0,1,0},
				{0,0,0,1}
			};
			vertex RY[4] = {
				{cp, 0, sp, 0},
				{0,1,0,0},
				{-sp, 0, cp, 0},
				{0,0,0,1}
			};

			ball.vertices[j][i] = matrix_multiply(RZ, vstart);
			ball.vertices[j][i] = matrix_multiply(RY, ball.vertices[j][i]);
			ball.normals[j][i] = subtract(ball.vertices[j][i], pos);
		}
	}

}


/***********************************
  FUNCTION: draw_sphere() 
  ARGS: int rs, int vs
  RETURN: none
  DOES: draws a sphere from quads
************************************/
/*TODO: stitch sphere vertices together to make faces
don't call gl directly, use make_triangle and make_quad instead*/
void draw_sphere()
{
	for (int i = 0; i < ball.rs; i++) {
		for (int j = 0; j < ball.vs; j++) {
			//draw_triangle(ball.vertices[i][j], ball.vertices[i+1][j], ball.vertices[i+1][j+1], 7);
			//draw_triangle(ball.vertices[i+1][j+1], ball.vertices[i][j+1], ball.vertices[i][j], 7);
			draw_quad(ball.vertices, i, j, 7);
		}
	}
}

/*************************************
FUNCTION: make_*; reuse your stitcher code here.
*************************************/
void make_cube_smart( double size ){
	vertex v = {-size - 0.1, size, size-0.1, 1};
	verts_cube[0] = v;
	v.x = size + 0.1;
	verts_cube[1] = v;
	v.z = -size +0.1;
	verts_cube[2] = v;
	v.x = -size - 0.1;
	verts_cube[3] = v;
	v.y = -size;
	v.z = size -0.1;
	verts_cube[4] = v;
	v.x = size + 0.1;
	verts_cube[5] = v;
	v.z = -size + 0.1;
	verts_cube[6] = v;
	v.x = -size - 0.1;
	verts_cube[7] = v;

}

void make_plane_smart( double size ){
	vertex v = {-1.1, size/10, 0.9, 1};
	verts_plane[0] = v;
	v.x = 1.1;
	verts_plane[1] = v;
	v.z = -0.9;
	verts_plane[2] = v;
	v.x = -1.1;
	verts_plane[3] = v;
	v.y = -size/10;
	v.z = 0.9;
	verts_plane[4] = v;
	v.x = 1.1;
	verts_plane[5] = v;
	v.z = -0.9;
	verts_plane[6] = v;
	v.x = -1.1;
	verts_plane[7] = v;

}


void make_wall_smart(int i, double length, double width, double height){
	vertex v = {-width, height, length, 1};
	walls[i].vertices[0] = v;
	v.x = width;
	walls[i].vertices[1] = v;
	v.z = -length;
	walls[i].vertices[2] = v;
	v.x = -width;
	walls[i].vertices[3] = v;
	v.y = -height;
	v.z = length;
	walls[i].vertices[4] = v;
	v.x = width;
	walls[i].vertices[5] = v;
	v.z = -length;
	walls[i].vertices[6] = v;
	v.x = -width;
	walls[i].vertices[7] = v;
}

void make_hole (int index, double radius) {
	vertex vstart = {radius,  -radius, 0, 1};
	vertex pos = {0, -0.05, 0, 1};
	holes[index].pos = pos;
	double theta = 0;
	double h = 0;
	for (int i=0; i <=  hole_vs; i++, h+= 0.1/ hole_vs) {
		for (int j=0; j <  hole_rs; theta += 2*PI/hole_rs, j++) {
			double c = cos(theta);
		    double s = sin(theta);
			vertex RY[4] = {
				{c, 0, s, 0}, 
				{0,1,0,0}, 
				{-s,0,c,0}, 
				{0,0,0,1}
			};
			vertex T[4] = {
				{1,0,0,0},
				{0,1,0,h},
				{0,0,1,0},
				{0,0,0,1}
			};
			
			holes[index].vertices[j][i] = matrix_multiply(RY, vstart);
			holes[index].vertices[j][i] = matrix_multiply(T,holes[index].vertices[j][i]);
		}
	}
}


void draw_triangle(vertex vertices[], int ic1, int ic2, int ic3, int ic) {
  glBegin(crt_render_mode); 
  {
    glColor3fv(colors[ic]);
	glTexCoord2fv(tex_coords[0]);
    /*note the explicit use of homogeneous coords below: glVertex4f*/
    glVertex4f(vertices[ic1].x, vertices[ic1].y, vertices[ic1].z, vertices[ic1].w);
	glTexCoord2fv(tex_coords[1]);
    glVertex4f(vertices[ic2].x, vertices[ic2].y, vertices[ic2].z, vertices[ic2].w);
	glTexCoord2fv(tex_coords[2]);
    glVertex4f(vertices[ic3].x, vertices[ic3].y, vertices[ic3].z, vertices[ic3].w);
  }
  glEnd();
}

void draw_cube()
{
//top
  //draw_triangle(verts_cube, 0,2,1,BLUE);
  //draw_triangle(verts_cube, 0,3,2,RED);

  glBindTexture(GL_TEXTURE_2D, tex_names[WOOD2_TEX]);
  draw_triangle(verts_cube, 4,5,1,BROWN);
  draw_triangle(verts_cube, 0,4,1,BROWN);
  draw_triangle(verts_cube, 5,6,2,BROWN);
  draw_triangle(verts_cube, 1,5,2,BROWN);
  draw_triangle(verts_cube, 3,2,6,BROWN);
  draw_triangle(verts_cube, 7,3,6,BROWN);
  draw_triangle(verts_cube, 0,3,7,BROWN);
  draw_triangle(verts_cube, 4,0,7,BROWN);
  

  //bottom
  
  draw_triangle(verts_cube, 4,7,6,BROWN);
  draw_triangle(verts_cube, 4,6,5,BROWN);
}

void draw_plane()
{
//top
  glBindTexture(GL_TEXTURE_2D, tex_names[PLANE_TEX]);
  draw_quad(verts_plane, 0, 3, 2, 1, BROWN);
  //draw_triangle(verts_plane, 0,2,1,BROWN);
  //draw_triangle(verts_plane, 0,3,2,BROWN);

  /*
  draw_triangle(verts_plane, 4,5,1,BROWN);
  draw_triangle(verts_plane, 0,4,1,BROWN);
  draw_triangle(verts_plane, 5,6,2,BROWN);
  draw_triangle(verts_plane, 1,5,2,BROWN);
  draw_triangle(verts_plane, 3,2,6,BROWN);
  draw_triangle(verts_plane, 7,3,6,BROWN);
  draw_triangle(verts_plane, 0,3,7,BROWN);
  draw_triangle(verts_plane, 4,0,7,BROWN);
  

  //bottom
  
  draw_triangle(verts_plane, 4,7,6,BLACK);
  draw_triangle(verts_plane, 4,6,5,BLACK);
  */
}

void draw_walls()
{
	glBindTexture(GL_TEXTURE_2D, tex_names[WOOD_TEX]);
  for (int i = 0; i < 56; i++) {
	if (walls[i].vertices == NULL) continue;
	//top
	draw_triangle(walls[i].vertices, 0,2,1,DBROWN);
	draw_triangle(walls[i].vertices, 0,3,2,DBROWN);

  
	draw_triangle(walls[i].vertices, 4,5,1,DBROWN);
	draw_triangle(walls[i].vertices, 0,4,1,DBROWN);
	draw_triangle(walls[i].vertices, 5,6,2,DBROWN);
	draw_triangle(walls[i].vertices, 1,5,2,DBROWN);
	draw_triangle(walls[i].vertices, 3,2,6,DBROWN);
	draw_triangle(walls[i].vertices, 7,3,6,DBROWN);
	draw_triangle(walls[i].vertices, 0,3,7,DBROWN);
	draw_triangle(walls[i].vertices, 4,0,7,DBROWN);
  

	//bottom
  
	draw_triangle(walls[i].vertices, 4,7,6,DBROWN);
	draw_triangle(walls[i].vertices, 4,6,5,DBROWN);
  }
  
}


void draw_holes() {

	//side faces
	for (int index = 0; index < 40; index++) {
	for (int i = 0; i < hole_rs; i++) {
		draw_triangle(holes[index].pos, holes[index].vertices[i][hole_vs], holes[index].vertices[i+1][hole_vs], BLACK);
		
	}
	draw_triangle(holes[index].pos, holes[index].vertices[hole_rs-1][hole_vs], holes[index].vertices[0][hole_vs], BLACK);
	}
}


/***********************************
  FUNCTION: draw_triangle 
  ARGS: - a vertex array
        - 3 indices into the vertex array defining a triangular face
        - an index into the color array.
  RETURN: none
  DOES:  helper drawing function; draws one triangle. 
   For the normal to work out, follow left-hand-rule (i.e., ccw)
*************************************/
void draw_triangle(vertex v, vertex w, vertex z, int ic) {
  glBegin(crt_render_mode); 
  {
    glColor3fv(colors[ic]);
    /*note the explicit use of homogeneous coords below: glVertex4f*/
	
    glVertex4f(v.x, v.y, v.z, v.w);

    glVertex4f(w.x, w.y, w.z, w.w);

    glVertex4f(z.x, z.y, z.z, z.w);

  }
  glEnd();
  glFlush();
}

/*******************************************************
FUNCTION: real_rotation
ARGS: angle and axis
RETURN:
DOES: rotates shape, for real 
********************************************************/
/*TODO. Note: Absolutely no gl calls*/
/*can assume model-matrix stack contains at this point nothing but viewing transform */
void real_rotation(GLfloat deg, GLfloat x, GLfloat y, GLfloat z) {
	//find cosine and sine for deg
	double c = cos(deg);
	double s = sin(deg);

	//if x == 1 do x rotation
	if (x == 1) {
		//define x rotation matrix
		vertex real_rx[4] = {
			{1, 0, 0, 0},
			{0, c, -s, 0},
			{0, s, c, 0},
			{0, 0, 0, 1}
		};

		//switch by shape
			for (int i = 0; i < 8; i++) {
				verts_plane[i] = matrix_multiply(real_rx, verts_plane[i]);
			}
			for (int j = 0; j < 56; j++) {
				if (walls[j].vertices == NULL) continue;
				for (int i = 0; i < 8; i++) {
					walls[j].vertices[i] = matrix_multiply(real_rx, walls[j].vertices[i]);
				}
			}
			for (int k = 0; k < 40; k++) {
				for (int i = 0; i < hole_rs; i++) {
					for (int j = 0; j < hole_vs; j++) {
						holes[k].vertices[i][j] = matrix_multiply(real_rx, holes[k].vertices[i][j]);
					}
				}
				holes[k].pos = matrix_multiply(real_rx, holes[k].pos);
			}

			for (int i = 0; i < ball.rs; i++) {
				for (int j = 0; j < ball.vs; j++) {
					ball.vertices[i][j] = matrix_multiply(real_rx, ball.vertices[i][j]);
					ball.normals[i][j] = matrix_multiply(real_rx, ball.normals[i][j]);
				}
			}
			ball.pos = matrix_multiply(real_rx, ball.pos);
	}

	//if y == 1 do y rotation
	else if (y == 1) {
		//define y rotation matrix
		vertex real_ry[4] = {
			{c, 0, s, 0},
			{0, 1, 0, 0},
			{-s, 0, c, 0},
			{0, 0, 0, 1}
		};

			for (int i = 0; i < 8; i++) {
				verts_plane[i] = matrix_multiply(real_ry, verts_plane[i]);
			}
			for (int j = 0; j < 56; j++) {
				if (walls[j].vertices == NULL) continue;
				for (int i = 0; i < 8; i++) {
					walls[j].vertices[i] = matrix_multiply(real_ry, walls[j].vertices[i]);
				}
			}

			for (int k = 0; k < 40; k++) {
				for (int i = 0; i < hole_rs; i++) {
					for (int j = 0; j < hole_vs; j++) {
						holes[k].vertices[i][j] = matrix_multiply(real_ry, holes[k].vertices[i][j]);
					}
				}
				holes[k].pos = matrix_multiply(real_ry, holes[k].pos);
			}

			for (int i = 0; i < ball.rs; i++) {
				for (int j = 0; j < ball.vs; j++) {
					ball.vertices[i][j] = matrix_multiply(real_ry, ball.vertices[i][j]);
					ball.normals[i][j] = matrix_multiply(real_ry, ball.normals[i][j]);
				}
			}
			ball.pos = matrix_multiply(real_ry, ball.pos);
	}

	//if z == 1 do z rotation 
	else if (z == 1) {
		//define z rotation matrix
		vertex real_rz[4] = {
			{c, -s, 0, 0},
			{s, c, 0, 0},
			{0, 0, 1, 0},
			{0, 0, 0, 1}
		};

			for (int i = 0; i < 8; i++) {
				verts_plane[i] = matrix_multiply(real_rz, verts_plane[i]);
			}
			for (int j = 0; j < 56; j++) {
				if (walls[j].vertices == NULL) continue;
				for (int i = 0; i < 8; i++) {
					walls[j].vertices[i] = matrix_multiply(real_rz, walls[j].vertices[i]);
				}
			}

			for (int k = 0; k < 40; k++) {
				for (int i = 0; i < hole_rs; i++) {
					for (int j = 0; j < hole_vs; j++) {
						holes[k].vertices[i][j] = matrix_multiply(real_rz, holes[k].vertices[i][j]);
					}
				}
				holes[k].pos = matrix_multiply(real_rz, holes[k].pos);
			}

			for (int i = 0; i < ball.rs; i++) {
				for (int j = 0; j < ball.vs; j++) {
					ball.vertices[i][j] = matrix_multiply(real_rz, ball.vertices[i][j]);
					ball.normals[i][j] = matrix_multiply(real_rz, ball.normals[i][j]);
					}
				}
			ball.pos = matrix_multiply(real_rz, ball.pos);
		return;
	}
}

/*******************************************************
FUNCTION: real_translation
ARGS: translation amount along x, y, z
RETURN:
DOES: translates shape for real
********************************************************/
/*TODO. Note: Absolutely no gl calls */
/*can assume model-matrix stack contains at this point nothing but viewing transform*/
void real_translation(GLfloat x, GLfloat y, GLfloat z, int shape, int wallindex) {
	vertex real_t[4] = {
		{1,0,0,x},
		{0,1,0,y},
		{0,0,1,z},
		{0,0,0,1},
	};

	switch (shape) {
	case BOX: {
		for (int i = 0; i < 8; i++) {
			verts_cube[i] = matrix_multiply(real_t, verts_cube[i]);
		}
			   };break;
	case WALL: {
		for (int i = 0; i < 8; i++) {
			walls[wallindex].vertices[i] = matrix_multiply(real_t, walls[wallindex].vertices[i]);
		}
			   };break;
	case HOLE: {
		for (int i = 0; i < hole_rs; i++) {
			for (int j = 0; j < hole_vs; j++) {
				holes[wallindex].vertices[i][j] = matrix_multiply(real_t, holes[wallindex].vertices[i][j]);
			}
		}
		holes[wallindex].pos.x += x;
		holes[wallindex].pos.y += y;
		holes[wallindex].pos.z += z;
		}; break;
	case SPHERE: {
		for (int i = 0; i < ball.rs; i++) {
			for (int j = 0; j < ball.vs; j++) {
				ball.vertices[i][j] = matrix_multiply(real_t, ball.vertices[i][j]);
				ball.normals[i][j] = matrix_multiply(real_t, ball.normals[i][j]);
			}
		}
		ball.pos.x += x;
		ball.pos.y += y;
		ball.pos.z += z;
		}; break;
	} //end switch
}

void my_TimeOut(int id) { 
  if (inhole == 1) {
	  real_translation(0, -0.007, 0, SPHERE, 0);
  }

  else { 
  calc_ball_velocity();
  }
  //update screen

  my_display();

  glutTimerFunc(20, my_TimeOut, 0);/* schedule next timer event, 20 ms from now */
}

void my_Intersect(int id) {
	check_intersections();
	//glutTimerFunc(0.01, my_Intersect, 1);
}

void calc_ball_velocity () {
	//rot_limit keeps track of how far the plane is tilted
	//the ball's X and Z velocities should increase by the intensity of this rot_limit or not at all
	if (intersect[0] == 0) {
		ball.velocity[0] -= 0.001 * rot_limit[1];
	}
	else if (intersect[0] < 0 && rot_limit[1] < 0) {
		ball.velocity[0] -= 0.001 * rot_limit[1];
	}
	else if (intersect[0] > 0 && rot_limit[1] > 0) {
		ball.velocity[0] -= 0.001 * rot_limit[1];
	}
	if (intersect[1] == 0) {
		ball.velocity[2] += 0.001 * rot_limit[0];
	}
	else if (intersect[1] < 0 && rot_limit[1] > 0) {
		ball.velocity[0] -= 0.001 * rot_limit[1];
	}
	else if (intersect[1] > 0 && rot_limit[1] < 0) {
		ball.velocity[0] -= 0.001 * rot_limit[1];
	}

	
	//then friction should slow the ball
	//coefficient of friction between metal and wood is 0.2 - 0.6
	//force due to friction = u * N (coefficient of friction * normal) = u * mg (normal force on horizontal plane is weight of object)
	
	
	float friction = 0.2 * 0.0086;

	if (ball.velocity[0] != 0) {
		if (ball.velocity[0] < 0) { 
			if (ball.velocity[0] + friction > 0) { ball.velocity[0] = 0; }
			else {ball.velocity[0] += friction; }
		}
		else { 
			if (ball.velocity[0] - friction < 0) { ball.velocity[0] = 0; }
			else { ball.velocity[0] -= friction; }
		}
	}

	if (ball.velocity[2] != 0) {
		if (ball.velocity[2] < 0) { 
			if (ball.velocity[2] + friction > 0) { ball.velocity[2] = 0; }
			else {ball.velocity[2] += friction; }
		}
		else { 
			if (ball.velocity[2] - friction < 0) { ball.velocity[2] = 0; }
			else {ball.velocity[2] -= friction; }
		}
	}
	//to move the ball we have to rotate the plane back to flat, then rotate it back again
	check_intersections();
	real_rotation(-rot_limit[0] * 0.02, 1, 0, 0);
	real_rotation(-rot_limit[1] * 0.02, 0, 0, 1);
	real_translation(ball.velocity[0], 0, ball.velocity[2], SPHERE, 0);
	real_rotation(rot_limit[1] * 0.02, 0, 0, 1);
	real_rotation(rot_limit[0] * 0.02, 1, 0, 0);
	check_intersections();
} 

void make_walls () {
	//first four walls are highest and go all the way around the plane
	make_wall_smart(0, 0.9, 0.01, 0.05);
	real_translation(-1.1, 0, 0, WALL, 0);
	make_wall_smart(1, 0.9, 0.01, 0.05);
	real_translation(1.1, 0, 0, WALL, 1);
	make_wall_smart(2, 0.01, 1.1, 0.05);
	real_translation(0, 0, 0.9, WALL, 2);
	make_wall_smart(3, 0.01, 1.1, 0.05);
	real_translation(0, 0, -0.9, WALL, 3);

	//next walls

	//vertical walls ::  want to check the right/left (6/4) of these to halt horiz. velocities

	//leftmost (X= -0.92)
	make_wall_smart(5, 0.11, 0.01, 0.05);
	real_translation(-0.92, 0, 0.05, WALL, 5);

	//X= -0.75
	make_wall_smart(6, 0.06, 0.01, 0.05);
	real_translation(-0.75, 0, -0.72, WALL, 6);
	make_wall_smart(7, 0.16, 0.01, 0.05);
	real_translation(-0.75, 0, -0.35, WALL, 7);
	make_wall_smart(8, 0.11, 0.01, 0.05);
	real_translation(-0.75, 0, 0.05, WALL, 8);
	make_wall_smart(9, 0.2, 0.01, 0.05);
	real_translation(-0.75, 0, 0.5, WALL, 9);


	//left (X= -0.57)
	make_wall_smart(10, 0.06, 0.01, 0.05);
	real_translation(-0.57, 0, -0.85, WALL, 10);
	make_wall_smart(11, 0.06, 0.01, 0.05);
	real_translation(-0.57, 0, -0.60, WALL, 11);
	make_wall_smart(12, 0.06, 0.01, 0.05);
	real_translation(-0.57, 0, -0.35, WALL, 12);
	make_wall_smart(13, 0.12, 0.01, 0.05);
	real_translation(-0.57, 0, 0.04, WALL, 13);
	make_wall_smart(14, 0.06, 0.01, 0.05);
	real_translation(-0.57, 0, 0.51, WALL, 14);
	make_wall_smart(15, 0.06, 0.01, 0.05);
	real_translation(-0.57, 0, 0.85, WALL, 15);

	//left (X= -0.4)
	make_wall_smart(16, 0.1, 0.01, 0.05);
	real_translation(-0.4, 0, -0.65, WALL, 16);
	make_wall_smart(17, 0.08, 0.01, 0.05);
	real_translation(-0.4, 0, -0.33, WALL, 17);
	make_wall_smart(18, 0.12, 0.01, 0.05);
	real_translation(-0.4, 0, 0.04, WALL, 18);
	make_wall_smart(19, 0.15, 0.01, 0.05);
	real_translation(-0.4, 0, 0.53, WALL, 19);

	//X = -0.2
	make_wall_smart(20, 0.1, 0.01, 0.05);
	real_translation(-0.2, 0, -0.49, WALL, 20);
	make_wall_smart(21, 0.2, 0.01, 0.05);
	real_translation(-0.2, 0, 0.03, WALL, 21);
	make_wall_smart(22, 0.08, 0.01, 0.05);
	real_translation(-0.2, 0, 0.60, WALL, 22);

	//0.07
	make_wall_smart(23, 0.06, 0.01, 0.05);
	real_translation(0.02, 0, -0.58, WALL, 23);
	make_wall_smart(24, 0.08, 0.01, 0.05);
	real_translation(0.02, 0, -0.3, WALL, 24);
	make_wall_smart(25, 0.06, 0.01, 0.05);
	real_translation(0.02, 0, 0, WALL, 25);
	make_wall_smart(26, 0.15, 0.01, 0.05);
	real_translation(0.02, 0, 0.55, WALL, 26);

	//slight right X = 0.2
	make_wall_smart(27, 0.4, 0.01, 0.05);
	real_translation(0.2, 0, -0.5, WALL, 27);
	make_wall_smart(28, 0.17, 0.01, 0.05);
	real_translation(0.2, 0, 0.2, WALL, 28);

	//x = 0.4
	make_wall_smart(55, 0.1, 0.01, 0.05);
	real_translation(0.39, 0, 0.19, WALL, 55);

	//next right x =0.55
	make_wall_smart(29, 0.05, 0.01, 0.05);
	real_translation(0.55, 0, -0.55, WALL, 29);



	//next right x = 0.58
	make_wall_smart(30, 0.21, 0.01, 0.05);
	real_translation(0.58, 0, -0.02, WALL, 30);
	make_wall_smart(31, 0.21, 0.01, 0.05);
	real_translation(0.58, 0, 0.5, WALL, 31);

	//next right (X= 0.75)
	make_wall_smart(32, 0.06, 0.01, 0.05);
	real_translation(0.75, 0, -0.85, WALL, 32);

	make_wall_smart(33, 0.22, 0.01, 0.05);
	real_translation(0.75, 0, -0.39, WALL, 33);

	make_wall_smart(34, 0.22, 0.01, 0.05);
	real_translation(0.75, 0, 0.2, WALL, 34);

	make_wall_smart(35, 0.06, 0.01, 0.05);
	real_translation(0.75, 0, 0.65, WALL, 35);

	//rightmost vert. walls (X = 0.94)
	make_wall_smart(36, 0.06, 0.01, 0.05);
	real_translation(0.94, 0, -0.85, WALL, 36);
	make_wall_smart(37, 0.1, 0.01, 0.05);
	real_translation(0.94, 0, -0.55, WALL, 37);
	make_wall_smart(38, 0.2, 0.01, 0.05);
	real_translation(0.94, 0, -0.1, WALL, 38);

	//////////////////////////////////////////////////////
	//horizontal walls :: want to check the top/down (7/4) of these to halt vert. velocities

	//highest (Z = -0.74)
	
	make_wall_smart(39, 0.01, 0.1, 0.05);
	real_translation(-1.0, 0, -0.74, WALL, 39);
	make_wall_smart(40, 0.01, 0.3, 0.05);
	real_translation(-0.1, 0, -0.74, WALL, 40);

	//Z=-0.60
	make_wall_smart(41, 0.01, 0.06, 0.05);
	real_translation(-0.92, 0, -0.60, WALL, 41);
	make_wall_smart(42, 0.01, 0.06, 0.05);
	real_translation(0.26, 0, -0.60, WALL, 42);
	make_wall_smart(43, 0.01, 0.10, 0.05);
	real_translation(0.64, 0, -0.6, WALL, 43);

	//Z = - 0.4
	make_wall_smart(44, 0.01, 0.1, 0.05);
	real_translation(-1.0, 0, -0.4, WALL, 44);
	make_wall_smart(45, 0.01, 0.1, 0.05);
	real_translation(-0.3, 0, -0.4, WALL, 45);
	make_wall_smart(46, 0.01, 0.15, 0.05);
	real_translation(0.34, 0, -0.4, WALL, 46);

	//Z= - 0.2
	make_wall_smart(47, 0.01, 0.1, 0.05);
	real_translation(-0.86, 0, -0.2, WALL, 47);

	//Z= -0.1
	make_wall_smart(48, 0.01, 0.11, 0.05);
	real_translation(0.48, 0, -0.02, WALL, 48);

	//Z=0.1
	make_wall_smart(49, 0.01, 0.07, 0.05);
	real_translation(1.0, 0, 0.1, WALL, 49);

	//Z=0.2
	make_wall_smart(50, 0.01, 0.1, 0.05);
	real_translation(-0.1, 0, 0.22, WALL, 50);



	//Z=0.3
	make_wall_smart(51, 0.01, 0.11, 0.05);
	real_translation(-0.86, 0, 0.3, WALL, 51);

	//Z=0.33
	make_wall_smart(52, 0.01, 0.1, 0.05);
	real_translation(0.48, 0, 0.30, WALL, 52);

	//Z=0.33
	make_wall_smart(53, 0.01, 0.1, 0.05);
	real_translation(-0.3, 0, 0.38, WALL, 53);

	//Z=0.65
	make_wall_smart(54, 0.01, 0.12, 0.05);
	real_translation(0.3, 0, 0.63, WALL, 54);
}

void make_holes() {
	for (int i = 0; i < 40; i++) {
		make_hole(i, 0.06);
	}
	//top row of holes (Z=-0.81);
	real_translation(-1.0, 0, -0.81, HOLE, 0);
	real_translation(0.3, 0, -0.81, HOLE, 1);
	real_translation(0.67, 0, -0.81, HOLE, 2);
	real_translation(0.84, 0, -0.81, HOLE, 3);

	real_translation(1.01, 0, -0.73, HOLE, 4);

	real_translation(0.12, 0, -0.66, HOLE, 5);

	real_translation(-0.8, 0, -0.60, HOLE, 6);
	real_translation(-0.66, 0, -0.60, HOLE, 7);
	real_translation(-0.48, 0, -0.60, HOLE, 8);

	real_translation(0.64, 0, -0.52, HOLE, 9);

	real_translation(0.29, 0, -0.48, HOLE, 10);

	real_translation(-0.48, 0, -0.35, HOLE, 11);

	real_translation(-0.3, 0, -0.32, HOLE, 12);
	real_translation(-0.08, 0, -0.32, HOLE, 13);
	real_translation(0.28, 0, -0.32, HOLE, 14);

	real_translation(0.85, 0, -0.22, HOLE, 15);

	real_translation(0.66, 0, -0.18, HOLE, 16);

	real_translation(-0.68, 0, -0.14, HOLE, 17);
	
	real_translation(0.47, 0, -0.11, HOLE, 18);

	//origin///
	real_translation(-1.01, 0, 0, HOLE, 19);
	real_translation(-0.48, 0, 0, HOLE, 20);
	real_translation(0.1, 0, 0, HOLE, 21);
	//////////

	real_translation(-0.68, 0, 0.2, HOLE, 22);
	real_translation(1.01, 0, 0.2, HOLE, 23);


	real_translation(-0.28, 0, 0.3, HOLE, 24);
	real_translation(0.3, 0, 0.3, HOLE, 25);
	
	real_translation(-0.48, 0, 0.305, HOLE, 26);

	real_translation(-0.84, 0, 0.38, HOLE, 27);
	
	real_translation(0.66, 0, 0.39, HOLE, 28);
	real_translation(0.84, 0, 0.39, HOLE, 29);

	real_translation(0.1, 0, 0.42, HOLE, 30);

	real_translation(-1.01, 0, 0.51, HOLE, 31);
	real_translation(1.01, 0, 0.51, HOLE, 32);

	real_translation(-0.67, 0, 0.53, HOLE, 33);

	real_translation(-0.08, 0, 0.59, HOLE, 34);

	real_translation(-0.84, 0, 0.65, HOLE, 35);

	real_translation(0.30, 0, 0.73, HOLE, 36);
	real_translation(-0.48, 0, 0.8, HOLE, 37);

	
	real_translation(0.84, 0, 0.75, HOLE, 38);

	real_translation(0.30, 0, 0.52, HOLE, 39);
}


void check_intersections() {
	//Check for intersections against walls - brute force
	//TODO:  find smarter way of doing this?
	
	//rotate the plane to flat
	real_rotation(-rot_limit[0] * 0.02, 1, 0, 0);
	real_rotation(-rot_limit[1] * 0.02, 0, 0, 1);
	
	int foundintersectionx = 0;
	int foundintersectionz = 0;
	
	//if the translation's going to push it into the wall we only want it to go TO the wall
	for (int i = 4; i < 56; i++) {
		//check if it's hitting or inside the wall left-right wise
		if (((ball.pos.x - ball.radius + ball.velocity[0]) <= walls[i].vertices[5].x) && ((ball.pos.x + ball.radius + ball.velocity[0]) >= walls[i].vertices[4].x)  &&
			((ball.pos.z - ball.radius + ball.velocity[2]) <= walls[i].vertices[5].z) && ((ball.pos.z + ball.radius + ball.velocity[2]) >= walls[i].vertices[7].z)) {
			
			if (((ball.pos.x - ball.radius + ball.velocity[0]) < walls[i].vertices[5].x) && (ball.velocity[0] < 0)  && (ball.pos.x - ball.radius >= walls[i].vertices[4].x)) {
				ball.velocity[0] -= ((ball.pos.x - ball.radius + ball.velocity[0]) - walls[i].vertices[5].x);
				//printf("Ball velocity reduced because ball hit wall %d's right side", i);
				//rotate the plane back
				foundintersectionx = 1;
			}
			else if (((ball.pos.x + ball.radius + ball.velocity[0]) > (walls[i].vertices[4].x)) && (ball.velocity[0] > 0) && (ball.pos.x + ball.radius <= walls[i].vertices[5].x)) {

				ball.velocity[0] -= ((ball.pos.x + ball.radius + ball.velocity[0]) - walls[i].vertices[4].x);
				//printf("Ball velocity reduced because ball hit wall %d's left side", i);
				//rotate the plane back
				foundintersectionx = 1;
			}
			
			 if (((ball.pos.z + ball.radius + ball.velocity[2]) > walls[i].vertices[7].z) && (ball.velocity[2] > 0) && (ball.pos.z + ball.radius <= walls[i].vertices[5].z)) {
				ball.velocity[2] -= ((ball.pos.z + ball.radius + ball.velocity[2]) - walls[i].vertices[7].z);
				//printf("Ball velocity reduced because ball hit wall %d's top side", i);
				//rotate the plane back
				foundintersectionz = 1;
			}
			else if (((ball.pos.z - ball.radius + ball.velocity[2]) < walls[i].vertices[5].z) && (ball.velocity[2] < 0) && (ball.pos.z - ball.radius >= walls[i].vertices[7].z)) {
				ball.velocity[2] -= ((ball.pos.z - ball.radius + ball.velocity[2]) - walls[i].vertices[5].z);
				//printf("Ball velocity reduced because ball hit wall %d's bottom side", i);
				//rotate the plane back
				foundintersectionz = 1;
			}
			
			//printf("ball x:  %f  wall x:  %f  wall x:  %f", ball.pos.x, walls[i].vertices[5].x, walls[i].vertices[4].x);
		}
	}
	
	
	//Left Outer Wall (Wall 0)
	if (ball.pos.x - ball.radius <= walls[0].vertices[5].x) {
		//printf("ball x:  %f  wall x:  %f", ball.pos.x, walls[0].vertices[1].x);
		ball.velocity[0] = 0;
		if (ball.pos.x - ball.radius < walls[0].vertices[5].x) {
			real_translation((walls[0].vertices[5].x + (ball.radius-ball.pos.x)),0,0,SPHERE,0);
		}
	}
	//Right Outer Wall (Wall 1)
	if (ball.pos.x + ball.radius >= walls[1].vertices[4].x) {
		ball.velocity[0] = 0;
		if (ball.pos.x + ball.radius > walls[1].vertices[4].x) {
			real_translation((walls[1].vertices[4].x - ball.radius-ball.pos.x),0,0,SPHERE,0);
		}
	}
	//Bottom Outer Wall (Wall 2)
	if (ball.pos.z + ball.radius >= walls[2].vertices[7].z) {
		ball.velocity[2] = 0;

		if (ball.pos.z + ball.radius > walls[2].vertices[7].z) {
			real_translation(0,0,(walls[2].vertices[7].z - ball.radius-ball.pos.z),SPHERE,0);
		}
	}
	//Top Outer Wall (Wall 3)
	if (ball.pos.z - ball.radius <= walls[3].vertices[5].z) {
		ball.velocity[2] = 0;

		if (ball.pos.z - ball.radius < walls[3].vertices[5].z) {
			real_translation(0,0,(walls[3].vertices[5].z + (ball.radius-ball.pos.z)),SPHERE,0);
		}
	}

	//check the holes (if the ball's center of gravity is inside the hole)
	for (int i = 0; i < 40; i++) {
		if (ball.pos.x + ball.velocity[0] >= (holes[i].pos.x - 0.11) && ball.pos.x + ball.velocity[0] <= (holes[i].pos.x + 0.01) && 
			ball.pos.z + ball.velocity[2] >= (holes[i].pos.z - 0.05) && ball.pos.z + ball.velocity[2] <= (holes[i].pos.z + 0.05)) {
			real_translation(holes[i].pos.x - ball.pos.x-0.05, 0, holes[i].pos.z - ball.pos.z-0.02, SPHERE, 0);
			real_rotation(rot_limit[1] * 0.02, 0, 0, 1);
			real_rotation(rot_limit[0] * 0.02, 1, 0, 0);
			inhole = 1;
			return;
		}
	}
	
	/*
	//check the walls (note we have to check each edge)
	for (int i = 4; i < 56; i++) {
		//check if it's hitting or inside the wall left-right wise
		if (ball.pos.x - ball.radius < walls[i].vertices[5].x && ball.pos.x + ball.radius > walls[i].vertices[4].x  &&
			ball.pos.z - ball.radius < walls[i].vertices[5].z && ball.pos.z + ball.radius > walls[i].vertices[7].z) {
			
			if (ball.pos.x - ball.radius < walls[i].vertices[5].x && ball.velocity[0] < 0 ) {
				if (ball.pos.x - ball.radius < walls[i].vertices[5].x) {
					real_translation((walls[i].vertices[5].x + (ball.radius-ball.pos.x)),0,0,SPHERE,0);
				}
				ball.velocity[0] = 0;
				
				break;
			}
			else if (ball.pos.x + ball.radius > walls[i].vertices[4].x && ball.velocity[0] > 0) {
				ball.velocity[0] = 0;
				if (ball.pos.x + ball.radius > walls[i].vertices[4].x) {
					real_translation((walls[i].vertices[4].x - (ball.pos.x + ball.radius)),0,0,SPHERE,0);
				}
				
				break;
			}
			else if (ball.pos.z - ball.radius < walls[i].vertices[5].z && ball.velocity[2] < 0) {
				ball.velocity[2] = 0;
				if (ball.pos.z - ball.radius < walls[i].vertices[5].z ) {
					real_translation(0,0,(walls[i].vertices[5].z + (ball.radius-ball.pos.z)),SPHERE,0);
				}
				
				break;
			}
			else if (ball.pos.z + ball.radius > walls[i].vertices[7].z && ball.velocity[2] > 0) {
				ball.velocity[2] = 0;
				if (ball.pos.z + ball.radius > walls[i].vertices[7].z) {
					real_translation(0,0,(walls[i].vertices[7].z - (ball.radius+ball.pos.z)),SPHERE,0);
				}
				
				break;
			}
			//printf("ball x:  %f  wall x:  %f  wall x:  %f", ball.pos.x, walls[i].vertices[5].x, walls[i].vertices[4].x);
		}
	}
	
	*/
	
	//rotate the plane back
	real_rotation(rot_limit[1] * 0.02, 0, 0, 1);
	real_rotation(rot_limit[0] * 0.02, 1, 0, 0);
}

/************************************************
Taken from Simple Demo for GLSL    
www.lighthouse3d.com  

Function to read in text files and return them as a string
*************************************************/
char *textFileRead(char *fn) {

  FILE *fp;
  char *content = NULL;

  int count=0;

  if (fn != NULL) {
    fp = fopen(fn,"rt");

    if (fp != NULL) {
      
      fseek(fp, 0, SEEK_END);
      count = ftell(fp);
      rewind(fp);

      if (count > 0) {
	content = (char *)malloc(sizeof(char) * (count+1));
	count = fread(content,sizeof(char),count,fp);
	content[count] = '\0';
      }
      fclose(fp);
    }
  }
  else {
    fprintf(stderr, "Error: Unable to input shader file %s\n", fn);
  }
  return content;
}

/************************************************
Taken from Simple Demo for GLSL    
www.lighthouse3d.com  

Function to read in the shaders, compile and link them.
*************************************************/

void setShaders() {
  //variable to store errors
  int status = -1;

  // strings to hold shader code
  char *vs = NULL,*fs = NULL;

  // creates the shader object and links it to the handler
  v = glCreateShaderObjectARB(GL_VERTEX_SHADER_ARB);
  f = glCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB);

  // reads in the shader text into shader object
  // here is where you change which shaders to load
  vs = textFileRead("metal.vert");
  fs = textFileRead("metal.frag");

  // creates the source code for shaders
  glShaderSourceARB(v, 1, &vs,NULL);
  glShaderSourceARB(f, 1, &fs,NULL);

  free(vs);free(fs);

  // compile shaders
  glCompileShaderARB(v);
  glCompileShaderARB(f);

  // check for compile errors
  glGetObjectParameterivARB(f, GL_OBJECT_COMPILE_STATUS_ARB, &status);
  printf("compile status: %d\n", status);
  glGetObjectParameterivARB(v, GL_OBJECT_COMPILE_STATUS_ARB, &status);
  printf("compile status: %d\n", status);

  // link shaders
  p = glCreateProgramObjectARB();
  glAttachObjectARB(p,f);

  glAttachObjectARB(p,v);

  // link to program handler and use the shader
  glLinkProgramARB(p);

  // debug linking
  glGetObjectParameterivARB(f,  GL_OBJECT_LINK_STATUS_ARB, &status);
  printf("linking status: %d\n", status);
  glGetObjectParameterivARB(v,  GL_OBJECT_LINK_STATUS_ARB, &status);
  printf("linking status: %d\n", status);

}

/************************************************
Taken from Simple Demo for GLSL    
www.lighthouse3d.com  

Function to print the error log from infoLogARB

most of the time shows nothing :(
*************************************************/

void printInfoLog(GLhandleARB obj)
{
  int infologLength = 0;
  int charsWritten  = 0;
  char *infoLog;
  
  glGetObjectParameterivARB(obj, GL_OBJECT_INFO_LOG_LENGTH_ARB,
			    &infologLength);
  
  if (infologLength > 0)
    {
      infoLog = (char *)malloc(infologLength);
      glGetInfoLogARB(obj, infologLength, &charsWritten, infoLog);
      printf("%s\n",infoLog);
      free(infoLog);
    }
}


void lighting_setup () {
	//glLightModelf (GL_LIGHT_MODEL_COLOR_CONTROL,GL_SEPARATE_SPECULAR_COLOR);
	/*
	LITE *pl;
   pl = &my_lights[++num_lights];
   pl->dir[0]= pl->dir[1]= pl->dir[2] =0;
   pl->dir[1]= -1; 
   pl->amb[0]= pl->amb[1]=pl->amb[2] = 0.7;
   pl->diff[0] = 1;
   pl->diff[1] = 0;
   pl->diff[2] = 0;
   pl->spec[0] = pl->spec[1] = 0;
   pl->spec[2] = 1;
   pl->pos[0] = 1;
   pl->pos[1] = 15;
   pl->pos[2] = 15;
   pl->pos[3] = 1;
	*/
  GLfloat light0_amb[]      = {0.2, 0.2, 0.2, 1};
  GLfloat light0_diffuse[]  = {1, 1, 1, 1};
  GLfloat light0_specular[] = {1, 1, 1, 1};

  GLfloat light1_amb[]      = {0.2, 0.2, 0.2, 1};
  GLfloat light1_diffuse[]  = {1, 1, 1, 1};
  GLfloat light1_specular[] = {0, 0, 0, 1};

  GLfloat globalAmb[]     = {0.5, 0.5, 0.5, 1};



  GLfloat mat_amb_diff[]  = {.8,.8,.8,1};
  GLfloat mat_specular[]  = {1, 1, 1,1};

  GLfloat mat_high_shininess[] = {100};
  GLfloat mat_emission[]  = {.3,.3,.3	, 0};

  //enable lighting
  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);
  glEnable(GL_LIGHT1);
  glEnable(GL_NORMALIZE);

  // setup properties of point light 0
  glLightfv(GL_LIGHT0, GL_AMBIENT, light0_amb);
  glLightfv(GL_LIGHT0, GL_DIFFUSE, light0_diffuse);
  glLightfv(GL_LIGHT0, GL_SPECULAR, light0_specular);
  glLightfv(GL_LIGHT0, GL_POSITION, my_cam.look);
  glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, my_cam.dir);
  //GLfloat dir[4] = {0, 0, 0, 1};
  //glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, dir);
  
  glLightfv(GL_LIGHT1, GL_AMBIENT, light1_amb);
  glLightfv(GL_LIGHT1, GL_DIFFUSE, light1_diffuse);
  glLightfv(GL_LIGHT1, GL_SPECULAR, light1_specular);
  glLightfv(GL_LIGHT1, GL_POSITION, my_cam.look);
  glLightfv(GL_LIGHT1, GL_SPOT_DIRECTION, my_cam.dir);
  
  // reflective propoerites -- global ambiant light
  glLightModelfv(GL_LIGHT_MODEL_AMBIENT, globalAmb);
  
  // setup material properties
  
  float mat[4];
  mat[0] = 0.19225;
  mat[1] = 0.19225;
  mat[2] = 0.19225;
  mat[3] = 1.0;
  glMaterialfv(GL_FRONT, GL_AMBIENT, mat);
  mat[0] = 0.50754;
  mat[1] = 0.50754;
  mat[2] = 0.50754;
  glMaterialfv(GL_FRONT, GL_DIFFUSE, mat);
  mat[0] = 0.508273;
  mat[1] = 0.508273;
  mat[2] = 0.508273;
  glMaterialfv(GL_FRONT, GL_SPECULAR, mat);
  glMaterialf(GL_FRONT, GL_SHININESS, 0.4 * 128.0);
  glPushMatrix();
  int i;
  
 // GLfloat globalAmb[]     = {.5, .5, .5, .5};


  //enable lighting
  glEnable(GL_LIGHTING);
  glEnable(GL_NORMALIZE);
  glEnable(GL_COLOR_MATERIAL);

  // reflective properties -- global ambiant light
  glLightModelfv(GL_LIGHT_MODEL_AMBIENT, globalAmb);

  // this was for the flashlights


  //glLightfv(GL_LIGHT0, GL_AMBIENT, amb);
  //glLightfv(GL_LIGHT0, GL_DIFFUSE, dif);
  //glLightfv(GL_LIGHT0, GL_SPECULAR, spec);
  //glLightf(GL_LIGHT0, GL_SPOT_CUTOFF, 30.0);

 // glEnable(GL_LIGHT0);
  /*
  // setup properties of lighting
  for (i=1; i<=num_lights; i++) {
    glEnable(GL_LIGHT0+i);
    glLightfv(GL_LIGHT0+i, GL_AMBIENT, my_lights[i].amb);
    glLightfv(GL_LIGHT0+i, GL_DIFFUSE, my_lights[i].diff);
    glLightfv(GL_LIGHT0+i, GL_SPECULAR, my_lights[i].spec);
    glLightfv(GL_LIGHT0+i, GL_POSITION, my_lights[i].pos);
    if ((my_lights[i].dir[0] > 0) ||  (my_lights[i].dir[1] > 0) ||  (my_lights[i].dir[2] > 0)) {
      glLightf(GL_LIGHT0+i, GL_SPOT_CUTOFF, my_lights[i].angle);
      glLightfv(GL_LIGHT0+i, GL_SPOT_DIRECTION, my_lights[i].dir);
    }
  }
  */
}



void texture_setup() {
  // set pixel storage mode; see Red Book for details why.
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

  // generate all the textures we might use
  
  // load images from files
  load_bmp("./bmp/wood.bmp", wood_img, smallWidth, &tex_names[0]) ; 
  load_bmp("./bmp/wood2.bmp", wood2_img, smallWidth, &tex_names[1]);
  //load_bmp("./bmp/woodplane.bmp", woodplane_img, smallWidth, &tex_names[2]);
  load_bmp("./bmp/woodplanedrawn.bmp", woodplane_img, smallWidth, &tex_names[2]);
  
}

void load_bmp(char *fname, GLubyte img[], int size, GLuint *ptname) {
  FILE *fp;
  fp = fopen(fname,"rb") ;
  if(fp == NULL) {
    fprintf(stderr,"unable to open texture file %s\n", fname) ;
    exit(1) ;
  }

  fseek(fp,8,SEEK_SET) ;
  fread(img,size*size*3,1,fp) ;
  //bmp2rgb(img, size*size*3);
  fclose(fp) ;
  
  if (ptname) {
    // initialize the texture
    glGenTextures(1, ptname) ; 
    glBindTexture(GL_TEXTURE_2D,*ptname);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT) ;
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT) ; 
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR) ;
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR) ;
	
    
    glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,
		 size,size,
		 0,GL_RGB,GL_UNSIGNED_BYTE,img) ;
  }
  
  
}

void bmp2rgb(GLubyte img[], int size) {
  int i;
  GLubyte temp;

  for (i=0; i<size; i+=3) {
    temp = img[i+2];
    img[i+2] = img[i+1];
    img[i+1] = temp;

  }
}

vertex subtract(vertex a, vertex b) {
	vertex result;
	result.x = a.x - b.x;
	result.y = a.y - b.y;
	result.z = a.z - b.z;
	result.w = 1;

	normalize(result);
	return result;

}