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
#ifndef QNamedWindow_H
#define QNamedWindow_H

typedef struct _IplImage IplImage;

#include <QWidget>
//#include <QGLWidget>
#include <QSize>

class QNamedWindow : public QWidget
{
	Q_OBJECT
	
private:
	QString name;
	QTimer *timer;
	QPixmap pixmap;
	QImage qimg;
	QPoint mouse;
	bool bResizable;
	bool bBorderless;
	

protected:
	void paintEvent(QPaintEvent *event);
	void resizeEvent(QResizeEvent *event);
	void mousePressEvent(QMouseEvent *event);
	void mouseMoveEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);
	void dragEnterEvent(QDragEnterEvent *event);
	void dropEvent(QDropEvent *event);

public:
	bool bNewImage;
	QNamedWindow(QString name, bool bResizable = false, QWidget *parent = 0);
	void ShowImage(IplImage *image);
	void ShowImage(QPixmap pixmap);
	void SetMouseCallback(void (*mouseCallback)(int, int, int, int));
	void (*mouseCallback)(int, int, int, int);
	QPixmap Pixmap() {return pixmap;};
	static QPixmap toPixmap(IplImage *src);
	static IplImage *toImage(QImage image);


    // modifed from http://opencv-extension-library.googlecode.com/svn/trunk/QtOpenCV/qcv/qcvimage.cpp
    static IplImage *cvxCopyQImage(const QImage &qImage); // added for debug, solved From Clipboard bug 08.05.2014

    static QImage IplImage2QImage(const IplImage *iplImage); // added for debug remove when no longer required
    static IplImage* qImage2IplImage(QImage &qImage); // added for debug  remove wen no longer required



signals:
	void MouseMoveEvent(QMouseEvent *event);
	void MousePressEvent(QMouseEvent *event);
	void MouseReleaseEvent(QMouseEvent *event);
	void DragEnterEvent(QDragEnterEvent *event);
	void DropEvent(QDropEvent *event);
	void ResizeEvent(QResizeEvent *event);
};
#endif
