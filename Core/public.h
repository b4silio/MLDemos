/*
	Public inclusions for opencv

	Copyright (C) 2007 basilio noris

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/*!
 *	\file public.h
 *	\author Basilio Noris
 *	\date 25-11-06
 *      Public includes for opencv and types definitions
 */

#ifndef _PUBLIC_H_
#define _PUBLIC_H_

#ifdef MSVC
#pragma warning(disable:4996) // avoids the sprintf_s and fopen_s warning which dont exist on linux ;)
#pragma warning(disable:4244) // conversion from X to Y, possible loss of data
#pragma warning(disable:4305) // 'initializing' : truncation from X to Y
#endif

#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <float.h>
#include <cmath>

#ifdef WIN32
#define drand48() (rand()/(float)RAND_MAX) // not as accurate as it could be
#define srand48(seed) (srand(seed))
#endif

// types and macros
#include "types.h"

// opencv includes
/*
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/features2d/features2d.hpp>
#include <opencv2/video/tracking.hpp>
#include <opencv2/ml/ml.hpp>
#include <opencv2/legacy/legacy.hpp>
#include <opencv2/legacy/compat.hpp>
*/

#include <QColor>
static const QColor SampleColor [22]= {
	QColor(255,255,255), QColor(255,0,0), QColor(0,255,0), QColor(0,0,255),
	QColor(255,255,0), QColor(255,0,255), QColor(0,255,255),
	QColor(255,128,0), QColor(255,0,128), QColor(0,255,128),
	QColor(128,255,0), QColor(128,0,255), QColor(0,128,255),
	QColor(128,128,128), QColor(80,80,80), QColor(0,128,80),
	QColor(255,80,0), QColor(255,0,80), QColor(0,255,80),
	QColor(80,255,0), QColor(80,0,255), QColor(0,80,255)
};
static const int SampleColorCnt = 22;

#endif //_PUBLIC_H_
