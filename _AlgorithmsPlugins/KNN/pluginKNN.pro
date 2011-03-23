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
			$$MLDEMOS/canvas.h \
			$$MLDEMOS/datasetManager.h \
			$$MLDEMOS/mymaths.h \
			classifierKNN.h \
			regressorKNN.h \
			dynamicalKNN.h \
			interfaceKNNClassifier.h \
			interfaceKNNRegress.h \
			interfaceKNNDynamic.h \
			pluginKNN.h

SOURCES += 	\
			$$MLDEMOS/canvas.cpp \
			$$MLDEMOS/datasetManager.cpp \
			$$MLDEMOS/mymaths.cpp \
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

SOURCES += ANN/ANN.cpp \
	ANN/bd_fix_rad_search.cpp \
	ANN/bd_pr_search.cpp \
	ANN/bd_search.cpp \
	ANN/bd_tree.cpp \
	ANN/brute.cpp \
	ANN/kd_dump.cpp \
	ANN/kd_fix_rad_search.cpp \
	ANN/kd_pr_search.cpp \
	ANN/kd_search.cpp \
	ANN/kd_split.cpp \
	ANN/kd_tree.cpp \
	ANN/kd_util.cpp \
	ANN/perf.cpp
