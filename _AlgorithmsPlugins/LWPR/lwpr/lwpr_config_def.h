/** Set HAVE_LIBEXPAT to 1 if you have the EXPAT library installed, and you need
   support for reading LWPR models from XML files. On Linux/UNIX, the configure
   script will take care of this automatically. On Windows, you need to set this
   variable yourself, and you also need to make sure that the compiler finds 
   EXPAT's library and include files.
   \ingroup LWPR_C
*/   
#define HAVE_LIBEXPAT   0

/** Using this directive you can enable multi-threading at compile time. Set this
   to the desired number of threads. The number of cores in your machine is a good
   starting point, but how much speed improvement you get really depends on the machine,
   its configuration, and the learning task at hand.
   \ingroup LWPR_C
*/   
#define NUM_THREADS     1
