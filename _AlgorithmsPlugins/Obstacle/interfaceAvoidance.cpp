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
#include "interfaceAvoidance.h"
#include <QPixmap>
#include <QBitmap>
#include <QPainter>

using namespace std;

PluginAvoid::PluginAvoid()
	: widget(new QWidget())
{
}

void PluginAvoid::SetParams(ObstacleAvoidance *avoid)
{
	if(!avoid) return;
}

ObstacleAvoidance *PluginAvoid::GetObstacleAvoidance()
{
	ObstacleAvoidance *avoid = new DSAvoid();
	SetParams(avoid);
	return avoid;
}

void PluginAvoid::SaveOptions(QSettings &settings)
{
}

bool PluginAvoid::LoadOptions(QSettings &settings)
{
	return true;
}

void PluginAvoid::SaveParams(std::ofstream &file)
{
}

bool PluginAvoid::LoadParams(char *line, float value)
{
	return true;
}

Q_EXPORT_PLUGIN2(mld_DSAvoid, PluginAvoid)
