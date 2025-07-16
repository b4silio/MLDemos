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
#ifndef _MY_MATHS_H
#define _MY_MATHS_H

struct fVec
{
	fVec(const float x=0, const float y=0) : x(x),y(y){};
	fVec(const float* v) : x(v[0]),y(v[1]){};
	fVec(const fVec& v) : x(v.x),y(v.y){};
	union {
		float _[2];
		struct {float x,y;};
	};
	float& operator[] (unsigned int i){return _[i];};
	float& operator() (unsigned int i){return _[i];};

	//operators
	fVec operator-() const {return fVec(-x, -y);}

	fVec operator+(const fVec& v) const {return fVec(x + v.x, y + v.y);}
	fVec operator-(const fVec& v) const {return fVec(x - v.x, y - v.y);}
	float operator*(const fVec& v) const {return x*v.x + y*v.y;}

	fVec operator+(const float& v) const {return fVec(x + v, y + v);}
	fVec operator-(const float& v) const {return fVec(x - v, y - v);}
	fVec operator*(const float& d) const {return fVec(x * d, y * d);}
	fVec operator/(const float& d) const {return fVec(x / d, y / d);}

	bool operator==(const fVec& v) const {return x == v.x && y == v.y;}
	bool operator!=(const fVec& v) const {return x != v.x || y != v.y;}

	fVec& operator+=(const fVec& v) {x += v.x;y += v.y;return *this;}
	fVec& operator-=(const fVec& v) {x -= v.x;y -= v.y;return *this;}

	fVec& operator+=(const float& v) {x += v;y += v;return *this;}
	fVec& operator-=(const float& v) {x -= v;y -= v;return *this;}
	fVec& operator*=(const float& d) {x *= d;y *= d;return *this;}
	fVec& operator/=(const float& d) {
		if(d==0) {x = y = 0; return *this;}
		float inv = 1.f / d;x *= inv;y *= inv;return *this;}

	// other functions
	inline int size() const{return 2;}
	inline fVec& normalize() {if(x==0 && y==0){x=1;return *this;};float l = length();x /= l;y /= l;return *this;}
	inline float lengthSquared() const {return x * x + y * y;}
	inline float length() const {return sqrt(lengthSquared());}
};

#include <vector>
#include "types.h"

void operator += (fvec &a, const fvec b);
void operator -= (fvec &a, const fvec b);
void operator += (fvec &a, const float b);
void operator -= (fvec &a, const float b);
void operator *= (fvec &a, const float b);
void operator /= (fvec &a, const float b);
fvec operator + (const fvec a, const fvec b);
fvec operator - (const fvec a, const fvec b);
fvec operator + (const fvec a, const float b);
fvec operator - (const fvec a, const float b);
fvec operator * (const fvec a, const float b);
fvec operator / (const fvec a, const float b);
float operator * (const fvec a, const fvec b);

std::vector<fvec> interpolate(std::vector<fvec> a, int count);

#endif // _MY_MATHS_H
