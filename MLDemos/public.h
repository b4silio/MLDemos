/*********************************************************************
MLDemos: A User-Friendly visualization toolkit for machine learning
Copyright (C) 2010  Basilio Noris
Contact: mldemos@b4silio.com

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public License,
version 3 as published by the Free Software Foundation.

This library is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free
Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*********************************************************************/

/*!
 *	\file public.h
 *	\author Basilio Noris
 *	\date 25-11-06
 */

#ifndef _PUBLIC_H_
#define _PUBLIC_H_

#ifdef WIN32
#pragma warning(disable:4996) // avoids the sprintf_s and fopen_s warning which dont exist on linux ;)
#endif

// opencv includes
#include "cv.h"
#include "cvaux.h"
#include "highgui.h"

// types and macros
#include "types.h"
#include "mymaths.h"

/*!
 * The Filter class, one of the main tools to process images
 */
class Filter
{
public:
	/*!
		The main Processing function, takes an input image and if necessary can make modifications over it
		\param image the input image
	*/
	virtual void Apply(IplImage *){};

	/*!
		The secondary Processing function, usually does the same as the Apply()
		function but returns the modifications as an output image, leaving the source image intact
		\param image the input image
		\return returns the filtered image
	*/
	virtual IplImage *Process(IplImage *){return NULL;};

	/*!
		The Configuration function, can be useful for certain filters
		\param image the input image
		\param selection the region of the image the filter will be configured from
	*/
	virtual void Config(IplImage *, CvRect , IplImage * =0){};
};

/*!
 * The Frame Grabber virtual class
 */
class FrameGrabber
{
public:
	/*!
		Grabs the current frame
		\param frame the pointer to the frame that will be filled by the grabber
		\param index can be the index to a particular frame or to one of many streams (in case of stereo grabbers)
	*/
	virtual void GrabFrame(IplImage **, u32 =0){};
	
	/*!
		Get the resolution of the frame grabber (or of the current frame)
		\return returns the size of the current frame
	*/
	virtual CvSize GetSize(){return cvSize(0,0);};

	/*!
		Kills the grabber, freeing and releasing up the memory allocated by the grabber
	*/
	virtual void Kill(){};
};

#endif //_PUBLIC_H_
