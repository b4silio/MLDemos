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
#ifndef _ROC_H_
#define _ROC_H_

typedef std::pair<float, float> f32pair;
typedef std::vector<f32pair> rocData;

/*
IplImage *GetRocImage();
void roc_on_mouse( int event, int x, int y, int flags, void* param );
IplImage *RocImage(std::vector<std::vector<f32pair> > dataVector, std::vector<char *> legend = std::vector<char *>(), CvSize res = cvSize(512,512));
void RocCurve(std::vector<f32pair> data);
void RocCurves(std::vector<std::vector<f32pair> > dataVector, std::vector<char *> legend = std::vector<char *>());
void SaveRocImage(const char *filename);
*/

void SaveRoc(std::vector<f32pair> data, const char *filename);
std::vector<f32pair> LoadRoc(const char *filename);

std::vector<f32pair> FixRocData(std::vector<f32pair> data);
std::vector<float> GetBestFMeasures();
float GetBestThreshold(std::vector<f32pair> data);
std::vector<float> GetBestFMeasure(std::vector<f32pair> data);
std::pair<float,float> GetMicroMacroFMeasure(std::vector<f32pair> data);
float GetAveragePrecision(std::vector<f32pair> data);
float GetRocValueAt(std::vector<f32pair> data, float threshold);

#endif // _ROC_H_
