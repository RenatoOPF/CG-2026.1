#include <GL/glut.h>
#include <GL/glu.h>

    void init(void) 
{
    glClearColor(0.0,0.0,0.0,0.0); //cor preta no fundo
    gluOrtho2D(0.0,200.0,0.0,200); // dimensões de projeção
}
    void lineSegmt(void)
{
    glClear(GL_COLOR_BUFFER_BIT);//limpa tela
    glColor3f(1.0,0.0,0.0);//cor vermelha
    glBegin(GL_LINES);
    glVertex2i(0,0);
    glVertex2i(200,200); //linha reta indo de 0 a 200
    glEnd();
    glFlush(); //processa rotinas do openGL
}
    int main (int argc, char** argv)
{
    //sistema de janelas
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
    glutInitWindowSize(500,500);
    glutCreateWindow("Hello World");
    init(); //define parametros de rendering
    glutDisplayFunc(lineSegmt);//desenha o objeto
    glutMainLoop(); //controle de callback
}