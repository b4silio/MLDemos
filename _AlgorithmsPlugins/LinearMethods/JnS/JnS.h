// Beware: for Jade and Shibbs functions, the data matrix X passed to
// the ICA algorithms is *modified* : after the function has returned,
// the matrix B*X is equal to the original value of X.



void Jade (
	   double *B, /* Output.        Separating matrix. nbc*nbc */
	   double *X, /* Input/Output.  Data set nbc x nbs */
	   int nbc,   /* Input.         Number of sensors  */
	   int nbs    /* Input.         Number of samples  */
	   ) ;

void Shibbs (
	     double *B, /* Output.        Separating matrix. nbc*nbc */
	     double *X, /* Input/Output.  Data set nbc x nbs */
	     int nbc,   /* Input.         Number of sensors  */
	     int nbs    /* Input.         Number of samples  */
	     ) ;


// A is a stack of K M*M matrices.  The function returns an
// orthonormal M*M matrix which jointly diagonalizes `at best' this
// stack.  The function also updates the stack: upon return the stack
// is as diagonal as possible.

int JointDiago (
		double *A,  // Input/Output.     A stack of K matrices of size M*M
		double *R,  // Output.           The joint approximate diagonalizer 
		int M,      // Input.            Each matrix in the stack has size M*M
		int K,      // Input.            There are K such matrices in the `stack'
		double threshold // Input Accuracy 
		) ;






// Below: small functions called by the test programms; could be
// removed from the .h file in final versions.

void OutOfMemory() ;
void Transform (double *X, double *Trans, int n, int T)  ;
void Identity (double *Mat, int p) ;
