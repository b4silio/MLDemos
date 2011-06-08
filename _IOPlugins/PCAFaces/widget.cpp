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
#include <QtGui>
#include <QWidget>
#include <QSize>
#include <QPixmap>
#include <QImage>
#include <QTime>
#include <QTimer>
#include <QPushButton>
#include <QGridLayout>
  
#include "basicOpenCV.h"
#include <math.h>

#include "widget.h"


QPixmap QNamedWindow::toPixmap(IplImage *src)
{
	QPixmap pixmap;
	if(!src) return pixmap;
	if(src->nChannels == 4)
	{
		pixmap = QPixmap::fromImage(QImage((const unsigned char *)src->imageData,src->width, src->height, QImage::Format_RGB32)).copy();
	}
	else
	{
		IplImage *image = cvCreateImage(cvGetSize(src),8,4);
		cvCvtColor(src, image, src->nChannels==1 ? CV_GRAY2BGRA : CV_BGR2BGRA);
		QImage qimg = QImage((const unsigned char *)image->imageData, image->width, image->height, QImage::Format_RGB32);
		pixmap = QPixmap::fromImage(qimg).copy();
		cvReleaseImage(&image);
	}
	return pixmap;
}

IplImage *QNamedWindow::toImage( QImage image )
{
	if (image.isNull()) return NULL;
	const int w = image.width();
	const int h = image.height();
	IplImage *img = cvCreateImage(cvSize(w, h), 8, 3);
	uchar *pixels = image.bits();
	for(int i=0; i<h; i++)
	{
		for(int j=0; j<w; j++)
		{
			for (int c=0; c<3; c++)
				img->imageData[i*img->widthStep + j*3 + c] = pixels[i*w*3 + j*h*3 + c];
		}
	}
	return img;
}

QNamedWindow::QNamedWindow(QString name, bool bResizable, QWidget *parent)
: QWidget(parent), name(name)
{
	qRegisterMetaType<QImage>("QImage");

	setWindowTitle(name);
	setMouseTracking(true);
	setAcceptDrops(true);
	//setCursor(Qt::CrossCursor);
	this->bResizable = bResizable;
	if(!bResizable) setFixedSize(256,256);
	else if(parent) resize(parent->width(), parent->height());
	else resize(256,256);
	bBorderless = false;
	bNewImage = true;
	mouseCallback = NULL;
	show();
}

int counter = 0;
bool bRedrawing = false;
void QNamedWindow::ShowImage(IplImage *image)
{
	if(!image) return;
	bRedrawing = true;
	pixmap = toPixmap(image);
	if(!bResizable) setFixedSize(pixmap.width(), pixmap.height());
	else if (bNewImage && !isFullScreen()) resize(pixmap.width(), pixmap.height());
	bNewImage = false;
	bRedrawing = false;
	repaint();
	//update();
}

void QNamedWindow::ShowImage(QPixmap pixmap)
{
	if(pixmap.isNull()) return;
	bRedrawing = true;
	this->pixmap = pixmap;
	if(!bResizable) setFixedSize(pixmap.width(), pixmap.height());
	else if (bNewImage && !isFullScreen()) resize(pixmap.width(), pixmap.height());
	if(parent()) resize(((QWidget*)parent())->width(), ((QWidget*)parent())->height());
	bNewImage = false;
	bRedrawing = false;
	repaint();
	//update();
}

void QNamedWindow::paintEvent(QPaintEvent *)
{
	QPainter painter(this);
	painter.setBackgroundMode(Qt::OpaqueMode);
	painter.setBackground(Qt::black);
	if (pixmap.isNull()) {
		painter.setPen(Qt::white);
		//painter.drawText(rect(), Qt::AlignCenter, tr("No Image to Display..."));
		return;
	}

	if(parent() && (width() != ((QWidget*)parent())->width() || height() != ((QWidget*)parent())->height()))
	{
		resize(((QWidget*)parent())->width(), ((QWidget*)parent())->height());
	}
	painter.fillRect(QRect(0,0,width(),height()), Qt::black);
	if(!bRedrawing && width() && height() && !pixmap.isNull() && pixmap.width() && pixmap.height()) painter.drawPixmap(QRect(0,0,width(), height()), pixmap);
}

void QNamedWindow::resizeEvent(QResizeEvent *event)
{
	emit ResizeEvent(event);
	//if(!bResizable) resize(pixmap.width(), pixmap.height());
}

void QNamedWindow::mousePressEvent(QMouseEvent *event)
{
	int x = event->x(), y = event->y();
	int flags = 0, events = 0;
	if(event->buttons() == Qt::LeftButton) flags |= CV_EVENT_FLAG_LBUTTON;
	if(event->buttons() == Qt::RightButton) flags |= CV_EVENT_FLAG_RBUTTON;
	if(event->button() == Qt::LeftButton) events = CV_EVENT_LBUTTONDOWN;
	else if(event->button() == Qt::RightButton) events = CV_EVENT_RBUTTONDOWN;
	if(mouseCallback) mouseCallback(events, x, y, flags);
	emit MousePressEvent(event);
}

void QNamedWindow::mouseReleaseEvent(QMouseEvent *event)
{
	int x = event->x(), y = event->y();
	int flags = 0, events = 0;
	if(event->buttons() == Qt::LeftButton) flags |= CV_EVENT_FLAG_LBUTTON;
	if(event->buttons() == Qt::RightButton) flags |= CV_EVENT_FLAG_RBUTTON;
	if(event->button() == Qt::LeftButton) events = CV_EVENT_LBUTTONUP;
	else if(event->button() == Qt::RightButton) events = CV_EVENT_RBUTTONUP;
	if(mouseCallback) mouseCallback(events, x, y, flags);
	emit MouseReleaseEvent(event);
}

void QNamedWindow::mouseMoveEvent(QMouseEvent *event)
{
	int x = event->x(), y = event->y();
	int flags = 0, events = 0;
	if(event->buttons() == Qt::LeftButton) flags |= CV_EVENT_FLAG_LBUTTON;
	if(event->buttons() == Qt::RightButton) flags |= CV_EVENT_FLAG_RBUTTON;
	if(mouseCallback) mouseCallback(events, x, y, flags);
	emit MouseMoveEvent(event);
}

void QNamedWindow::dragEnterEvent(QDragEnterEvent *event)
{
	emit DragEnterEvent(event);
}

void QNamedWindow::dropEvent(QDropEvent *event)
{
	emit DropEvent(event);
}

void QNamedWindow::SetMouseCallback( void (*callbackFunction)(int,int,int,int) )
{
	if(callbackFunction) mouseCallback = callbackFunction;
}
