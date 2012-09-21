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

const CvScalar BasicOpenCV::color [22]= {
    CV_RGB(255,255,255), CV_RGB(255,0,0), CV_RGB(0,255,0), CV_RGB(0,0,255),
    CV_RGB(255,255,0), CV_RGB(255,0,255), CV_RGB(0,255,255),
    CV_RGB(255,128,0), CV_RGB(255,0,128), CV_RGB(0,255,128),
    CV_RGB(128,255,0), CV_RGB(128,0,255), CV_RGB(0,128,255),
    CV_RGB(128,128,128), CV_RGB(80,80,80), CV_RGB(0,128,80),
    CV_RGB(255,80,0), CV_RGB(255,0,80), CV_RGB(0,255,80),
    CV_RGB(80,255,0), CV_RGB(80,0,255), CV_RGB(0,80,255)
};

void cvDrawRect(IplImage *img, CvRect rect, CvScalar color, int thickness, int line_type, int shift)
{
    cvRectangle(img, cvPoint(rect.x, rect.y), cvPoint(rect.x+rect.width, rect.y+rect.height), color, thickness, line_type, shift);
}


// directions: 0=clockwise, 1: counterclockwise, 2: flip
IplImage *BasicOpenCV::Rotate90(IplImage *src, u32 direction)
{
    if(!src) return NULL;
    IplImage *dst = cvCreateImage(cvSize(src->height, src->width), src->depth, src->nChannels);
    if(direction == 0)
    {
        FOR(i, src->height)
        {
            FOR(j, src->width)
            {
                cvSet2D(dst, i, j, cvGet2D(src, src->height-j-1, i));
            }
        }
    }
    else if(direction == 1)
    {
        FOR(i, src->height)
        {
            FOR(j, src->width)
            {
                cvSet2D(dst, i, j, cvGet2D(src, j, src->width-i-1));
            }
        }
    }
    else
    {
        FOR(i, src->height)
        {
            FOR(j, src->width)
            {
                cvSet2D(dst, i, j, cvGet2D(src, j, i));
            }
        }
    }
    return dst;
}

IplImage *BasicOpenCV::Rotate(IplImage *src, float angle)
{
    IplImage* dst = cvCloneImage( src );

    cvNamedWindow( "src", 1 );
    cvShowImage( "src", src );

    float m[6];
    CvMat M = cvMat( 2, 3, CV_32F, m );
    int w = src->width;
    int h = src->height;

    float factor = 1.f;//resizing factor
    m[0] = (float)(factor*cosf(-angle*2*(float)CV_PI/180.f));
    m[1] = (float)(factor*sinf(-angle*2*(float)CV_PI/180.f));
    m[2] = w*0.5f;
    m[3] = -m[1];
    m[4] = m[0];
    m[5] = h*0.5f;

    cvGetQuadrangleSubPix( src, dst, &M);

    return dst;
}

void BasicOpenCV::integralImage(const IplImage *image, IplImage **intimage)
{
    u32 W = image->width, H = image->height;

    IMKILL((*intimage));

    // Create resulting images
    (*intimage) = cvCreateImage(cvGetSize(image),IPL_DEPTH_32S,image->nChannels);
    (*intimage)->origin = image->origin;

    u8 *i_m = (u8 *)(image->imageData);
    u32 *ii_m = (u32 *)((*intimage)->imageData);

    ii_m[0] = i_m[0];

    // Create the first row of the integral image
    for (register u32 x = 1; x < W; x++)
    {
        ii_m[x] = ii_m[x-1] + i_m[x];
    }

    // Compute each other row/column
    for (register u32 y = 1, Y = W, YY=0; y < H; y++, Y+=W, YY+=W)
    {
        // Keep track of the row sum
        u32 r = 0;

        for (register u32 x = 0; x < W; x++)
        {
            r += i_m[Y + x];
            ii_m[Y + x] = ii_m[YY + x] + r;
        }
    }
}

u32 BasicOpenCV::GetSum(IplImage *integral, int x, int y, int w, int h){
    u32 *data = (u32 *)integral->imageData;
    u32 width = integral->width;
    register u32 a = y*width + x;
    register u32 b = y*width + x + w;
    register u32 c = (y+h)*width + x;
    register u32 d = (y+h)*width + x + w;
    return data[d] - data[b] - data[c] + data[a];
}

u32 BasicOpenCV::GetSum(IplImage *integral, CvRect rect){
    u32 *data = (u32 *)integral->imageData;
    u32 w = integral->width;
    register u32 a = rect.y*w + rect.x;
    register u32 b = rect.y*w + rect.x + rect.width;
    register u32 c = (rect.y+rect.height)*w + rect.x;
    register u32 d = (rect.y+rect.height)*w + rect.x + rect.width;
    return data[d] - data[b] - data[c] + data[a];
}

float BasicOpenCV::MaximizeSquare(IplImage *image, int *x, int *y, int *s)
{
    bool bIsImageNew = false;
    if(image->nChannels != 1)
    {
        IplImage *tmp = cvCreateImage(cvGetSize(image), image->depth, 1);
        cvCvtColor(image, tmp, CV_BGR2GRAY);
        image = tmp;
        tmp = NULL;
        bIsImageNew = true;
    }

    IplImage *integral = NULL;
    integralImage(image, &integral);

    u32 *data = (u32 *)integral->imageData;
    u32 w = integral->width;
    u32 total = data[integral->height*integral->width-1];

    // aaaand we start the actual maximization
    float maxSum = FLT_MIN;
    float sum;

    u32 minSize = 10;
    u32 sizeLimit = (u32)(min(image->width, image->height)*0.9);
    u32 xstep = image->width > 100 ? image->width / 100 : 1;
    u32 ystep = image->height > 100 ? image->height / 100 : 1;
    u32 sstep = sizeLimit > 30 ? sizeLimit / 30 : 1;

    if(!total)
    {
        (*x) = (*y) = 0;
        (*s) = 1;
        if(bIsImageNew) IMKILL(image);
        IMKILL(integral);
        return 0;
    }

    register u32 yw, ysw, yws, ysws, a, b, c, d;
    for (register u32 cs=minSize; cs < sizeLimit; cs+=2)
    {
        float tsum = cs*cs*255.f;
        for (register u32 cy=0; cy<image->height-cs; cy+=ystep)
        {
            yw = cy*w;
            ysw = (cy+cs)*w;
            yws = yw+cs;
            ysws = ysw+cs;
            for (register u32 cx=0; cx<image->width-cs; cx+=xstep)
            {
                a = yw + cx;
                b = yws + cx;
                c = ysw + cx;
                d = ysws + cx;
                u32 csum = data[d] - data[b] - data[c] + data[a];
                //float sum = csum*ratio - (total - csum)*(1-ratio);
                if(total == 0) csum = 0;
                else
                {
                    sum = csum / tsum * csum / (float)total;
                }
                if(sum > maxSum)
                {
                    maxSum = sum;
                    (*x) = cx;
                    (*y) = cy;
                    (*s) = cs;
                }
            }
        }
    }
    if(bIsImageNew) IMKILL(image);
    IMKILL(integral);
    return maxSum;
}

void BasicOpenCV::cvCopyFlipped(IplImage *src, IplImage *dst)
{
    assert(src);
    assert(dst);
    assert(src->width == dst->width && src->height == dst->height);
    if(src->origin != IPL_ORIGIN_TL)
        cvFlip(src, dst);
    else
        cvCopy(src,dst);
    dst->origin = IPL_ORIGIN_TL;
}

void BasicOpenCV::DisplayHueSatHist(IplImage* src)
{
    if(!src) return;
    IplImage* h_plane = cvCreateImage( cvGetSize(src), 8, 1 );
    IplImage* s_plane = cvCreateImage( cvGetSize(src), 8, 1 );
    IplImage* v_plane = cvCreateImage( cvGetSize(src), 8, 1 );
    IplImage* planes[] = { h_plane, s_plane };
    IplImage* hsv = cvCreateImage( cvGetSize(src), 8, 3 );
    u32 h_bins = 30, s_bins = 32;
    s32 hist_size[] = {h_bins, s_bins};
    f32 h_ranges[] = { 0, 180 }; /* hue varies from 0 (~0red) to 180 (~360red again) */
    f32 s_ranges[] = { 0, 255 }; /* saturation varies from 0 (black-gray-white) to 255 (pure spectrum color) */
    f32* ranges[] = { h_ranges, s_ranges };
    s32 scale = 10;
    IplImage* hist_img = cvCreateImage( cvSize(h_bins*scale,s_bins*scale), 8, 3 );
    CvHistogram* hist;
    f32 max_value = 0;

    cvCvtColor( src, hsv, CV_BGR2HSV );
    cvCvtPixToPlane( hsv, h_plane, s_plane, v_plane, 0 );
    hist = cvCreateHist( 2, hist_size, CV_HIST_ARRAY, ranges, 1 );
    cvCalcHist( planes, hist, 0, 0 );
    cvGetMinMaxHistValue( hist, 0, &max_value, 0, 0 );
    cvZero( hist_img );

    FOR(h, h_bins){
        FOR(s, s_bins){
            f32 bin_val = cvQueryHistValue_2D( hist, h, s );
            s32 intensity = cvRound(bin_val*255/max_value);
            cvRectangle( hist_img, cvPoint( h*scale, s*scale ),
                         cvPoint( (h+1)*scale - 1, (s+1)*scale - 1),
                         CV_RGB(intensity,intensity,intensity), /* draw a grayscale histogram.
													   if you have idea how to do it
													   nicer let us know */
                         CV_FILLED );
        }
    }
    cvNamedWindow( "H-S Histogram", 1 );
    cvShowImage( "H-S Histogram", hist_img );
}

void BasicOpenCV::CreateHistogramImage(IplImage *src, IplImage *dst, int bins, int channels)
{
    if(!src || !dst) return;
    bool bRed, bGreen, bBlue, bGray;
    switch(channels)
    {
    default:
    case 0:
        bRed = bGreen = bBlue = bGray = true;
        break;
    case 1:
        bGreen = bBlue = bGray = false;
        bRed = true;
        break;
    case 2:
        bBlue = bRed = bGray = false;
        bGreen = true;
        break;
    case 3:
        bRed = bGreen = bGray = false;
        bBlue = true;
        break;
    }

    bool bGrayscale = src->nChannels == 1;

    int rh[256], gh[256], bh[256], kh[256];
    memset(rh, 0, 256*sizeof(int));
    memset(gh, 0, 256*sizeof(int));
    memset(bh, 0, 256*sizeof(int));
    memset(kh, 0, 256*sizeof(int));
    if(bGrayscale)
    {
        FOR(y, src->height)
        {
            FOR(x, src->width)
            {
                u32 index = y*src->widthStep + x;
                int pixel = (u8)src->imageData[index];
                kh[pixel]++;
            }
        }
    }
    else
    {
        FOR(y, src->height)
        {
            FOR(x, src->width)
            {
                u32 index = y*src->widthStep + x*3;
                int b = (u8)src->imageData[index+0];
                int g = (u8)src->imageData[index+1];
                int r = (u8)src->imageData[index+2];
                int k = (r+g+b)/3;
                rh[r]++;
                gh[g]++;
                bh[b]++;
                kh[k]++;
            }
        }
    }

    int step = 256/bins;
    int *brh, *bgh, *bbh, *bkh;
    brh = new int[bins];
    bgh = new int[bins];
    bbh = new int[bins];
    bkh = new int[bins];

    FOR(i, bins)
    {
        brh[i] = 0;
        bgh[i] = 0;
        bbh[i] = 0;
        bkh[i] = 0;
        FOR(c, step)
        {
            brh[i] += rh[i*step+c];
            bgh[i] += gh[i*step+c];
            bbh[i] += bh[i*step+c];
            bkh[i] += kh[i*step+c];
        }
    }

    int maxrgb = 0;
    FOR(i, bins)
    {
        maxrgb = max(maxrgb, max(brh[i],max(bgh[i],max(bbh[i],bkh[i]))));
    }

    cvSet(dst, CV_RGB(40,40,40));

    int w = dst->width;
    int h = dst->height;
    int total = src->width*src->height;
    if(!bGrayscale)
    {
        if(bRed)
        {
            for (int i=0; i < bins; i++)
            {
                // we compute the x and y coordinates for this color
                int x1 = (int)(i / (float)bins * w);
                int x2 = (int)((i+1) / (float)bins * w);
                int y1 = h;
                int y2 = h - (int)(brh[i] / (float)maxrgb  * h);
                cvRectangle(dst, cvPoint(x1,y1), cvPoint(x2,y2), CV_RGB(255,0,0),-1);
                cvLine(dst, cvPoint(x1,y2), cvPoint(x2,y2), CV_RGB(0,0,0));
            }
        }
        if(bGreen)
        {
            for (int i=0; i < bins; i++)
            {
                // we compute the x and y coordinates for this color
                int x1 = (int)(i / (float)bins * w);
                int x2 = (int)((i+1) / (float)bins * w);
                int y1 = h;
                int y2 = h - (int)(bgh[i] / (float)maxrgb * h);
                cvRectangle(dst, cvPoint(x1,y1), cvPoint(x2,y2), CV_RGB(0,255,0),-1);
                cvLine(dst, cvPoint(x1,y2), cvPoint(x2,y2), CV_RGB(0,0,0));
            }
        }
        if(bBlue)
        {
            for (int i=0; i < bins; i++)
            {
                // we compute the x and y coordinates for this color
                int x1 = (int)(i / (float)bins * w);
                int x2 = (int)((i+1) / (float)bins * w);
                int y1 = h;
                int y2 = h - (int)(bbh[i] / (float)maxrgb * h);
                cvRectangle(dst, cvPoint(x1,y1), cvPoint(x2,y2), CV_RGB(0,0,255),-1);
                cvLine(dst, cvPoint(x1,y2), cvPoint(x2,y2), CV_RGB(0,0,0));
            }
        }
    }
    if(bGrayscale || bGray)
    {
        for (int i=0; i < bins; i++)
        {
            // we compute the x and y coordinates for this color
            int x1 = (int)(i / (float)bins * w);
            int x2 = (int)((i+1) / (float)bins * w);
            int y1 = h;
            int y2 = h - (int)(bkh[i] / (float)maxrgb * h);
            //cvRectangle(dst, cvPoint(x1,y1), cvPoint(x2,y2), CV_RGB(0,0,0),-1);
            cvRectangle(dst, cvPoint(x1,y1), cvPoint(x2,y2), CV_RGB(255,255,255),-1);
            cvLine(dst, cvPoint(x1,y2), cvPoint(x2,y2), CV_RGB(0,0,0));
        }
    }

    KILL(brh);
    KILL(bgh);
    KILL(bbh);
    KILL(bkh);
}

void BasicOpenCV::RGB2NCC(IplImage *image, IplImage *red, IplImage *green)
{
    u32 w = image->width;
    u32 h = image->height;

    u32 intensity, rM, gM;
    u8 *pix = (u8 *)image->imageData;
    FOR(i, w*h){
        intensity = pix[i*3] + pix[i*3+1] + pix[i*3+2];
        if (intensity > 0){
            if (intensity < 60){
                if (pix[i*3] + pix[i*3+1] < 10 || pix[i*3] + pix[i*3+2] < 10){
                    rM = gM = 0;
                }
                else{
                    rM = pix[i*3+2] *255 / intensity;
                    gM = pix[i*3+1] *255 / intensity;
                }
            }
            else{
                rM = pix[i*3+2] *255 / intensity;
                gM = pix[i*3+1] *255 / intensity;
            }
        }
        else{
            rM = gM = 0;
        }
        red->imageData[i] = (u8)(rM);
        green->imageData[i] = (u8)(gM);
    }
}

void BasicOpenCV::BinaryMedian(IplImage *src, IplImage *dst)
{
    u32 w = src->width;
    u32 h = src->height;

    FOR(j, h){
        dst->imageData[j*w] = 0;
        dst->imageData[j*w + w-1] = 0;
    }

    FOR(j, w){
        dst->imageData[j] = 0;
        dst->imageData[(h-1)*w + j] = 0;
    }

    for (u32 j=1; j < h-1; j++)
    {
        for (u32 i=1; i < w-1; i++)
        {
            u8 c = 0;
            c =(((u8)src->imageData[(j-1)*w + i] >0) +
                ((u8)src->imageData[j*w + i-1]   >0) +
                ((u8)src->imageData[j*w + i]     >0) +
                ((u8)src->imageData[j*w + i+1]   >0) +
                ((u8)src->imageData[(j+1)*w + i] >0) );
            dst->imageData[j*w + i] = (c > 2)*255;
        }
    }
}

void BasicOpenCV::RemoveNoise(IplImage * src)
{
    //get the size of input_image (src)
    CvSize sz = cvSize( src->width & -2, src->height & -2 );

    //create  temp-image
    IplImage* pyr = cvCreateImage( cvSize(sz.width/2, sz.height/2), src->depth, src->nChannels );

    cvPyrDown( src, pyr, 7);	//pyr DOWN
    cvPyrUp( pyr, src, 7);		//and UP
    cvReleaseImage(&pyr);		//release temp
    pyr = NULL;
}

IplImage *BasicOpenCV::Deinterlace(IplImage *image)
{
    if(!image) return NULL;
    IplImage *fields = cvCreateImage(cvGetSize(image), 8, 3);
    fields->origin = image->origin;
    u32 height = image->height;
    u32 width = image->width;
    u32 step = image->widthStep;
    for(u32 i = 0; i < height; i += 2){
        FOR(j, width){
            // first field
            fields->imageData[(i>>1)*step + j*3] = (u8)image->imageData[i*step + j*3];
            fields->imageData[(i>>1)*step + j*3+1] = (u8)image->imageData[i*step + j*3+1];
            fields->imageData[(i>>1)*step + j*3+2] = (u8)image->imageData[i*step + j*3+2];

            // second field
            fields->imageData[(((i>>1)+(height>>1))*step + j*3  )] = (u8)image->imageData[(i+1)*step + j*3];
            fields->imageData[(((i>>1)+(height>>1))*step + j*3+1)] = (u8)image->imageData[(i+1)*step + j*3+1];
            fields->imageData[(((i>>1)+(height>>1))*step + j*3+2)] = (u8)image->imageData[(i+1)*step + j*3+2];
        }
    }
    return fields;
}

IplImage *BasicOpenCV::GetField(IplImage *image, u32 field)
{
    if(!image) return NULL;
    u32 height = image->height;
    u32 width = image->width;
    u32 step = image->widthStep;
    IplImage *fields = cvCreateImage(cvSize(width, height>>1), 8, 3);
    fields->origin = image->origin;
    for(u32 i = 0; i < height; i += 2){
        i += field ? 1 : 0;
        for(u32 j=0; j<width; j++){
            fields->imageData[(i>>1)*step + j*3  ] = (u8)image->imageData[i*step + j*3  ];
            fields->imageData[(i>>1)*step + j*3+1] = (u8)image->imageData[i*step + j*3+1];
            fields->imageData[(i>>1)*step + j*3+2] = (u8)image->imageData[i*step + j*3+2];
        }
    }
    return fields;
}

IplImage *BasicOpenCV::Half2Full(IplImage *image)
{
    IplImage *newImage = cvCreateImage(cvSize(image->width, image->height*2),image->depth, image->nChannels);
    newImage->origin = image->origin;
    u32 step = newImage->widthStep;
    u32 chan = newImage->nChannels;
    FOR(i, (u32)newImage->height){
        FOR(j, (u32)newImage->width){
            newImage->imageData[i*step +j*chan  ] = (u8)image->imageData[(i>>1)*step + j*chan  ];
            newImage->imageData[i*step +j*chan+1] = (u8)image->imageData[(i>>1)*step + j*chan+1];
            newImage->imageData[i*step +j*chan+2] = (u8)image->imageData[(i>>1)*step + j*chan+2];
        }
    }
    return newImage;
}

void BasicOpenCV::Half2Full(IplImage *src, IplImage *dst)
{
    dst->origin = src->origin;
    u32 step = dst->widthStep;
    u32 chan = dst->nChannels;
    u32 stepSrc = src->widthStep;
    FOR(i, (u32)dst->height){
        FOR(j, (u32)dst->width){
            dst->imageData[i*step +j*chan  ] = (u8)src->imageData[(i>>1)*stepSrc + j*chan  ];
            dst->imageData[i*step +j*chan+1] = (u8)src->imageData[(i>>1)*stepSrc + j*chan+1];
            dst->imageData[i*step +j*chan+2] = (u8)src->imageData[(i>>1)*stepSrc + j*chan+2];
        }
    }
}

IplImage *BasicOpenCV::Half2Demi(IplImage *image)
{
    IplImage *newImage = cvCreateImage(cvSize(image->width/2, image->height),image->depth, image->nChannels);
    newImage->origin = image->origin;
    u32 step = newImage->widthStep;
    u32 stepSrc = image->widthStep;
    u32 chan = newImage->nChannels;
    FOR(i, (u32)newImage->height){
        FOR(j, (u32)newImage->width){
            newImage->imageData[i*step +j*chan  ] = (u8)image->imageData[i*stepSrc + (j<<1)*chan  ];
            newImage->imageData[i*step +j*chan+1] = (u8)image->imageData[i*stepSrc + (j<<1)*chan+1];
            newImage->imageData[i*step +j*chan+2] = (u8)image->imageData[i*stepSrc + (j<<1)*chan+2];
        }
    }
    return newImage;
}

void BasicOpenCV::Half2Demi(IplImage *src, IplImage *dst)
{
    dst->origin = src->origin;
    u32 step = dst->widthStep;
    u32 stepSrc = src->widthStep;
    u32 chan = dst->nChannels;
    FOR(i, (u32)dst->height){
        FOR(j, (u32)dst->width){
            dst->imageData[i*step +j*chan  ] = (u8)src->imageData[i*stepSrc + (j<<1)*chan  ];
            dst->imageData[i*step +j*chan+1] = (u8)src->imageData[i*stepSrc + (j<<1)*chan+1];
            dst->imageData[i*step +j*chan+2] = (u8)src->imageData[i*stepSrc + (j<<1)*chan+2];
        }
    }
}

IplImage *BasicOpenCV::Half(IplImage *src)
{
    CvSize size = cvGetSize(src);
    size.width /= 2;
    size.height /= 2;
    IplImage *dst = cvCreateImage(size, src->depth, src->nChannels);
    dst->origin = src->origin;
    cvResize(src, dst, CV_INTER_CUBIC);
    return dst;
}

void BasicOpenCV::Half(IplImage **src)
{
    IplImage *img = (*src);
    CvSize size = cvGetSize(img);
    size.width /= 2;
    size.height /= 2;
    IplImage *dst = cvCreateImage(size, img->depth, img->nChannels);
    dst->origin = img->origin;
    cvResize(img, dst, CV_INTER_CUBIC);
    IMKILL(img);
    (*src) = dst;
}

IplImage *BasicOpenCV::Resize(IplImage *src, CvSize size)
{
    IplImage *dst = cvCreateImage(size, src->depth, src->nChannels);
    dst->origin = src->origin;
    cvResize(src, dst, CV_INTER_CUBIC);
    return dst;
}

void BasicOpenCV::Resize(IplImage **src,CvSize size)
{
    IplImage *img = (*src);
    if(!img){
        (*src) = cvCreateImage(size, 8, 1);
        return;
    }
    if(size.width == img->width && size.height == img->height) return;
    IplImage *dst = cvCreateImage(size,img->depth,img->nChannels);
    dst->origin = img->origin;
    cvResize(img,dst,CV_INTER_CUBIC);
    IMKILL(img);
    (*src) = dst;
}

// 1 = red, 2 = blue, 
void BasicOpenCV::ChannelSubtraction(IplImage *src , IplImage *dst, u32 first, u32 second)
{
    bool bYellow = first == 4 || second == 4;
    IplImage *channels[5];
    FOR(i,5)
    {
        channels[i] = cvCreateImage(cvGetSize(src), IPL_DEPTH_32F, 1);
    }
    cvSplit(src, channels[0], channels[1], channels[2], channels[3]);
    if(bYellow)
    {
        cvAddWeighted(channels[0],0.5,channels[1],0.5,0,channels[4]);
    }
    cvSub(channels[first], channels[second], dst);
    FOR(i,5)
    {
        cvReleaseImage(&channels[i]); channels[i] = NULL;
    }
}

void BasicOpenCV::Divide(IplImage *img1, IplImage *img2 )
{
    if(!img1 || !img2) return;
    cvDiv(img1,img2,img1);
}

IplImage *BasicOpenCV::Crop(IplImage *image,CvRect selection)
{
    if(!image) return NULL;
    IplImage *dest = cvCreateImage(cvSize(selection.width, selection.height),image->depth,image->nChannels);
    dest->origin = image->origin;
    ROI(image, selection);
    cvCopy(image, dest);
    unROI(image);
    return dest;
}

int BasicOpenCV::otsuThreshold(CvMat* prob, CvHistogram* hist)
{
    cvCalcHist((IplImage**)&prob, hist, 0, NULL);

    /*
	Instead of calling cvNormalizeHist(hist, 1.0), we can :
	in the first loop, compute the sum of pi and 
	in the second loop, normalize the pi before doing calculation.
	This save us some times.
	*/

    float w0 = 0;
    float w1 = 1;
    float mu0 = 0;
    float mu1 = 0;

    CvMat mat;
    cvGetMat(hist->bins, &mat, 0, 1);

    float sum = 0; /* used to normalized the histogram */

    float* ptrHist = mat.data.fl;
    int i;
    for (i=1; i<=256; i++) {
        mu1 = mu1 + i* *ptrHist;
        sum += *ptrHist;
        ptrHist++;
    }
    mu1 /= sum;

    int threshold = 0;
    float sigma;
    float sigmaMax = 0;
    ptrHist = mat.data.fl;
    float pi;
    for (i=1; i<=256; i++) {
        pi = *ptrHist;
        pi /= sum; /* for normalization */

        mu0 = mu0*w0;
        mu1 = mu1*w1;
        w0  = w0 + pi;
        w1  = w1 - pi;
        mu0 = (mu0 + (i* pi)) /w0;
        mu1 = (mu1 - (i* pi)) /w1;

        sigma = (w0*w1*(mu1-mu0)*(mu1-mu0));

        if (sigma > sigmaMax){
            threshold = i;
            sigmaMax = sigma;
        }
        ptrHist++;
    }

    return threshold;
}


IplImage *BasicOpenCV::BoxPlot(std::vector<float> data, float maxVal, float minVal)
{
    int res = 400;
    int hpad = 15;
    int pad = -10;
    IplImage *boxplot = cvCreateImage(cvSize(100,res+(-2*pad)), 8, 3);
    cvZero(boxplot);

    if(!data.size()) return boxplot;
    float mean = 0;
    float sigma = 0;
    FOR(i, data.size()) mean += data[i] / data.size();
    FOR(i, data.size()) sigma += powf(data[i]-mean,2);
    sigma = sqrtf(sigma/data.size());

    if(data.size() > 1 && maxVal == -FLT_MAX)
    {
        FOR(i, data.size()) maxVal = max(maxVal, data[i]);
    }
    if(data.size() > 1 && minVal == FLT_MAX)
    {
        FOR(i, data.size()) minVal = min(minVal, data[i]);
    }
    if(data.size() == 1)
    {
        if(minVal == FLT_MAX) minVal = data[0]/2;
        if(maxVal == -FLT_MAX) maxVal = data[0]*3/2;
    }

    float edge = minVal;
    float delta = maxVal - minVal;

    float top, bottom, median, quartLow, quartHi;
    vector<float> outliers;
    vector<float> sorted;

    if(data.size() > 1)
    {
        if(sigma==0)
        {
            sorted = data;
        }
        else
        {
            // we look for outliers using the 3*sigma rule
            FOR(i, data.size())
            {
                if (data[i] - mean < 3*sigma)
                    sorted.push_back(data[i]);
                else outliers.push_back(data[i]);
            }
        }
        if(!sorted.size()) return boxplot;
        sort(sorted.begin(), sorted.end());
        int count = sorted.size();
        int half = count/2;
        bottom = sorted[0];
        top = sorted[sorted.size()-1];

        median = count%2 ? sorted[half] : (sorted[half] + sorted[half - 1])/2;
        if(count < 4)
        {
            quartLow = bottom;
            quartHi = top;
        }
        else
        {
            quartLow = half%2 ? sorted[half/2] : (sorted[half/2] + sorted[half/2 - 1])/2;
            quartHi = half%2 ? sorted[half*3/2] : (sorted[half*3/2] + sorted[half*3/2 - 1])/2;
        }
    }
    else
    {
        top = bottom = median = quartLow = quartHi = data[0];
    }

    cvDrawLine(boxplot, cvPoint(hpad+35, boxplot->height - (int)((bottom-edge)/delta*res) + pad),	cvPoint(hpad+65, boxplot->height - (int)((bottom-edge)/delta*res) + pad), CV_RGB(255,255,255), 1, CV_AA);
    cvDrawLine(boxplot, cvPoint(hpad+35, boxplot->height - (int)((top-edge)/delta*res) + pad),		cvPoint(hpad+65, boxplot->height - (int)((top-edge)/delta*res) + pad), CV_RGB(255,255,255), 1, CV_AA);
    cvDrawLine(boxplot, cvPoint(hpad+50, boxplot->height - (int)((bottom-edge)/delta*res) + pad),	cvPoint(hpad+50, boxplot->height - (int)((top-edge)/delta*res) + pad), CV_RGB(255,255,255), 1, CV_AA);
    cvDrawRect(boxplot, cvPoint(hpad+30, boxplot->height - (int)((quartLow-edge)/delta*res) + pad),cvPoint(hpad+70, boxplot->height - (int)((quartHi-edge)/delta*res) + pad), CV_RGB(255,255,255), -1, CV_AA);
    cvDrawLine(boxplot, cvPoint(hpad+25, boxplot->height - (int)((median-edge)/delta*res) + pad),	cvPoint(hpad+75, boxplot->height - (int)((median-edge)/delta*res) + pad), CV_RGB(0,0,0), 1, CV_AA);

    FOR(i, outliers.size())
    {
        //cvDrawCircle(boxplot, cvPoint(hpad+50, boxplot->height - (outliers[i])/delta*res + pad), 2, CV_RGB(255,255,255), -1, CV_AA);
    }

    CvFont font;
    cvInitFont(&font, CV_FONT_HERSHEY_PLAIN, .6, .6, 0, 1, CV_AA);
    char text[255];
    sprintf(text, "%.3f", median);
    cvPutText(boxplot, text, cvPoint(hpad+30, boxplot->height - (int)((median-edge)/delta*res) + pad), &font, CV_RGB(0,0,0));
    sprintf(text, "%.3f", top);
    cvPutText(boxplot, text, cvPoint(3, max((int)(font.line_type*font.vscale),boxplot->height - (int)((top-edge)/delta*res) + pad + (int)(font.line_type*font.vscale/2))), &font, CV_RGB(255,255,255));
    sprintf(text, "%.3f", bottom);
    cvPutText(boxplot, text, cvPoint(3, min(boxplot->height, boxplot->height - (int)((bottom-edge)/delta*res) + pad + (int)(font.line_type*font.vscale/2))), &font, CV_RGB(255,255,255));
    /*
	sprintf(text, "%.3f", quartLow);
	cvPutText(boxplot, text, cvPoint(10, boxplot->height - (quartLow-edge)/delta*res + pad + (int)(font.line_type*font.vscale/2)), &font, CV_RGB(255,255,255));
	sprintf(text, "%.3f", quartHi);
	cvPutText(boxplot, text, cvPoint(10, boxplot->height - (quartHi-edge)/delta*res + pad + (int)(font.line_type*font.vscale/2)), &font, CV_RGB(255,255,255));
	*/
    return boxplot;
}

IplImage *BasicOpenCV::BoxPlot(std::vector<std::vector<float> > data, float maxVal, float minVal)
{
    IplImage *boxplot = 0;
    FOR(i, data.size())
    {
        IplImage *plot = BoxPlot(data[i], maxVal, minVal);
        if(!boxplot) boxplot = cvCreateImage(cvSize(plot->width*data.size(), plot->height), 8, 3);
        ROI(boxplot, cvRect(plot->width*i,0,plot->width, plot->height));
        cvCopy(plot, boxplot);
        unROI(boxplot);
        IMKILL(plot);
    }
    cvXorS(boxplot, CV_RGB(255,255,255), boxplot);
    return boxplot;
}

void cvDrawGradient( IplImage *image, CvRect rect, CvScalar color1, CvScalar color2, bool vertical )
{
    if(!image) return;
    CvScalar color;
    if(vertical)
    {
        FOR(y, rect.height)
        {
            float ratio = y / (float)rect.height;
            FOR(i, 4) color.val[i] = color1.val[i] + (color2.val[i] - color1.val[i])*ratio;
            FOR(x, rect.width)
            {
                cvSet2D(image, y+rect.y, x+rect.x, color);
            }
        }
    }
    else
    {
        FOR(x, rect.width)
        {
            float ratio = x / (float)rect.width;
            FOR(i, 4) color.val[i] = color1.val[i] + (color2.val[i] - color1.val[i])*ratio;
            FOR(y, rect.height)
            {
                cvSet2D(image, y+rect.y, x+rect.x, color);
            }
        }
    }
}
