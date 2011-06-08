/*********************************************************************
MLDemos: A User-Friendly visualization toolkit for machine learning
Copyright (C) 2010  Basilio Noris
Contact: mldemos@b4silio.com

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free
Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*********************************************************************/
#include "drawUtils.h"
#include "basicMath.h"
using namespace std;

#define RES 256
void DrawEllipse(float *mean, float *sigma, float rad, QPainter *painter, QSize size)
{
	if(mean[0] != mean[0] || mean[1] != mean[1]) return; // nan
	float a = sigma[0], b = sigma[1], c = sigma[2];
	float L[4];
	L[0] = a; L[1] = 0; L[2] = b; L[3] = sqrtf(c*a-b*b);
	if(L[3] != L[3]) L[3] = 0;
	FOR(i,4) L[i] /= sqrtf(a);

	const int segments = 64;
	float oldX = FLT_MAX, oldY = FLT_MAX;
	for (float theta=0; theta <= PIf*2.f; theta += (PIf*2.f)/segments)
	{
		float x = cosf(theta)*rad;
		float y = sinf(theta)*rad;
		float nx = L[0]*x;
		float ny = L[2]*x + L[3]*y;
		nx += mean[0];
		ny += mean[1];
		if(oldX != FLT_MAX)
		{
			painter->drawLine(
				QPointF(nx*size.width(),ny*size.height()),
				QPointF(oldX*size.width(),oldY*size.height())
				);
		}
		oldX = nx;
		oldY = ny;
	}
}

void DrawEllipse(float *mean, float *sigma, float rad, QPainter *painter, Canvas *canvas)
{
	if(mean[0] != mean[0] || mean[1] != mean[1]) return; // nan
	float a = sigma[0], b = sigma[1], c = sigma[2];
	float L[4];
	L[0] = a; L[1] = 0; L[2] = b; L[3] = sqrtf(c*a-b*b);
	if(L[3] != L[3]) L[3] = 0;
	FOR(i,4) L[i] /= sqrtf(a);

	const int segments = 64;
	float oldX = FLT_MAX, oldY = FLT_MAX;
	for (float theta=0; theta <= PIf*2.f; theta += (PIf*2.f)/segments)
	{
		float x = cosf(theta)*rad;
		float y = sinf(theta)*rad;
		float nx = L[0]*x;
		float ny = L[2]*x + L[3]*y;
		nx += mean[0];
		ny += mean[1];
		if(oldX != FLT_MAX)
		{
			painter->drawLine(canvas->toCanvasCoords(nx,ny), canvas->toCanvasCoords(oldX, oldY));
		}
		oldX = nx;
		oldY = ny;
	}
}

void DrawArrow( const QPointF &ppt, const QPointF &pt, double sze, QPainter &painter)
{
	QPointF pd, pa, pb;
	double tangent;

	pd = ppt - pt;
	if (pd.x() == 0 && pd.y() == 0)
		return;
	tangent = atan2 ((double) pd.y(), (double) pd.x());
        pa.setX(sze * cos (tangent + PIf / 7.f) + pt.x());
        pa.setY(sze * sin (tangent + PIf / 7.f) + pt.y());
        pb.setX(sze * cos (tangent - PIf / 7.f) + pt.x());
        pb.setY(sze * sin (tangent - PIf / 7.f) + pt.y());
	//-- connect the dots...
	painter.drawLine(pt, ppt);
        painter.drawLine(pt, pa);
	painter.drawLine(pt, pb);
}

QColor ColorFromVector(fvec a)
{
	// angle is between 0 and 1;
        float angle = atan2(a[0], a[1]) / (2*PIf) + 0.5f;
	vector<fvec> colors;
	#define Col2Col(r,g,b) {fvec c;c.resize(3); c[0] = r;c[1] = g;c[2] = b; colors.push_back(c);}

	Col2Col(0,0,255);
	Col2Col(255,0,255);
	Col2Col(255,0,0);
	Col2Col(255,255,0);
	Col2Col(0,255,0);
	Col2Col(0,255,255);

	// find where the angle fits in the color list
	int index = (int)(angle*(colors.size())) % colors.size();
	fvec c1 = colors[index];
	fvec c2 = colors[(index+1)%colors.size()];

	// compute the ratio between c1 and c2
	float remainder = angle*(colors.size()) - (int)(angle*(colors.size()));
	fvec c3 = c1*(1-remainder) + c2*remainder;
	return QColor(c3[0],c3[1],c3[2]);
}
