#ifndef OPENCVINCLUDES_H
#define OPENCVINCLUDES_H

#include <types.h>
#include <public.h>

#include <vector>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/features2d/features2d.hpp>
#include <opencv2/video/tracking.hpp>
#include <opencv2/ml/ml.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <QImage>

static QImage const Mat2QImage(cv::Mat& mat)
{
    QImage img(mat.cols, mat.rows, QImage::Format_RGB32);
    cv::Mat image;
    if (mat.channels() == 3) { image = mat; }
    else { cv::cvtColor(mat, image, CV_GRAY2BGR); }
    FOR(i, img.height()) {
        FOR(j, img.width()) {
            cv::Vec3b c = image.at< cv::Vec3b >(i,j);
            img.setPixel(j, i, qRgb(c[2],c[1],c[0]));
        }
    }
    return img;
}

static QImage const Mat2QImage(IplImage* image)
{
    cv::Mat mat = cv::cvarrToMat(image);
    return Mat2QImage(mat);
}

static QPixmap toPixmap(IplImage* src){

    QPixmap pixmap;
    if (src->nChannels == 4) {
        pixmap = QPixmap::fromImage(QImage((const unsigned char*)src->imageData,src->width, src->height, QImage::Format_RGB32)).copy();
    } else {
        IplImage* image = cvCreateImage(cvGetSize(src),8,4);
        cvCvtColor(src, image, src->nChannels==1 ? CV_GRAY2BGRA : CV_BGR2BGRA);
        QImage qimg = QImage((const unsigned char*)image->imageData, image->width, image->height, QImage::Format_RGB32);
        pixmap = QPixmap::fromImage(qimg).copy();
        cvReleaseImage(&image);
    }
    return pixmap;
}

/**
   Functions to convert between OpenCV's cv::Mat and Qt's QImage and QPixmap.

   Andy Maloney
   https://asmaloney.com/2013/11/code/converting-between-cvmat-and-qimage-or-qpixmap
**/

// NOTE: This does not cover all cases - it should be easy to add new ones as required.
  inline QImage  cvMatToQImage( const cv::Mat &inMat )
  {
     switch ( inMat.type() )
     {
        // 8-bit, 4 channel
        case CV_8UC4:
        {
           QImage image( inMat.data,
                         inMat.cols, inMat.rows,
                         static_cast<int>(inMat.step),
                         QImage::Format_ARGB32 );

           return image;
        }

        // 8-bit, 3 channel
        case CV_8UC3:
        {
           QImage image( inMat.data,
                         inMat.cols, inMat.rows,
                         static_cast<int>(inMat.step),
                         QImage::Format_RGB888 );

           return image.rgbSwapped();
        }

        // 8-bit, 1 channel
        case CV_8UC1:
        {
           static QVector<QRgb>  sColorTable( 256 );

           // only create our color table the first time
           if ( sColorTable.isEmpty() )
           {
              for ( int i = 0; i < 256; ++i )
              {
                 sColorTable[i] = qRgb( i, i, i );
              }
           }

           QImage image( inMat.data,
                         inMat.cols, inMat.rows,
                         static_cast<int>(inMat.step),
                         QImage::Format_Indexed8 );

           image.setColorTable( sColorTable );

           return image;
        }

        default:
           //qWarning() << "ASM::cvMatToQImage() - cv::Mat image type not handled in switch:" << inMat.type();
           break;
     }

     return QImage();
  }

  inline QPixmap cvMatToQPixmap( const cv::Mat &inMat )
  {
     return QPixmap::fromImage( cvMatToQImage( inMat ) );
  }

  // If inImage exists for the lifetime of the resulting cv::Mat, pass false to inCloneImageData to share inImage's
  // data with the cv::Mat directly
  //    NOTE: Format_RGB888 is an exception since we need to use a local QImage and thus must clone the data regardless
  //    NOTE: This does not cover all cases - it should be easy to add new ones as required.
  inline cv::Mat QImageToCvMat( const QImage &inImage, bool inCloneImageData = true )
  {
     switch ( inImage.format() )
     {
        // 8-bit, 4 channel
        case QImage::Format_ARGB32:
        case QImage::Format_ARGB32_Premultiplied:
        {
           cv::Mat  mat( inImage.height(), inImage.width(),
                         CV_8UC4,
                         const_cast<uchar*>(inImage.bits()),
                         static_cast<size_t>(inImage.bytesPerLine())
                         );

           return (inCloneImageData ? mat.clone() : mat);
        }

        // 8-bit, 3 channel
        case QImage::Format_RGB32:
        case QImage::Format_RGB888:
        {
           if ( !inCloneImageData )
           {
              //qWarning() << "ASM::QImageToCvMat() - Conversion requires cloning because we use a temporary QImage";
           }

           QImage   swapped;

           if ( inImage.format() == QImage::Format_RGB32 ) swapped = inImage.convertToFormat( QImage::Format_RGB888 );
           else swapped = inImage.rgbSwapped();

           return cv::Mat( swapped.height(), swapped.width(),
                           CV_8UC3,
                           const_cast<uchar*>(swapped.bits()),
                           static_cast<size_t>(swapped.bytesPerLine())
                           ).clone();
        }

        // 8-bit, 1 channel
        case QImage::Format_Indexed8:
        {
           cv::Mat  mat( inImage.height(), inImage.width(),
                         CV_8UC1,
                         const_cast<uchar*>(inImage.bits()),
                         static_cast<size_t>(inImage.bytesPerLine())
                         );

           return (inCloneImageData ? mat.clone() : mat);
        }

        default:
           //qWarning() << "ASM::QImageToCvMat() - QImage format not handled in switch:" << inImage.format();
           break;
     }

     return cv::Mat();
  }

  // If inPixmap exists for the lifetime of the resulting cv::Mat, pass false to inCloneImageData to share inPixmap's data
  // with the cv::Mat directly
  //    NOTE: Format_RGB888 is an exception since we need to use a local QImage and thus must clone the data regardless
  inline cv::Mat QPixmapToCvMat( const QPixmap &inPixmap, bool inCloneImageData = true )
  {
     return QImageToCvMat( inPixmap.toImage(), inCloneImageData );
  }

  void SimplifyContour ( const std::vector<cv::Point>& contourIn, std::vector<cv::Point>& contourOut, float tolerance )
  {
      //-- copy points.

      int numOfPoints;
      numOfPoints = contourIn.size();

      CvPoint* cvpoints;
      cvpoints = new CvPoint[ numOfPoints ];

      for( int i=0; i<numOfPoints; i++)
      {
          int j = i % numOfPoints;

          cvpoints[ i ].x = contourIn[ j ].x;
          cvpoints[ i ].y = contourIn[ j ].y;
      }

      //-- create contour.

      CvContour	contour;
      CvSeqBlock	contour_block;

      cvMakeSeqHeaderForArray
      (
          CV_SEQ_POLYLINE,
          sizeof(CvContour),
          sizeof(CvPoint),
          cvpoints,
          numOfPoints,
          (CvSeq*)&contour,
          &contour_block
      );

      printf( "length = %f \n", cvArcLength( &contour ) );

      //-- simplify contour.

      CvMemStorage* storage;
      storage = cvCreateMemStorage( 1000 );

      CvSeq *result = 0;
      result = cvApproxPoly
      (
          &contour,
          sizeof( CvContour ),
          storage,
          CV_POLY_APPROX_DP,
          cvContourPerimeter( &contour ) * tolerance,
          0
      );

      //-- contour out points.

      contourOut.clear();
      for( int j=0; j<result->total; j++ )
      {
          CvPoint * pt = (CvPoint*)cvGetSeqElem( result, j );

          contourOut.push_back( cv::Point() );
          contourOut.back().x = (float)pt->x;
          contourOut.back().y = (float)pt->y;
      }

      //-- clean up.

      if( storage != NULL )
          cvReleaseMemStorage( &storage );

      delete[] cvpoints;
  }
#endif // OPENCVINCLUDES_H
