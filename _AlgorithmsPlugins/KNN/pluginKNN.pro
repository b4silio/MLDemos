# ##########################
# Configuration      #
# ##########################
TEMPLATE = lib
CONFIG += plugin
NAME = mld_KNN
MLPATH =../..

include($$MLPATH/MLDemos_variables.pri)
###########################
# Source Files            #
###########################
FORMS += paramsKNN.ui paramsKNNDynamic.ui paramsKNNRegress.ui
HEADERS +=	\
			canvas.h \
			datasetManager.h \
			mymaths.h \
			classifierKNN.h \
			regressorKNN.h \
			dynamicalKNN.h \
			interfaceKNNClassifier.h \
			interfaceKNNRegress.h \
			interfaceKNNDynamic.h \
			pluginKNN.h

SOURCES += 	\
			classifierKNN.cpp \
			regressorKNN.cpp \
			dynamicalKNN.cpp \
			interfaceKNNClassifier.cpp \
			interfaceKNNRegress.cpp \
			interfaceKNNDynamic.cpp \
			pluginKNN.cpp

###########################
# Dependencies            #
###########################
HEADERS += ANN/ANN.h \
	ANN/ANNperf.h \
	ANN/ANNx.h \
	ANN/bd_tree.h \
	ANN/kd_fix_rad_search.h \
	ANN/kd_pr_search.h \
	ANN/kd_search.h \
	ANN/kd_split.h \
	ANN/kd_tree.h \
	ANN/kd_util.h \
	ANN/pr_queue.h \
	ANN/pr_queue_k.h

OTHER_FILES += \
    plugin.json
