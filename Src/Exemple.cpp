/* Fonctionnement de GLUt                       */
/*                                              */
/* Auteur: Nicolas JANEY                        */
/* nicolas.janey@univ-fcomte.fr                 */
/* Janvier 2019                                 */

#include <stdlib.h>
#include <stdio.h>
#include <math.h>


#include <GL/glut.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <cmath>

#include "./Pos3D.h"

/* Variables globales                           */

static int wTx = 480;      // Resolution horizontale de la fenetre
static int wTy = 480;      // Resolution verticale de la fenetre
static int wPx = 50;       // Position horizontale de la fenetre
static int wPy = 50;       // Position verticale de la fenetre
static float rx = 0.0F;
static float ry = 0.0F;
static float rz = 0.0F;
static int mx;
static float r1 = 0.0F;
static int mouseActive = 0;
static int nb = 100;
static int nP = 0;
static int aff = 0;

static int taille = 4;
static int isLine = 0;		// Affichage fil de fer

						   /* Fonction d'initialisation des parametres     */
						   /* OpenGL ne changeant pas au cours de la vie   */
						   /* du programme                                 */


/* Calcul la position d'un point sur une courbe  */
/* B-Spline controlee par quatre sommets         */
/* g : le tableau des 4 sommets de controle      */
/* t : la valeur de calcul du point              */
/*     t a prendre dans l'intervalle [0.0,1.0]   */
/* mb : la matrice de base                       */
/* p : le point resultat                         */

static void determinationPositionSurBSpline(Pos3D** g, double t, double mb[4][4], Pos3D* p) {
	double vt[4] = { t * t * t,t * t,t,1.0 };
	double vtmb[4] = { 0.0,0.0,0.0,0.0 };
	for (int j = 0; j < 4; j++) {
		for (int k = 0; k < 4; k++)
			vtmb[j] += vt[k] * mb[k][j];
	}
	p->x = p->y = p->z = 0.0;
	for (int j = 0; j < 4; j++) {
		p->x += vtmb[j] * g[j]->x;
		p->y += vtmb[j] * g[j]->y;
		p->z += vtmb[j] * g[j]->z;
	}
}

/* Calcul les points definissant une courbe      */
/* B-Spline par morceaux definie par un ensemble */
/* de sommets de controle                        */
/* tPos : le tableau des sommets de controle     */
/* n : le nombre de sommets de contr�le          */
/* mb : la matrice de base                       */
/* nb : le nombre de points a calculer           */
/* tRes : le tableau de points resultat          */

static void calculBSpline(Pos3D** tPos, int n, double mb[4][4], int nb, Pos3D** tRes) {
	for (int i = 0; i < nb; i++) {
		double pos = i / (nb - 1.0) * (n - 3);
		int nb = (int)pos;
		if (nb == n - 3)
			nb = n - 4;
		double t = pos - nb;
		determinationPositionSurBSpline(&tPos[nb], t, mb, tRes[i]);
	}
}

void mySolidCube(double c) {
	c /= 2.0;
	glBegin(GL_QUADS);
	glNormal3f(0.0F, 0.0F, 1.0F);
	glVertex3d(c, c, c); // 1 
	glVertex3d(-c, c, c); // 2 
	glVertex3d(-c, -c, c); // 3 
	glVertex3d(c, -c, c); // 4 
	glNormal3f(0.0F, 1.0F, 0.0F);
	glVertex3d(c, c, c); // 1 
	glVertex3d(c, c, -c); // 5 
	glVertex3d(-c, c, -c); // 6 
	glVertex3d(-c, c, c); // 2 
	glNormal3f(1.0F, 0.0F, 0.0F);
	glVertex3d(c, c, c); // 1 
	glVertex3d(c, -c, c); // 4 
	glVertex3d(c, -c, -c); // 7 
	glVertex3d(c, c, -c); // 5 
	glNormal3f(0.0F, -1.0F, 0.0F);
	glVertex3d(c, -c, c); // 4 
	glVertex3d(-c, -c, c); // 3 
	glVertex3d(-c, -c, -c); // 8 
	glVertex3d(c, -c, -c); // 7 
	glNormal3f(-1.0F, 0.0F, 0.0F);
	glVertex3d(-c, -c, c); // 3 
	glVertex3d(-c, c, c); // 2 
	glVertex3d(-c, c, -c); // 6 
	glVertex3d(-c, -c, -c); // 8 
	glNormal3f(0.0F, 0.0F, -1.0F);
	glVertex3d(-c, c, -c); // 6 
	glVertex3d(c, c, -c); // 5 
	glVertex3d(c, -c, -c); // 7 
	glVertex3d(-c, -c, -c); // 8 
	glEnd();
}

void startPlateforme() {
	glPushMatrix();
	glScalef(10.0F, 0.5F, 10.0F);
	mySolidCube(1.0F);
	glPopMatrix();
}

void brasRobot(float r1, float r2) { 
	glPushMatrix(); 
	glRotatef(r1, 0.0F, 1.0F, 0.0F); 
	glTranslatef(1.5F, 0.0F, 0.0F); 
	glScalef(3.0F, 1.0F, 1.0F); 
	glPushMatrix(); 
	glScalef(3.0F, 1.0F, 1.0F); 
	mySolidCube(1.0); 
	glPopMatrix(); 
	glTranslatef(1.5F, 0.0F, 0.0F); 
	glRotatef(r2, 0.0F, 1.0F, 0.0F); 
	glTranslatef(1.5F, 0.0F, 0.0F); 
	glPushMatrix(); 
	glScalef(3.0F, 0.8F, 0.8F); 
	mySolidCube(1.0); 
	glPopMatrix(); 
	glPopMatrix(); 
}

static void mySolidCylindre(double hauteur, double rayon, int ns) {
	/* Protection contre la modification de la normale */
	/* et du flag normalisation                        */
	GLboolean nm = glIsEnabled(GL_NORMALIZE);
	if (!nm)
		glEnable(GL_NORMALIZE);
	float normale[4];
	glGetFloatv(GL_CURRENT_NORMAL, normale);
	/* Modelisation geometrique */
	glPushMatrix();
	hauteur /= 2.0F;
	glBegin(GL_QUAD_STRIP);
	for (int i = 0; i <= ns; i++) {
		float a = (2 * 3.14*i) / ns;
		float cs = cos(a);
		float sn = -sin(a);
		glNormal3f(cs, 0.0F, sn);
		float x = rayon*cs;
		float z = rayon*sn;
		glVertex3f(x, hauteur, z);
		glVertex3f(x, -hauteur, z);
	}
	glEnd();
	glPopMatrix();
	/* Restoration de la normale et du flag normalisation */
	glNormal3f(normale[0], normale[1], normale[2]);
	if (!nm)
		glDisable(GL_NORMALIZE);
}

void brasRobotCylindre(float r1, float r2) {
	glPushMatrix();
	glRotatef(r1, 0.0F, 1.0F, 0.0F);
	glTranslatef(1.5F, 0.0F, 0.0F);
	glPushMatrix();
	glRotatef(90, 0.0, 0.0, 1.0);
	mySolidCylindre(3.0, 1.0, 100);
	glPopMatrix();
	glTranslatef(1.5F, 0.0F, 0.0F);
	glRotatef(r2, 0.0F, 1.0F, 0.0F);
	glTranslatef(1.5F, 0.0F, 0.0F);
	glPushMatrix();
	glRotatef(90, 0.0, 0.0, 1.0);
	mySolidCylindre(3.0, 0.8, 100);
	glPopMatrix();
	glPushMatrix();
	glTranslated(2.0, 0.0, 0.0);
	glScaled(0.5, 1.0, 1.0);
	mySolidCube(2.0);
	glPopMatrix();


	glPopMatrix();
}




static void init(void) {
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glDepthFunc(GL_LESS);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_NORMALIZE);
}

/* Scene dessinee                               */
static void scene(void) {
	//glPushMatrix();
	//glRotatef(r1, 1.0F, 0.0F, 0.0F);
	//mySolidCube(1.0);
	//glPopMatrix();

	/*glPushMatrix(); 
	for (int i = 0; i < 3; i++) { 
		glPushMatrix(); 
		glRotatef(i*120.0F, 0.0F, 1.0F, 0.0F); 
		glTranslatef(1.5F, 0.0F, 0.0F); 
		mySolidCube(2.0); 
		glPopMatrix(); 
	} 
	glPopMatrix();*/

	glPushMatrix();
	brasRobotCylindre(0.0, 10.0);
	startPlateforme();
	glPopMatrix();
}

/* Fonction executee lors d'un rafraichissement */
/* de la fenetre de dessin                      */

static void display(void) {
	//printf("D\n");
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glPushMatrix();
	glRotated(45.0, 1.0, 1.0, 0.0);
	glScalef(0.05F,0.05F,0.05F);
	scene();
	glPopMatrix();
	glFlush();
	glutSwapBuffers();
	int error = glGetError();
	if (error != GL_NO_ERROR)
		printf("Attention erreur %d\n", error);
}

/* Fonction executee lors d'un changement       */
/* de la taille de la fenetre OpenGL            */

static void reshape(int wx, int wy) {
	printf("R\n");
	wTx = wx;
	wTy = wy;
	glViewport(0, 0, wx, wy);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-1.0, 1.0, -1.0, 1.0, -1.0, 1.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

/* Fonction executee lorsqu'aucun evenement     */
/* n'est en file d'attente                      */

static void idle(void) {
	//printf("I\n");
	r1 += 1.0F;
	glutPostRedisplay();
}

/* Fonction executee lors de l'appui            */
/* d'une touche alphanumerique du clavier       */

static void keyboard(unsigned char key, int x, int y) {
	switch (key) {
	case 0x0D:
		taille = (taille == 10) ? 4 : 10;
		glutPostRedisplay();
		break;
	case 0x20:
		aff = (aff + 1) % 6;
		glutPostRedisplay();
		break;
	case 0x1B:
		exit(0);
		break;
	}
}

/* Fonction executee lors de l'appui            */
/* d'une touche speciale du clavier :           */
/*   - touches de curseur                       */
/*   - touches de fonction                      */

static void special(int key, int x, int y) {
	switch (key) {
	case GLUT_KEY_UP:
		rx++;
		glutPostRedisplay();
		break;
	case GLUT_KEY_DOWN:
		rx--;
		glutPostRedisplay();
		break;
	case GLUT_KEY_LEFT:
		ry++;
		glutPostRedisplay();
		break;
	case GLUT_KEY_RIGHT:
		ry--;
		glutPostRedisplay();
		break;
	case GLUT_KEY_PAGE_UP:
		switch (glutGetModifiers()) {
		case GLUT_ACTIVE_ALT:
			nb++;
			glutPostRedisplay();
			break;
		case GLUT_ACTIVE_CTRL:
			nP++;
			if (nP == nb)
				nP = nb - 1;
			glutPostRedisplay();
			break;
		default:
			rz++;
			glutPostRedisplay();
		}
		break;
	case GLUT_KEY_PAGE_DOWN:
		switch (glutGetModifiers()) {
		case GLUT_ACTIVE_ALT:
			nb--;
			if (nb == 1)
				nb = 2;
			if (nP == nb)
				nP = nb - 1;
			glutPostRedisplay();
			break;
		case GLUT_ACTIVE_CTRL:
			nP--;
			if (nP < 0)
				nP = 0;
			glutPostRedisplay();
			break;
		default:
			rz--;
			glutPostRedisplay();
		}
		break;
	}
}

/* Fonction executee lors de l'utilisation      */
/* de la souris sur la fenetre                  */

static void mouse(int button, int state, int x, int y) {
	printf("M  %4d %4d %4d %4d\n", button, state, x, y);
	/*if (state == GLUT_DOWN) {
		mouseActive = 1;
		mx = x;
	}
	if (state == GLUT_UP) {
		mouseActive = 0;
	}*/
}

/* Fonction executee lors du passage            */
/* de la souris sur la fenetre                  */
/* avec un bouton presse                        */

static void mouseMotion(int x, int y) {
	printf("MM %4d %4d\n", x, y);
	/*r1 += (x - mx);
	mx = x;
	glutPostOverlayRedisplay();*/
}

/* Fonction executee lors du passage            */
/* de la souris sur la fenetre                  */
/* sans bouton presse                           */

static void passiveMouseMotion(int x, int y) {
	printf("PM %4d %4d\n", x, y);
}

/* Fonction ex�cut�e automatiquement            */
/* lors de l'ex�cution de la fonction exit()    */

static void clean(void) {
	printf("Bye\n");
}

/* Fonction principale                          */
int main(int argc, char **argv) {

	atexit(clean);

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);
	glutInitWindowSize(wTx, wTy);
	glutInitWindowPosition(wPx, wPy);
	glutCreateWindow("Gestion �v�nementielle de GLUt");
	init();
	glutKeyboardFunc(keyboard);
	glutSpecialFunc(special);
	glutReshapeFunc(reshape);

	glutMouseFunc(mouse);
	glutMotionFunc(mouseMotion);
	//glutPassiveMotionFunc(passiveMouseMotion);
	glutReshapeFunc(reshape);
	//glutIdleFunc(NULL);
	glutDisplayFunc(display);
	glutMainLoop();
	return(0);
}