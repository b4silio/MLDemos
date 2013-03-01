/*!
 * \file cameraGrabber.h
 * \author Basilio Noris
 * \date 19-03-11
 */
#ifndef _CAMERAGRABBER_H_
#define _CAMERAGRABBER_H_

#include "basicOpenCV.h"

/*!
 *	Camera Frame Grabber, grabs images from an input webcam
 */
class CameraGrabber
{
private:
	u32 currentFrame;
	CvCapture *capture;
	u32 width;
	u32 height;
	f32 framerate;

public:
	/*!
		The Standard Constructor
		\param filename the input file name
	*/
	CameraGrabber();

	/*!
		Gets the resolution of the video
		\return returns the resolution of the video
	*/
	CvSize GetSize();

	/*!
		Gets the video framerate
		\return returns the video framerate
	*/
	f32 GetFramerate();

	/*!
		The Grabbing function
		\param frame the destination frame pointer
		\index not used
	*/
	void GrabFrame(IplImage **frame, u32 index=0);

	/*!
		The Kill function, frees up the buffers allocated by the grabber
	*/
	void Kill();

};

#endif //_CAMERAGRABBER_H_
