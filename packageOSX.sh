#!/bin/sh

appName='MLDemos'
myPath=$appName'.app/Contents/'
opencvPath='/Users/basilio/Code/OpenCV-2.3.1a/build/lib/'
qtPath='/Users/basilio/QtSDK/Desktop/Qt/4.8.0/gcc/lib/'

names=( libopencv_core libopencv_legacy libopencv_highgui libopencv_ml libopencv_features2d libopencv_imgproc libopencv_calib3d libopencv_video libopencv_flann libopencv_contrib libopencv_gpu libopencv_objdetect)
namesQT=( QtGui QtCore QtNetwork QtOpenGL OpenGL AGL QtSvg QtXml QtWebKit phonon QtDBus QtXmlPatterns)
namesQTadd=( QtWebKit phonon QtDBus QtXmlPatterns)
framework='@executable_path/../Frameworks/'
frameworkQT='.framework/Versions/4/'

#first we deploy
echo 'macdeployqt' $appName'.app'
eval 'macdeployqt' $appName'.app'

# we start with changing the links in the executable
for name in ${names[@]}
do
	echo 'install_name_tool -change lib/'$name'.2.3.dylib' $framework$name'.2.3.dylib' $myPath'MacOS/'$appName
	eval 'install_name_tool -change lib/'$name'.2.3.dylib' $framework$name'.2.3.dylib' $myPath'MacOS/'$appName
#	echo 'install_name_tool -change '$opencvPath$name'.2.3.dylib' $framework$name'.2.3.dylib' $myPath'MacOS/'$appName
#	eval 'install_name_tool -change '$opencvPath$name'.2.3.dylib' $framework$name'.2.3.dylib' $myPath'MacOS/'$appName
done;
echo '\n'

# then we copy the opencv library files
for name in ${names[@]}
do
	echo 'cp '$opencvLibs$name'.2.3.dylib ' $myPath'/Frameworks/'
	eval 'cp '$opencvLibs$name'.2.3.dylib ' $myPath'/Frameworks/'
done;
echo '\n'

# now we change the id on the libs we have copied inside the package
for name in ${names[@]}
do
	echo 'install_name_tool -id '$framework$name'.2.3.dylib' $myPath'Frameworks/'$name'.2.3.dylib'
	eval 'install_name_tool -id '$framework$name'.2.3.dylib' $myPath'Frameworks/'$name'.2.3.dylib'
done;
echo '\n'

# then we change the links in the libs we have copied inside the package
for name in ${names[@]}
do
	for name2 in ${names[@]}
	do
		echo 'install_name_tool -change '$opencvPath$name'.2.3.dylib' $framework$name'.2.3.dylib' $myPath'Frameworks/'$name2'.2.3.dylib'
		eval 'install_name_tool -change '$opencvPath$name'.2.3.dylib' $framework$name'.2.3.dylib' $myPath'Frameworks/'$name2'.2.3.dylib'
	done;
	echo '\n'
done;

for name in ${namesQT[@]}
do
	for name2 in ${names[@]}
	do
		echo 'install_name_tool -change '$name$frameworkQT$name $framework$name$frameworkQT$name $myPath'Frameworks/'$name2'.2.3.dylib'
		eval 'install_name_tool -change '$name$frameworkQT$name $framework$name$frameworkQT$name $myPath'Frameworks/'$name2'.2.3.dylib'
	done;
	echo '\n'
done;


# copy frameworks from namesQTadd to the package
# will need to create the folders Resources and Versions/4/ first
for name in ${namesQTadd[@]}
do
		echo 'mkdir '$myPath'Frameworks/'$name'.framework'
		eval 'mkdir '$myPath'Frameworks/'$name'.framework'
		echo 'mkdir '$myPath'Frameworks/'$name'.framework/Resources'
		eval 'mkdir '$myPath'Frameworks/'$name'.framework/Resources'
		echo 'mkdir '$myPath'Frameworks/'$name'.framework/Versions'
		eval 'mkdir '$myPath'Frameworks/'$name'.framework/Versions'
		echo 'mkdir '$myPath'Frameworks/'$name'.framework/Versions/4'
		eval 'mkdir '$myPath'Frameworks/'$name'.framework/Versions/4'
		echo 'cp /Library/Frameworks/'$name$frameworkQT$name $myPath'Frameworks/'$name$frameworkQT$name
		eval 'cp /Library/Frameworks/'$name$frameworkQT$name $myPath'Frameworks/'$name$frameworkQT$name
done;

for name in ${namesQT[@]}
do
	for name2 in ${namesQTadd[@]}
	do
		echo 'install_name_tool -change '$name$frameworkQT$name $framework$name$frameworkQT$name $myPath'Frameworks/'$name2$frameworkQT$name2
		eval 'install_name_tool -change '$name$frameworkQT$name $framework$name$frameworkQT$name $myPath'Frameworks/'$name2$frameworkQT$name2
	done;
	echo '\n'
done;


# last we go through our plugins to change these guys as well
echo 'mkdir '$myPath'MacOS/plugins'
eval 'mkdir '$myPath'MacOS/plugins'
echo 'mkdir '$myPath'MacOS/help'
eval 'mkdir '$myPath'MacOS/help'
echo 'cp plugins/*.dylib '$myPath'MacOS/plugins/'
eval 'cp plugins/*.dylib '$myPath'MacOS/plugins/'
echo 'cp help/* '$myPath'MacOS/help/'
eval 'cp help/* '$myPath'MacOS/help/'

for filename in plugins/*.dylib
do
	# copy the file in the proper folder
	for name in ${names[@]}
	do
		echo 'install_name_tool -change lib/'$name'.2.3.dylib' $framework$name'.2.3.dylib' $myPath'MacOS/'$filename
		eval 'install_name_tool -change lib/'$name'.2.3.dylib' $framework$name'.2.3.dylib' $myPath'MacOS/'$filename
#		echo 'install_name_tool -change '$opencvPath$name'.2.3.dylib' $framework$name'.2.3.dylib' $myPath'MacOS/'$filename
#		eval 'install_name_tool -change '$opencvPath$name'.2.3.dylib' $framework$name'.2.3.dylib' $myPath'MacOS/'$filename
	done;
	for name in ${namesQT[@]}
	do
		echo 'install_name_tool -change '$qtPath$name$frameworkQT$name $framework$name$frameworkQT$name $myPath'MacOS/'$filename
		eval 'install_name_tool -change '$qtPath$name$frameworkQT$name $framework$name$frameworkQT$name $myPath'MacOS/'$filename
	done;
	echo '\n'
done;

#echo 'otool -L' $myPath'MacOS/'$appName
#eval 'otool -L' $myPath'MacOS/'$appName
#for filename in plugins/*.dylib
#do
#	echo 'otool -L' $myPath'Resources/'$filename
#	eval 'otool -L' $myPath'Resources/'$filename
#done;

