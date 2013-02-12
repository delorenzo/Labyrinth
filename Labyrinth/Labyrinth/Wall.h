#ifndef WALL_H
#define WALL_H
#include <GL/glut.h>
#include <GL/gl.h>
#include <GL/glu.h>

class Wall
{
private:
public:
	struct vertex {
		GLfloat x;
		GLfloat y;
		GLfloat z;
		GLfloat w;
	};

	vertex start;
	vertex end;

	Wall::Wall(vertex a, vertex b)
	{
		start.x = a.x;
		start.y = a.y;
		start.z = a.z;
		start.w = a.w;
		
		end.x = b.x;
		end.y = b.y;
		end.z = b.z;
		end.w = b.w;
	}
};
#endif

