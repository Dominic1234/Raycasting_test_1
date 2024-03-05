#include <stdio.h>
#include <stdlib.h>
#include <GL/glut.h>
#include <math.h>

#define DEBUG 0
#define reX 1024
#define reY 512
#define PI  3.141592653589793238
#define P2  PI/2
#define P3  3*PI/2
#define DR  0.00872665  	//half a degree in radians
#define SP  32
#define esc 27
#define E   0		//Empty
#define W   1		//Walls
#define D   2		//Doors
#define S   3		//Stairs

unsigned char* door_tex;	//Door texture
float px, py, pz, pdx, pdy, pdz, pa; //player positions

typedef struct {
	int w,a,s,d;	//button states
}ButtonKeys;
ButtonKeys keys;


void drawPlayer() {
	glColor3f(1,0,0);
	glPointSize(8);
	glBegin(GL_POINTS);
	glVertex2i(px,py);
	glEnd();

	glLineWidth(3);
	glBegin(GL_LINES);
	glVertex2i(px+pdx*5, py+pdy*5);
	glEnd();
}

int mapX = 8, mapY = 8, mapS = 64;

int map[] = {
W, W, W, W, W, W, W, W,
S, E, W, E, E, E, E, W,
W, E, W, E, E, E, E, W,
W, E, W, E, E, E, E, W,
W, E, D, E, E, E, E, W,
W, E, W, E, E, E, E, W,
W, E, W, E, E, E, E, W,
W, W, W, W, W, W, W, W,
};

void drawMap2D() {
	int x,y,xo,yo;
	for(y=0; y < mapY; y++) {
		for(x=0; x < mapX; x++) {
			if(map[y*mapX+x] == W)      {glColor3f(1,1,1);}
			else if(map[y*mapX+x] == D) {glColor3f(1,0,1);}
			else if(map[y*mapX+x] == S) {glColor3f(1,0,0);}
			else                        {glColor3f(0,0,0);}
			xo = x*mapS; yo = y*mapS;
			glBegin(GL_QUADS);
			glVertex2i(xo     +1, yo     +1);
			glVertex2i(xo     +1, yo+mapS-1);
			glVertex2i(xo+mapS-1, yo+mapS-1);
			glVertex2i(xo+mapS-1, yo     +1);
			glEnd();
		}
	}
}

float dist(float ax, float ay, float bx, float by, float ang) {
	return (sqrt((bx-ax)*(bx-ax)+(by-ay)*(by-ay)));
}

void drawFloor3D(int w, int x, int y, int z) {
	glColor3f(0.4961,0.42578,0.359375);
	glLineWidth(4);
	glBegin(GL_LINES);
	glVertex2i(w,x);
	glVertex2i(y,z);
	glEnd();
}

void drawRoof3D(int w, int x, int y, int z) {
	glColor3f(0.65, 0.65, 0.55);
	glLineWidth(4);
	glBegin(GL_LINES);
	glVertex2i(w,x);
	glVertex2i(y,z);
	glEnd();
}

void drawObj3D(int r, float ra, float disT, float lineO, float lineH) {

	glLineWidth(4);
	glBegin(GL_LINES);
	glVertex2i(r*4+530,lineO);
	glVertex2i(r*4+530,lineH+lineO);
	glEnd();
}

void drawRays3D() {
	int r, mx, my, mp, dof, obj;
	float rx, ry, ra, xo, yo, disT;
	ra = pa-(DR*30); if(ra<0){ra+=2*PI;} if(ra>2*PI){ra-=2*PI;}
	for(r = 0; r < 120; r++) {

//---Check Horizontal Lines---
		dof = 0;
		float disH=100000000, hx=px, hy=py;
		float aTan=-1/tan(ra);
		if(ra > PI){		//looking up
			ry=(((int)py>>6)<<6)-0.0001;
			rx=(py-ry)*aTan+px;
			yo=-64; xo=-yo*aTan;
		}
		if(ra < PI){		//looking down
			ry=(((int)py>>6)<<6)+64;
			rx=(py-ry)*aTan+px;
			yo= 64; xo=-yo*aTan;
		}
		if(ra==0 || ra == PI){rx=px; ry=py; dof=8;}	//looking straight left or right
		while(dof<8) {
			mx = (int)(rx)>>6; my = (int)(ry)>>6; mp=my*mapX+mx;
			if(mp > 0 && mp<mapX*mapY && map[mp]>E){	//hit object
				hx = rx; hy = ry;
				disH = dist(px,py,hx,hy,ra);
				dof=8; obj = map[mp];
			}
			else {
				rx+=xo;
				ry+=yo;
				dof+=1;
			}//next line
		}

//---Check Vertical Lines---
		dof = 0;
		float disV=100000000, vx=px, vy=py;
		float nTan=-tan(ra);
		if(ra > P2 && ra < P3) {		//looking left
			rx=(((int)px>>6)<<6)-0.0001;
			ry=(px-rx)*nTan+py;
			xo=-64;
			yo=-xo*nTan;
		}
		if(ra < P2 || ra > P3){		//looking right
			rx=(((int)px>>6)<<6)+64;
			ry=(px-rx)*nTan+py;
			xo= 64;
			yo=-xo*nTan;
		}
		if(ra==0 || ra == PI){rx=px; ry=py; dof=8;}	//looking straight up or down
		while(dof<8) {
			mx = (int)(rx)>>6; my = (int)(ry)>>6; mp=my*mapX+mx;
			if(mp > 0 && mp<mapX*mapY && map[mp]>E){	//hit object
				vx = rx; vy = ry;
				disV = dist(px,py,vx,vy,ra);
				dof=8; obj = map[mp];
			}
			else {
				rx+=xo;
				ry+=yo;
				dof+=1;
			}//next line
		}
		if(obj > 0){
			if(disV<disH)     {rx=vx;ry=vy; disT=disV;}	//vertical ray hit
			else if(disH<disV){rx=hx;ry=hy; disT=disH;}		//horizontal ray hit
		}
		float ca=pa-ra;
		if(ca<0){ca+=2*PI;} if(ca>2*PI){ca-=2*PI;} disT=disT*cos(ca);	//fix fisheye
		float lineH=(mapS*320)/disT;
		if(lineH>320) {lineH=320;}	//line height
		float lineO=160-lineH/2;	//line offset
		//Check for kind of object
		if(obj == W) {
			glColor3f(0.81250*(1-(disT/reY)),0.81250*(1-(disT/reY)),0.68750*(1-(disT/reY)));
		}
		else if(obj == D) {
			glColor3f(0.51172*(1-(disT/reY)),0.50781*(1-(disT/reY)),0.39063*(1-(disT/reY)));
		}
		else if(obj == S) {
			glColor3f(0      *(1-(disT/reY)),0      *(1-(disT/reY)),0      *(1-(disT/reY)));
		}
		else { printf("%d\n",obj);}
		glLineWidth(1);
		glBegin(GL_LINES);
		glVertex2i(px,py);
		glVertex2i(rx,ry);
		glEnd();

//---Draw 3D---
		drawObj3D(r, ra, disT, lineO, lineH);
		drawFloor3D(r*4+530,320,r*4+530,lineO+lineH);
		drawRoof3D(r*4+530, 0, r*4+530, lineO);
		ra+=DR; if(ra<0){ra+=2*PI;} if(ra>2*PI){ra-=2*PI;}
	}
}

void display() {
	//buttons
	if(keys.a == 1) {	//Strafe Left
		int pos = map[((int)(((py+pdy)/reY)*mapY)*mapX+(int)(((px-pdx)/(reX/2))*mapX))];
		if(DEBUG)printf("%d*%d+%d=%d\n", (int)(((py+pdy)/reY)*mapY), mapX, (int)(((px-pdx)/(reX/2))*mapX), ((int)(((py+pdy)/reY)*mapY)*mapX+(int)(((px-pdx)/(reX/2))*mapX)));
		if(pos == E || pos == D) {
			px-=pdx; py+=pdy;
		}
	}
	if(keys.d == 1) {	//Strafe Right
		int pos = map[((int)(((py-pdy)/reY)*mapY)*mapX+(int)(((px+pdx)/(reX/2))*mapX))];
		if(DEBUG)printf("%d*%d+%d=%d\n", (int)(((py-pdy)/reY)*mapY), mapX, (int)(((px+pdx)/(reX/2))*mapX), ((int)(((py-pdy)/reY)*mapY)*mapX+(int)(((px+pdx)/(reX/2))*mapX)));
		if(pos == E || pos == D) {
			px+=pdx; py-=pdy;
		}
	}
	if(keys.w == 1 && (px+pdx >= 0 && px+pdx <= reX/2) && (py+pdy > 0 && py+pdy < reY)) {
		int pos = map[((int)(((py+pdy)/reY)*mapY)*mapX+(int)(((px+pdx)/(reX/2))*mapX))];
		if(DEBUG)printf("%d*%d+%d=%d\n", (int)(((py+pdy)/reY)*mapY), mapX, (int)(((px+pdx)/(reX/2))*mapX), ((int)(((py+pdy)/reY)*mapY)*mapX+(int)(((px+pdx)/(reX/2))*mapX)));
		if(pos == E || pos == D) {
			px+=pdx; py+=pdy;
		}
		if(DEBUG)printf("px=%f, py=%f\n", px, py);
	}
	if(keys.s == 1 && (px-pdx > 0 && px-pdx < reX/2) && (py-pdy > 0 && py-pdy < reY)) {
		int pos = map[((int)(((py-pdy)/reY)*mapY)*mapX+(int)(((px-pdx)/(reX/2))*mapX))];
		if(DEBUG)printf("%d*%d+%d=%d\n", (int)(((py-pdy)/reY)*mapY), mapX, (int)(((px-pdx)/(reX/2))*mapX), ((int)(((py-pdy)/reY)*mapY)*mapX+(int)(((px-pdx)/(reX/2))*mapX)));
		if(pos == E || pos == D) {
			px-=pdx; py-=pdy;
		}
		if(DEBUG)printf("px=%f, py=%f\n", px, py);
	}
	glutPostRedisplay();

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	drawMap2D();
	drawPlayer();
	drawRays3D();
	glutSwapBuffers();
}

void sp_buttons(int key, int x, int y) {
	if(key == GLUT_KEY_LEFT) {		//Left turn
		pa-=0.1;
		if(pa < 0)    {
			pa+=2*PI;
		}
		pdx = cos(pa)*5; pdy = sin(pa)*5;
	}
	if(key == GLUT_KEY_RIGHT) {	//Right turn
		pa+=0.1;
		if(pa > 2*PI) {
			pa-=2*PI;
		}
		pdx = cos(pa)*5; pdy = sin(pa)*5;
	}
	glutPostRedisplay();
}

void init() {
	glClearColor(0.3,0.3,0.3,0);
	gluOrtho2D(0,1024,512,0);
	pa = 0; px = 300; py = 300; pdx = cos(pa)*5; pdy = sin(pa)*5;
}

void ButtonDown(unsigned char key, int x, int y) {
	if(key=='a') {keys.a = 1;}
	if(key=='d') {keys.d = 1;}
	if(key=='w') {keys.w = 1;}
	if(key=='s') {keys.s = 1;}
	glutPostRedisplay();
}

void ButtonUp(unsigned char key, int x, int y) {
	if(key=='a') {keys.a = 0;}
	if(key=='d') {keys.d = 0;}
	if(key=='w') {keys.w = 0;}
	if(key=='s') {keys.s = 0;}
	glutPostRedisplay();
}

void resize(int w, int h) {
	glutReshapeWindow(reX, reY);
}

int main(int argc, char* argv[]) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
	glutInitWindowSize(reX, reY);
	glutInitWindowPosition(200, 200);
	glutCreateWindow("3D-Test-1");
	init();
	glutDisplayFunc(display);
	glutReshapeFunc(resize);
	glutKeyboardFunc(ButtonUp);
	glutKeyboardFunc(ButtonDown);
	glutSpecialFunc(sp_buttons);
	glutMainLoop();
	return 0;
}
