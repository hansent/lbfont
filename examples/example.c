#include <stdlib.h>
#include <GLUT/glut.h>

#include "../lbfont.h"

int frame=0,time=0,timebase=0;

int window_width=0, window_height=0;

float mouse_x=0.0, mouse_y=0.0;

LBFont f;



void mousemove(GLint x,GLint y)
{
	mouse_x = x/(float)window_width;
	mouse_y = x/(float)window_height;
	glutPostRedisplay();
}


void keyboard(unsigned char key, int x, int y){
	if (key == 'm')
		glEnable(GL_MULTISAMPLE);
	else {
		glDisable(GL_MULTISAMPLE);
	}

}

void reshape(int width, int height) {
	window_width=width;
	window_height=height;
	glViewport(0,0,width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-.5, 1.5, -.5, 1.5, -1, 1);
    glMatrixMode(GL_MODELVIEW);
	glEnable(GL_MULTISAMPLE);
}

void idle(void) {
    glutPostRedisplay();
}

void display(void) {
	glClearColor(.5,.5,.5,1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glPushMatrix();
	//glScalef(scale, scale,.5);
	glRotatef(mouse_x*360, 0,0,1);
	renderLBFontString("pymt",&f);
	glPopMatrix();
	glutSwapBuffers();
	
	frame++;
	time=glutGet(GLUT_ELAPSED_TIME);
	if (time - timebase > 5000) {
		printf("FPS:%5.2f\n",frame*5000.0/(time-timebase));
		timebase = time;		
		frame = 0;
	}
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowPosition(600,0);
    glutInitWindowSize(800, 800);
    glutCreateWindow("LBFont Example");
    
	glutMotionFunc(mousemove);
	glutKeyboardFunc(keyboard);
	glutReshapeFunc(reshape);
    glutIdleFunc(idle);
	glutDisplayFunc(display);
	
	initLBFont("/Library/Fonts/Arial.ttf", &f);
	
    glutMainLoop();
}