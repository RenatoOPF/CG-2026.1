//
//  picole.c
//
//  Created by Marcelo Costa on 03/2023.
//  Copyright (c) 2023 Universidade Federal de Alagoas - UFAL. All rights reserved.
//


#include <GL/glut.h> 
//LINUX #include <GL/glut.h> 


void init(void)
{
    /* selecionar cor de fundo (preto) */
    glClearColor (1.0, 1.0, 1.0, 1.0);
        
    gluOrtho2D(0.0, 500.0, 0.0, 500.0); //Definindo os limites da Porta de Visao (ViewPort)
    
}

void picoleFunc(void)
{
    /* Limpar todos os pixels  */
    glClear (GL_COLOR_BUFFER_BIT);
    
    /* Desenhar um polígono branco*/
    glColor3f (0.0, 0.0, 0.0);
    glBegin(GL_POLYGON);
        glVertex2f (249.0f, 250.0f);
        glVertex2f (251.0f, 100.0f);
        glVertex2f (251.0f, 250.0f);
        glVertex2f (249.0f, 100.0f);
    glEnd();
    
    /* Desenhar um triangulo branco*/
    glColor3f (1.0, 0.0, .0);
    glBegin(GL_TRIANGLES);
        glVertex2f (250.0f, 250.0f);
        glVertex2f (230.0f, 200.0f);
        glVertex2f (270.0f, 200.0f);
    glEnd();
    
        
    /* Não esperar! */
    glFlush ();
}



int main(int argc, char** argv) {
    
	glutInit(&argc, argv);
    
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
	glutInitWindowSize(500, 400);
	glutInitWindowPosition(200, 200);
	glutCreateWindow("Picole");
    
	init();
    
	glutDisplayFunc(picoleFunc);
    
	glutMainLoop();
    
	    
}