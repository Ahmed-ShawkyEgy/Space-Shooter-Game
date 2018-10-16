//	Includes
#include <stdlib.h>
#include <iostream>
#include <glut.h>
#include <math.h>
#include <vector>

using namespace std;


// Constants

const int SCREEN_WIDTH = 1000;
const int SCREEN_HEIGHT = 900;
const int RIGHT_DIRECTION = 1, LEFT_DIRECTION = -1;

//-----------------
//	Methods Signatures
void Display();
void anime();
void init();
void Key(unsigned char key, int x, int y);
void Mouse(int x, int y);
void animateBullets();
void animateHazards();
void moveEnemy();
void EnemyFireTimer(int v);
// Structs

struct point
{
	float x, y;
	point() {}
	point(float a, float b){x = a; y = b;}
};

struct shape
{
	point center;
	float width, height;
	shape() {}
	shape(point center,float width, float height) 
	{
		this->center = center;
		this->width = width, this->height = height; 
	}

	bool outOfBorders()
	{
		float x = center.x, y = center.y;
		return x > SCREEN_WIDTH || x < 0 || y > SCREEN_HEIGHT || y < 0;
	}
	
};



//-----------------

//	Global Variables

// Status Bar
	point statusBarPos;

// Player Variables
	shape player;
	float playerSpeed ; // TODO Remove
	int playerFireRate;
	bool playerIsAlive;

// Bullet Vairables
	float bulletWidth, bulletHeight , bulletSpeed;
	vector<shape> bullets;


// Enemy Variables
	shape enemy;
	float enemySpeed;
	float t,enemyDirection;
	point p0, p1, p2, p3;
	bool enemyIsAlive;
	int enemyHealth;

// Obstacles Variables
	vector<shape> obstacles;

// Hazards Variables
	vector<shape> hazards;
	float hazardSpeed;
	float hazarWidth, hazardHeight;

// PowerUps Variables
	vector<shape> powerUps;

//-----------------


// Drawing Functions

void drawRect(point topLeft , point bottomRight)
{
	glBegin(GL_QUADS); 
	glVertex3f(topLeft.x,topLeft.y, 0.0f);
	glVertex3f(bottomRight.x, topLeft.y, 0.0f);
	glVertex3f(bottomRight.x,bottomRight.y, 0.0f);
	glVertex3f(topLeft.x,bottomRight.y, 0.0f);
	glEnd();
}

void drawTriangle(point p1, point p2,point p3)
{
	glBegin(GL_TRIANGLES);
	glVertex3f(p1.x, p1.y, 0.0f);
	glVertex3f(p2.x, p2.y, 0.0f);
	glVertex3f(p3.x,p3.y, 0.0f);
	glEnd();
}

void drawPlayer(shape s)
{
	float x = s.center.x, y = s.center.y , width = s.width , height = s.height;
	float wingWidth = width / 10 , wingHeight = height/2;
	drawRect(point(x-width/2,y+height/10) , point(x+width/2 , y));// body
	drawRect(point(x-width/2,y+ wingHeight) , point(x -width/2 + wingWidth, y)); // left wing
	drawRect(point(x + width / 2 - wingWidth, y + wingHeight) , point( x + width / 2, y)); // right wing
	drawTriangle(point(x - width/5, y) , point(x, y + height), point(x + width/5, y)); // Head
}

// TODO Implement
void drawEnemy(shape s)
{
	float x = s.center.x, y = s.center.y, width = s.width, height = s.height;
	drawRect(point(x - width / 2, y + height / 2), point(x+width/2 , y-height/2));
}

void drawBullet(shape s)
{
	float width = s.width, height = s.height;
	float xCenter = s.center.x , yLower  = s.center.y ;
	float xLeft = xCenter - width / 2, xRight = xCenter + width / 2;
	drawRect(point(xLeft, yLower + height), point(xRight, yLower)); // Body
	drawTriangle(point(xLeft, yLower + height), point(xCenter, yLower + height + height / 2), point(xRight , yLower+height)); // Head
}

// TODO Implement
void drawObstacle(shape s) 
{
	drawEnemy(s);
}

// TODO Implement
void drawHazard(shape s)
{
	drawBullet(s);
}

  // Helper Functions

point bezier(float t, point p0, point p1, point p2, point p3)
{
	point res = point(0, 0);
	res.x = pow((1 - t), 3)*p0.x + 3 * t*pow((1 - t), 2)*p1.x + 3 * pow(t, 2)*(1 - t)*p2.x + pow(t, 3)*p3.x;
	res.y = pow((1 - t), 3)*p0.y + 3 * t*pow((1 - t), 2)*p1.y + 3 * pow(t, 2)*(1 - t)*p2.y + pow(t, 3)*p3.y;
	return res;
}

int random(int lower, int upper)
{
	return (rand() % (upper-lower+1)) + lower;
}

bool collide(shape a, shape b)
{
	/*float ax1 = a.center.x - a.width / 2;
	float ax2 = a.center.x + a.width / 2;
	float ay1 = a.center.y + a.height / 2;
	float ay2 = a.center.y - a.height / 2;

	float bx1 = b.center.x - b.width / 2;
	float bx2 = b.center.x + b.width / 2;
	float by1 = b.center.y + b.height / 2;
	float by2 = b.center.y - b.height / 2;
*/
	return a.center.x < b.center.x + b.width &&
		a.center.x + a.width > b.center.x &&
		a.center.y < b.center.y + b.height &&
		a.height + a.center.y > b.center.y;
}

void print(point p, char *string)
{
	float x = p.x, y = p.y;
	int len, i;

	//set the position of the text in the window using the x and y coordinates
	glRasterPos2f(x, y);

	//get the length of the string to display
	len = (int)strlen(string);

	//loop to display character by character
	for (i = 0; i < len; i++)
	{
		glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, string[i]);
	}
}
//----------------- 

// Dynamic Actions

void fireBullet(point location)
{
	bullets.push_back(shape(location , bulletWidth , bulletHeight));
}

void fireHazard(point location)
{
	hazards.push_back(shape(location, hazarWidth, hazardHeight));
}

void destroyAtIndex(int index,vector<shape> &shapes)
{
	// Swap this element with the last one to pop from the vector
	shape tmp = shapes[shapes.size()-1];
	shapes[shapes.size()-1] = shapes[index];
	shapes[index] = tmp;
	shapes.pop_back();
}


//----------------- 

void main(int argc, char** argr)
{
	init();
	glutInit(&argc, argr);
	glutInitWindowSize(SCREEN_WIDTH,SCREEN_HEIGHT);
	glutInitWindowPosition(50, 50);
	glutCreateWindow("Space Shooter");
	glutDisplayFunc(Display);
	glutIdleFunc(anime);
	glutKeyboardFunc(Key);      // sets the Keyboard handler function; called when a key is pressed
	glutTimerFunc(0, EnemyFireTimer, 0); // sets the Timer handler function; which runs every `Threshold` milliseconds (1st argument)
	glutPassiveMotionFunc(Mouse);
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);	
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	gluOrtho2D(0.0, SCREEN_WIDTH, 0.0, SCREEN_HEIGHT);
	glutMainLoop();
}


void init()
{
	// Status Bar
	statusBarPos = point(10, SCREEN_HEIGHT - 30);

	// Player Variables
	player = shape(point(200.0f, 20.0f), 50.0f, 30.0f);
	playerSpeed = 20.0f ;
	playerFireRate = 50;
	playerIsAlive = true;

	// Bullet Variables
	bulletWidth = 10.0f, bulletHeight = 10.0f , bulletSpeed = 18.0f;

	// Enemy Variables
	enemySpeed = 0.005;
	enemyDirection = 1;
	enemy = shape(point(), 50, 50);
	enemyIsAlive = true;
	enemyHealth = 100;

	// Enemy Path variables
	t = 1;
	p0 = point(enemy.width, SCREEN_HEIGHT/1.3);
	p3 = point(SCREEN_WIDTH - enemy.height,SCREEN_HEIGHT/1.3);

	// Hazards Variables
	hazardSpeed = 10.0f;
	hazarWidth = 10.0f, hazardHeight = 10.0f;
}

void Display(void)
{
	//glClearColor(0.0f, 0.0f,0.0f, 0.0f); // update the background color
	glClear(GL_COLOR_BUFFER_BIT);

	// Render bullets
	for (unsigned i = 0; i < bullets.size(); i++)
	{
		shape bullet = bullets[i];
		drawBullet(shape(bullet.center, bullet.width, bullet.height));
	}

	// Render Hazards
	for (unsigned i = 0; i < hazards.size(); i++)
	{
		shape hazard = hazards[i];
		glPushMatrix();
		glTranslatef(hazard.center.x, hazard.center.y, 0);
		glRotated(180,1,0, 0);
		glTranslatef(-hazard.center.x , -hazard.center.y,0);
		drawHazard(shape(hazard.center, hazard.width, hazard.height));
		glPopMatrix();
	}

	if(playerIsAlive)
		drawPlayer(player); 
	if(enemyIsAlive)
		drawEnemy(enemy);

	char* health[20];
	sprintf((char *)health, "Health = %d", enemyHealth);
	print(statusBarPos, (char *)health);

	glFlush();
}

void anime()
{
	animateHazards();
	animateBullets();

	if (enemyIsAlive)
		moveEnemy();
	
	

	for (int i = 0; i < 1e7; i++);
	glutPostRedisplay();
}

void Key(unsigned char key, int x, int y)
{
	
	switch (key)
	{
	case ' ': // Fire
		if (playerIsAlive && bullets.size() < playerFireRate)
		{
			fireBullet(player.center);
		}
		break;

	default:
		break;
	}

	glutPostRedisplay();

}


void Mouse(int x, int y) 
{
	if(playerIsAlive)
	player.center.x = x;

	glutPostRedisplay();
}






void EnemyFireTimer(int v)
{
	if (!enemyIsAlive)
		return;
	int chance = random(1, 100);
	if (chance<80) // 80% enemy will shoot now
		fireHazard(enemy.center);
	
	glutTimerFunc(1 * 1000, EnemyFireTimer, 0);
}


void animateHazards()
{
	for (unsigned i = 0; i < hazards.size(); i++)
	{
		shape &hazard = hazards[i];
		if (hazard.outOfBorders())
			destroyAtIndex(i--, hazards);
		else
		{
			if (playerIsAlive && collide(hazard, player))
			{
				destroyAtIndex(i--, hazards);
				playerIsAlive = false;
			}
			else
				hazard.center.y -= hazardSpeed;
		}
	}
}

void animateBullets()
{
	for (unsigned i = 0; i < bullets.size(); i++)
	{
		shape &bullet = bullets[i];
		if (bullet.outOfBorders())
			destroyAtIndex(i--, bullets);
		else
		{
			if (enemyIsAlive && collide(bullet, enemy))
			{
				enemyHealth--;
				destroyAtIndex(i--, bullets);
				if(enemyHealth<=0)
					enemyIsAlive = false;
			}
			else
				bullet.center.y += bulletSpeed;
		}
	}
}


void moveEnemy()
{
	if (t <= 0 || t >= 1)
	{
		if (t <= 0)
			enemyDirection = RIGHT_DIRECTION;
		else if (t >= 1)
			enemyDirection = LEFT_DIRECTION;

		p1 = point(random(0, SCREEN_WIDTH), random(SCREEN_HEIGHT / 2, SCREEN_HEIGHT - enemy.height));
		p2 = point(random(0, SCREEN_WIDTH), random(SCREEN_HEIGHT / 2, SCREEN_HEIGHT - enemy.height));

	}

	point newEnemyPos = bezier(t, p0, p1, p2, p3);
	if (newEnemyPos.y + enemy.height > SCREEN_HEIGHT) // Clamp position to frame borders
		newEnemyPos.y = enemy.center.y;

	enemy.center = newEnemyPos;
	t += enemyDirection * enemySpeed;
}
