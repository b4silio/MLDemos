##############################
#                            #
#       MLDemos Bundle       #
#                            #
##############################

TEMPLATE = subdirs
# the main software
CONFIG += ordered
SUBDIRS = MLDemos 3rdParty
MLDemos.file = MLDemos/MLDemos.pro
3rdParty.file = _3rdParty/3rdParty.pro

# algorithm plugins
ALGOPATH = _AlgorithmsPlugins
#SUBDIRS += Projections
SUBDIRS += Obstacle GMM KernelMethods KNN Projections LWPR Maximizers OpenCV SEDS XMeans ESMLR
GMM.file = $$ALGOPATH/GMM/pluginGMM.pro
KernelMethods.file = $$ALGOPATH/KernelMethods/pluginKernel.pro
KNN.file = $$ALGOPATH/KNN/pluginKNN.pro
Projections.file = $$ALGOPATH/Projections/pluginProjections.pro
LWPR.file = $$ALGOPATH/LWPR/pluginLWPR.pro
Obstacle.file = $$ALGOPATH/Obstacle/pluginAvoidance.pro
SEDS.file = $$ALGOPATH/SEDS/pluginSEDS.pro
Maximizers.file = $$ALGOPATH/Maximizers/pluginMaximizers.pro
OpenCV.file = $$ALGOPATH/OpenCV/pluginOpenCV.pro
ESMLR.file = $$ALGOPATH/ESMLR/pluginESMLR.pro
XMeans.file = $$ALGOPATH/XMeans/pluginXMeans.pro

# input plugins
INPUTPATH = _IOPlugins
SUBDIRS += PCAFaces RandomEmitter
SUBDIRS += WebImport
PCAFaces.file = $$INPUTPATH/PCAFaces/pluginPCAFaces.pro
RandomEmitter.file = $$INPUTPATH/RandomEmitter/pluginRandomEmitter.pro
WebImport.file = $$INPUTPATH/WebImport/pluginWebImport.pro
