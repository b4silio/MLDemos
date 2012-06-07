#ifndef PLUGINFLAME_PLUGIN_H
#define PLUGINFLAME_PLUGIN_H

#include <QtDeclarative/QDeclarativeExtensionPlugin>

class PluginFLAMEPlugin : public QDeclarativeExtensionPlugin
{
    Q_OBJECT

public:
    void registerTypes(const char *uri);
};

#endif // PLUGINFLAME_PLUGIN_H

