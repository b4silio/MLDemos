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
#include "public.h"
#include <algorithm>
#include "basicOpenCV.h"

using namespace std;

const cv::Scalar BasicOpenCV::color [22]= {
    CV_RGB(255,255,255), CV_RGB(0,0,255), CV_RGB(0,255,0), CV_RGB(255,0,0),
    CV_RGB(0, 255,255), CV_RGB(255,0,255), CV_RGB(255,255,0),
    CV_RGB(0,128,255), CV_RGB(255,0,128), CV_RGB(128,255,0),
    CV_RGB(128,255,0), CV_RGB(128,0,255), CV_RGB(0,128,255),
    CV_RGB(128,128,128), CV_RGB(80,80,80), CV_RGB(0,128,80),
    CV_RGB(255,80,0), CV_RGB(255,0,80), CV_RGB(0,255,80),
    CV_RGB(80,255,0), CV_RGB(80,0,255), CV_RGB(0,80,255)
};
