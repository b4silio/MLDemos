//#include <config.h>
//Seems to be the only thing required from config.h here
#define RETURN_IF_NULL(x) if (!x) { return ; }
#include <gsl/gsl_errno.h>
#include <gsl/gsl_vector.h>

/* Compile all the inline matrix functions */

#define COMPILE_INLINE_STATIC
#include "../build.h"
#include <gsl/gsl_matrix.h>

