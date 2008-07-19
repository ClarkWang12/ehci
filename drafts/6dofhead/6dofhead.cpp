#include "cv.h"
#include "highgui.h"
#include "ehci.h"
#include <stdio.h>
#include <ctype.h>
#include <vector>
//..
//opengl related includes
#include <GL/glut.h>    // Header File For The GLUT Library 
#include <GL/gl.h>	// Header File For The OpenGL32 Library
#include <GL/glu.h>	// Header File For The GLu32 Library

#define READFROMIMAGEFILE 0
#define MYFOCUS 602
#define MODELSCALE 100

/* white ambient light at half intensity (rgba) */
GLfloat LightAmbient[] = { 0.5f, 0.5f, 0.5f, 1.0f };

/* super bright, full intensity diffuse light. */
GLfloat LightDiffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };

/* position of light (x, y, z, (position of light)) */
GLfloat LightPosition[] = { 0.0f, 0.0f, -1000.0f, 1.0f };


/* The number of our GLUT window */
int window; 
int light;

double projectionMatrix[16];



IplImage *image = 0, *grey = 0, *prev_grey = 0, *pyramid = 0, *prev_pyramid = 0, *swap_temp;



int refX,refY;
int lastHeadW,lastHeadH;

int initialGuess=1;
int drawSine=0;

float theta = 0;//3.1415;
/*	float myRot[] = { 1,  0,  0,  			translation_vector[0],
	               0, cos(theta),  -sin(theta),  	translation_vector[1],
	               0, sin(theta),   cos(theta),  	translation_vector[2],
	               0,  0,  0,  0 };*/




#define NUMPTS 8
float scale = 1.0;


struct triangle{
	float vert[3][3];
};

triangle triangles[3036];

int win_size = 10;
const int MAX_COUNT = 500;
CvPoint2D32f* points[2] = {0,0}, *swap_points;
char* status = 0;

int night_mode = 0;

CvPoint pt;
CvCapture* capture = 0;


CvPoint upperHeadCorner = cvPoint(0,0);
int headWidth, headHeight;


int cvLoop(double glPositMatrix[16],int initialGuess);


void drawReferenceAxis(){

	glBegin(GL_LINES);
	glColor3f(1.0f,0.0f,0.0);
	glVertex3f( 0.0f,0.0f,0.0f);
	glVertex3f( 1000.0f,0.0f,0.0f);	
	glEnd();


	glBegin(GL_LINES);
	glColor3f(0.0f,1.0f,0.0);
	glVertex3f(0.0f, 0.0f,0.0f);
	glVertex3f(0.0f, 1000.0f,0.0f);	
	glEnd();

	glBegin(GL_LINES);
	glColor3f(0.0f,1.0f,0.0);
	glVertex3f(100.0f, 0.0f,0.0f);
	glVertex3f(100.0f, 1000.0f,0.0f);	
	glEnd();

	glBegin(GL_LINES);
	glColor3f(0.0f,0.0f,1.0);
	glVertex3f( 0.0f,0.0f,0.0f);
	glVertex3f( 0.0f,0.0f,1000.0f);	
	glEnd();

}


//terminar funcao para inserir novos pontos
/*
CvPoint newFeaturePoints[1000];
int newFeaturePointsCount=0;

void insertFeature(int x, int y){
	newFeaturePoints
}*/


void loadRaw(char* file){

	float p1[3],p2[3],p3[3];
	FILE* in = fopen("head.raw","r");
	//if(in!=NULL) printf("Good\n");
	char temp[200];
	//fscanf(in,"%s",&temp);
	//fscanf(in,"%f",&p1[0]);
	//	printf("Temp %s %f\n",temp,p1[0]);
	//	return;
	int tcount=0;
	float mScale= 1.0;
	//float deltaX=1.874,deltaY=-1.999,deltaZ=-2.643;
	float deltaX=0.0,deltaY=0.0,deltaZ=0.0;

	while( fscanf(in,"%f%f%f%f%f%f%f%f%f",&p1[0],&p1[1],&p1[2],&p2[0],&p2[1],&p2[2],&p3[0],&p3[1],&p3[2])!=EOF){
		triangles[tcount].vert[0][0]=(p1[0]+deltaX)*mScale;
		triangles[tcount].vert[0][1]=(p1[1]+deltaY)*mScale;
		triangles[tcount].vert[0][2]=(p1[2]+deltaZ)*mScale;

		triangles[tcount].vert[1][0]=(p2[0]+deltaX)*mScale;
		triangles[tcount].vert[1][1]=(p2[1]+deltaY)*mScale;
		triangles[tcount].vert[1][2]=(p2[2]+deltaZ)*mScale;

		triangles[tcount].vert[2][0]=(p3[0]+deltaX)*mScale;
		triangles[tcount].vert[2][1]=(p3[1]+deltaY)*mScale;
		triangles[tcount].vert[2][2]=(p3[2]+deltaZ)*mScale;

		tcount++;
		//printf("Tcount %d\n",tcount);
	}
	fclose(in);


}

//TODO: fix arguments
void drawSinusoidalHead(float scale,int headWidth){
	for(int i=0;i<headWidth;i+=8){
			float fx = (1.6667 * i/(1.0*lastHeadW)) - 0.332;			
			float fy = 0;//(1.6667 * py/(1.0*lastHeadH)) - 0.332;
			float fz = sin(fx*3.141593);//cos((i-0.5*lastHeadW)/lastHeadW* 1.2 *3.141593);
			float deltaX = -((1.6667 * refX/(1.0*lastHeadW)) - 0.332)*scale;
			float deltaY = -((1.6667 * refY/(1.0*lastHeadH)) - 0.332)*scale;
			float deltaZ = -sin(deltaX*3.141593)*scale;

			glBegin(GL_LINES);
			glColor3f(0.0f,1.0f,0.0);
			glVertex3f(fx*scale+deltaX, +0.332*scale   -deltaY, fz*scale+deltaZ);
			glVertex3f(fx*scale+deltaX, -1.3347*scale  -deltaY, fz*scale+deltaZ);	
			glEnd();

	}
}

void drawHeadModel(){
	glBegin(GL_TRIANGLES);
	

	float deltaX = -(refX/(1.0*lastHeadW)*5.0-2.5);// 1.0f;//-((1.6667 * refX/(1.0*lastHeadW)) - 0.332);
	printf("Refx %d x %f delta %f\n",refX,refX/(1.0*lastHeadW),deltaX);
	float deltaY = refY/(1.0*lastHeadH)*7.5-3.75;//0.0f;//-((1.6667 * refY/(1.0*lastHeadH)) - 0.332);
	float deltaZ = -4.0f * cos(deltaX/2.5*3.141593);
	printf("DeltaZ %f\n",deltaZ);

	for(int i=0;i<3036;i++){
		//glColor3f (i/3036.0, 0.0, 0.0);
		glColor3d (0.0, 0.0, 1.0);
		

	

		float v1x = triangles[i].vert[1][0] - triangles[i].vert[0][0];
		float v1y = triangles[i].vert[1][1] - triangles[i].vert[0][1];
		float v1z = triangles[i].vert[1][2] - triangles[i].vert[0][2];

		float v2x = triangles[i].vert[2][0] - triangles[i].vert[0][0];
		float v2y = triangles[i].vert[2][1] - triangles[i].vert[0][1];
		float v2z = triangles[i].vert[2][2] - triangles[i].vert[0][2];

		float nx = v1y*v2z-v2y*v1z;
		float ny = v1z*v2x-v2z*v1x;
		float nz = v1x*v2y-v2x*v1y;
		glEnable(GL_NORMALIZE);
		glNormal3f( nx,ny,nz);
		glDisable(GL_NORMALIZE);
		float scale = 1.3*20.0f;
		

		
		for(int j=0;j<3;j++){
			//printf(" Vert %f %f %f\n",triangles[i].vert[j][0],triangles[i].vert[j][1],triangles[i].vert[j][2]-5);
			glVertex3f(	scale* (triangles[i].vert[j][0]+deltaX),
						scale* (triangles[i].vert[j][1]+deltaY),
						scale* (triangles[i].vert[j][2]+deltaZ));
		}
	}

	glEnd();
}




/* The main drawing function. */
void DrawGLScene(void)
{  


	CvMatr32f rotation_matrix = new float[9];
	CvVect32f translation_vector = new float[3];
	double glPositMatrix[16];
	int detected = cvLoop(glPositMatrix,initialGuess);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);		// Clear The Screen And The Depth Buffer
	glLoadIdentity();				// Reset The View

	if(initialGuess){

		setInitialRTMatrix(rotation_matrix,translation_vector);
		updateGlPositMatrix(rotation_matrix,translation_vector,glPositMatrix);	
	}



	//printf("Trans %lf %lf %lf\n",glPositMatrix[12],glPositMatrix[13],glPositMatrix[14]);

	if(initialGuess){
		glMatrixMode( GL_MODELVIEW );
		glLoadIdentity();		

		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();

		float mScale = 10000;

		glOrtho(-10*mScale/headWidth*0.05,+10*mScale/headWidth,-9*mScale/headHeight,+10*mScale/headHeight*0.1,-1000.0f,100.0f);

		glBegin(GL_TRIANGLES);
		for(int i=0;i<3036;i++){
			//glColor3d ((GLdouble)i/3036.0, 0.0, 0.0);
			glColor3d (0.0, 0.0, 1.0);
			for(int j=0;j<3;j++){
				glVertex3f( triangles[i].vert[j][0],triangles[i].vert[j][1],triangles[i].vert[j][2]);
			}
		}
		glEnd();

		glBegin(GL_LINES);
		glColor3f(1.0f, 1.0f, 1.0f);
		glVertex3f(0, 0,1.0f);
		glVertex3f(100, 100,1.0f);
		glEnd();

	}
	else{
		glMatrixMode( GL_MODELVIEW );
		glLoadMatrixd( glPositMatrix );

		glMatrixMode(GL_PROJECTION);
		//glLoadIdentity();				// Reset The Projection Matrix
		glLoadMatrixd( projectionMatrix );

		//look in +z direction, the same as POSIT
		gluLookAt(0, 0, 0 , 0, 0, +1.0, 0, -1, 0);

		drawReferenceAxis();

		if(drawSine)drawSinusoidalHead(MODELSCALE,lastHeadW);

		drawHeadModel();
	}



	// since this is double buffered, swap the buffers to display what just got drawn.
	glutSwapBuffers();
	if(detected) initialGuess=0;
	else initialGuess=1;

}

/* The function called when our window is resized (which shouldn't happen, because we're fullscreen) */
void ReSizeGLScene(GLsizei Width, GLsizei Height)
{
	if (Height==0)				// Prevent A Divide By Zero If The Window Is Too Small
		Height=1;

	glViewport(0, 0, Width, Height);		// Reset The Current Viewport And Perspective Transformation

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	gluPerspective(45.0f,(GLfloat)Width/(GLfloat)Height,0.1f,100.0f);
	glMatrixMode(GL_MODELVIEW);
}

/* The function called whenever a normal key is pressed. */
void keyPressed(unsigned char key, int x, int y) 
{

	switch (key) {    
	case 27: // kill everything.
		/* shut down our window */
		glutDestroyWindow(window); 

		/* exit the program...normal termination. */
		exit(1);                   	
		break; // redundant.

	case 's':
		drawSine = !drawSine;
	case 'i':
		initialGuess = !initialGuess;
		printf("Initial guess now is %d\n",initialGuess);
		break;
	case 76: 
	case 108: // switch the lighting.
		printf("L/l pressed; light is: %d\n", light);
		light = light ? 0 : 1;              // switch the current value of light, between 0 and 1.
		printf("Light is now: %d\n", light);
		if (!light) {
			glDisable(GL_LIGHTING);
		} else {
			glEnable(GL_LIGHTING);
		}
		break;


	default:
		break;
	}	
}

/* A general OpenGL initialization function.  Sets all of the initial parameters. */
void InitGL(GLsizei Width, GLsizei Height)	// We call this right after our OpenGL window is created.
{

	setGLProjectionMatrix(projectionMatrix,MYFOCUS);


	glEnable(GL_TEXTURE_2D);                    // Enable texture mapping.


	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);	// This Will Clear The Background Color To Black
	glClearDepth(1.0);				// Enables Clearing Of The Depth Buffer
	glDepthFunc(GL_LESS);			// The Type Of Depth Test To Do
	glEnable(GL_DEPTH_TEST);			// Enables Depth Testing
	glShadeModel(GL_SMOOTH);			// Enables Smooth Color Shading

	//float specReflection[] = { 0.8f, 0.8f, 0.8f, 1.0f };
	//glMaterialfv(GL_FRONT, GL_SPECULAR, specReflection);
	//glMateriali(GL_FRONT, GL_SHININESS, 96);





	glEnable ( GL_COLOR_MATERIAL ) ;

	//float colorBlue[] = { 0.0f, 0.0f, 1.0f, 1.0f };
	//glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, colorBlue);
	//glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);





	glPolygonMode(GL_FRONT, GL_LINE);
	glPolygonMode(GL_BACK, GL_LINE);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();				// Reset The Projection Matrix

	gluPerspective(45.0f,(GLfloat)Width/(GLfloat)Height,0.1f,100.0f);	// Calculate The Aspect Ratio Of The Window

	glMatrixMode(GL_MODELVIEW);

	// set up light number 1.
	//    glLightfv(GL_LIGHT1, GL_AMBIENT, LightAmbient);  // add lighting. (ambient)
	glLightfv(GL_LIGHT1, GL_DIFFUSE, LightDiffuse);  // add lighting. (diffuse).
	glLightfv(GL_LIGHT1, GL_POSITION,LightPosition); // set light position.
	glEnable(GL_LIGHT1);                             // turn light 1 on.
	//    glEnable(GL_LIGHTING);
}

void openGLCustomInit(int argc, char** argv ){

	glutInit(&argc, argv);  

	/* Select type of Display mode:   
     Double buffer 
     RGBA color
     Alpha components supported 
     Depth buffer */  
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_ALPHA | GLUT_DEPTH);  

	/* get a 640 x 480 window */
	glutInitWindowSize(640, 480);  

	/* the window starts at the upper left corner of the screen */
	glutInitWindowPosition(500, 0);  

	/* Open a window */  
	window = glutCreateWindow("6DOF Head Tracking OpenGL - EHCI - Daniel Lelis Baggio");

	/* Register the function to do all our OpenGL drawing. */
	glutDisplayFunc(&DrawGLScene);  

	/* Go fullscreen.  This is as soon as possible. */
	//    glutFullScreen();
	//   glutReshapeWindow(640,480);

	/* Even if there are no events, redraw our gl scene. */
	glutIdleFunc(&DrawGLScene);

	/* Register the function called when our window is resized. */
	glutReshapeFunc(&ReSizeGLScene);

	/* Register the function called when the keyboard is pressed. */
	glutKeyboardFunc(&keyPressed);

	/* Register the function called when special keys (arrows, page down, etc) are pressed. */
	//    glutSpecialFunc(&specialKeyPressed);

	/* Initialize our window. */
	InitGL(640, 480);


	/* Start Event Processing Engine */  
	glutMainLoop();  

}



/*
 * This function detects where the head is and returns the openGL matrix associated 
 * in case it doesn't work (if the grabed image was too dark, or if it was one of 
 * the first frames, during initialization), it will return 0. It returns 1 otherwise
 */

int cvLoop(double glPositMatrix[16],int initialGuess){

	static int flags = 0;

	IplImage* frame = 0;
	int i, k, c;
	static int numberOfTrackingPoints=0;



	if(READFROMIMAGEFILE){
		frame = cvLoadImage( "head.jpg", 1 );
	}
	else{
		frame = cvQueryFrame( capture );
		if( !frame )
			return 0;
	}

	if( !image )
	{
		/* allocate all the buffers */
		image = cvCreateImage( cvGetSize(frame), 8, 3 );
		image->origin = frame->origin;
		grey = cvCreateImage( cvGetSize(frame), 8, 1 );
		prev_grey = cvCreateImage( cvGetSize(frame), 8, 1 );
		pyramid = cvCreateImage( cvGetSize(frame), 8, 1 );
		prev_pyramid = cvCreateImage( cvGetSize(frame), 8, 1 );
		points[0] = (CvPoint2D32f*)cvAlloc(MAX_COUNT*sizeof(points[0][0]));
		points[1] = (CvPoint2D32f*)cvAlloc(MAX_COUNT*sizeof(points[0][0]));
		status = (char*)cvAlloc(MAX_COUNT);
		flags = 0;
	}

	cvCopy( frame, image, 0 );
	cvCvtColor( image, grey, CV_BGR2GRAY );

	if( night_mode )
		cvZero( image );





	getHeadPosition(image, &upperHeadCorner,&headWidth,&headHeight );


	printf("Head x %d head y %d width %d height %d\n",upperHeadCorner.x,upperHeadCorner.y,headWidth,headHeight);	

	if(initialGuess){
		//automatic initialization won't work in case face was not detected
		if((headWidth == 0) || (headHeight==0)) return 0;				
		if((upperHeadCorner.x>=0)&&(upperHeadCorner.y>=0)&&
				(upperHeadCorner.x+headWidth< cvGetSize(grey).width) && (upperHeadCorner.y+headHeight< cvGetSize(grey).height))
			numberOfTrackingPoints = insertNewPoints(grey,upperHeadCorner.x+(int)(0.25*headWidth),upperHeadCorner.y+(int)(0.25*headHeight),
					(int)(headWidth*0.5),(int)(headHeight*0.5),points[0]);
		lastHeadW = headWidth;
		lastHeadH = headHeight;
	}


	if( numberOfTrackingPoints > 0 )
	{
		cvCalcOpticalFlowPyrLK( prev_grey, grey, prev_pyramid, pyramid,
				points[0], points[1], numberOfTrackingPoints, cvSize(win_size,win_size), 3, status, 0,
				cvTermCriteria(CV_TERMCRIT_ITER|CV_TERMCRIT_EPS,20,0.03), flags );
		flags |= CV_LKFLOW_PYR_A_READY;
		for( i = k = 0; i < numberOfTrackingPoints; i++ )
		{

			if( !status[i] )
				continue;

			points[1][k++] = points[1][i];
			cvCircle( image, cvPointFrom32f(points[1][i]), 3, CV_RGB(0,200+20*i,0), -1, 8,0);
		}
		numberOfTrackingPoints = k;
	}
	




	CvMatr32f rotation_matrix = new float[9];
	CvVect32f translation_vector = new float[3];


	if(numberOfTrackingPoints >=NUMPTS){
		//getPositMatrix uses points[1] obtained from cvCalcOpticalFlowPyrLK
		getPositMatrix(image,initialGuess, rotation_matrix,translation_vector,
				numberOfTrackingPoints,MYFOCUS,points[1],upperHeadCorner,
				headWidth,headHeight,&refX,&refY,MODELSCALE);
		updateGlPositMatrix(rotation_matrix,translation_vector,glPositMatrix);	

	}
	printf("Number of tracking points %d %d\n",numberOfTrackingPoints,initialGuess);

	CV_SWAP( prev_grey, grey, swap_temp );
	CV_SWAP( prev_pyramid, pyramid, swap_temp );
	CV_SWAP( points[0], points[1], swap_points );


	cvShowImage( "6dofHead", image );

	c = cvWaitKey(10);

	if(numberOfTrackingPoints<NUMPTS)
		return 0;
	else
		return 1;
}

int main( int argc, char** argv )
{



	char rawFile[]="head.raw";
	loadRaw(rawFile);

	if( argc == 1 || (argc == 2 && strlen(argv[1]) == 1 && isdigit(argv[1][0])))
		capture = cvCaptureFromCAM( argc == 2 ? argv[1][0] - '0' : 0 );
	else if( argc == 2 )
		capture = cvCaptureFromAVI( argv[1] );

	if( !capture )
	{
		fprintf(stderr,"Could not initialize capturing...\n");
		return -1;
	}

	cvNamedWindow( "6dofHead", 1 );

	openGLCustomInit(argc,argv);



	cvReleaseCapture( &capture );
	cvDestroyWindow("6dofHead");

	return 0;
}
