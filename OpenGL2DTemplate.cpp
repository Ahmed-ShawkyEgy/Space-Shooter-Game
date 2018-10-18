//	Includes
#include <stdlib.h>
#include <iostream>
#include <glut.h>
#include <math.h>
#include <vector>

using namespace std;


// Constants

const int SCREEN_WIDTH = 1800;
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
void enemyFireTimer(int v);
void obstacleFireTimer(int v);
void obstacleTimer(int v);
void powerUpTimer(int v);
void moveObstacle();
void reviveEnemy(int v);
void animateDestroyerPowerUp();
void animateFirePowerUp();
float max(float a, float b);
float min(float a, float b);

// Structs

struct point
{
	float x, y;
	point() {}
	point(float a, float b) { x = a; y = b; }
};

struct shape
{
	point center;
	float width, height;
	shape() {}
	shape(point center, float width, float height)
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
point healthBarPos;
int healthBarWidth;

// Player Variables
shape player;
point lastLocation;
float playerSpeed; // TODO Remove
int playerFireRate;
bool playerIsAlive;
int myScore;

// Bullet Vairables
float bulletWidth, bulletHeight, bulletSpeed;
vector<shape> bullets;


// Enemy Variables
shape enemy;
point enemyLastPoint;
float enemySpeed;
float t, enemyDirection;
point p0, p1, p2, p3;
bool enemyIsAlive;
int enemyHealth, enemyFullHealth;
int enemyFireRate; // Bullets per second

// Obstacles Variables
bool obstacleIsAlive;
shape obstacle;
float obstacleSpeed, obstacleWidth, obstacleHeight;
int obstacleDirection;
int obstacleFireRate;

// Hazards Variables
vector<shape> hazards;
float hazardSpeed;
float hazarWidth, hazardHeight;

// PowerUps Variables
vector<shape> destroyerPowerUps;
vector<shape> fireRatePowerUps;
float powerUpWidth, powerUpSpeed;

//-----------------


// Drawing Functions

void drawCircle(point p, float r) {
	float x = p.x, y = p.y;
	glPushMatrix();
	glTranslatef(x, y, 0);
	GLUquadric *quadObj = gluNewQuadric();
	gluDisk(quadObj, 0, r, 50, 50);
	glPopMatrix();
}

void drawRect(point topLeft, point bottomRight)
{
	glBegin(GL_QUADS);
	glVertex3f(topLeft.x, topLeft.y, 0.0f);
	glVertex3f(bottomRight.x, topLeft.y, 0.0f);
	glVertex3f(bottomRight.x, bottomRight.y, 0.0f);
	glVertex3f(topLeft.x, bottomRight.y, 0.0f);
	glEnd();
}

void drawTriangle(point p1, point p2, point p3)
{
	glBegin(GL_TRIANGLES);
	glVertex3f(p1.x, p1.y, 0.0f);
	glVertex3f(p2.x, p2.y, 0.0f);
	glVertex3f(p3.x, p3.y, 0.0f);
	glEnd();
}

void drawPlayer(shape s)
{
	float x = s.center.x, y = s.center.y, width = s.width, height = s.height;
	float wingWidth = width / 10, wingHeight = height / 2;

	glColor3f(0, 1, 0);
	drawRect(point(x - width / 2, y + height / 10), point(x + width / 2, y));// body
	drawRect(point(x - width / 2, y + wingHeight), point(x - width / 2 + wingWidth, y)); // left wing
	drawRect(point(x + width / 2 - wingWidth, y + wingHeight), point(x + width / 2, y)); // right wing
	drawTriangle(point(x - width / 5, y), point(x, y + height), point(x + width / 5, y)); // Head
	glColor3f(1, 1, 1);
}


void drawEnemy(shape s)
{
	float x = s.center.x, y = s.center.y, width = s.width, height = s.height;
	float halfWidth = width / 2, halfHeight = height / 2;

	drawRect(point(x - halfWidth, y), point(x + halfWidth, y - 2*(halfHeight/3)));

	glColor3f(1, 0, 0);
	drawCircle(point(x - halfWidth / 2, y-halfHeight/3), halfWidth / 4);
	drawCircle(point(x + halfWidth / 2, y - halfHeight / 3), halfWidth / 4);

	glColor3f(0.6, 0.8, 0.5);
	drawRect(point(x - 2 * halfWidth / 3, y - 2 * halfHeight / 3), point(x + 2 * halfWidth / 3, y - halfHeight));
	glColor3f(1,1,1);

	glColor3f(1, 0,1);
	drawTriangle(point(x-halfWidth/2,y), point(x,y+halfHeight), point(x+halfWidth/2,y));

	glColor3f(1, 1, 1);
}

void drawBullet(shape s)
{
	glColor3f(0.9,0.9,0);
	float width = s.width, height = s.height;
	float xCenter = s.center.x, yLower = s.center.y;
	float xLeft = xCenter - width / 2, xRight = xCenter + width / 2;
	drawRect(point(xLeft, yLower + height), point(xRight, yLower)); // Body
	drawTriangle(point(xLeft, yLower + height), point(xCenter, yLower + height + height / 2), point(xRight, yLower + height)); // Head
	glColor3f(1,1,1);
}


void drawObstacle(shape s)
{
	float x = s.center.x, y = s.center.y, width = s.width, height = s.height;
	float w = width / 3, h = height / 2;

	drawRect(point(x-w/2 , y+h), point(x+w/2,y-h));


	glColor3f(1, 0, 1);
	drawCircle(point(x - w / 4, y), w / 6);
	drawCircle(point(x + w / 4, y), w / 6);

	glColor3f(0.5, 0, 0);
	drawTriangle(point(x-w-w/2,y), point(x-w/2,y+h), point(x-w/2,y-h));
	drawTriangle(point(x + w + w / 2, y), point(x + w / 2, y + h), point(x + w / 2, y - h));

	glColor3f(1, 1, 1);
}


void drawHazard(shape s)
{
	drawBullet(s);
}


void drawDestroyPowerUp(shape s)
{
	float x = s.center.x, y = s.center.y, width = s.width, height = s.height;
	float w = width / 3, h = height / 2;

	glColor3f(1,0,0);
	glBegin(GL_POLYGON);
	glVertex3f(x, y+h, 0.0f);
	glVertex3f(x+w, y + h/2, 0.0f);
	glVertex3f(x+w, y - h/2, 0.0f);
	glVertex3f(x, y - h, 0.0f);
	glVertex3f(x - w, y - h / 2, 0.0f);
	glVertex3f(x - w, y + h / 2, 0.0f);

	glEnd();
	glColor3f(1, 1, 1);
}

void drawFireRatePowerUp(shape s)
{
	glColor3f(0, 1, 0.5);
	drawCircle(s.center, s.width);
	glColor3f(1, 1, 1);
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
	return (rand() % (upper - lower + 1)) + lower;
}

bool collide(shape a, shape b)
{
	float eps =10;
	return a.center.x < b.center.x + b.width + eps &&
		a.center.x + a.width + eps > b.center.x &&
		a.center.y < b.center.y + b.height + eps &&
		a.height + a.center.y + eps > b.center.y;
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

float max(float a, float b)
{
	return a >= b ? a : b;
}

float min(float a, float b)
{
	return a <= b ? a : b;
}

//----------------- 

// Dynamic Actions

void fireBullet(point location)
{
	bullets.push_back(shape(location, bulletWidth, bulletHeight));
}

void fireHazard(point location)
{
	hazards.push_back(shape(location, hazarWidth, hazardHeight));
}

void destroyAtIndex(int index, vector<shape> &shapes)
{
	// Swap this element with the last one to pop from the vector
	shape tmp = shapes[shapes.size() - 1];
	shapes[shapes.size() - 1] = shapes[index];
	shapes[index] = tmp;
	shapes.pop_back();
}


//----------------- 

void main(int argc, char** argr)
{
	init();
	glutInit(&argc, argr);
	glutInitWindowSize(SCREEN_WIDTH, SCREEN_HEIGHT);
	glutInitWindowPosition(50, 50);
	glutCreateWindow("Space Shooter");
	glutDisplayFunc(Display);
	glutIdleFunc(anime);
	glutKeyboardFunc(Key);

	glutTimerFunc(0, enemyFireTimer, 0);
	glutTimerFunc(0, obstacleTimer, 0);
	glutTimerFunc(0, obstacleFireTimer, 0);
	glutTimerFunc(0, powerUpTimer, 0);

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
	healthBarPos = point(SCREEN_WIDTH / 2, SCREEN_HEIGHT - 30);
	healthBarWidth = 400;
	myScore = 0;


	// Player Variables
	player = shape(point(200.0f, 20.0f), 50.0f, 30.0f);
	playerSpeed = 20.0f;
	playerFireRate = 1;
	playerIsAlive = true;
	lastLocation = point(0, 0);

	// Bullet Variables
	bulletWidth = 10.0f, bulletHeight = 10.0f, bulletSpeed = 18.0f;

	// Enemy Variables
	enemySpeed = 0.004;
	enemyDirection = 1;
	enemy = shape(point(), 80, 80);
	enemyIsAlive = true;
	enemyHealth = enemyFullHealth = 10;
	enemyFireRate = 1;
	enemyLastPoint = point(0, 0);

	// Enemy Path variables
	t = 1;
	p0 = point(enemy.width, SCREEN_HEIGHT / 1.2);
	p3 = point(SCREEN_WIDTH - enemy.height, SCREEN_HEIGHT / 1.2);

	// Hazards Variables
	hazardSpeed = 10.0f;
	hazarWidth = 10.0f, hazardHeight = 10.0f;

	// Obstacle Variables
	obstacleHeight = 50;
	obstacleWidth = 100;
	obstacleSpeed = 10;
	obstacleDirection = RIGHT_DIRECTION;
	obstacleIsAlive = false;
	obstacleFireRate = 1;

	// PowerUp variables
	powerUpWidth = 20;
	powerUpSpeed = 10;
}

void Display(void)
{
	

	glClear(GL_COLOR_BUFFER_BIT);
	glClearColor(0.0f, 0.0f,0.15f, 0.0f); // update the background color
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
		glRotated(180,0,0,1);
		glTranslatef(-hazard.center.x, -hazard.center.y, 0);
		drawHazard(shape(hazard.center, hazard.width, hazard.height));
		glPopMatrix();
	}

	// Render Destroyer PowerUps
	for (unsigned i = 0; i < destroyerPowerUps.size(); i++)
	{
		shape powerUp = destroyerPowerUps[i];
		drawDestroyPowerUp(powerUp);
	}
	// Render fireRate PowerUps
	for (unsigned i = 0; i < fireRatePowerUps.size(); i++)
	{
		shape powerUp = fireRatePowerUps[i];
		drawFireRatePowerUp(powerUp);
	}


	if (playerIsAlive)
	{
		glPushMatrix();
		glTranslatef(player.center.x, player.center.y, 0);
		if (player.center.x > lastLocation.x)
		{
			glRotated(-20,0,0,1);
		}
		else if(player.center.x < lastLocation.x)
			glRotated(20,0,0,1);
		glTranslatef(-player.center.x, -player.center.y, 0);
		drawPlayer(player);
		glPopMatrix();
		lastLocation = player.center;
		
	}


	if (enemyIsAlive)
	{
		glPushMatrix();
		float angle = atan((enemy.center.y-enemyLastPoint.y) / (enemy.center.x - enemyLastPoint.x) );
		angle *= 10;
		glTranslatef(enemy.center.x, enemy.center.y, 0);
		glRotated(angle, 0, 0, 1);
		glTranslatef(-enemy.center.x, -enemy.center.y, 0);
		drawEnemy(enemy);
		glPopMatrix();
		enemyLastPoint = enemy.center;
	}
	if (obstacleIsAlive)
		drawObstacle(obstacle);


	char* score[20];
	sprintf((char *)score, "Score = %d", myScore);
	print(statusBarPos, (char *)score);



	glColor3f(0.4, 0, 0);
	int xHealthRightPos = healthBarPos.x + healthBarWidth / 2;
	int xHealthLeftPos = healthBarPos.x - healthBarWidth / 2;

	drawRect(point(xHealthLeftPos, SCREEN_HEIGHT), point(xHealthRightPos, SCREEN_HEIGHT - 20));
	glColor3f(1, 0, 0);
	drawRect(point(xHealthLeftPos, SCREEN_HEIGHT), point(xHealthLeftPos + (enemyHealth * (xHealthRightPos - xHealthLeftPos) / enemyFullHealth), SCREEN_HEIGHT - 20));
	glColor3f(1, 1, 1);

	glPointSize(3);
	glBegin(GL_POINTS);
	for(int i = 0;i<50;i++)
		glVertex3f(random(0,SCREEN_WIDTH),random(0,SCREEN_HEIGHT), 0.0f);
	glEnd();

	glFlush();
}

void anime()
{
	animateHazards();
	animateBullets();
	animateDestroyerPowerUp();
	animateFirePowerUp();

	if (enemyIsAlive)
		moveEnemy();

	if (obstacleIsAlive)
		moveObstacle();


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
	if (playerIsAlive)
	{
		player.center.x = x;
	}

	glutPostRedisplay();
}


void powerUpTimer(int v)
{
	int chance = random(1, 100);
	int pos = random(10, SCREEN_WIDTH - 10);
	if (chance < 50) 
	{
		destroyerPowerUps.push_back(shape(point(pos, SCREEN_HEIGHT), powerUpWidth*2, powerUpWidth*2));
	}
	else
	{
		fireRatePowerUps.push_back(shape(point(pos, SCREEN_HEIGHT), powerUpWidth, powerUpWidth));
	}

	glutTimerFunc(2 * 1000, powerUpTimer, 0);
}


void obstacleTimer(int v)
{
	if (!obstacleIsAlive)
	{
		obstacle = shape(point(obstacleWidth + 1, SCREEN_HEIGHT / 1.5), obstacleWidth, obstacleHeight);
		obstacleDirection = RIGHT_DIRECTION;
	}
	obstacleIsAlive = true;
	glutTimerFunc(10 * 1000, obstacleTimer, 0);
}

void obstacleFireTimer(int v)
{
	if (obstacleIsAlive)
	{
		int chance = random(1, 100);
		if (chance < 80) // 80% enemy will shoot now
			fireHazard(obstacle.center);
	}
	glutTimerFunc(1000 / obstacleFireRate, obstacleFireTimer, 0);
}

void enemyFireTimer(int v)
{
	if (enemyIsAlive)
	{
		int chance = random(1, 100);
		if (chance < 80) // 80% enemy will shoot now
			fireHazard(enemy.center);
	}
	glutTimerFunc(1000 / enemyFireRate, enemyFireTimer, 0);
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
				myScore++;
				enemyHealth--;
				destroyAtIndex(i--, bullets);
				if (enemyHealth <= 0)
				{
					enemyIsAlive = false;
					glutTimerFunc(5000, reviveEnemy, 0);
				}
			}
			else if (obstacleIsAlive && collide(bullet, obstacle))
			{
				destroyAtIndex(i--, bullets);
				obstacleIsAlive = false;
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


void moveObstacle()
{
	obstacle.center.x += obstacleSpeed * obstacleDirection;
	if (obstacle.outOfBorders())
	{
		obstacleDirection *= -1;
		obstacle.center.x += obstacleSpeed * obstacleDirection;
	}
}

void animateDestroyerPowerUp()
{
	for (unsigned i = 0; i < destroyerPowerUps.size(); i++)
	{
		shape &powerUp = destroyerPowerUps[i];
		if (powerUp.outOfBorders())
			destroyAtIndex(i--, destroyerPowerUps);
		else
		{
			if (playerIsAlive && collide(powerUp, player))
			{
				destroyAtIndex(i--, destroyerPowerUps);
				myScore += enemyHealth / 2;
				enemyHealth -= enemyHealth / 2;

			}
			else
				powerUp.center.y -= hazardSpeed;
		}
	}
}

void animateFirePowerUp()
{
	for (unsigned i = 0; i < fireRatePowerUps.size(); i++)
	{
		shape &powerUp = fireRatePowerUps[i];
		if (powerUp.outOfBorders())
			destroyAtIndex(i--, fireRatePowerUps);
		else
		{
			if (playerIsAlive && collide(powerUp, player))
			{
				destroyAtIndex(i--, fireRatePowerUps);
				playerFireRate *= 2;
			}
			else
				powerUp.center.y -= hazardSpeed;
		}
	}
}

void reviveEnemy(int v)
{
	enemyFullHealth *= 2;
	enemyHealth = enemyFullHealth;
	enemyIsAlive = true;
}



