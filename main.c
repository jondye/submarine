#include <GL/glut.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <string.h>

#ifdef WIN32
#include <windows.h>
#endif

#include "textureLoad.c"

#define WIN_X 400
#define WIN_Y 400

/* in case math.h does not define PI */
#ifndef PI
#define PI 3.141593
#endif

/* types */
typedef struct {
  GLfloat position[3];
  GLfloat xVelocity, yVelocity, zVelocity;
  int active;
} bubble;

/* Constants */
#define NUMBER_OF_TEXTURES 3
#define SAND 0
#define GROUND 1
#define WOOD 2
#define CONE_SEGMENTS 10
#define MAX_BUBBLES 60
#define TIME_BETWEEN_BUBBLES 0.3
#define BOUYANCY 10
#define BUBBLE_BOUNCE 0.3
#define SUBMARINE_SEGMENTS 16
#define OUTSIDE 0
#define IN_SUB 1
#define SUB_ACCELERATION 0.4
#define WATER_RESISTANCE 0.7
#define SUB_BOUNCE 0.5
#define WATER_SIDES_SUBDIVISION 3
#define WATER_TOP_SUBDIVISION 10
#define SPOTLIGHT_WIDTH 30

/* Variables */
GLuint textures[NUMBER_OF_TEXTURES];
char textureFiles[NUMBER_OF_TEXTURES][20] = {"sand.rgb", "surface.rgb", "tex0-28420"};
int light0 = 1;
int light1 = 1;
int light2 = 1;
int light3 = 1;
int light4 = 1;
int light5 = 1;
int light6 = 1;
bubble bubbles[MAX_BUBBLES];
int viewPosition = OUTSIDE;
struct {
  GLfloat x, y, z;
  GLfloat xVelocity, yVelocity, zVelocity;
  GLfloat dive;
  GLfloat turn;
} sub;


/* Display lists */
GLuint ground;
GLuint tank;
GLuint waterBack;
GLuint waterFront;
GLuint lights;
GLuint aerator;
GLuint submarine;

/* Callbacks */
void Display(void);
void Reshape(int, int);
void Special(int, int, int);
void Keyboard(unsigned char, int, int);
void Idle(void);
void Menu(int);

/* Initialisation */
void InitMenu(void);
void InitGround(void);
void InitTank(void);
void InitTextures(void);
void InitWater(void);
void InitLights(void);
void InitAerator(void);
void InitBubbles(void);
void InitSubmarine(void);

/* Drawing functions */
void DrawLights(void);
void DrawBubbles(void);
void DrawSubmarine(void);

/* Helper functions */
int HitShelf(bubble);
int HitWall(bubble);
void AccelerateSubmarine(GLfloat);
void CollisionDetection(void);
void SubdivideXY(GLfloat[3], GLfloat[3], GLfloat*, int);
void SubdivideYZ(GLfloat[3], GLfloat[3], GLfloat*, int);
void SubdivideXZ(GLfloat[3], GLfloat[3], GLfloat*, int);
float RandF(void);
void Normalise(GLfloat[3]);

int main(int argc, char *argv[]) {
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
  glutInitWindowSize(WIN_X, WIN_Y);
  glutCreateWindow("CGV Assessment 2001 - Candidate 28420");

  printf("\n\n**************************************************\n");
  printf("*     CGV Assessment 2001 - Candidate 28420      *\n");
  printf("**************************************************\n");
  printf("\nKEYS:\n\n");
  printf("1-6\t\tTurn lights 1-6 on and off\n");
  printf("f\t\tMove submarine forwards\n");
  printf("j\t\tMove submarine backwards\n");
  printf("UP ARROW\tTilt submarine up\n");
  printf("DOWN ARROW\tTilt submarine down\n");
  printf("LEFT ARROW\tRotate submarine to port\n");
  printf("RIGHT ARROW\tRotate submarine to starboard\n");
  printf("i\t\tSwitch view to inside submarine (also in MMB menu)\n");
  printf("o\t\tSwitch view to outside submarine (also in MMB menu)\n");
  printf("ESC\t\tExit program (also in MMB menu)\n\n");

  glClearColor(0.3, 0.3, 0.3, 1.0); /* Grey background */
  /* smooth shading */
  glShadeModel(GL_SMOOTH);
  glEnable(GL_DEPTH_TEST);
  /* enable lighting */
  glEnable(GL_LIGHTING);
  /* enable blending */
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_BLEND);

  /* initialise random numbers */
  srand((unsigned int) time(NULL));

  /* Initialise */
  InitMenu();
  InitTextures();
  InitGround();
  InitTank();
  InitWater();
  InitLights();
  InitAerator();
  InitBubbles();
  InitSubmarine();

  /* register callbacks */
  glutDisplayFunc(Display);
  glutReshapeFunc(Reshape);
  glutSpecialFunc(Special);
  glutKeyboardFunc(Keyboard);
  glutIdleFunc(Idle);

  glutMainLoop();

  return 0;
}

/**************************************************/
/* CALLBACKS                                      */
/**************************************************/

void Display() {
  GLfloat ambientUnderwaterLight[] = {0.5, 0.5, 1.0, 1.0};
  GLfloat ambientLight[] = {0.7, 0.7, 0.7, 1.0};

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  /* Set up the viewpoint */
  glLoadIdentity();
  if(viewPosition == OUTSIDE) {
    /* set the viewpoint outside the tank looking in */
    gluLookAt(100.0, 50.0, 150.0,
	      0.0, 25.0, 0.0,
	      0.0, 1.0, 0.0);
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambientLight);
  }
  else {
    /* set the viewpoint to point in the direction of the sub */
    glRotatef(sub.dive, 1.0, 0.0, 0.0);
    glRotatef(-sub.turn-90.0, 0.0, 1.0, 0.0);
    /* translate to the position of the sub */
    glTranslatef(-sub.x, -sub.y, -sub.z);
    /* underwater lighting effect */
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambientUnderwaterLight);
  }

  /* Draw the scene */
  DrawLights();
  if(light0) glEnable(GL_LIGHT0); else glDisable(GL_LIGHT0);
  if(light1) glEnable(GL_LIGHT1); else glDisable(GL_LIGHT1);
  if(light2) glEnable(GL_LIGHT2); else glDisable(GL_LIGHT2);
  if(light3) glEnable(GL_LIGHT3); else glDisable(GL_LIGHT3);
  if(light4) glEnable(GL_LIGHT4); else glDisable(GL_LIGHT4);
  if(light5) glEnable(GL_LIGHT5); else glDisable(GL_LIGHT5);
  if(light6) glEnable(GL_LIGHT6); else glDisable(GL_LIGHT6);
  glCallList(ground);
  glCallList(tank);
  glCallList(waterBack);
  glCallList(aerator);
  DrawBubbles();
  if(viewPosition != IN_SUB) DrawSubmarine();
  glCallList(waterFront);

  glFlush();
  glutSwapBuffers();
  return;
}

void Reshape(int w, int h) {
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(45.0, (float)w/(float)h, 1.0, 400.0);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glViewport(0, 0, w, h);
}

void Menu(int id) {
  if(id < 0) {
    /* user selected exit */
    printf("\n");
    exit(0);
  }
  /* set the view position according to the selection */
  viewPosition = id;
}

void Keyboard(unsigned char key, int x, int y) {
  switch(key) {
  case '1':
    light1 = 1 - light1;
    break;
  case '2':
    light2 = 1 - light2;
    break;
  case '3':
    light3 = 1 - light3;
    break;
  case '4':
    light4 = 1 - light4;
    break;
  case '5':
    light5 = 1 - light5;
    break;
  case '6':
    light6 = 1 - light6;
    break;
  case 'f':
    /* Move the sub forwards */
    AccelerateSubmarine(SUB_ACCELERATION);
    break;
  case 'j':
    /* Move the sub backwards */
    AccelerateSubmarine(-SUB_ACCELERATION);
    break;
  case 'i':
    viewPosition = IN_SUB;
    break;
  case 'o':
    viewPosition = OUTSIDE;
    break;
  case 27:
    /* exit */
    printf("\n");
    exit(0);
    break;
  }
  return;
}

void Special(int key, int x, int y) {
  switch(key) {
  case GLUT_KEY_UP:
    /* Tilt submarine up */
    sub.dive -= 1.0;
    if(sub.dive < -60.0) sub.dive = -60.0;
    break;
  case GLUT_KEY_DOWN:
    /* Tilt submarine down */
    sub.dive += 1.0;
    if(sub.dive > 60.0) sub.dive = 60.0;
    break;
  case GLUT_KEY_LEFT:
    /* Rotate submarine to port */
    sub.turn += 2.0;
    if(sub.turn > 360.0) sub.turn -= 360.0;
    break;
  case GLUT_KEY_RIGHT:
    /* Rotate submarine to starboard */
    sub.turn -= 2.0;
    if(sub.turn < 0.0) sub.turn += 360.0;
    break;
  }
}

void Idle() {
  static clock_t old = 0, last = 0;
  static int ticks = 0;
  clock_t new, elapsed;
  float elapsedSecs;
  GLfloat velAdj;
  static float t = 0;

  ticks++;
  new=clock();

  /* if 0.5 seconds passed, calculate average frame rate */
  if((new - old)/CLOCKS_PER_SEC > 0.5) {
    elapsed=(new - old);
    old = new;
    fprintf(stderr, "FPS: %.1f   \r",  ticks * (float) CLOCKS_PER_SEC / (float) elapsed);
    ticks = 0;
  }
  
  /* update submarine position */
  elapsedSecs = (new - last)/(float)CLOCKS_PER_SEC;
  velAdj = WATER_RESISTANCE * elapsedSecs;
  sub.x += sub.xVelocity * elapsedSecs;
  sub.y += sub.yVelocity * elapsedSecs;
  sub.z += sub.zVelocity * elapsedSecs;
  CollisionDetection();
  /* water resistance */
  sub.xVelocity -= velAdj * sub.xVelocity;
  sub.yVelocity -= velAdj * sub.yVelocity;
  sub.zVelocity -= velAdj * sub.zVelocity;
  
  last = new;
  glutPostRedisplay();
}

/**************************************************/
/* INITIALISATION                                 */
/**************************************************/

void InitMenu(void) {
  glutCreateMenu(Menu);
  glutAddMenuEntry("Fixed View", OUTSIDE);
  glutAddMenuEntry("Submarine View", IN_SUB);
  glutAddMenuEntry("Exit", -1);
  glutAttachMenu(GLUT_MIDDLE_BUTTON);
}

void InitTextures(void) {
  GLubyte textureBuffer[256][256][4];
  int i;

  glGenTextures(NUMBER_OF_TEXTURES, textures);

  for(i = 0; i < NUMBER_OF_TEXTURES; i++) {
    /* load the texture */
    open_image_file(textureFiles[i], textureBuffer);
    
    /* apply the texture */
    glBindTexture(GL_TEXTURE_2D, textures[i]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 256, 256,
		 0, GL_RGBA, GL_UNSIGNED_BYTE, textureBuffer);
  }
}

void InitGround(void) {
  GLfloat groundVertices[][3] = {{-75.0, -5.0, 50.0}, {-75.0, -5.0, -50.0},
			       {75.0, -5.0, -50.0}, {75.0, -5.0, 50.0}};
  GLfloat groundColor[] = {1.0, 1.0, 1.0, 1.0};
  
  ground = glGenLists(1);
  if(ground != 0) {
    glNewList(ground, GL_COMPILE);
    
      /* set the material properties of the ground */
      glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, groundColor);
      glMaterialfv(GL_FRONT, GL_SPECULAR, groundColor);
      glMaterialf(GL_FRONT, GL_SHININESS, 0);
      /* Select the ground texture and enable it */
      glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
      glBindTexture(GL_TEXTURE_2D, textures[GROUND]);
      glEnable(GL_TEXTURE_2D);
      /* draw the ground */
      glBegin(GL_QUADS);
        glNormal3f(0.0, 1.0, 0.0);
	glTexCoord2i(0,1); glVertex3fv(groundVertices[0]);
	glTexCoord2i(1,1); glVertex3fv(groundVertices[1]);
	glTexCoord2i(1,0); glVertex3fv(groundVertices[2]);
	glTexCoord2i(0,0); glVertex3fv(groundVertices[3]);
      glEnd();
      
      glDisable(GL_TEXTURE_2D);
    glEndList();
  }
  else {
    fprintf(stderr, "ERROR: Unable to create display list for ground");
    exit(1);
  }
}

void InitTank(void) {
  GLfloat tankColor[] = {0.4, 0.8, 0.8, 0.5};
  GLfloat baseColor[] = {0.8, 0.8, 0.8, 1.0};
  GLfloat lidColor[] = {0.5, 0.5, 0.5, 1.0};
  GLfloat shelfColor[] = {0.2, 0.2, 0.2, 1.0};
  GLfloat sandColor[] = {0.4, 0.4, 1.0, 1.0};
  GLfloat glassVertices[][3] = {{-50.0, 0.0, 25.0}, {-50.0, 0.0, -25.0},
				{50.0, 0.0, -25.0}, {50.0, 0.0, 25.0},
				{-50.0, 50.0, 25.0}, {-50.0, 50.0, -25.0},
				{50.0, 50.0, -25.0}, {50.0, 50.0, 25.0}};
  GLfloat glassNormals[][3] = {{-1.0, 0.0, 0.0}, {0.0, 0.0, -1.0},
			       {1.0, 0.0, 0.0}, {0.0, 0.0, 1.0}};
  GLfloat baseVertices[][3] = {{-55.0, -5.0, 30.0}, {-55.0, -5.0, -30.0},
			       {55.0, -5.0, -30.0}, {55.0, -5.0, 30.0}};
  GLfloat baseNormals[][3] = {{-0.707, 0.707, 0.0}, {0.0, 0.707, -0.707},
			      {0.707, 0.707, 0.0}, {0.0, 0.707, 0.707}};
  GLfloat lidVertices[][3] = {{-40.0, 60.0, 15.0}, {-40.0, 60.0, -15.0},
			      {40.0, 60.0, -15.0}, {40.0, 60.0, 15.0}};
  GLfloat lidNormals[][3] = {{-0.707, 0.707, 0.0}, {0.0, 0.707, -0.707},
			     {0.707, 0.707, 0.0}, {0.0, 0.707, 0.707}};
  int i, j;

  tank = glGenLists(1);
  if(tank != 0) {
    glNewList(tank, GL_COMPILE);

    /* Set the material properties of the glass */
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, tankColor);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, tankColor);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 100);
    /* Draw the glass */
      for(i = 0; i < 4; i++) {
	glBegin(GL_LINE_LOOP);
	  glNormal3fv(glassNormals[i]);
	  glVertex3fv(glassVertices[i]);
	  glVertex3fv(glassVertices[i+4]);
	  if(i == 3) { /* last edge */
	    glVertex3fv(glassVertices[4]);
	    glVertex3fv(glassVertices[0]);
	  }
	  else { /* other edges */
	    glVertex3fv(glassVertices[i+5]);
	    glVertex3fv(glassVertices[i+1]);
	  }
	glEnd();
      }

      /* set the material properties of the sand */
      glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, sandColor);
      glMaterialfv(GL_FRONT, GL_SPECULAR, sandColor);
      glMaterialf(GL_FRONT, GL_SHININESS, 0);
      /* Select the sand texture and enable it */
      glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
      glBindTexture(GL_TEXTURE_2D, textures[SAND]);
      glEnable(GL_TEXTURE_2D);
      /* Draw the bottom of the tank */
      glBegin(GL_QUADS);
        glNormal3f(0.0, 1.0, 0.0);
	  glTexCoord2f(0.0, 0.0);
	  glVertex3fv(glassVertices[0]);
	  glTexCoord2f(0.0, 1.0);
	  glVertex3fv(glassVertices[1]);
	  glTexCoord2f(1.0, 1.0);
	  glVertex3fv(glassVertices[2]);
	  glTexCoord2f(1.0, 0.0);
	  glVertex3fv(glassVertices[3]);
      glEnd();

      /* Select the wood texture for the base and the lid */
      glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
      glBindTexture(GL_TEXTURE_2D, textures[WOOD]);
      glEnable(GL_TEXTURE_2D);

      /* Set the material properties of the base */
      glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, baseColor);
      glMaterialfv(GL_FRONT, GL_SPECULAR, baseColor);
      glMaterialf(GL_FRONT, GL_SHININESS, 64);
     
      /* Draw the base */
      glBegin(GL_QUADS);
      for(i = 0; i < 4; i++) {
	glNormal3fv(baseNormals[i]);
        glTexCoord2i(1,0);
	glVertex3fv(glassVertices[i]);
        glTexCoord2i(0,0);
	glVertex3fv(baseVertices[i]);
	if(i == 3) { /* last side */
	  glTexCoord2i(0,1);
	  glVertex3fv(baseVertices[0]);
	  glTexCoord2i(1,1);
	  glVertex3fv(glassVertices[0]);
	}
	else { /* other sides */
	  glTexCoord2i(0,1);
	  glVertex3fv(baseVertices[i+1]);
	  glTexCoord2i(1,1);
	  glVertex3fv(glassVertices[i+1]);
	}
      }
      glEnd();

      /* Set the material properties of the lid */
      glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, lidColor);
      glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, lidColor);
      glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 64);

      /* Draw the sides of the lid */
      glBegin(GL_QUADS);
        for(i = 0; i< 4; i++) {
	  glNormal3fv(lidNormals[i]);
	  glTexCoord2i(1, 0);
	  glVertex3fv(glassVertices[i+4]);
	  glTexCoord2i(0, 0);
	  glVertex3fv(lidVertices[i]);
	  if(i == 3) { /* last side */
	    glTexCoord2i(0, 1);
	    glVertex3fv(lidVertices[0]);
	    glTexCoord2i(1, 1);
	    glVertex3fv(glassVertices[4]);
	  }
	  else { /* other sides */
	    glTexCoord2i(0, 1);
	    glVertex3fv(lidVertices[i+1]);
	    glTexCoord2i(1, 1);
	    glVertex3fv(glassVertices[i+5]);
	  }
	}

	/* Draw the top of the lid */
	/* REF ? */
	glNormal3f(0.0, 1.0, 0.0);
	glTexCoord2i(1, 0); glVertex3fv(lidVertices[0]);
	glTexCoord2i(0, 0); glVertex3fv(lidVertices[1]);
	glTexCoord2i(0, 1); glVertex3fv(lidVertices[2]);
	glTexCoord2i(1, 1); glVertex3fv(lidVertices[3]);
      glEnd();

      glDisable(GL_TEXTURE_2D);

      /* Set the material properties of the shelf */
      glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, shelfColor);
      glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, shelfColor);
      glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 64);

      /* Draw the shelf */
      glPushMatrix();
	glTranslatef(0.0, 20.5, -20.0); 
        glScalef(20.0, 1.0, 10.0);
        glutSolidCube(1.0);
      glPopMatrix();

    glEndList();
  }
  else {
    fprintf(stderr, "ERROR: Unable to create display list for tank\n");
    exit(1);
  }
  
}

void InitWater(void) {
  int i, j;
  GLfloat waterColor[] = {0.2, 0.2, 0.8, 0.3};
  GLfloat waterVertices[][3] = {{-50.0, 0.0, -25.0}, {50.0, 0.0, -25.0},
				 {50.0, 0.0, 25.0}, {-50.0, 0.0, 25.0},
				 {-50.0, 40.0, -25.0}, {50.0, 40.0, -25.0},
				 {50.0, 40.0, 25.0}, {-50.0, 40.0, 25.0}};
  GLfloat topVertices[WATER_TOP_SUBDIVISION+1][WATER_TOP_SUBDIVISION+1][3];
  GLfloat sideVertices[WATER_SIDES_SUBDIVISION+1][WATER_SIDES_SUBDIVISION+1][3];

  waterBack = glGenLists(1);
  if(waterBack != 0) {
    waterFront = waterBack + 1;

    /* Create the back of the water */
    glNewList(waterBack, GL_COMPILE);
      /* Set the material properties to water */
      glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, waterColor);
      glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, waterColor);
      glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 100);

      /* subdivide the plane */
      SubdivideXY(waterVertices[0], 
		  waterVertices[5], 
		  (GLfloat *)sideVertices, 
		  WATER_SIDES_SUBDIVISION);

      /* draw the back of the water */
      glBegin(GL_QUADS);
        glNormal3f(0.0, 0.0, -1.0);
        for(i = 0; i < WATER_SIDES_SUBDIVISION; i++) {
	  for(j = 0; j < WATER_SIDES_SUBDIVISION; j++) {
	    glVertex3fv(sideVertices[i][j]);
	    glVertex3fv(sideVertices[i+1][j]);
	    glVertex3fv(sideVertices[i+1][j+1]);
	    glVertex3fv(sideVertices[i][j+1]);	    
	  }
	}
      glEnd();
    glEndList();

    /* Create the front of the water */
    glNewList(waterFront, GL_COMPILE);
      /* Set the material properties to water */
      glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, waterColor);
      glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, waterColor);
      glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 100);    

      /***** draw the top *****/
      /* subdivide the plane */
      SubdivideXZ(waterVertices[4], 
		  waterVertices[6],
		  (GLfloat *)topVertices,
		  WATER_TOP_SUBDIVISION);

      /* draw the water */
      glBegin(GL_QUADS);
        glNormal3f(0.0, 1.0, 0.0);
        for(i = 0; i < WATER_TOP_SUBDIVISION; i++) {
	  for(j = 0; j < WATER_TOP_SUBDIVISION; j++) {
	    glVertex3fv(topVertices[i][j]);
	    glVertex3fv(topVertices[i+1][j]);
	    glVertex3fv(topVertices[i+1][j+1]);
	    glVertex3fv(topVertices[i][j+1]);	    
	  }
	}
      glEnd();

      /***** draw the RHS *****/
      /* subdivide the plane */
      SubdivideYZ(waterVertices[1], 
		  waterVertices[6], 
		  (GLfloat *)sideVertices,
		  WATER_SIDES_SUBDIVISION);

      /* draw the water */
      glBegin(GL_QUADS);
        glNormal3f(1.0, 0.0, 0.0);
        for(i = 0; i < WATER_SIDES_SUBDIVISION; i++) {
	  for(j = 0; j < WATER_SIDES_SUBDIVISION; j++) {
	    glVertex3fv(sideVertices[i][j]);
	    glVertex3fv(sideVertices[i+1][j]);
	    glVertex3fv(sideVertices[i+1][j+1]);
	    glVertex3fv(sideVertices[i][j+1]);	    
	  }
	}
      glEnd();

      /***** draw the front *****/
      /* subdivide the plane */
      SubdivideXY(waterVertices[2], 
		  waterVertices[7], 
		  (GLfloat *)sideVertices,
		  WATER_SIDES_SUBDIVISION);
      
      /* draw the water */
      glBegin(GL_QUADS);
        glNormal3f(0.0, 0.0, 1.0);
        for(i = 0; i < WATER_SIDES_SUBDIVISION; i++) {
	  for(j = 0; j < WATER_SIDES_SUBDIVISION; j++) {
	    glVertex3fv(sideVertices[i][j]);
	    glVertex3fv(sideVertices[i+1][j]);
	    glVertex3fv(sideVertices[i+1][j+1]);
	    glVertex3fv(sideVertices[i][j+1]);	    
	  }
	}
      glEnd();

      /***** draw the LHS *****/
      /* subdivide the plane */
      SubdivideYZ(waterVertices[3],
		  waterVertices[4],
		  (GLfloat *)sideVertices,
		  WATER_SIDES_SUBDIVISION);

      /* draw the water */
      glBegin(GL_QUADS);
        glNormal3f(-1.0, 0.0, 0.0);
        for(i = 0; i < WATER_SIDES_SUBDIVISION; i++) {
	  for(j = 0; j < WATER_SIDES_SUBDIVISION; j++) {
	    glVertex3fv(sideVertices[i][j]);
	    glVertex3fv(sideVertices[i+1][j]);
	    glVertex3fv(sideVertices[i+1][j+1]);
	    glVertex3fv(sideVertices[i][j+1]);	    
	  }
	}
      glEnd();

    glEndList();
  }
  else {
    fprintf(stderr, "ERROR: Unable to create display list for the water\n");
    exit(1);
  }
}

void InitLights(void) {
  int i, j;
  GLfloat roomLightColor[] = {0.04, 0.04, 0.03, 1.0};
  GLfloat realLightPositions[][4] = {{0.0, 400.0, 100.0, 1.0}, 
				     {-30.0, 50.0, 12.0, 1.0}, {-30.0, 50.0, -12.0, 1.0}, 
				     {0.0, 50.0, 12.0, 1.0}, {0.0, 50.0, -12.0, 1.0}, 
				     {30.0, 50.0, 12.0, 1.0}, {30.0, 50.0, -12.0, 1.0}};
				 
  GLfloat lightDirection[3] = {0.0, -1.0, 0.0};
  GLfloat fakeLightColor[] = {1.0, 1.0, 0.8, 1.0};
  GLfloat lightEmission[] = {1.0, 1.0, 1.0, 1.0};
  GLfloat black[] = {0.0, 0.0, 0.0, 1.0};
  GLfloat fakeLightPositions[][3] = {{-30.0, 54.0, 12.0}, {-30.0, 54.0, -12.0},
				     {0.0, 54.0, 12.0}, {0.0, 54.0, -12.0},
				     {30.0, 54.0, 12.0}, {30.0, 54.0, -12.0}};
  GLfloat lightShadeVertices[CONE_SEGMENTS*2][3];
  GLfloat lightShadeNormals[CONE_SEGMENTS][3];
  GLfloat lightRadius = 3;


  /* Calculate the light shade */
  for(i = 0; i < CONE_SEGMENTS; i++) {
    lightShadeNormals[i][0] = sin((2*PI/CONE_SEGMENTS)*i);
    lightShadeNormals[i][1] = 0.0;
    lightShadeNormals[i][2] = cos((2*PI/CONE_SEGMENTS)*i);
    lightShadeVertices[i][0] = (lightRadius + 0.2) * lightShadeNormals[i][0];
    lightShadeVertices[i][1] = -lightRadius;
    lightShadeVertices[i][2] = (lightRadius + 0.2) * lightShadeNormals[i][2];
    lightShadeVertices[i+CONE_SEGMENTS][0] = lightShadeVertices[i][0];
    lightShadeVertices[i+CONE_SEGMENTS][1] = lightRadius;
    lightShadeVertices[i+CONE_SEGMENTS][2] = lightShadeVertices[i][2];
  }

  lights = glGenLists(1);
  if(lights != 0) {
    glNewList(lights, GL_COMPILE);

      /* Set the light properties */
      glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, fakeLightColor);
      glMaterialfv(GL_FRONT, GL_SPECULAR, fakeLightColor);
      glMaterialfv(GL_FRONT, GL_EMISSION, lightEmission);
      glMaterialf(GL_FRONT, GL_SHININESS, 128);
      /* draw the lights */
      for(i = 0; i < 6; i++) {
	glPushMatrix();
	  glTranslatef(fakeLightPositions[i][0],
		       fakeLightPositions[i][1],
		       fakeLightPositions[i][2]);
	  glutSolidSphere(lightRadius, 15, 15);
	glPopMatrix();
      }
      /* draw the shades */
      glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, black);
      glMaterialfv(GL_FRONT, GL_SPECULAR, black);
      glMaterialfv(GL_FRONT, GL_EMISSION, black);
      glMaterialf(GL_FRONT, GL_SHININESS, 128);
      for(i = 0; i < 6; i++) {
	glPushMatrix();
	  glTranslatef(fakeLightPositions[i][0],
		       fakeLightPositions[i][1],
		       fakeLightPositions[i][2]);
	  glBegin(GL_QUAD_STRIP);
	    for(j = 0; j < CONE_SEGMENTS; j++) {
	      glNormal3fv(lightShadeNormals[j]);
	      glVertex3fv(lightShadeVertices[j]);
	      glVertex3fv(lightShadeVertices[j+CONE_SEGMENTS]);
	    }
	    glNormal3fv(lightShadeNormals[0]);
	    glVertex3fv(lightShadeVertices[0]);
	    glVertex3fv(lightShadeVertices[CONE_SEGMENTS]);
	  glEnd();
	glPopMatrix();
      }


      /* Place the real lights */
      /* room light */
      glLightfv(GL_LIGHT0, GL_AMBIENT, roomLightColor);
      glLightfv(GL_LIGHT0, GL_DIFFUSE, roomLightColor);
      glLightfv(GL_LIGHT0, GL_SPECULAR, roomLightColor);
      glLightfv(GL_LIGHT0, GL_POSITION, realLightPositions[0]);
      glLightf(GL_LIGHT0, GL_SPOT_CUTOFF, 180.0);
      
      /* spot lights */
      glLightfv(GL_LIGHT1, GL_POSITION, realLightPositions[1]);
      glLightfv(GL_LIGHT1, GL_SPOT_DIRECTION, lightDirection);
      glLightf(GL_LIGHT1, GL_SPOT_CUTOFF, SPOTLIGHT_WIDTH);
      
      glLightfv(GL_LIGHT2, GL_POSITION, realLightPositions[2]);
      glLightfv(GL_LIGHT2, GL_SPOT_DIRECTION, lightDirection);
      glLightf(GL_LIGHT2, GL_SPOT_CUTOFF, SPOTLIGHT_WIDTH);

      glLightfv(GL_LIGHT3, GL_POSITION, realLightPositions[3]);
      glLightfv(GL_LIGHT3, GL_SPOT_DIRECTION, lightDirection);
      glLightf(GL_LIGHT3, GL_SPOT_CUTOFF, SPOTLIGHT_WIDTH);
      
      glLightfv(GL_LIGHT4, GL_POSITION, realLightPositions[4]);
      glLightfv(GL_LIGHT4, GL_SPOT_DIRECTION, lightDirection);
      glLightf(GL_LIGHT4, GL_SPOT_CUTOFF, SPOTLIGHT_WIDTH);

      glLightfv(GL_LIGHT5, GL_POSITION, realLightPositions[5]);
      glLightfv(GL_LIGHT5, GL_SPOT_DIRECTION, lightDirection);
      glLightf(GL_LIGHT5, GL_SPOT_CUTOFF, SPOTLIGHT_WIDTH);
      
      glLightfv(GL_LIGHT6, GL_POSITION, realLightPositions[6]);
      glLightfv(GL_LIGHT6, GL_SPOT_DIRECTION, lightDirection);
      glLightf(GL_LIGHT6, GL_SPOT_CUTOFF, SPOTLIGHT_WIDTH);

    glEndList();
  }
  else {
    fprintf(stderr, "ERROR: Unable to create display list for lights\n");
    exit(1);
  }
}

void InitAerator(void) {
  int i;
  GLfloat topRadius = 1.0;
  GLfloat bottomRadius = 4.0;
  GLfloat height = 7.0;
  GLfloat aeratorVertices[CONE_SEGMENTS*2][3];
  GLfloat aeratorNormals[CONE_SEGMENTS][3];
  GLfloat aeratorInsideColor[] = {0.0, 0.0, 1.0, 1.0};
  GLfloat aeratorOutsideColor[] = {0.2, 0.07, 0.02, 1.0};

  aerator = glGenLists(1);
  if(aerator != 0) {

    /* Calculate the aerator */
    for(i = 0; i < CONE_SEGMENTS; i++) {
      aeratorVertices[i][0] = bottomRadius * sin((2*PI/CONE_SEGMENTS)*i);
      aeratorVertices[i][1] = 0.0;
      aeratorVertices[i][2] = bottomRadius * cos((2*PI/CONE_SEGMENTS)*i);
      aeratorVertices[i+CONE_SEGMENTS][0] = topRadius * sin((2*PI/CONE_SEGMENTS)*i);
      aeratorVertices[i+CONE_SEGMENTS][1] = height;
      aeratorVertices[i+CONE_SEGMENTS][2] = topRadius * cos((2*PI/CONE_SEGMENTS)*i);
      aeratorNormals[i][0] = height * sin((2*PI/CONE_SEGMENTS)*i);
      aeratorNormals[i][1] = bottomRadius - topRadius;
      aeratorNormals[i][2] = height * cos((2*PI/CONE_SEGMENTS)*i);
      Normalise(aeratorNormals[i]);
    }

    glNewList(aerator, GL_COMPILE);
      glPushMatrix();
        glTranslatef(0.0, 0.0, -20.0);
	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
	/* set the material properties for the outdide... */
	glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, aeratorOutsideColor);
	glMaterialfv(GL_FRONT, GL_SPECULAR, aeratorOutsideColor);
	glMaterialf(GL_FRONT, GL_SHININESS, 0);
	/* ...and for the inside */
	glMaterialfv(GL_BACK, GL_AMBIENT_AND_DIFFUSE, aeratorInsideColor);
	glMaterialfv(GL_BACK, GL_SPECULAR, aeratorInsideColor);
	glMaterialf(GL_BACK, GL_SHININESS, 128);
	
	/* Draw the aerator */
	glBegin(GL_QUAD_STRIP);
	  for(i = 0; i < CONE_SEGMENTS; i++) {
	    glNormal3fv(aeratorNormals[i]);
	    glVertex3fv(aeratorVertices[i+CONE_SEGMENTS]);
	    glVertex3fv(aeratorVertices[i]);
	  }
	  glNormal3fv(aeratorNormals[0]);
	  glVertex3fv(aeratorVertices[CONE_SEGMENTS]);
	  glVertex3fv(aeratorVertices[0]);
	glEnd();

	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_FALSE);
      
      glPopMatrix();
    glEndList();
  }
  else {
    fprintf(stderr, "ERROR: Unable to create display list for aerator\n");
    exit(1);
  }
}

void InitBubbles(void) {
  int i;

  /* Disable all the bubbles */
  for(i = 0; i < MAX_BUBBLES; i++) bubbles[i].active = 0;
  /* set point characteristics */
  glPointSize(4);
  glEnable(GL_POINT_SMOOTH);
  glHint(GL_POINT_SMOOTH_HINT, GL_FASTEST);
}

void InitSubmarine(void) {
  int i;
  const GLfloat radius = 1.5;
  GLfloat submarineColor[] = {0.0124, 0.1288, 0.1992, 1.0};
  GLfloat propellerGuardVertices[SUBMARINE_SEGMENTS*2][3];
  GLfloat propellerGuardNormals[SUBMARINE_SEGMENTS][3];

  submarine = glGenLists(1);
  if(submarine != 0) {
    glNewList(submarine, GL_COMPILE);

      /* Set the material properties of the submarine */
      glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, submarineColor);
      glMaterialfv(GL_FRONT, GL_SPECULAR, submarineColor);
      glMaterialf(GL_FRONT, GL_SHININESS, 100);
      
      /* Draw the body */
      glPushMatrix();
        glScalef(4.0, 1.0, 1.0);
	glutSolidSphere(1.5, SUBMARINE_SEGMENTS, SUBMARINE_SEGMENTS);
      glPopMatrix();

      /* calculate the propeller_guard */
      for(i = 0; i < SUBMARINE_SEGMENTS; i++) {
	propellerGuardNormals[i][0] = 0.0;
	propellerGuardNormals[i][1] = sin((2*PI/SUBMARINE_SEGMENTS)*i);
	propellerGuardNormals[i][2] = cos((2*PI/SUBMARINE_SEGMENTS)*i);
	propellerGuardVertices[i][0] = 5.5;
	propellerGuardVertices[i][1] = radius * propellerGuardNormals[i][1]; 
	propellerGuardVertices[i][2] = radius * propellerGuardNormals[i][2];
	propellerGuardVertices[i+SUBMARINE_SEGMENTS][0] = 6.5;
	propellerGuardVertices[i+SUBMARINE_SEGMENTS][1] = propellerGuardVertices[i][1];
	propellerGuardVertices[i+SUBMARINE_SEGMENTS][2] = propellerGuardVertices[i][2];
      }

      /* draw the propeller guard */
      glBegin(GL_QUAD_STRIP);
        for(i = 0; i < SUBMARINE_SEGMENTS; i++) {
	  glNormal3fv(propellerGuardNormals[i]);
	  glVertex3fv(propellerGuardVertices[i]);
	  glVertex3fv(propellerGuardVertices[i+SUBMARINE_SEGMENTS]);
	}
	glNormal3fv(propellerGuardNormals[0]);
	glVertex3fv(propellerGuardVertices[0]);
	glVertex3fv(propellerGuardVertices[SUBMARINE_SEGMENTS]);	  
      glEnd();

      /* draw the tower */
      glPushMatrix();
        glTranslatef(0.0, 1.5, 0.0);
	glScalef(3.0, 1.0, 1.0);
	glutSolidCube(1.0);
      glPopMatrix();

      /* draw the fins */
      /* elevators */
      glPushMatrix();
        glTranslatef(-2.5, 0.0, 0.0);
	glScalef(3, 1.0, 8.0);
	glutSolidCube(0.5);
      glPopMatrix();
      /* steering */
      glPushMatrix();
        glTranslatef(6.75, 0.0, 0.0);
	glScalef(2.0, 8*radius, 1.0);
	glutSolidCube(0.25);
      glPopMatrix();

    glEndList();
  }
  else {
    fprintf(stderr, "ERROR: Unable to create display list for submarine\n");
    exit(1);
  }

  sub.x = 0.0;
  sub.y = 20.0;
  sub.z = 0.0;
  sub.xVelocity = 0.0;
  sub.yVelocity = 0.0;
  sub.zVelocity = 0.0;
  sub.dive = 0.0;
  sub.turn = 0.0;
}

/**************************************************/
/* DRAWING FUNCTIONS                              */
/**************************************************/
void DrawLights() {
  GLfloat lightColor[] = {1.0, 1.0, 1.0, 1.0};

  /* Make the lights blue to give an underwater effect */
  if(viewPosition == IN_SUB) {
    lightColor[0] = 0.4;
    lightColor[1] = 0.4;
    light0 = 0;
  }
  else light0 = 1;

  /* Draw the lights */
  glCallList(lights);

  /* Set the properties of the lights */
  glLightfv(GL_LIGHT1, GL_AMBIENT, lightColor);
  glLightfv(GL_LIGHT1, GL_DIFFUSE, lightColor);
  glLightfv(GL_LIGHT1, GL_SPECULAR, lightColor);

  glLightfv(GL_LIGHT2, GL_AMBIENT, lightColor);
  glLightfv(GL_LIGHT2, GL_DIFFUSE, lightColor);
  glLightfv(GL_LIGHT2, GL_SPECULAR, lightColor);

  glLightfv(GL_LIGHT3, GL_AMBIENT, lightColor);
  glLightfv(GL_LIGHT3, GL_DIFFUSE, lightColor);
  glLightfv(GL_LIGHT3, GL_SPECULAR, lightColor);

  glLightfv(GL_LIGHT4, GL_AMBIENT, lightColor);
  glLightfv(GL_LIGHT4, GL_DIFFUSE, lightColor);
  glLightfv(GL_LIGHT4, GL_SPECULAR, lightColor);

  glLightfv(GL_LIGHT5, GL_AMBIENT, lightColor);
  glLightfv(GL_LIGHT5, GL_DIFFUSE, lightColor);
  glLightfv(GL_LIGHT5, GL_SPECULAR, lightColor);

  glLightfv(GL_LIGHT6, GL_AMBIENT, lightColor);
  glLightfv(GL_LIGHT6, GL_DIFFUSE, lightColor);
  glLightfv(GL_LIGHT6, GL_SPECULAR, lightColor);

}

void DrawBubbles(void) {
  int i;
  GLfloat bubbleColor[] = {0.8, 0.8, 1.0, 0.2};
  static float timeSinceBubble = 0;
  static clock_t oldTime = 0;
  clock_t newTime;
  float elapsed;
  GLfloat acceleration;
  int active = 0;

  newTime = clock();
  elapsed = (float) (newTime - oldTime)/(float) CLOCKS_PER_SEC;
  oldTime = newTime;
  timeSinceBubble += elapsed;
  acceleration = elapsed * BOUYANCY;

  /* Set the material properties of the bubbles */
  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, bubbleColor);
  glMaterialfv(GL_FRONT, GL_SPECULAR, bubbleColor);
  glMaterialf(GL_FRONT, GL_SHININESS, 50);

  for(i = 0; i < MAX_BUBBLES; i++) {
    if(bubbles[i].active) {
      active++;
      /* Calculate new bubble position */
      bubbles[i].position[0] += bubbles[i].xVelocity * elapsed;
      bubbles[i].position[1] += bubbles[i].yVelocity * elapsed;
      bubbles[i].position[2] += bubbles[i].zVelocity * elapsed;
      /* accelerate bubble upwards */
      bubbles[i].yVelocity += acceleration * (1 + 0.5*(RandF() - 0.5));
      /* collision detection */
      if(bubbles[i].position[1] > 40.0) bubbles[i].active = 0; /* burst at water surface */
      else {
	if(HitShelf(bubbles[i])) {
	  /* Move the bubble under the shelf */
	  bubbles[i].position[1] -= bubbles[i].position[1] - 19.2;
	  bubbles[i].yVelocity = -bubbles[i].yVelocity * BUBBLE_BOUNCE;
	}
	if(HitWall(bubbles[i])) {
	  /* bounce of wall */
	  bubbles[i].position[2] -= bubbles[i].position[2] + 24.2;
	  bubbles[i].zVelocity = -bubbles[i].zVelocity * BUBBLE_BOUNCE;
	}
	/* Draw the bubble */
	if(viewPosition == IN_SUB) {
	  /* Draw bubbles as spheres when inside the tank */
	  glPushMatrix();
	    glTranslatef(bubbles[i].position[0],
			 bubbles[i].position[1],
			 bubbles[i].position[2]);
	    glutSolidSphere(0.8, 10, 10);
	  glPopMatrix();
	}
	else {
	  /* Draw bubbles as points when outside the tank */
	  glBegin(GL_POINTS);
	    glVertex3fv(bubbles[i].position);
	  glEnd();
	}
      }
    }
    else if(timeSinceBubble > TIME_BETWEEN_BUBBLES) { /* time to draw a new bubble */
      /* release a new bubble */
      bubbles[i].position[0] = 0.0;
      bubbles[i].position[1] = 7.0;
      bubbles[i].position[2] = -20.0;
      bubbles[i].xVelocity = 4 * (RandF() - 0.5);
      bubbles[i].yVelocity = 0.0;
      bubbles[i].zVelocity = 4 * (RandF() - 0.5);
      bubbles[i].active = 1;
      timeSinceBubble -= TIME_BETWEEN_BUBBLES;
    }
  }
  if(active == MAX_BUBBLES) fprintf(stderr, "WARNING: MAX BUBBLES REACHED               \n");
  fprintf(stderr, "\t\tActive bubbles: %2i\r", active);
}

void DrawSubmarine(void) {
  glPushMatrix();
    /* move submarine into position */
    glTranslatef(sub.x, sub.y, sub.z);
    glRotatef(sub.turn, 0.0, 1.0, 0.0);
    glRotatef(sub.dive, 0.0, 0.0, 1.0);
    /* Draw submarine */
    glCallList(submarine);
  glPopMatrix();
}

/**************************************************/
/* MISC FUNCTIONS                                 */
/**************************************************/
float RandF() {
  return (float)rand()/(float)RAND_MAX;
}

int HitShelf(bubble b) {
  if(b.position[1] >= 19.2 && b.position[2] < -15.0 &&
     b.position[0] > -10.0 && b.position[0] < 10.0) return 1;
  return 0;
}

int HitWall(bubble b) {
  if(b.position[2] < -24.2) return 1;
  return 0;
}

void AccelerateSubmarine(GLfloat acceleration) {
  GLfloat x1, y1, z1;
  GLfloat x2, y2, z2;
  double theta;

  /* acceleration vector before rotation is:
     x = -acceleration
     y = 0
     z = 0
  */

  /* rotate around z axis */
  theta = sub.dive * (PI/180);
  x1 = -acceleration*cos(theta);
  y1 = -acceleration*sin(theta);
  z1 = 0.0;

  /* rotate around y axis */
  theta = sub.turn * (PI/180);
  x2 = x1*cos(theta) + z1*sin(theta);
  y2 = y1;
  z2 = -x1*sin(theta) + z1*cos(theta);

  /* add to velocity vector */
  sub.xVelocity += x2;
  sub.yVelocity += y2;
  sub.zVelocity += z2;
  
}

void CollisionDetection(void) {
  int i;
  GLfloat boundingBox[][3] = {{-6.0, -1.5, -2.0}, {-6.0, -1.5, 2.0},
			       {-6.0, 2.0, -2.0}, {-6.0, 2.0, 2.0},
			      {7.25, 2.0, 2.0}, {7.25, 2.0, -2.0},
			      {7.25, -1.5, 2.0}, {7.25, -1.5, -2.0}};
  GLfloat xAdj = 0.0, yAdj = 0.0, zAdj = 0.0;
  GLfloat tmpX, tmpY, tmpZ;
  GLfloat diveR, turnR;
  
  diveR = sub.dive * (PI/180);
  turnR = sub.turn * (PI/180);
  /* rotate and translate the bounding box */
  for(i = 0; i < 8; i++) {
    /* rotate bounding box around z */
    tmpX = boundingBox[i][0]*cos(diveR) - boundingBox[i][1]*sin(diveR);
    tmpY = boundingBox[i][0]*sin(diveR) + boundingBox[i][1]*cos(diveR);
    tmpZ = boundingBox[i][2];
    /* rotate bounding box around y */
    boundingBox[i][0] = tmpX*cos(turnR) + tmpZ*sin(turnR);
    boundingBox[i][1] = tmpY;
    boundingBox[i][2] = -tmpX*sin(turnR) + tmpZ*cos(turnR);
    /* translate bounding box */
    boundingBox[i][0] += sub.x;
    boundingBox[i][1] += sub.y;
    boundingBox[i][2] += sub.z;
  }

  /* check if any of the bounding box vertices have hit anything */
  for(i = 0; i < 8; i++) {
    /* x */
    if(boundingBox[i][0] < -50.0 && -50.0 - boundingBox[i][0] > xAdj)
      xAdj = -50.0 - boundingBox[i][0];
    if(boundingBox[i][0] > 50.0 && 50.0 - boundingBox[i][0] < xAdj)
      xAdj = 50.0 - boundingBox[i][0];
    /* y */
    if(boundingBox[i][1] < 0.0 && 0.0 - boundingBox[i][1] > yAdj)
      yAdj = 0.0 - boundingBox[i][1];
    if(i > 5 && boundingBox[i][1] > 40.0 && 40.0 - boundingBox[i][1] < yAdj)
      yAdj = 40.0 - boundingBox[i][1];
    /* z */
    if(boundingBox[i][2] < -25.0 && -25.0 - boundingBox[i][2] > zAdj)
      zAdj = -25.0 - boundingBox[i][2];
    if(boundingBox[i][2] > 25.0 && 25.0 - boundingBox[i][2] < zAdj)
      zAdj = 25.0 - boundingBox[i][2];
  }
  if(sub.y > 40.0) yAdj = 40.0 - sub.y;

  /* Adjust submarine position and velocity if we have hit anything */
  if(xAdj != 0.0) {
    sub.x += xAdj;
    sub.xVelocity = -sub.xVelocity * SUB_BOUNCE;
  }
  if(yAdj != 0.0) {
    sub.y += yAdj;
    sub.yVelocity = -sub.yVelocity * SUB_BOUNCE;
  }
  if(zAdj != 0.0) {
    sub.z += zAdj;
    sub.zVelocity = -sub.zVelocity * SUB_BOUNCE;
  }
}

void SubdivideXY(GLfloat v1[3], GLfloat v2[3], GLfloat* vertices, int div) {
  int i, j, index;

  for(i = 0; i < div+1; i++) {
    for(j = 0; j < div+1; j++) {
      index = i*(div+1)*3 + j*3;
      /* x */
      if(i == 0)
	vertices[index] = v1[0];
      else if(i == div)
	vertices[index] = v2[0];
      else
	vertices[index] = v1[0] + i/(float)div * (v2[0] - v1[0]);
      /* y */
      if(j == 0)
	vertices[index+1] = v1[1];
      else if(j == div)
	vertices[index+1] = v2[1];
      else
	vertices[index+1] = v1[1] + j/(float)div * (v2[1] - v1[1]);
      /* z */
      vertices[index+2] = v1[2];
    }
  }
}
 
void SubdivideYZ(GLfloat v1[3], GLfloat v2[3], GLfloat* vertices, int div) {
  int i, j, index;

  for(i = 0; i < div+1; i++) {
    for(j = 0; j < div+1; j++) {
      index = i*(div+1)*3 + j*3;
      /* x */
      vertices[index] = v1[0];
      /* y */
      if(i == 0)
	vertices[index+1] = v1[1];
      else if(i == div)
	vertices[index+1] = v2[1];
      else
	vertices[index+1] = v1[1] + i/(float)div * (v2[1] - v1[1]);
      /* z */
      if(j == 0)
	vertices[index+2] = v1[2];
      else if(j == div)
	vertices[index+2] = v2[2];
      else
	vertices[index+2] = v1[2] + j/(float)div * (v2[2] - v1[2]);
    }
  }
} 

void SubdivideXZ(GLfloat v1[3], GLfloat v2[3], GLfloat* vertices, int div) {
  int i, j, index;

  for(i = 0; i < div+1; i++) {
    for(j = 0; j < div+1; j++) {
      index = i*(div+1)*3 + j*3;
      /* x */
      if(i == 0)
	vertices[index] = v1[0];
      else if(i == div)
	vertices[index] = v2[0];
      else
	vertices[index] = v1[0] + i/(float)div * (v2[0] - v1[0]);
      /* y */
      vertices[index+1] = v1[1];
      /* z */
      if(j == 0)
	vertices[index+2] = v1[2];
      else if(j == div)
	vertices[index+2] = v2[2];
      else
	vertices[index+2] = v1[2] + j/(float)div * (v2[2] - v1[2]);
    }
  }
} 

GLfloat *CrossProduct(GLfloat m1[3], GLfloat m2[3], GLfloat result[3]) {
  result[0] = m1[1]*m2[2] - m2[1]*m1[2];
  result[1] = m2[0]*m1[2] - m1[0]*m2[2];
  result[2] = m1[0]*m2[1] - m2[0]*m1[1];
  return result;
}

void Normalise(GLfloat v[3]) {
  GLfloat length;

  length = sqrt(v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);
  v[0] = v[0]/length;
  v[1] = v[1]/length;
  v[2] = v[2]/length;
  return;
}

GLfloat *Difference(GLfloat m1[3], GLfloat m2[3], GLfloat result[3]) {
  result[0] = m1[0] - m2[0];
  result[1] = m1[1] - m2[1];
  result[2] = m1[2] - m2[2];
  return result;
}
