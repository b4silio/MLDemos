macdeployqt-4.6 MLDemos.app

install_name_tool -change /Users/basilio/Code/OpenCV-2.1.0/build/lib/Release/libcxcore.dylib @executable_path/../Frameworks/libcxcore.dylib MLDemos.app/Contents/MacOS/MLDemos
install_name_tool -change /Users/basilio/Code/OpenCV-2.1.0/build/lib/Release/libcv.dylib @executable_path/../Frameworks/libcv.dylib MLDemos.app/Contents/MacOS/MLDemos
install_name_tool -change /Users/basilio/Code/OpenCV-2.1.0/build/lib/Release/libcvaux.dylib @executable_path/../Frameworks/libcvaux.dylib MLDemos.app/Contents/MacOS/MLDemos
install_name_tool -change /Users/basilio/Code/OpenCV-2.1.0/build/lib/Release/libhighgui.dylib @executable_path/../Frameworks/libhighgui.dylib MLDemos.app/Contents/MacOS/MLDemos
install_name_tool -change /Users/basilio/Code/OpenCV-2.1.0/build/lib/Release/libml.dylib @executable_path/../Frameworks/libml.dylib MLDemos.app/Contents/MacOS/MLDemos

install_name_tool -id @executable_path/../Frameworks/libcv.dylib MLDemos.app/Contents/Frameworks/libcv.dylib
install_name_tool -id @executable_path/../Frameworks/libcxcore.dylib MLDemos.app/Contents/Frameworks/libcxcore.dylib
install_name_tool -id @executable_path/../Frameworks/libcvaux.dylib MLDemos.app/Contents/Frameworks/libcvaux.dylib
install_name_tool -id @executable_path/../Frameworks/libhighgui.dylib MLDemos.app/Contents/Frameworks/libhighgui.dylib
install_name_tool -id @executable_path/../Frameworks/libml.dylib MLDemos.app/Contents/Frameworks/libml.dylib

install_name_tool -change /Users/basilio/Code/OpenCV-2.1.0/build/lib/Release/libcxcore.dylib @executable_path/../Frameworks/libcxcore.dylib MLDemos.app/Contents/Frameworks/libcv.dylib
install_name_tool -change /Users/basilio/Code/OpenCV-2.1.0/build/lib/Release/libcxcore.dylib @executable_path/../Frameworks/libcxcore.dylib MLDemos.app/Contents/Frameworks/libcvaux.dylib
install_name_tool -change /Users/basilio/Code/OpenCV-2.1.0/build/lib/Release/libcxcore.dylib @executable_path/../Frameworks/libcxcore.dylib MLDemos.app/Contents/Frameworks/libhighgui.dylib
install_name_tool -change /Users/basilio/Code/OpenCV-2.1.0/build/lib/Release/libcxcore.dylib @executable_path/../Frameworks/libcxcore.dylib MLDemos.app/Contents/Frameworks/libml.dylib
install_name_tool -change /Users/basilio/Code/OpenCV-2.1.0/build/lib/Release/libcv.dylib @executable_path/../Frameworks/libcv.dylib MLDemos.app/Contents/Frameworks/libhighgui.dylib
install_name_tool -change /Users/basilio/Code/OpenCV-2.1.0/build/lib/Release/libcv.dylib @executable_path/../Frameworks/libcv.dylib MLDemos.app/Contents/Frameworks/libcvaux.dylib
install_name_tool -change /Users/basilio/Code/OpenCV-2.1.0/build/lib/Release/libml.dylib @executable_path/../Frameworks/libml.dylib MLDemos.app/Contents/Frameworks/libcvaux.dylib
install_name_tool -change /Users/basilio/Code/OpenCV-2.1.0/build/lib/Release/libhighgui.dylib @executable_path/../Frameworks/libhighgui.dylib MLDemos.app/Contents/Frameworks/libcvaux.dylib

