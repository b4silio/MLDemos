#!/bin/sh

appName='MLDemos'
myPath=$appName'.app/Contents/'
opencvPath='/Users/basilio/Code/OpenCV-2.2.0/build/lib/Release/'
#opencvPath='/usr/local/lib/'

names=( libopencv_core libopencv_legacy libopencv_highgui libopencv_ml libopencv_features2d libopencv_imgproc libopencv_calib3d libopencv_video libopencv_flann)
namesQT=( QtGui QtCore )
framework='@executable_path/../Frameworks/'
frameworkQT='.framework/Versions/4/'

#first we deploy
echo 'macdeployqt-4.6' $appName'.app'
eval 'macdeployqt-4.6' $appName'.app'

# we start with changing the links in the executable
for name in ${names[@]}
do
	echo 'install_name_tool -change '$opencvPath$name'.dylib' $framework$name'.dylib' $myPath'MacOS/'$appName
	eval 'install_name_tool -change '$opencvPath$name'.dylib' $framework$name'.dylib' $myPath'MacOS/'$appName
done;
echo '\n'

# then we copy the opencv library files
for name in ${names[@]}
do
	echo 'cp '$opencvPath$name'.dylib ' $myPath'/Frameworks/'
	eval 'cp '$opencvPath$name'.dylib ' $myPath'/Frameworks/'
done;
echo '\n'

# now we change the id on the libs we have copied inside the package
for name in ${names[@]}
do
	echo 'install_name_tool -id '$framework$name'.dylib' $myPath'Frameworks/'$name'.dylib'
	eval 'install_name_tool -id '$framework$name'.dylib' $myPath'Frameworks/'$name'.dylib'
done;
echo '\n'

# then we change the links in the libs we have copied inside the package
for name in ${names[@]}
do
	for name2 in ${names[@]}
	do
		echo 'install_name_tool -change '$opencvPath$name'.dylib' $framework$name'.dylib' $myPath'Frameworks/'$name2'.dylib'
		eval 'install_name_tool -change '$opencvPath$name'.dylib' $framework$name'.dylib' $myPath'Frameworks/'$name2'.dylib'
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
		echo 'install_name_tool -change '$opencvPath$name'.dylib' $framework$name'.dylib' $myPath'MacOS/'$filename
		eval 'install_name_tool -change '$opencvPath$name'.dylib' $framework$name'.dylib' $myPath'MacOS/'$filename
	done;
	for name in ${namesQT[@]}
	do
		echo 'install_name_tool -change '$name$frameworkQT$name $framework$name$frameworkQT$name $myPath'MacOS/'$filename
		eval 'install_name_tool -change '$name$frameworkQT$name $framework$name$frameworkQT$name $myPath'MacOS/'$filename
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

