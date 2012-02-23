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
#include "pluginProjections.h"
#include "interfaceProjections.h"
#include "interfaceICAProjection.h"
#include "interfacePCAProjection.h"
#include "interfaceLDAProjection.h"
#include "interfaceKPCAProjection.h"

using namespace std;

PluginProjections::PluginProjections()
{
    //classifiers.push_back(new ClassProjections());
    projectors.push_back(new ICAProjection());
    projectors.push_back(new PCAProjection());
    projectors.push_back(new LDAProjection());
    projectors.push_back(new KPCAProjection());
}

Q_EXPORT_PLUGIN2(mld_Projections, PluginProjections)
