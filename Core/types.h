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
#ifndef __TYPES_H__
#define __TYPES_H__

#include <vector>
#include <map>
// type definitions, u <- unsigned, s <- signed, f <- floating point, X: number of bits
typedef unsigned char u8;
typedef unsigned short int u16;
typedef unsigned int u32;
typedef unsigned long u64;
typedef char s8;
typedef short int s16;
typedef int s32;
typedef long s64;
typedef float f32;
typedef double f64;

typedef std::vector<double> dvec;
typedef std::vector<float> fvec;
typedef std::vector<unsigned int> uvec;
typedef std::vector<int> ivec;
typedef std::vector<bool> bvec;

typedef std::pair<int,int> ipair;
typedef std::pair<float,float> fpair;
typedef std::pair<double,double> dpair;

#include <algorithm>
using std::max;
using std::min;
//#ifndef max
//#define max(a,b) (a > b ? a : b)
//#define min(a,b) (a < b ? a : b)
//#endif

// fancy macros
#define FOR(i,length) for(u32 i=0; i<(u32)(length); i++) 
#define FORIT(a,x,y) for(std::map<x,y>::iterator it = a.begin(); it != a.end(); it++)
#define SWAP(x,y) x^=y^=x^=y // fast swap for pointers or integers
#define MAX3(a,b,c) max(a,max(b,c))
#define MIN3(a,b,c) min(a,min(b,c))
#define TRIM(a,b,c) min(max(a,b),c)
#define Truncate(a,b) a = ( a>=b ? b : a)

// delete macros
#define DEL(a) if(a){delete a; a=NULL;} // safe delete pointer
#define KILL(a) if(a){delete [] a; a=NULL;} // safe delete []
#define IMKILL(a) if(a) {cvReleaseImage(&(a)); (a)=NULL;} // safe delete image
// reading pixels from an image
#undef RGB
#define RGB(image,i) (image->imageData[i])
#define to255(x) ((x<<8)-x) // fast multiply *255
// define an image's region of interest
#define ROI(a,b) cvSetImageROI(a,b)
#define unROI(a) cvResetImageROI(a)

// definitions for the matlab-style tic toc
#define TICTOC u64 tic_time = 0;
#define tic tic_time = (u64)(cvGetTickCount()/cvGetTickFrequency())
#define toc printf("Elapsed time is %.3f seconds.\n",((u64)(cvGetTickCount()/cvGetTickFrequency()) - tic_time)/1000000.0f)
#define etoc ((u64)(cvGetTickCount()/cvGetTickFrequency()) - tic_time)

//
#include <string.h>
static bool endsWith(const char *a,const char *b)
{
	bool yeah = true;
	int lena = strlen(a);
	int lenb = strlen(b);
	for (int i=0; i<lenb; i++)
	{
		yeah &= a[lena - lenb + i] == b[i];
	}
	return yeah;
}


#endif // __TYPES_H__
