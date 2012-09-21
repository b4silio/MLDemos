/*********************************************************************
MLDemos: A User-Friendly visualization toolkit for machine learning
Copyright (C) 2012 - Otpal Vittoz ( Otpal@otpal.org )
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

#ifndef _ANIMATIONLABEL_H_
#define _ANIMATIONLABEL_H_

#include <QVBoxLayout>
#include <QLabel>
#include <QMovie>

/**
 * AnimationLabel
 *
 * Uses animation from the path to display it in a QLabel.
 * In this case we implement a spinning weel.
 */
class AnimationLabel : public QWidget
{
	Q_OBJECT

public:
    AnimationLabel( QString  animationPath,
                    QWidget* parent         );
    AnimationLabel( QString  animationPath,
                    QSize    size,
                    QWidget* parent         );

    virtual ~AnimationLabel();

public slots:
    void Start();
    void Stop();

private:
    QPointer<QLabel> container;
    QPointer<QMovie> animation;

    void init( const QString animationPath,
               const QSize size             );
};

#endif // _ANIMATIONLABEL_H_
