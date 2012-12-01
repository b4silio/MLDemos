#include "asvm.h"

using namespace std;

asvm::asvm(const asvm& o)
{
    numAlpha = o.numAlpha;
    numBeta = o.numBeta;
    dim = o.dim;
    b0 = o.b0;
    lambda = o.lambda;
    strcpy(type,o.type);

    if(o.alpha)
    {
        alpha = new double[numAlpha];
        memcpy(alpha, o.alpha, numAlpha*sizeof(double));
    }else alpha = 0;
    if(o.beta)
    {
        beta = new double[numBeta];
        memcpy(beta, o.beta, numBeta*sizeof(double));
    }else beta = 0;
    if(o.gamma)
    {
        gamma = new double[dim];
        memcpy(gamma, o.gamma, dim*sizeof(double));
    }else gamma = 0;
    if(o.y)
    {
        y = new int[numAlpha];
    }else y = 0;
    if(o.target)
    {
        target = new double[dim];
        memcpy(target, o.target, dim*sizeof(double));
    }else target = 0;
    if(o.temp)
    {
        temp = new double[dim];
        memcpy(temp, o.temp, dim*sizeof(double));
    }else temp = 0;
    if(o.temp1)
    {
        temp1 = new double[dim];
        memcpy(temp1, o.temp1, dim*sizeof(double));
    }else temp1 = 0;
    if(o.temp2)
    {
        temp2 = new double[dim];
        memcpy(temp2, o.temp1, dim*sizeof(double));
    }else temp2 = 0;
    if(o.svalpha)
    {
        svalpha = new double*[numAlpha];
        for(int i=0; i<numAlpha; i++)
        {
            svalpha[i] = new double[dim];
            memcpy(svalpha[i], o.svalpha[i], dim*sizeof(double));
        }
    }else svalpha = 0;
    if(o.svbeta)
    {
        svbeta = new double*[numBeta];
        for(int i=0; i<numBeta; i++)
        {
            svbeta[i] = new double[dim];
            memcpy(svbeta[i], o.svbeta[i], dim*sizeof(double));
        }
    }else svbeta = 0;
    if(o.tempMat)
    {
        tempMat = new double*[dim];
        for(int i=0; i<dim; i++)
        {
            tempMat[i] = new double[dim];
            memcpy(tempMat[i], o.tempMat[i], dim*sizeof(double));
        }
    }else tempMat = 0;
}

asvm::~asvm()
{
    KILL(alpha);
    KILL(beta);
    KILL(gamma);
    KILL(y);
    KILL(target);
    KILL(temp);
    KILL(temp1);
    KILL(temp2);

    if(svalpha)
    {
        for(int i=0; i<numAlpha; i++) delete [] svalpha[i];
        KILL(svalpha);
    }
    if(svbeta)
    {
        for(int i=0; i<numBeta; i++) delete [] svbeta[i];
        KILL(svbeta);
    }
    if(tempMat)
    {
        for(int i=0; i<dim; i++) delete [] tempMat[i];
        KILL(tempMat);
    }
}

asvm& asvm::operator=(const asvm& o)
{
    if(&o == this) return *this;
    KILL(alpha);
    KILL(beta);
    KILL(gamma);
    KILL(y);
    KILL(target);
    KILL(temp);
    KILL(temp1);
    KILL(temp2);

    if(svalpha)
    {
        for(int i=0; i<numAlpha; i++) delete [] svalpha[i];
        KILL(svalpha);
    }
    if(svbeta)
    {
        for(int i=0; i<numBeta; i++) delete [] svbeta[i];
        KILL(svbeta);
    }
    if(tempMat)
    {
        for(int i=0; i<dim; i++) delete [] tempMat[i];
        KILL(tempMat);
    }

    numAlpha = o.numAlpha;
    numBeta = o.numBeta;
    dim = o.dim;
    b0 = o.b0;
    lambda = o.lambda;
    strcpy(type,o.type);

    if(o.alpha)
    {
        alpha = new double[numAlpha];
        memcpy(alpha, o.alpha, numAlpha*sizeof(double));
    }
    if(o.beta)
    {
        beta = new double[numBeta];
        memcpy(beta, o.beta, numBeta*sizeof(double));
    }
    if(o.gamma)
    {
        gamma = new double[dim];
        memcpy(gamma, o.gamma, dim*sizeof(double));
    }
    if(o.y)
    {
        y = new int[numAlpha];
    }
    if(o.target)
    {
        target = new double[dim];
        memcpy(target, o.target, dim*sizeof(double));
    }
    if(o.temp)
    {
        temp = new double[dim];
        memcpy(temp, o.temp, dim*sizeof(double));
    }
    if(o.temp1)
    {
        temp1 = new double[dim];
        memcpy(temp1, o.temp1, dim*sizeof(double));
    }
    if(o.temp2)
    {
        temp2 = new double[dim];
        memcpy(temp2, o.temp1, dim*sizeof(double));
    }
    if(o.svalpha)
    {
        svalpha = new double*[numAlpha];
        for(int i=0; i<numAlpha; i++)
        {
            svalpha[i] = new double[dim];
            memcpy(svalpha[i], o.svalpha[i], dim*sizeof(double));
        }
    }
    if(o.svbeta)
    {
        svbeta = new double*[numBeta];
        for(int i=0; i<numBeta; i++)
        {
            svbeta[i] = new double[dim];
            memcpy(svbeta[i], o.svbeta[i], dim*sizeof(double));
        }
    }
    if(tempMat)
    {
        tempMat = new double*[dim];
        for(int i=0; i<dim; i++)
        {
            tempMat[i] = new double[dim];
            memcpy(tempMat[i], o.tempMat[i], dim*sizeof(double));
        }
    }
    return *this;
}

void asvm::setSVMFromOptimizedSol(double *x, asvmdata* dat)
{
    cout<<"Parsing result..."<<endl;
    unsigned int tclass = dat->target_class;
    int n = dat->num_alpha ;
    int c = dat->num_beta ;

    dim = dat->dim;

    temp = new double[dim];
    temp1 = new double[dim];
    temp2 = new double[dim];

    tempMat = new double*[dim];
    for(unsigned int i=0;i<dim;i++)
        tempMat[i] = new double[dim];

    //calculating alpha and beta tolerance
    double temp = 0.0;
    for(int i =0; i<n; i++)
        if(x[i]>temp)
            temp = x[i];
    double alpha_tol = temp*1e-8;

    temp = x[n];
    for(int i = n; i<n+c; i++)
        if(x[i]>temp)
            temp = x[i];
    double beta_tol = temp*1e-8;

    //filtering alpha and beta from the processed data

    deque<int> alphasv_index;
    deque<int> betasv_index;
    deque<double> alpha_array;
    deque<double> beta_array;

    for(int i =0; i<n; i++)
        if(x[i]>alpha_tol)
        {
            alphasv_index.push_back(i);
            alpha_array.push_back(x[i]);
        }

    for(int i =n; i<c+n; i++)
        if(x[i]>beta_tol)
        {
            betasv_index.push_back(i-n);
            beta_array.push_back(x[i]);
        }

    numAlpha = alphasv_index.size();
    numBeta = betasv_index.size();

    cout<<"Found "<<numAlpha<<" Alpha and "<<numBeta<<" Beta"<<endl;

    if(numAlpha > 0)
    {
        svalpha = new double*[numAlpha];
        y = new int[numAlpha];
        for(unsigned int i =0; i<numAlpha; i++)
            svalpha[i] = new double[dim];
    }

    if(numBeta > 0)
    {
        svbeta = new double*[numBeta];
        for(unsigned int i =0; i<numBeta; i++)
            svbeta[i] = new double[dim*2];
    }

    if(numAlpha > 0)
    {
        KILL(alpha);
        alpha = new double[numAlpha];
        for(unsigned int i=0;i<numAlpha;i++)
            alpha[i] = alpha_array[i];
    }
    if(numBeta > 0)
    {
        KILL(beta);
        beta = new double[numBeta];
        for(unsigned int i=0;i<numBeta;i++)
            beta[i] = beta_array[i];
    }
    KILL(gamma);
    KILL(target);
    gamma = new double[dim];
    target = new double[dim];

    for(unsigned int i =0; i<dim; i++)
        gamma[i] = x[n+c+i];

    for(unsigned int i =0; i< dim; i++)
        target[i] = dat->tar[tclass].targ[i];

    int count_alpha=0;
    int count_beta=0;
    int index_alpha=0;
    int index_beta=0;
    for(unsigned int i=0;i<dat->tar.size();i++)
        for(unsigned int j=0;j<dat->tar[i].traj.size();j++)
            for(unsigned int k=0;k<dat->tar[i].traj[j].nPoints-1;k++)
            {


                if(numAlpha > 0 && index_alpha < alphasv_index.size())
                {
                    if(alphasv_index[index_alpha] == count_alpha++)
                    {
                        for(unsigned int l=0;l<dim;l++)
                            svalpha[index_alpha][l] = dat->tar[i].traj[j].coords[k][l];

                        y[index_alpha] = dat->tar[i].traj[j].y[k];

                        index_alpha++;
                    }

                }
                if(i == tclass)
                {
                    if(numBeta > 0 && index_beta < betasv_index.size())
                    {
                        if(betasv_index[index_beta] == count_beta++)
                        {
                            for(unsigned int l=0;l<dim;l++)
                                svbeta[index_beta][l] = dat->tar[i].traj[j].coords[k][l];
                            for(unsigned int l=dim;l<2*dim;l++)
                                svbeta[index_beta][l] = dat->tar[i].traj[j].vel[k][l-dim];

                            index_beta++;
                        }
                    }
                }

            }

    b0 = 0;
    lambda = dat->lambda;
    strcpy(type, dat->type);

    calcb0();
}

void asvm::calcb0()
{
    if(numAlpha > 0)
    {
        double tmpval = 0.0;
        for(unsigned int i =0; i<numAlpha; i++)
        {
            if(y[i]== -1)
                tmpval += -1.0-getclassifiervalue(svalpha[i]);
            else
                tmpval += 1.0-getclassifiervalue(svalpha[i]);
        }

        b0 = tmpval/numAlpha;
    }
    else
        b0 = 0.0;
}

double asvm::getclassifiervalue(double *point)
{
    double val=b0;
    for(unsigned int i =0; i<dim; i++)
    {
        temp[i] = 0;
        temp1[i] = 0;
        temp2[i] = 0;
    }

    for(unsigned int i=0;i<numAlpha;i++)
    {
        for(unsigned int j=0; j<dim; j++)
            temp1[j] = svalpha[i][j];
        val += y[i]*alpha[i]*getkernel(point,temp1, lambda, type, dim);
    }

    for(unsigned int i=0;i<numBeta;i++)
    {
        for(unsigned int j=0; j<dim; j++)
        {
            temp1[j] = svbeta[i][j];		//coordinate
            temp2[j] = svbeta[i][dim+j];	//velocity
        }

        getfirstkernelderivative(point, temp1, lambda, type, 2, temp, dim);
        val += beta[i]*arraydot(temp, temp2, dim);
    }

    for(unsigned int i =0; i<dim;i++)
        temp[i] = 0;

    getfirstkernelderivative(point, target, lambda, type, 2, temp, dim);
    for(unsigned int i=0;i<dim;i++)
    {
        val -= gamma[i]*temp[i];
    }
    return val;
}

void asvm::getclassifierderivative(double *point, double* derivative)
{
    unsigned int i,j;

    for(i =0; i<dim; i++)
    {
        temp[i] = 0;
        temp1[i] = 0;
        temp2[i] = 0;
        derivative[i] = 0;
    }

    for( i=0;i<numAlpha;i++)
    {
        for( j=0; j<dim; j++)
            temp1[j] = svalpha[i][j];
        getfirstkernelderivative(point, temp1, lambda, type, 1, temp2, dim);

        for( j=0; j<dim; j++)
            derivative[j] += y[i]*alpha[i]*temp2[j];
    }

    for(i=0;i<numBeta;i++)
    {
        for( j=0; j<dim; j++)
        {
            temp1[j] = svbeta[i][j];		//coordinate
            temp2[j] = svbeta[i][dim+j];	//velocity
        }

        getsecondkernelderivative(point, temp1, dim, lambda, type, tempMat);
        MatrixVectorMultipy(tempMat, temp2, temp, dim, dim);
        for(j=0;j<dim;j++)
            derivative[j] += beta[i]*temp[j];
    }

    getsecondkernelderivative(point, target, dim, lambda, type, tempMat);
    for(j=0;j<dim;j++)
        for(i=0;i<dim;i++)
            derivative[j] -= gamma[i]*tempMat[j][i];

}

void asvm::printinfo()
{
    cout<<endl<<"************************** Learned SVM **************************"<<endl
       <<"Dimension: "<<dim<<endl
      <<"Type: "<<type<<endl
     <<"No. of alpha: "<<numAlpha<<endl
    <<"No. of beta: "<<numBeta<<endl
    <<"b0: "<<b0<<endl;

    cout<<"Target: "<<endl;
    for(unsigned int i=0;i<dim;i++)
        cout<<target[i]<<"  ";
    cout<<endl;

    cout<<"Alpha.*Y: "<<endl;
    for(unsigned int i=0;i<numAlpha;i++)
        cout<<alpha[i]*y[i]<<"  ";
    cout<<endl;
    cout<<"Beta: "<<endl;
    for(unsigned int i=0;i<numBeta;i++)
        cout<<beta[i]<<"  ";
    cout<<endl;
    cout<<"Gamma: "<<endl;
    for(unsigned int i=0;i<dim;i++)
        cout<<gamma[i]<<"  ";
    cout<<endl;
    cout<<"SV_alpha: "<<endl;
    for(unsigned int i=0;i<numAlpha;i++)
    {
        cout<<"  --> ";
        for(unsigned int j=0;j<dim;j++)
            cout<<svalpha[i][j]<<"  ";
        cout<<endl;
    }
    cout<<"SV_beta: "<<endl;
    for(unsigned int i=0;i<numBeta;i++)
    {
        cout<<"  --> ";
        for(unsigned int j=0;j<2*dim;j++)
            cout<<svbeta[i][j]<<"  ";
        cout<<endl;
    }
    cout<<"*****************************************************************"<<endl;
}

void asvm::saveToFile(const char* filename)
{

    FILE *fp = fopen(filename, "w");
    if(!fp)
    {
        cout<<"ERROR: Cannot write to file <"<<filename<<">. SVM object not saved!"<<endl;
        return;
    }
    fprintf(fp, "%s\n", type);
    fprintf(fp, "%d \n %lf \n %lf \n %d \n %d\n", dim, lambda, b0, numAlpha, numBeta);

    for(unsigned int i=0; i<dim; i++)
        fprintf(fp, "%lf\t", target[i]);
    fprintf(fp, "\n");

    for(unsigned int i = 0; i<numAlpha; i++)
        fprintf(fp, "%lf\t", alpha[i]);
    fprintf(fp, "\n");

    for(unsigned int i = 0; i<numAlpha; i++)
        fprintf(fp, "%d\t", y[i]);
    fprintf(fp, "\n");

    if(numBeta == 0)
        fprintf(fp, "%lf\t", 0.0);
    else
        for(unsigned int i = 0; i<numBeta; i++)
            fprintf(fp, "%lf\t", beta[i]);
    fprintf(fp, "\n");

    for(unsigned int i = 0; i<dim; i++)
        fprintf(fp, "%lf\t", gamma[i]);
    fprintf(fp, "\n");

    for(unsigned int i = 0; i<numAlpha; i++)
    {
        for(unsigned int j =0; j<dim; j++)
            fprintf(fp, "%lf\t", svalpha[i][j]);
        fprintf(fp, "\n");
    }
    fprintf(fp, "\n");

    if(numBeta == 0)
        for(unsigned int j=0;j<dim*2;j++)
            fprintf(fp, "%lf\t", 0.0);
    else
        for(unsigned int i = 0; i<numBeta; i++)
        {
            for(unsigned int j =0; j<dim*2; j++)
                fprintf(fp, "%lf\t", svbeta[i][j]);
            fprintf(fp, "\n");
        }
    fprintf(fp, "\n");

    fclose(fp);

    cout<<"Model saved to file "<<filename<<endl;
}
