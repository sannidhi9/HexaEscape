#include<windows.h>
#include <GL/glut.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <time.h>
#include<windows.h>
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define TIMER_PERIOD 25
#define SCALE_SPEED 1.020 //the scale amount applied to the hexagons in every frame
#define HEXAGON_DELAY 1250 //the amount of miliseconds passed between hexagons
#define COLOR_CHANGE true //true for repeatedly color change, false for no color change

typedef struct {
	float r, g, b;
} color_t;

typedef struct {
	color_t color;	 //randomly generated r,g,b color
	float scale;	 //scale of the hexagon, which increases in time
	int missingPart; //the empty part of the hexagon so that player can pass through
	int obstacle;
} hexagon_t;

typedef struct {
	bool isStarted, //checks if the game is started or is over
		pause,		//checks if the game is paused
		animate;
} game_control_t;

typedef struct {
	int current,
		max = -1;
} score_t;

score_t score; //keeps current and the maximum score
int width, height;
game_control_t game;
hexagon_t hexagons[4];
int input = 0;	   //the screen is splitted into 6 parts and as players uses arrow keys, the value of this variable changes accordingly
int timerCount = 0;//counts how many times timer function runned
color_t background;//background color

float maxScale; //after this scale, the scale of the hexagons will be initialized to initialScale
float initialScale; //

float rotation;	   //rotation of all objects, hexagons and players, which changes in time
float rotateSpeed;//rotation speed, which randomly switch between 1 and 3

float scale;	  //scale of the whole screen, used in the initial animation and in game in general

//alien cordinate
GLfloat LightColor[][3]={1,1,0,   0,1,1,   0,1,0};
GLfloat AlienBody[][2]={{-4,9}, {-6,0}, {0,0}, {0.5,9}, {0.15,12}, {-14,18}, {-19,10}, {-20,0},{-6,0}};
GLfloat AlienCollar[][2]={{-9,10.5}, {-6,11}, {-5,12}, {6,18}, {10,20}, {13,23}, {16,30}, {19,39}, {16,38},
						  {10,37}, {-13,39}, {-18,41}, {-20,43}, {-20.5,42}, {-21,30}, {-19.5,23}, {-19,20},
						  {-14,16}, {-15,17},{-13,13},  {-9,10.5}};
GLfloat ALienFace[][2]={{-6,11}, {-4.5,18}, {0.5,20}, {0.,20.5}, {0.1,19.5}, {1.8,19}, {5,20}, {7,23}, {9,29},
						{6,29.5}, {5,28}, {7,30}, {10,38},{11,38}, {11,40}, {11.5,48}, {10,50.5},{8.5,51}, {6,52},
						{1,51}, {-3,50},{-1,51}, {-3,52}, {-5,52.5}, {-6,52}, {-9,51}, {-10.5,50}, {-12,49}, {-12.5,47},
						{-12,43}, {-13,40}, {-12,38.5}, {-13.5,33},{-15,38},{-14.5,32},  {-14,28}, {-13.5,33}, {-14,28},
						{-13.8,24}, {-13,20}, {-11,19}, {-10.5,12}, {-6,11} } ;
GLfloat ALienBeak[][2]={{-6,21.5}, {-6.5,22}, {-9,21}, {-11,20.5}, {-20,20}, {-14,23}, {-9.5,28}, {-7,27}, {-6,26.5},
						{-4.5,23}, {-4,21}, {-6,19.5}, {-8.5,19}, {-10,19.5}, {-11,20.5} };
int i=0;
GLint CI=0;
float xOne=0,yOne=0;
int stoneAngle=0;
//end alien
void initializeGlobals()
{
	input = 0;
	scale = 1;
	score.current = 0;
	game.isStarted = false;
	game.pause = false;
	game.animate = false;
	rotateSpeed = (rand() % 100) / 50.0 + 1;
	rotation = 0;
	float frameNeeded = HEXAGON_DELAY / TIMER_PERIOD;
	float scaleMultiplier = powf(SCALE_SPEED, frameNeeded);
	float hexagonScales[4];
	hexagonScales[3] = 0.2;
	for (int i = 2; i >= 0; i--)
		hexagonScales[i] = hexagonScales[i + 1] / scaleMultiplier;
	initialScale = hexagonScales[2];
	maxScale = hexagonScales[3] * pow(scaleMultiplier,3);
	for (int i = 0; i < 4; i++)
	{
		hexagons[i].color = { rand() % 100 / 200.0f + 0.5f, rand() % 100 / 200.0f + 0.5f,rand() % 100 / 200.0f + 0.5f };
		hexagons[i].scale = hexagonScales[i];
		hexagons[i].missingPart = rand() % 6;
		if (hexagons[i].missingPart==3)
            hexagons[i].missingPart=(hexagons[i].missingPart+1)%6;
        hexagons[i].obstacle = 3;

	}
	background = { rand() % 100 / 300.0f, rand() % 100 / 300.0f ,rand() % 100 / 300.0f };
}
//Alien Char
void DrawAlienBody()
{
	glColor3f(0,1,0);				//BODY color
	glBegin(GL_POLYGON);
	for(i=0;i<=8;i++)
		glVertex2fv(AlienBody[i]);
	glEnd();

	glColor3f(0,0,0);			//BODY Outline
	glLineWidth(1);
	glBegin(GL_LINE_STRIP);
	for(i=0;i<=8;i++)
		glVertex2fv(AlienBody[i]);
	glEnd();

	glBegin(GL_LINES);                //BODY effect
		glVertex2f(-13,11);
		glVertex2f(-15,9);
	glEnd();
}
void DrawAlienCollar()
{
	glColor3f(1,0,0);				//COLLAR
	glBegin(GL_POLYGON);
	for(i=0;i<=20 ;i++)
		glVertex2fv(AlienCollar[i]);
	glEnd();

	glColor3f(0,0,0);				//COLLAR outline
	glBegin(GL_LINE_STRIP);
	for(i=0;i<=20 ;i++)
		glVertex2fv(AlienCollar[i]);
	glEnd();
}
void DrawAlienFace()
{
	//glColor3f(0.6,0.0,0.286);				//FACE
	//glColor3f(0.8,0.2,0.1);
	//glColor3f(0,0.5,1);
	glColor3f(0,0,1);
	glBegin(GL_POLYGON);
	for(i=0;i<=42 ;i++)
		glVertex2fv(ALienFace[i]);
	glEnd();

	glColor3f(0,0,0);				//FACE outline
	glBegin(GL_LINE_STRIP);
	for(i=0;i<=42 ;i++)
		glVertex2fv(ALienFace[i]);
	glEnd();

	glBegin(GL_LINE_STRIP);      //EAR effect
		glVertex2f(3.3,22);
		glVertex2f(4.4,23.5);
		glVertex2f(6.3,26);
	glEnd();
}
void DrawAlienBeak()
{
	glColor3f(1,1,0);				//BEAK color
	glBegin(GL_POLYGON);
	for(i=0;i<=14 ;i++)
		glVertex2fv(ALienBeak[i]);
	glEnd();

	glColor3f(0,0,0);				//BEAK outline
	glBegin(GL_LINE_STRIP);
	for(i=0;i<=14 ;i++)
		glVertex2fv(ALienBeak[i]);
	glEnd();
}
void DrawAlienEyes()
{

	glColor3f(0,1,1);

	glPushMatrix();
	glRotated(-10,0,0,1);
	glTranslated(-6,32.5,0);      //Left eye
	glScalef(2.5,4,0);
	glutSolidSphere(1,20,30);
	glPopMatrix();

	glPushMatrix();
	glRotated(-1,0,0,1);
	glTranslated(-8,36,0);							//Right eye
	glScalef(2.5,4,0);
	glutSolidSphere(1,100,100);
	glPopMatrix();
}
void DrawAlien()
{
	DrawAlienBody();
	DrawAlienCollar();
	DrawAlienFace();
	DrawAlienBeak();
	DrawAlienEyes();
}
void DrawSpaceshipBody()
{
	glColor3f(1,0,0);				//BASE

	glPushMatrix();
	glScalef(70,20,1);
	glutSolidSphere(1,50,50);
	glPopMatrix();

	glPushMatrix();							//LIGHTS
	glScalef(3,3,1);
	glTranslated(-20,0,0);			//1
	glColor3fv(LightColor[(CI+0)%3]);
	glutSolidSphere(1,1000,1000);
	glTranslated(5,0,0);					//2
	glColor3fv(LightColor[(CI+1)%3]);
	glutSolidSphere(1,1000,1000);
	glTranslated(5,0,0);					//3
	glColor3fv(LightColor[(CI+2)%3]);
	glutSolidSphere(1,1000,1000);
	glTranslated(5,0,0);					//4
	glColor3fv(LightColor[(CI+0)%3]);
	glutSolidSphere(1,1000,1000);
	glTranslated(5,0,0);					//5
	glColor3fv(LightColor[(CI+1)%3]);
	glutSolidSphere(1,1000,1000);
	glTranslated(5,0,0);					//6
	glColor3fv(LightColor[(CI+2)%3]);
	glutSolidSphere(1,1000,1000);
	glTranslated(5,0,0);					//7
	glColor3fv(LightColor[(CI+0)%3]);
	glutSolidSphere(1,1000,1000);
	glTranslated(5,0,0);					//8
	glColor3fv(LightColor[(CI+1)%3]);
	glutSolidSphere(1,1000,1000);
	glTranslated(5,0,0);					//9
	glColor3fv(LightColor[(CI+2)%3]);
	glutSolidSphere(1,1000,1000);

	glPopMatrix();
}
void DrawSteeringWheel()
{
	glPushMatrix();
	glLineWidth(3);
	glColor3f(0.20,0.,0.20);
	glScalef(7,4,1);
	glTranslated(-1.9,5.5,0);
	glutWireSphere(1,8,8);
	glPopMatrix();

}
void DrawSpaceshipDoom()
{
	glColor4f(0.7,1,1,0.0011);
	glPushMatrix();
	glTranslated(0,20,0);
	glScalef(25,30,0.7);
	glutSolidSphere(0.7,25,25);
	glPopMatrix();
}

void DrawSpaceShipLazer() {

	glColor3f(1, 0, 0);
	glPushMatrix();
	glBegin(GL_POLYGON);           //Lazer stem
		glVertex2f(-55 ,10);
		glVertex2f(-55 ,30);
		glVertex2f(-50, 30);
		glVertex2f(-50 ,10);
	glEnd();

	float xMid =0,yMid =0;
	//Mid point of the lazer horizontal
	xMid = (55+50)/2.0;
	yMid = (25+35)/2.0;

	//Rotating about the point ,20
	glTranslated(-xMid, yMid, 0);
	glRotated(0, 0, 0 ,1);
	glTranslated(xMid , -yMid ,0);

	//find mid point of top of lazer stem
	float midPoint = -(55+50)/2.0;

	glBegin(GL_POLYGON);           //Lazer horizontal stem
		glVertex2f(midPoint + 10 ,25);
		glVertex2f(midPoint + 10 ,35);
		glVertex2f(midPoint - 10 ,35);
		glVertex2f(midPoint - 10 ,25);
	glEnd();

	glPopMatrix();
}

void circle()
{
glPushMatrix();
	glPushMatrix();
	glTranslated(4,19,0);
	DrawAlien();
	glPopMatrix();
	DrawSteeringWheel();
	DrawSpaceshipBody();
	DrawSpaceShipLazer();
	glEnd();
	glPopMatrix();
}


void drawString(const char* string)
{
	glPushMatrix();
	while (*string)
		glutStrokeCharacter(GLUT_STROKE_ROMAN, *string++);
	glPopMatrix();
}

void displayBackground()
{
	glColor4f(0, 0, 0, 0.1);
	for (int i = 0; i < 3; i++)
	{
		glPushMatrix();
		glRotatef(i * 120.0 + rotation, 0, 0, 1);
		glBegin(GL_TRIANGLES);
		glVertex2f(0, 0);
		glVertex2f(-1500 / sqrt(3), -1500);
		glVertex2f(1500 / sqrt(3), -1500);
		glEnd();
		glPopMatrix();
	}
}
void displayObstacle(int x,int y,int r){
#define PI 3.1415
	float angle;
	glBegin(GL_POLYGON);
	for (int i = 0; i < 100; i++)
	{
		angle = 2 * PI*i / 100;
		glVertex2f(x + r * cos(angle), y + r * sin(angle));
	}
	glEnd();
}

void displayHexagons()
{
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 6; j++)
		{
			if (j == hexagons[i].missingPart)
				continue;
            glColor3f(hexagons[i].color.r, hexagons[i].color.g, hexagons[i].color.b);
			glPushMatrix();
			glScalef(hexagons[i].scale, hexagons[i].scale, 0);
			glRotatef(j * 60.0 + rotation, 0, 0, 1);
			glTranslatef(0, -100 * sqrt(3), 0);
			if (j==3){
                displayObstacle(0,50,25);
			}

            glRectf(-100, 0, 100, -5);
			glPopMatrix();
		}
	}
}

void displayPlayer()
{
	glPushMatrix();
	glColor3f(1, 1, 1);
	glRotatef(input * 60.0 + rotation, 0, 0, 1);
	glTranslatef(0, -200, 0);
	circle();
	glPopMatrix();
}

void displayUI()
{
	glPushMatrix();
	if (!game.isStarted)
	{
		glColor4f(0, 0, 0, 0.7);
		glRectf(-300, -100, 300, 100);
		glColor3f(1, 1, 1);
		glTranslatef(-100, 0, 0);
		glScalef(0.3, 0.3, 0);
		drawString("F1 to Start");
		glTranslatef(-40, -100, 0);
		glScalef(0.5, 0.5, 0);
		drawString("Arrow keys to move < >");

		if (score.max != -1)
		{
			char str[100];
			glTranslatef(150, -200, 0);
			sprintf(str, "Max Score: %d", score.max);
			drawString(str);
			glTranslatef(0, -150, 0);
			sprintf(str, "Last Score: %d", score.current);
			drawString(str);
		}

	}
	else
	{
		glTranslatef(-290, 280, 0);
		glScalef(0.1, 0.1, 0);
		if (!game.pause)
			drawString("F2 to Pause");
		else
			drawString("F2 to Continue");
		char str[100];
		glPopMatrix();
		glPushMatrix();
		sprintf(str, "Score: %d", score.current);
		glTranslatef(230, 280, 0);
		glScalef(0.1, 0.1, 0);
		drawString(str);
		glPopMatrix();
		glPushMatrix();
		glTranslatef(230, -280, 0);
		glScalef(0.1, 0.1, 0);
		drawString("F1 to reset");
	}
	glPopMatrix();

}

//
// To display onto window using OpenGL commands
//
void display()
{
	glClearColor(background.r, background.g, background.b, 0);
	glClear(GL_COLOR_BUFFER_BIT);

	glLoadIdentity();
	glScalef(scale, scale, 0);
	if (game.animate)
		glRotatef(rotation, 0, 0, 1);

	displayBackground();
	displayHexagons();
	displayPlayer();
	displayUI();

	glutSwapBuffers();
}

//
// key function for ASCII charachters like ESC, a,b,c..,A,B,..Z
//
void ASCIIKeyDown(unsigned char key, int x, int y)
{
	if (key == 27)
		exit(0);
}

void ASCIIKeyUp(unsigned char key, int x, int y)
{
}


//
// Special Key like F1, F2, F3, Arrow Keys, Page UP, ...
//
void SpecialKeyDown(int key, int x, int y)
{
	if (game.isStarted && !game.pause)
	{
		switch (key) {
		case GLUT_KEY_LEFT:
			input = (input + 5) % 6;
			break;
		case GLUT_KEY_RIGHT:
			input = (input + 1) % 6;
			break;
		}
		if (input==3){
            game.isStarted = false;
            if (score.current > score.max)
                score.max = score.current;
		}

	}
	if (key == GLUT_KEY_F1)
	{
		initializeGlobals();
		game.animate = true;
		scale = 9;

	}
	else if (key == GLUT_KEY_F2)
		game.pause = !game.pause;
}


void SpecialKeyUp(int key, int x, int y)
{
}


//
// When a click occurs in the window,
// It provides which button
// buttons : GLUT_LEFT_BUTTON , GLUT_RIGHT_BUTTON
// states  : GLUT_UP , GLUT_DOWN
// x, y is the coordinate of the point that mouse clicked.
//
void MouseClick(int button, int stat, int x, int y)
{

}


void MouseMove(int x, int y)
{
}


void MouseDownMove(int x, int y)
{
}


//
// This function is called when the window size changes.
// w : is the new width of the window in pixels.
// h : is the new height of the window in pixels.
//
void reshape(int w, int h)
{
	width = w;
	height = h;
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-w / 2, w / 2, -h / 2, h / 2, -1, 1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

}

void onTimer(int v) {
	glutTimerFunc(TIMER_PERIOD, onTimer, 0);
	//initial animation of the game
	if (game.animate)
	{
		timerCount++;
		scale -= 0.1;
		rotation += 9;
		if (scale <= 1)
		{
			scale = 1;
			rotation = 0;
			game.animate = false;
			game.isStarted = true;
		}
	}
	if (game.isStarted && !game.pause)
	{
		timerCount++;
		rotation += rotateSpeed;
		for (int i = 0; i < 4; i++)
		{
			if (fabs(hexagons[i].scale - 1.130) < 0.01)
				if (input != hexagons[i].missingPart)
				{
					game.isStarted = false;
					if (score.current > score.max)
						score.max = score.current;
				}
				else
					score.current++;
			hexagons[i].scale *= SCALE_SPEED;
			if (hexagons[i].scale >= maxScale)
			{
				hexagons[i].scale = initialScale;
			}
		}
		if (timerCount % 25 == 0 && COLOR_CHANGE)
		{
			for (int i = 0; i < 4; i++)
			{
				hexagons[i].color = { rand() % 100 / 200.0f + 0.5f, rand() % 100 / 200.0f + 0.5f,rand() % 100 / 200.0f + 0.5f };
			}
			background = { rand() % 100 / 300.0f, rand() % 100 / 300.0f ,rand() % 100 / 300.0f };
			rotateSpeed = (rand() % 100) / 20.0 - 2.5f;
		}
		else if (timerCount % 50 == 0)
		{
			int rnd = rand() % 60 - 30;
			if (rnd == 0)
				rnd = 180;
			rotation += rnd;
			timerCount = 0;
		}
	}
	glutPostRedisplay();
}

int main(int argc, char *argv[])
{
	initializeGlobals();
	srand(time(NULL));
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
	glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
	glutCreateWindow("Hexa Escape");

	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	//
	// keyboard registration
	glutKeyboardFunc(ASCIIKeyDown);
	glutKeyboardUpFunc(ASCIIKeyUp);

	glutSpecialFunc(SpecialKeyDown);
	glutSpecialUpFunc(SpecialKeyUp);
	//
	// mouse registration
	//
	glutMouseFunc(MouseClick);
	glutPassiveMotionFunc(MouseMove);
	glutMotionFunc(MouseDownMove);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glutTimerFunc(TIMER_PERIOD, onTimer, 0);

	glutMainLoop();
}
