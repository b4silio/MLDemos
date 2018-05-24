#include <public.h>
#include "cameraGrabber.h"

CameraGrabber::CameraGrabber()
{
    input.open(0);
}

void CameraGrabber::GrabFrame(cv::Mat& image)
{
    if(input.isOpened()) input >> image;
}

void CameraGrabber::Kill()
{
    input.release();
}
