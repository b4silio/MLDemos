# ##########################
# Configuration      #
# ##########################
TEMPLATE = lib
CONFIG += plugin
NAME = mld_CCA
MLPATH =../..

include($$MLPATH/MLDemos_variables.pri)
###########################
# Source Files            #
###########################
# add here all the files in your project

# the forms contain the widgets for changing the hyperparameters
FORMS += \
    paramsCCA.ui

HEADERS +=	\
        pluginCCA.h\
        interfaceCCAProjection.h\
        projectorCCA.h


SOURCES += 	\
    interfaceCCAProjection.cpp\
    projectorCCA.cpp \
    pluginCCA.cpp


###########################
# Dependencies            #
###########################
# add here the dependencies to third party libraries (e.g. the one in 3rdParty)
HEADERS += pluginProjection/interfaceProjection.h\
           Core/Projector.h


HEADERS +=  JnS/Matutil.h \
            JnS/JnS.h

HEADERS +=  liblinear/linear.h \
            liblinear/tron.h \
            liblinear/blasp.h \
            liblinear/blas.h

OTHER_FILES += \
    plugin.json

