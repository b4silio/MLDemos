/*
 * Copyright (C) 2010 Learning Algorithms and Systems Laboratory, EPFL, Switzerland
 * Author: Eric Sauser
 * email:   eric.sauser@a3.epf.ch
 * website: lasa.epfl.ch
 *
 * Permission is granted to copy, distribute, and/or modify this program
 * under the terms of the GNU General Public License, version 2 or any
 * later version published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 * Public License for more details
 */

#ifndef MATHLIBCOMMON_H_
#define MATHLIBCOMMON_H_

#ifdef WIN32
#include "windows.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <iostream>

#define USE_MATHLIB_NAMESPACE

#define MATHLIB_USE_DOUBLE_AS_REAL

#ifdef MATHLIB_USE_DOUBLE_AS_REAL
  typedef double REALTYPE;
#else
  typedef float REALTYPE;
#endif

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

#define PRINT_WIDTH     11
#define PRINT_PRECISION  6
#define PRINT_BEGIN(out)    streamsize p=out.precision(PRINT_PRECISION);        \
                            streamsize w=out.width();                           \
                            ios_base::fmtflags fmtfl = out.setf( ios::fixed );
#define PRINT_END(out)      out.flags(fmtfl);  \
                            out.precision(p);  \
                            out.width(w);




#endif /*MATHLIBCOMMON_H_*/
