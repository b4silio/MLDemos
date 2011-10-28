/*********************************************************************
MLDemos: A User-Friendly visualization toolkit for machine learning
Copyright (C) 2010  Basilio Noris
Contact: mldemos@b4silio.com

Evolution-Strategy Mixture of Logisitics Regression
Copyright (C) 2011  Stephane Magnenat
Contact: stephane at magnenat dot net

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

#include "pluginMLR.h"
#include "interfaceESMLRClassifier.h"

PluginMLR::PluginMLR()
{
	//classifiers.push_back(new ClassRRMLR());
	classifiers.push_back(new ClassESMLR());
}

#ifndef PLUGIN_CLUSTER
#ifndef PLUGIN_CLASSIFY
Q_EXPORT_PLUGIN2(mld_MLR, PluginMLR)
#endif
#endif
