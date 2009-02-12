#ifndef EHCI_H_
#define EHCI_H_


#include "cv.h"
#include "highgui.h"

void updateGlPositMatrix(CvMatr32f rotation_matrix,CvVect32f translation_vector,double glPositMatrix[16]);
void setInitialRTMatrix(CvMatr32f rotation_matrix,CvVect32f translation_vector);


/*uses ViolaJones to find head position
 *returns upperHeadCorner, headWidth, and headHeight
 */
void getHeadPosition(IplImage* frame, CvPoint* upperHeadCorner,int* headWidth,int* headHeight );


void getPositMatrix(IplImage* myImage,int initialGuess, CvMatr32f rotation_matrix, CvVect32f translation_vector,
		int numOfTrackingPoints,int focus,CvPoint2D32f* points, CvPoint upperHeadCorner, 
		int headWidth, int headHeight, float modelScale);

int insertNewPoints(IplImage* grey, int headX,int headY,int width, int height,
		CvPoint2D32f* points);

void setGLProjectionMatrix(double projectionMatrix[16], double focus);

int cvLoop(double glPositMatrix[16],int initialGuess,int focus,float modelScale, CvCapture* capture,
		int* refX,int * refY,int* myLastHeadW, int* myLastHeadH);

//returns last captured frame
IplImage* getCurrentFrame();






#endif /*EHCI_H_*/