/*********************************************************************
MLDemos: A User-Friendly visualization toolkit for machine learning
Copyright (C) 2012 - Otpal Vittoz ( Otpal@otpal.org )
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

#include "animationlabel.h"

AnimationLabel::AnimationLabel( QString animationPath,
                                QWidget* parent        ) : QWidget( parent )
{
    init( animationPath, QSize() );
}

AnimationLabel::AnimationLabel( QString  animationPath,
                                QSize    size,
                                QWidget* parent         ) : QWidget( parent )
{
    init( animationPath, size );
}

AnimationLabel::~AnimationLabel()
{
    container->deleteLater();
    animation->deleteLater();
}

void AnimationLabel::init( const QString animationPath,
                           const QSize   size           )
{
    // We create the QMovie for the animation
    animation = new QMovie( animationPath );
	QSize s = size;

    if ( s.isEmpty() ) {
        animation->jumpToNextFrame();
        s = animation->currentPixmap().size();
	}
    animation->jumpToFrame( 0 );
    setVisible( true );
    container = new QLabel( this );
    // We'll set a fixed size to the QLabel because we don't want to be resized
    container->setFixedSize( s );
    container->setMovie( animation );

    QVBoxLayout* layout = new QVBoxLayout( this );
    // Remove the all the extra space add the movie to our layout and show it
    layout->setSpacing( 1 );
    layout->setMargin( 1 );
    layout->addWidget( container );
    setLayout( layout );
    setFixedSize( s );
}


void AnimationLabel::Start()
{
    // Check if the movie can be started.
    if ( ( !animation.isNull()                             ) &&
         ( ( animation->state() == QMovie::Paused     ) ||
           ( animation->state() == QMovie::NotRunning )    )    ) {
        // start the animation... and reveal ourself.
        animation->start();
        animation->jumpToFrame( 0 );
	}
    setVisible( true );
}


void AnimationLabel::Stop()
{
    // Check if the animation can be stopped.
    if ( ( !animation.isNull() )                   &&
         ( animation->state() == QMovie::Running )    ) {
        // It can so we'll stop the animation... and hide.
        animation->stop();
    }
    setVisible( false );
}
