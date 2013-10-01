#ifndef PLUGINRANDOMKERNEL_H
#define PLUGINRANDOMKERNEL_H

#include <vector>
#include <interfaces.h>

class PluginRandomKernel : public QObject, public CollectionInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "PluginRandomKernel" FILE "plugin.json")
    Q_INTERFACES(CollectionInterface)
public:
    PluginRandomKernel();

    QString GetName(){return "RandomKernel collection";}
};

#endif // PLUGINRANDOMKERNEL_H
