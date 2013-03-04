MLDemos: A User-Friendly visualization toolkit for machine learning
Copyright (C) 2010  Basilio Noris
Contact: mldemos@b4silio.com


======================
Running the Binaries
======================

Windows / Mac OSX:
------------------
download and unzip the software package and run the executable

Linux:
------
apt-get install the MLDemos package (debian / ubuntu)
or
compile the software (see below)
or
install Wine (www.winehq.org) and run the windows version (works on several of the latest versions of Wine)


======================
Compiling the software
======================

The software requires the following libraries:

- Qt 4.8 or later
- OpenCV 2.3 or later
- Boost 1.47 or later

IDE
---
While not compulsory, it is advised to use QtCreator to manage the project (loading the MLDemos_full.pro project file).
QtCreator has the added benefit of solving upper/lower-case conflicts that sometime pop up within the codebase.
Alternatively, you can run qmake in the mldemos folder.

Windows
-------
Windows requires MinGW and g++ to compile the code.
To properly install this, you might have to:

1 - Download and install MinGW
2 - Download and install Qt 4.8 for MinGW. During install point to MinGW install folder
3 - Download and unzip Boost 1.47 (or later) on your system
4 - Download and install OpenCV 2.4 (ensure you have MinGW libraries under %opencvpath%/lib/x86/mingw)
	If mingw libraries are present and compatible with your install of MinGW from (1), skip to (5)
5.a) Download and install CMake and run CMake-gui (included in CMake)
5.b) Open the OpenCV folder as a project
5.c) Run Configure, set "Specify native compilers -> MinGW/g++" (you may have to find the path of g++.exe)
5.d) Uncheck unavailable items starting with WITH_* (likely Cuda, Eigen, OpenEXR, PVAPI) as well as BUILD_* (e.g. docs and python bindings)
5.e) Rerun Configure and then run Generate to create the makefiles
5.f) Open a command prompt / msys console and access the %opencvpath%/build folder
5.g) Run make, this will generate all the opencv libraries and executables
5.h) Add the executable and dll folder (%opencvpath%/build/bin) to the windows PATH environment variable
6) - Download and install and run QtCreator
7.a) Under tools/options look for Qt Versions, ensure that Qt 4.8 MinGW and MinGW/g++ were found correctly
7.b) If Qt was not found, add a manual Qt version and look for qmake.exe in the Qt folder
7.c) If g++ was not found, go to toolchains and add a manual toolchain, and set for MinGW32-Make.exe
8.a) Open MLDemos_full.pro within QtCreator
8.b) Configure the project for the proper version of Qt (4.8 MinGW)
8.c) Open the file MLDemos_Variables.pri within QtCreator (or any text editor)
8.d) Set the paths for OpenCV and Boost in the .pri file
9 - Run Build All and go grab a coffee, it's going to take some time

Mac OSX
-------
While Mac OSX requires the development tools to be installed (XCode), the compilation is done via qmake/Qt Creator rather than via XCode.
To compile the code you will have to:

0 - Install the development tools suite (XCode) from the install dvd / the internet
1 - Download and install Qt 4.8
2 - Download and install OpenCV
	If the OpenCV install provides a full framework, skip to (4)
3.a) Download and install CMake, and run CMake.app (included in CMake)
3.b) Open the OpenCV folder within CMake
3.c) Run Configure and set Use default native compiler - Unix Makefiles)
3.d) Uncheck unavailable items starting with WITH_* (likely Cuda, Eigen, OpenEXR, PVAPI) as well as BUILD_* (e.g. docs and python bindings)
3.e) Rerun Configure and then run Generate to create the Makefiles
3.f) Open a console and access the %opencvpath%/build folder
3.g) Run "make" to compile, and then "sudo make install" to move the libraries and binaries in the path
4 - Download and unpack the Boost library somewhere (e.g. /usr/local/boost)
5 - Download and install and run QtCreator
6.a) Under tools/preferences look for Qt Version, ensure that Qt 4.8 was found correctly
6.b) If not found, add a manual Qt Version and browse for the qmake executable
7.a) Open MLDemos_full.pro within QtCreator
7.b) Configure the project for the proper version of Qt
8 - Run Build All and go grab a coffee, it's going to take some time

Linux
-----
1 - apt-get install Qt
2 - apt-get install OpenCV (libcv)
3 - apt-get install Boost (libboost)
4 - apt-get install QtCreator
5.a) Open MLDemos_full.pro within QtCreator
5.b) Configure the project for the proper version of Qt
6) Run Build All and sip from the coffee you already have at your side

