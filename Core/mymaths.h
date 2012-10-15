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

#include <fstream>
#include <vector>
#include <cmath>
#include "types.h"

struct fVec
{
	friend std::ostream& operator<<(std::ostream& output, const fVec& v) {
		output << "(" <<  v.x << ", " << v.y <<")";
		return output;  // for multiple << operators.
	}
	friend std::istream & operator>>(std::istream &input, fVec &v) {
		input >> v.x >> v.y;
		return input;  // for multiple >> operators.
	}

    fVec(const float x=0, const float y=0) : x(x),y(y){}
    fVec(const float* v) : x(v[0]),y(v[1]){}
    fVec(const fVec& v) : x(v.x),y(v.y){}
    fVec(fvec v) {x=v.size()>1?v[0]:0;x=v.size()>1?v[1]:0;}
    //fVec(const fvec &v) {x=v.size()>1?v[0]:0;x=v.size()>1?v[1]:0;}
	union {
		float _[2];
		struct {float x,y;};
	};
    float& operator[] (unsigned int i){return _[i];}
    float& operator() (unsigned int i){return _[i];}

	fVec& operator= (const fVec &v) {
		if (this != &v) {
			x = v.x;
			y = v.y;
		}
		return *this;
	}

	operator fvec () const {
		fvec a;
		a.resize(2);
		a[0] = x;
		a[1] = y;
		return a;
	}

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

void operator += (fvec &a, const fvec b);
void operator -= (fvec &a, const fvec b);
void operator += (fvec &a, const float b);
void operator -= (fvec &a, const float b);
void operator *= (fvec &a, const float b);
void operator /= (fvec &a, const float b);
//fvec& operator << (fvec &a, const fvec b);
//fvec& operator << (fvec &a, const float b);
fvec operator + (const fvec a, const fvec b);
fvec operator - (const fvec a, const fvec b);
fvec operator + (const fvec a, const float b);
fvec operator - (const fvec a, const float b);
fvec operator * (const fvec a, const float b);
fvec operator / (const fvec a, const float b);
float operator * (const fvec a, const fvec b); // dot product
bool operator == (const fvec a, const fvec b);
bool operator == (const fvec a, const float b);
bool operator != (const fvec a, const fvec b);
bool operator != (const fvec a, const float b);
//dvec operator = (const fvec a);
//void operator = (dvec &a, const fvec b);

std::vector<fvec> interpolate(std::vector<fvec> a, int count);
std::vector<fvec> interpolateSpline(std::vector<fvec> a, int count);

// generate random sample from normal distribution
static inline float ranf()
{
	return rand()/(float)RAND_MAX;
}

static inline float RandN(float mean=0.f, float sigma=1.f)
{
	float x1, x2, w, y1, y2;

	do {
		x1 = 2.0 * ranf() - 1.0;
		x2 = 2.0 * ranf() - 1.0;
		w = x1 * x1 + x2 * x2;
	} while ( w >= 1.0 );

	w = sqrt( (-2.0 * log( w ) ) / w );
	y1 = x1 * w;
	//y2 = x2 * w;
	return y1*sigma + mean;
}

static inline fvec RandN(int dim, float mean=0.f, float sigma=1.f)
{
	if(!dim) return fvec();
	fvec res;
	res.resize(dim);
	FOR(d,dim) res[d] = RandN(mean, sigma);
	return res;
}

fvec RandCovMatrix(int dim, float minLambda);

#endif // _MY_MATHS_H
