#include <windows.h>
#include <GL/glut.h>
#include <stdio.h>

void display(void)
{
/*  clear all pixels  */
    glClear (GL_COLOR_BUFFER_BIT);

/*  draw white polygon (rectangle) with corners at
 *  (0.25, 0.25, 0.0) and (0.75, 0.75, 0.0)  
 */
 //	float myColor[]={};
    glColor3f (1.0,1.0,1.0);
 //   glPolygonMode(GL_FRONT,GL_POINT);
//    glPolygonMode(GL_FRONT,GL_LINE);
   glPolygonMode(GL_FRONT,GL_FILL);
	// glPointSize(4);
//	 glBegin(GL_POINTS);
//    glBegin(GL_LINES);
//    glBegin(GL_LINE_STRIP);
//    glBegin(GL_LINE_LOOP);
//	glColor3f (0.0,1.0,1.0);

	glBegin(GL_POLYGON);
	    glColor3f (0.0,1.0,1.0);
        glVertex3f (0.25, 0.25, 0.0);
	    glColor3f (1.0,0.0,1.0);
        glVertex3f (0.75, 0.25, 0.0);
 	    glColor3f (1.0,1.0,0.0);
        glVertex3f (0.75, 0.75, 0.0);
  	    glColor3f (1.0,0.0,0.0);
        glVertex3f (0.25, 0.75, 0.0);
    glEnd();

/*  don't wait!  
 *  start processing buffered OpenGL routines 
 */
    glFlush ();
}

void init (void) 
{
/*  select clearing (background) color       */
    glClearColor (0.0, 0.0, 0.0, 0.0);
    glShadeModel (GL_SMOOTH);

}
 

void reshape(int w, int h)
{
   glViewport (0, 0, (GLsizei) w, (GLsizei) h);
   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   glOrtho(0.0, 1.0, 0.0, 1.0, -1.0, 1.0);
   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();
}
 
/* 
 *  Declare initial window size, position, and display mode
 *  (single buffer and RGBA).  Open window with ¡°hello¡±
 *  in its title bar.  Call initialization routines.
 *  Register callback function to display graphics.
 *  Enter main loop and process events.
 */
int main(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode (GLUT_SINGLE | GLUT_RGB);
    glutInitWindowSize (250, 250); 
    glutInitWindowPosition (100, 100);
    glutCreateWindow ("HELLO");
	init ();
	glutReshapeFunc(reshape); 
    glutDisplayFunc(display); 
    glutMainLoop();
    return 0;   /* ISO C requires main to return int. */
}