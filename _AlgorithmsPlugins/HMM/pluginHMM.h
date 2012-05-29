#ifndef PLUGINHMM_H
#define PLUGINHMM_H

#include <vector>
#include <interfaces.h>

class PluginHMM : public QObject, public CollectionInterface
{
    Q_OBJECT
    Q_INTERFACES(CollectionInterface)
public:
    PluginHMM();

    QString GetName(){return "Hidden Markov Models Collection";};
};

#endif // PLUGINHMM_H
