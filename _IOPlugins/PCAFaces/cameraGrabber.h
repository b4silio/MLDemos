/*!
 * \file cameraGrabber.h
 * \author Basilio Noris
 * \date 19-03-11
 */
#ifndef _CAMERAGRABBER_H_
#define _CAMERAGRABBER_H_

#include "basicOpenCV.h"
#include <opencv2/videoio.hpp>

/*!
 *	Camera Frame Grabber, grabs images from an input webcam
 */
class CameraGrabber
{
private:
    cv::VideoCapture input;

public:
	/*!
		The Standard Constructor
		\param filename the input file name
	*/
	CameraGrabber();

	/*!
		The Grabbing function
		\param frame the destination frame pointer
	*/
    void GrabFrame(cv::Mat &image);

	/*!
		The Kill function, frees up the buffers allocated by the grabber
	*/
	void Kill();

};

#endif //_CAMERAGRABBER_H_
