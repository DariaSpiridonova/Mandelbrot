#include <GL/glut.h>

void display() {
glClear(GL_COLOR_BUFFER_BIT);
glBegin(GL_TRIANGLES);
glColor3f(1.0, 0.0, 0.0);
glVertex2f(-0.5, -0.5);
glColor3f(0.0, 1.0, 0.0);
glVertex2f(0.5, -0.5);
glColor3f(0.0, 0.0, 1.0);
glVertex2f(0.0, 0.5);
glEnd();
glutSwapBuffers();
}

int main(int argc, char** argv) {
glutInit(&argc, argv);
glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
glutInitWindowSize(500, 500);
glutCreateWindow("OpenGL Triangle");
glutDisplayFunc(display);
glutMainLoop();
return 0;
}

