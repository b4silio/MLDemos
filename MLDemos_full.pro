##############################
#                            #
#       MLDemos Bundle       #
#                            #
##############################

TEMPLATE = subdirs
# the main software
CONFIG += ordered
SUBDIRS = Core 3rdParty MLDemos
Core.file = Core/Core.pro
3rdParty.file = _3rdParty/3rdParty.pro
MLDemos.file = MLDemos/MLDemos.pro
MLDemos.depends = Core

# algorithm plugins
ALGOPATH = _AlgorithmsPlugins
#SUBDIRS += Kernel
SUBDIRS += Obstacle GMM Kernel KNN Projections LWPR Maximizers OpenCV SEDS
# SUBDIRS += HMM
GMM.file = $$ALGOPATH/GMM/pluginGMM.pro
Kernel.file = $$ALGOPATH/KernelMethods/pluginKernel.pro
KNN.file = $$ALGOPATH/KNN/pluginKNN.pro
Projections.file = $$ALGOPATH/Projections/pluginProjections.pro
LWPR.file = $$ALGOPATH/LWPR/pluginLWPR.pro
Obstacle.file = $$ALGOPATH/Obstacle/pluginAvoidance.pro
SEDS.file = $$ALGOPATH/SEDS/pluginSEDS.pro
Maximizers.file = $$ALGOPATH/Maximizers/pluginMaximizers.pro
OpenCV.file = $$ALGOPATH/OpenCV/pluginOpenCV.pro
XMeans.file = $$ALGOPATH/XMeans/pluginXMeans.pro
HMM.file = $$ALGOPATH/HMM/pluginHMM.pro

# input plugins
INPUTPATH = _IOPlugins
SUBDIRS += PCAFaces WebImport
#SUBDIRS += ImportTimeseries
PCAFaces.file = $$INPUTPATH/PCAFaces/pluginPCAFaces.pro
RandomEmitter.file = $$INPUTPATH/RandomEmitter/pluginRandomEmitter.pro
WebImport.file = $$INPUTPATH/WebImport/pluginWebImport.pro
ImportTimeseries.file = $$INPUTPATH/ImportTimeseries/pluginImportTimeseries.pro
