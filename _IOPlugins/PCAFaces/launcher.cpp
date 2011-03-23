#include "public.h"
#include "basicMath.h"
#include "basicOpenCV.h"
#include "aviGrabber.h"
#include "viGrabber.h"
#include "sampleManager.h"


SampleManager trainSamples;

void on_mouse( int event, int x, int y, int flags, void* param );
u32 select_object = 0;
CvRect selection;
CvPoint origin;
bool bConstrainSelection = false;
IplImage *image = 0;

void GetFromGrabber(FrameGrabber *grabber);

s32 main(s32 argc, char **argv)
{
	FrameGrabber *grabber = 0;

	if (argc == 1) grabber = new VIGrabber();
	else
	{
		char *source = argv[1];
		grabber = new AviGrabber(source);
	}

	GetFromGrabber(grabber);

	grabber->Kill();
	DEL(grabber);
}


/****************************************************/
/*     mouse listener (for filter configuration)    */
/****************************************************/
void on_mouse( int event, int x, int y, int flags, void* param )
{
    if( !image )
        return;

    if( image->origin )
        y = image->height - y;

    if( select_object )
    {
        selection.x = MIN(x,origin.x);
        selection.y = MIN(y,origin.y);
        selection.width = selection.x + CV_IABS(x - origin.x);
        selection.height = selection.y + CV_IABS(y - origin.y);
        
        selection.x = MAX( selection.x, 0 );
        selection.y = MAX( selection.y, 0 );
        selection.width = MIN( selection.width, image->width );
        selection.height = MIN( selection.height, image->height );
        selection.width -= selection.x;
        selection.height -= selection.y;
		if(bConstrainSelection)
		{
			selection.width = min(selection.width, selection.height);
			selection.height = selection.width;
		}
    }

    switch( event )
    {
    case CV_EVENT_LBUTTONDOWN:
        origin = cvPoint(x,y);
        selection = cvRect(x,y,0,0);
        select_object = 1;
        break;
    case CV_EVENT_LBUTTONUP:
        select_object = 0;
        break;
    }
}


void GetFromGrabber(FrameGrabber *grabber)
{
	if(!grabber) return;

	IplImage *sample = cvCreateImage(grabber->GetSize(), 8, 3);
	bConstrainSelection = true;
	for(;;)
	{
		trainSamples.Show();
		grabber->GrabFrame(&image);
		cvCopy(image, sample);

		cvRectangle(image, cvPoint(selection.x, selection.y), cvPoint(selection.x+selection.width, selection.y+selection.height), CV_RGB(0,0,0),3);
		cvRectangle(image, cvPoint(selection.x, selection.y), cvPoint(selection.x+selection.width, selection.y+selection.height), CV_RGB(255,255,255));

		cvNamedWindow("sample");
		cvSetMouseCallback("sample", on_mouse);
		cvShowImage("sample", image);

		int c = cvWaitKey(1);
		if(c==27) break;
		if(c==' ')
		{
			trainSamples.AddSample(image, selection);
		}
		if(c=='s') trainSamples.Save("newSamples.png");
	}
	cvDestroyWindow("sample");
	trainSamples.Hide();
	IMKILL(image);
	trainSamples.Save("newSamples.png");
}
