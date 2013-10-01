#include<ctime>
#include<sstream>
#include<dlib/rand.h>
#include "randomKernelUtils.h"

#define PI          3.141592658

float svm_predict(const model *model_, const feature_node *x)
{
    double *dec_values = Malloc(double, model_->nr_class);
    float label=svm_predict_values(model_, x, dec_values);
    free(dec_values);
    return label;
}

float svm_predict_values(const struct model *model_, const struct feature_node *x, double *dec_values)
{
    int idx;
    int n;
    if(model_->bias>=0)
        n=model_->nr_feature+1;
    else
        n=model_->nr_feature;
    double *w=model_->w;
    int nr_class=model_->nr_class;
    int i;
    int nr_w;
    if(nr_class==2 && model_->param.solver_type != MCSVM_CS)
        nr_w = 1;
    else
        nr_w = nr_class;

    const feature_node *lx=x;
    //printf("nr_w: %d\n", nr_w);

    for(i=0;i<nr_w;i++)
        dec_values[i] = 0;

    for(; (idx=lx->index)!=-1; lx++)
    {
        // the dimension of testing data may exceed that of training
        if(idx<=n)
            for(i=0;i<nr_w;i++)
            {
                //printf("idx: %d, i: %d\n", idx, i);
                dec_values[i] += w[(idx-1)*nr_w+i]*lx->value;
            }
    }
    //printf("dec_values[0]: %lf, dec_values[1]: %lf, dec_values[2]: %lf\n", dec_values[0], dec_values[1], dec_values[2]);


    if(nr_class==2)
    {
        //printf("dec_values: %f, %f\n", dec_values[0], dec_values[1]);
        //printf("label0: %d, label1: %d\n", model_->label[0], model_->label[1]);
        //return (dec_values[0]>0)?1:-1;
        return dec_values[0];
    }
    else
    {
        int dec_max_idx = 0;
        for(i=1;i<nr_class;i++)
        {
            if(dec_values[i] > dec_values[dec_max_idx])
                dec_max_idx = i;
        }
        return model_->label[dec_max_idx];
    }

}

int RandFourierMap(int nKernelType, const fvec& x, const std::vector<fvec> &W, const fvec &b, fvec &result)
{
    int nErrCode = 0;
    if(W.empty() || b.empty())
    {
        return 1;
    }
    int nKernelRank = W[0].size();
    switch(nKernelType)
    {
    case RAND_KERNEL_RBF:
        for(int rank_idx = 0; rank_idx < nKernelRank; ++rank_idx)
        {
            float sum = 0.0;
            for(int dim_idx = 0; dim_idx < x.size(); ++dim_idx)
            {
                sum += W[dim_idx][rank_idx] * x[dim_idx];
            }
            result.push_back(sqrt(2.0 / nKernelRank) * cos(sum + b[rank_idx]));
        }
        break;
    default:
        nErrCode = 1;
    }

    return nErrCode;
}

int RandFourierFactorize(int nKernelType, int nKernelRank, float fGamma, const std::vector<fvec>& X, std::vector<fvec> &G, std::vector<fvec> &W, fvec &b)
{
    int nErrCode = 0;

    int m = X.size();
    if(m == 0)
    {
        return nErrCode;
    }

    int dim = X[0].size();
    G.clear();
    W.clear();
    b.clear();

    //generate random features

    dlib::rand r;
    std::ostringstream seed;
    seed << (unsigned int) time(0);
    r.set_seed(seed.str());

    switch(nKernelType)
    {
    case RAND_KERNEL_RBF:

        for(int ind = 0; ind < dim; ++ind)
        {
            fvec tmp_vec;
            for(int rank_idx = 0; rank_idx < nKernelRank; ++rank_idx)
            {
                tmp_vec.push_back(r.get_random_gaussian() * fGamma);
            }
            W.push_back(tmp_vec);
        }
        for(int ind = 0; ind < nKernelRank; ++ind)
        {
            b.push_back(r.get_random_float() * 2.0 * PI);
        }
        for(int ind = 0; ind < m; ++ind)
        {
            fvec tmp_vec;
            RandFourierMap(nKernelType, X[ind], W, b, tmp_vec);
            G.push_back(tmp_vec);
        }
        break;

    default:
        //unknown kernel type
        //printf("unknown type\n");
        nErrCode = 1;
        break;
    };

    return nErrCode;
}
