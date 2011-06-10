##############################
#                            #
#       MLDemos Bundle       #
#                            #
##############################

TEMPLATE = subdirs
# the main software
SUBDIRS = MLDemos
MLDemos.file = MLDemos/MLDemos.pro

# algorithm plugins
ALGOPATH = _AlgorithmsPlugins
SUBDIRS += Obstacle GMM KernelMethods KNN KPCA LinearMethods LWPR Maximizers SEDS OpenCV

#SUBDIRS += Obstacle GMM KernelMethods KNN KPCA LinearMethods LWPR Maximizers OpenCV SEDS
GMM.file = $$ALGOPATH/GMM/pluginGMM.pro
KernelMethods.file = $$ALGOPATH/KernelMethods/pluginKernel.pro
KNN.file = $$ALGOPATH/KNN/pluginKNN.pro
KPCA.file = $$ALGOPATH/KPCA/pluginKPCA.pro
LinearMethods.file = $$ALGOPATH/LinearMethods/pluginLinear.pro
LWPR.file = $$ALGOPATH/LWPR/pluginLWPR.pro
Obstacle.file = $$ALGOPATH/Obstacle/pluginAvoidance.pro
SEDS.file = $$ALGOPATH/SEDS/pluginSEDS.pro
Maximizers.file = $$ALGOPATH/Maximizers/pluginMaximizers.pro
OpenCV.file = $$ALGOPATH/OpenCV/pluginOpenCV.pro

# input plugins
INPUTPATH = _IOPlugins
#SUBDIRS += PCAFaces
PCAFaces.file = $$INPUTPATH/PCAFaces/pluginPCAFaces.pro
RandomEmitter.file = $$INPUTPATH/RandomEmitter/pluginRandomEmitter.pro

