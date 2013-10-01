#include "pluginRandomKernel.h"
#include "interfaceRSVMClassifier.h"
#include "interfaceRGPRegressor.h"

using namespace std;
PluginRandomKernel::PluginRandomKernel()
{
    classifiers.push_back(new ClassRSVM());
    regressors.push_back(new RegrRGPR());
}

//Q_EXPORT_PLUGIN2(mld_RandomKernel, PluginRandomKernel)
