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
#include "pluginKernel.h"
#include "interfaceMVM.h"
#include "interfaceSVMClassifier.h"
#include "interfaceSVMCluster.h"
#include "interfaceSVMRegress.h"
#include "interfaceSVMDynamic.h"
#include "interfaceRVMClassifier.h"
#include "interfaceRVMRegress.h"
#include "interfaceKMCluster.h"
#include "interfaceMRVMClassifier.h"

using namespace std;

PluginKernel::PluginKernel()
{
    classifiers.push_back(new ClassSVM());
    classifiers.push_back(new ClassRVM());
    clusterers.push_back(new ClustKM());
    clusterers.push_back(new ClustSVM());
    regressors.push_back(new RegrSVM());
    regressors.push_back(new RegrRVM());
    dynamicals.push_back(new DynamicSVM());
    //classifiers.push_back(new ClassMVM());
    //classifiers.push_back(new ClassMRVM());
}

#ifndef PLUGIN_CLUSTER
#ifndef PLUGIN_CLASSIFY
//Q_EXPORT_PLUGIN2(mld_KernelMethods, PluginKernel)
#endif
#endif
