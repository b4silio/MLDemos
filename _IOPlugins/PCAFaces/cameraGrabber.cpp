#include <public.h>
#include "cameraGrabber.h"

CameraGrabber::CameraGrabber()
:capture(0), width(0), height(0), framerate(30)
{
	capture = cvCaptureFromCAM(CV_CAP_ANY);
        if(capture)
        {
            width = (u32)cvGetCaptureProperty(capture, CV_CAP_PROP_FRAME_WIDTH);
            height = (u32)cvGetCaptureProperty(capture, CV_CAP_PROP_FRAME_HEIGHT);
            framerate = (f32)cvGetCaptureProperty(capture, CV_CAP_PROP_FPS);
        }
}

void CameraGrabber::GrabFrame(IplImage **frame, u32 )
{
    if(!capture)
    {
        return;
    }
	IplImage *pFrame = cvQueryFrame( capture );
	if(!pFrame) return;
	if (!(*frame) || (*frame)->width != pFrame->width || (*frame)->height != pFrame->height)
	{
		if ((*frame)) cvReleaseImage(frame);
		(*frame) = cvCreateImage(cvGetSize(pFrame), pFrame->depth, pFrame->nChannels);
	}
	if(pFrame->origin != IPL_ORIGIN_TL)
		cvFlip(pFrame, (*frame));
	else
		cvCopy(pFrame,(*frame));
}

void CameraGrabber::Kill()
{
    if(capture) cvReleaseCapture(&capture);
}

f32 CameraGrabber::GetFramerate()
{
	return framerate;
}

CvSize CameraGrabber::GetSize()
{
	return cvSize(width, height);
}
