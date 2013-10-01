#ifndef RANDOMKERNELUTILS_H
#define RANDOMKERNELUTILS_H

#include<liblinear//linear.h>
#include<types.h>

#define Malloc(type,n) (type *)malloc((n)*sizeof(type))

/*random feature type*/
enum { RANDOM_FOURIER, RANDOM_NYSTROM };
/*random kernel type: currently only RBF is supported*/
enum { RAND_KERNEL_RBF };

int RandFourierMap(
        int nKernelType,                        //I: kernel type 0 - RBF
        const fvec&                x,           //I: original feature
        const std::vector< fvec >& W,           //I: random weights
        const fvec&                b,           //I: random offsets
        fvec&                      result       //O: mapped feature
        );

int RandFourierFactorize(
        int nKernelType,                        //I: kernel type 0 - RBF
        int nKernelRank,                        //I: kernel rank - higher rank generates more samples to approximate the kernel
        float fGamma,                           //I: kernel width
        const std::vector< fvec >& X,           //I: original features
        std::vector< fvec >& G,                 //O: mapped features
        std::vector< fvec >& W,                 //O: random weights
        fvec& b                                 //O: random offsets
        );

/*predict function provided by liblinear cannot handle float values, define our own here...*/
float svm_predict(const model *model_, const feature_node *x);
float svm_predict_values(const struct model *model_, const struct feature_node *x, double *dec_values);
#endif // RANDOMKERNELUTILS_H
