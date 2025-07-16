#include "SOGP.h"
#include <string.h>
#include <QDebug>


//Add a chunk of data
void SOGP::addM(const Matrix& in,const Matrix& out){
    for(int i=1;i<=in.Ncols();i++)
    {
        add(in.Column(i),out.Column(i));
    }
}

//Add this input and output to the GP
void SOGP::add(const ColumnVector& in,const ColumnVector& out){
    double kstar =m_params.m_kernel->kstar(in);

    if(current_size==0)
    {//First point is easy
        C.ReSize(1,1);
        Q.ReSize(1,1);
        //Equations 2.46 with q, r, and s collapsed
        alpha=out.t()/(kstar+m_params.s20);
        C(1,1)=-1/(kstar+m_params.s20);
        Q(1,1)=1/kstar;

        current_size=1;
        BV = in;
        return;
    }

    //We already have data
    //perform the kernel
    ColumnVector k=m_params.m_kernel->kernelM(in,BV);

    RowVector m=k.t()*alpha;
    double s2 = kstar+(k.t()*C*k).AsScalar();

    if(s2<1e-12){//For numerical stability..from Csato's Matlab code?
        //printf("SOGP::Small s2 %lf\n",s2);
        s2=1e-12;
    }

    //Update scalars
    //page 33 - Assumes Gaussian noise
    double r = -1/(m_params.s20+s2);
    //printf("out %d m %d r %f\n",out.Nrows(),m.Ncols(),r);
    RowVector q = -r*(out.t()-m);

    //projection onto current BV
    ColumnVector ehat = Q*k;//Appendix G, section c
    //residual length
    double gamma = kstar-(k.t()*ehat).AsScalar();//Ibid
    if(gamma<1e-12){//Numerical instability?
        //printf("SOGP::Gamma (%lf) < 0\n",gamma);
        gamma=0;
    }

    if(gamma<1e-6 && m_params.capacity!= -1){//Nearly singular, do a sparse update (e_tol)
        //printf("SOGP::Sparse! %lf \n",gamma);
        double eta = 1/(1+gamma*r);//Ibid
        ColumnVector shat = C*k+ehat;//Appendix G section e
        alpha=alpha+shat*(q*eta);//Appendix G section f
        C=C+r*eta*shat*shat.t();//Ibid
    }
    else{//Full update
        //printf("SOGP::Full!\n");
        //Expansions are messy
        RowVector expander(1);

        //s is of length N+1
        expander(1)=1;
        ColumnVector s = C*k & expander;//Apendix G section e

        //Add a row to alpha
        expander.ReSize(alpha.Ncols());
        for(int i=0;i<alpha.Ncols();i++)
            expander(i+1)=0;
        alpha = alpha & expander;
        //Update alpha
        alpha=alpha+s*q;//Equations 2.46

        //Add Row to C
        expander.ReSize(C.Ncols());
        for(int i=0;i<C.Ncols();i++)
            expander(i+1)=0;
        C = C&expander;
        //Add a Column to C
        expander.ReSize(C.Nrows());
        for(int i=0;i<C.Nrows();i++)
            expander(i+1)=0;
        C = C | expander.t();
        //Update C
        C = C + r*s*s.t();//Ibid

        //Save the data, N++
        BV = BV | in;
        current_size++;

        //Add row to Gram Matrix
        expander.ReSize(Q.Ncols());
        for(int i=0;i<Q.Ncols();i++)
            expander(i+1)=0;
        Q = Q&expander;
        //Add column
        expander.ReSize(Q.Nrows());
        for(int i=0;i<Q.Nrows();i++)
            expander(i+1)=0;
        Q = Q | expander.t();
        //Add one more to ehat
        expander.ReSize(1);
        expander(1)=-1;
        ehat = ehat & expander;
        //Update gram matrix
        Q = Q + (1/gamma)*ehat*ehat.t();//Equation 3.5
    }

    //Delete BVs if necessay...maybe only 2 per iteration?
    while(current_size > m_params.capacity && m_params.capacity>0){//We're too big!
        double minscore=0,score;
        int minloc=-1;
        //Find the minimum score
        for(int i=1;i<=current_size;i++){
            score = alpha.Row(i).SumSquare()/(Q(i,i)+C(i,i));
            if(i==1 || score<minscore){
                minscore=score;
                minloc=i;
            }
        }
        //Delete it
        delete_bv(minloc);
    }

    if(m_params.capacity >= 0)
    {
        //Delete for geometric reasons - Loop?
        double minscore=0,score;
        int minloc=-1;
        for(int i=1;i<=current_size;i++){
            score = 1/Q(i,i);
            if(i==1 || score<minscore){
                minscore=score;
                minloc=i;
            }
        }
        if(minscore<1e-9){
            delete_bv(minloc);
        }
    }
}

//Delete a BV.  Very messy
void SOGP::delete_bv(int loc){
    //First swap loc to the last spot
    RowVector alphastar = alpha.Row(loc);
    alpha.Row(loc)=alpha.Row(alpha.Nrows());
    //Now C
    double cstar = C(loc,loc);
    ColumnVector Cstar = C.Column(loc);
    Cstar(loc)=Cstar(Cstar.Nrows());
    Cstar=Cstar.Rows(1,Cstar.Nrows()-1);
    ColumnVector Crep=C.Column(C.Ncols());
    Crep(loc)=Crep(Crep.Nrows());
    C.Row(loc)=Crep.t();;
    C.Column(loc)=Crep;
    //and Q
    double qstar = Q(loc,loc);
    ColumnVector Qstar = Q.Column(loc);
    Qstar(loc)=Qstar(Qstar.Nrows());
    Qstar=Qstar.Rows(1,Qstar.Nrows()-1);
    ColumnVector Qrep=Q.Column(Q.Ncols());
    Qrep(loc)=Qrep(Qrep.Nrows());
    Q.Row(loc)=Qrep.t();
    Q.Column(loc)=Qrep;

    //Ok, now do the actual removal  Appendix G section g
    alpha= alpha.Rows(1,alpha.Nrows()-1);
    ColumnVector qc = (Qstar+Cstar)/(qstar+cstar);
    for(int i=1;i<=alpha.Ncols();i++)
        alpha.Column(i)-=alphastar(i)*qc;
    C = C.SymSubMatrix(1,C.Ncols()-1) + (Qstar*Qstar.t())/qstar - ((Qstar+Cstar)*(Qstar+Cstar).t())/(qstar+cstar);
    Q = Q.SymSubMatrix(1,Q.Ncols()-1) - (Qstar*Qstar.t())/qstar;

    //And the BV
    BV.Column(loc)=BV.Column(BV.Ncols());
    BV=BV.Columns(1,BV.Ncols()-1);

    current_size--;
}


//Predict on a chunk of data.
ReturnMatrix SOGP::predictM(const Matrix& in, ColumnVector &sigconf,bool conf){
    //printf("SOGP::Predicting on %d points\n",in.Ncols());
    Matrix out(alpha.Ncols(),in.Ncols());
    sigconf.ReSize(in.Ncols());
    for(int c=1;c<=in.Ncols();c++)
        out.Column(c) = predict(in.Column(c),sigconf(c),conf);
    out.Release();
    return out;
}

//Predict the output and uncertainty for this input.
ReturnMatrix SOGP::predict(const ColumnVector& in, double &sigma,bool conf){
    double kstar = m_params.m_kernel->kstar(in);
    ColumnVector k=m_params.m_kernel->kernelM(in,BV);

    ColumnVector out;
    if(current_size==0){
        sigma = kstar+m_params.s20;
        //We don't actually know the correct output dimensionality
        //So return nothing.
        out.ReSize(0);
    }
    else{
        out=(k.t()*alpha).t();//Page 33
        sigma=m_params.s20 + kstar + (k.t()*C*k).AsScalar();//Ibid..needs s2 from page 19
    }

    if(sigma<0){//Numerical instability?
        printf("SOGP:: sigma (%lf) < 0!\n",sigma);
        sigma=0;
    }

    //Switch to a confidence (0-100)
    if(conf){
        //Normalize to one
        sigma /= kstar+m_params.s20;
        //switch diretion
        sigma = 1-sigma;
        //and times 100;
        sigma *=100;
        //sigma = (1-sigma)*100;
    }
    else
        sigma=sqrt(sigma);
    
    out.Release();
    return out;

}

//Log probability of this data
double SOGP::log_prob(const ColumnVector& in, const ColumnVector& out){
    static const double ls2pi= log(sqrt(2*M_PI));  //Only compute once
    double sigma;
    double out2;
    if(current_size == 0){         //mu = zero, sigma = kappa.
        sigma=sqrt(m_params.m_kernel->kstar(in)+m_params.s20); //Is this right?  V_0=kstar, v_1 = s20
        out2=out.SumSquare();
    }
    else{
        ColumnVector mu = predict(in,sigma);
        mu-=out;
        out2=mu.SumSquare();
    }
    return(-ls2pi -log(sigma) -.5*out2/(sigma*sigma));
}

#define VER 16

bool SOGP::printTo(FILE *fp,bool ascii){
    if(!fp){
        printf("SOGP::save error\n");
        return false;
    }

    fprintf(fp,"SOGP version %d\n",VER);
    fprintf(fp,"current_size: %d\n",current_size);

    m_params.printTo(fp,ascii);
    printMatrix(alpha,fp,"alpha",ascii);
    printMatrix(C,fp,"C",ascii);
    printMatrix(Q,fp,"Q",ascii);
    printMatrix(BV,fp,"BV",ascii);
    return true;
}

bool SOGP::readFrom(FILE *fp,bool ascii){
    if(!fp){
        printf("SOGP::load error\n");
        return false;
    }
    int ver;
    fscanf(fp,"SOGP version %d\n",&ver);
    if(ver!=VER){
        printf("SOGP is version %d, file is %d\n",VER,ver);
        return false;
    }

    fscanf(fp,"current_size: %d\n",&current_size);

    m_params.readFrom(fp,ascii);
    readMatrix(alpha,fp,"alpha",ascii);
    readMatrix(C,fp,"C",ascii);
    readMatrix(Q,fp,"Q",ascii);
    readMatrix(BV,fp,"BV",ascii);
    return true;
}



