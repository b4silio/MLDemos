
#include "asvmdata.h"
using namespace std;

void asvmdata::printToFile(const char* filename)
{
	FILE* fp = fopen(filename, "w");

	fprintf(fp, "%d\n%d\n", (int)tar.size(), dim);
	for(unsigned int i=0;i<tar.size();i++)
	{
		fprintf(fp, "%d\n", (int)tar[i].traj.size());
		for(unsigned int j=0;j<tar[i].traj.size();j++)
		{
			fprintf(fp, "%d\n", tar[i].traj[j].nPoints);
			for(unsigned int k=0;k<tar[i].traj[j].nPoints;k++)
			{
				for(unsigned int l=0;l<dim;l++)
					fprintf(fp, "%lf\t", tar[i].traj[j].coords[k][l]);

				fprintf(fp, "\n");
			}
		}
	}

	fclose(fp);
}

bool asvmdata::loadFromFile(const char *filename)
{

	FILE *fp = fopen(filename,"r");
	int  dum;
	int tmpInt;

	if(fp)
	{
		dum =fscanf(fp, "%d", &tmpInt);
		dum=fscanf(fp, "%d", & dim);

		tar.resize(tmpInt);


		for(unsigned int i=0; i< tar.size(); i++)
		{
            tar[i].dim = dim;
			tar[i].targ = new double[ dim];
			for(unsigned int l=0; l< dim; l++)
				tar[i].targ[l] = 0.0;

			dum = fscanf(fp, "%d", &tmpInt);
			tar[i].traj.resize(tmpInt);
			for(unsigned int j=0; j< tar[i].traj.size();j++)
			{
                unsigned int nPoints;
                dum=fscanf(fp, "%d", &nPoints);
                trajectory& traj = tar[i].traj[j];
                traj.nPoints = nPoints;
                traj.dim = dim;
                traj.coords = new double*[ nPoints];
                traj.y = new int[ nPoints];
                traj.vel = new double*[ nPoints];

                for(unsigned int k=0; k< nPoints; k++)
				{
                    traj.coords[k] = new double[ dim];
                    traj.vel[k] = new double[ dim];

					for(unsigned int l=0; l< dim; l++)
					{
                        dum=fscanf(fp, "%lf", &( traj.coords[k][l]));
					}
                    traj.y[k] = i;
				}
				for(unsigned int l=0; l< dim; l++)
                    tar[i].targ[l] +=  traj.coords[ nPoints-1][l];

			}
			for(unsigned int l=0; l< dim; l++)
				tar[i].targ[l] /=  tar[i].traj.size();
		}

		fclose(fp);
	}
	else
	{
		cout<<endl<<"Error: data file \""<<filename<<"\" could not be opened!"<<endl;
		return false;
	}

	isOkay  =  true;
	return true;

}

asvmdata::asvmdata(const asvmdata& other)
{
	dim = other.dim;
	lambda = other.lambda;
	initial = other.initial;
	strcpy(type, other.type);
	target_class = other.target_class;

    isOkay = other.isOkay;
    num_alpha = other.num_alpha;
    num_beta = other.num_beta;
    if(other.labels)
    {
        labels = new int[num_alpha];
        memcpy(labels, other.labels, num_alpha*sizeof(int));
    } else labels = 0;
    if(other.matkgh)
    {
        int matkghCount = num_alpha + num_beta + dim;
        matkgh = new double*[matkghCount];
        for(int i=0; i<matkghCount; i++)
        {
            matkgh[i] = new double[matkghCount];
            memcpy(matkgh[i], other.matkgh[i], matkghCount*sizeof(double));
        }
    }else matkgh = 0;

	tar.resize(other.tar.size());
	for(unsigned int i=0;i<tar.size();i++)
	{
		tar[i].targ = new double[dim];
		for(unsigned int k=0;k<dim;k++)
			tar[i].targ[k] = other.tar[i].targ[k];

		tar[i].traj.resize(other.tar[i].traj.size());
		for(unsigned int j=0;j<tar[i].traj.size();j++)
		{
			tar[i].traj[j].nPoints = other.tar[i].traj[j].nPoints;

			tar[i].traj[j].y = new int[other.tar[i].traj[j].nPoints];
			for(unsigned int k=0;k<tar[i].traj[j].nPoints;k++)
				tar[i].traj[j].y[k] = other.tar[i].traj[j].y[k];

			tar[i].traj[j].coords = new double*[other.tar[i].traj[j].nPoints];
			tar[i].traj[j].vel = new double*[other.tar[i].traj[j].nPoints];
			for(unsigned int k=0;k<tar[i].traj[j].nPoints;k++)
			{
				tar[i].traj[j].coords[k] = new double[dim];
				tar[i].traj[j].vel[k] = new double[dim];
			}

			for(unsigned int k=0;k<tar[i].traj[j].nPoints;k++)
				for(unsigned int l=0;l<dim;l++)
				{
					tar[i].traj[j].coords[k][l] = other.tar[i].traj[j].coords[k][l];
					tar[i].traj[j].vel[k][l] = other.tar[i].traj[j].vel[k][l];
				}
		}

	}

}

asvmdata::~asvmdata()
{
    if(matkgh)
    {
        int matkghCount = num_alpha + num_beta + dim;
        for(int i=0; i<matkghCount; i++)
        {
            delete [] matkgh[i];
        }
        delete [] matkgh;
        matkgh = 0;
    }
    if(labels)
    {
        delete [] labels;
        labels = 0;
    }
    num_alpha = 0;
    num_beta = 0;
}

asvmdata& asvmdata::operator=(const asvmdata& other)
{
    if(&other == this) return *this;
	dim = other.dim;
	lambda = other.lambda;
	initial = other.initial;
	strcpy(type, other.type);
	target_class = other.target_class;

    isOkay = other.isOkay;
    if(matkgh)
    {
        int matkghCount = num_alpha + num_beta + dim;
        for(int i=0; i<matkghCount; i++)
        {
            delete [] matkgh[i];
        }
        delete [] matkgh;
        matkgh = 0;
    }
    num_alpha = other.num_alpha;
    num_beta = other.num_beta;
    KILL(labels);
    if(other.labels)
    {
        labels = new int[num_alpha];
        memcpy(labels, other.labels, num_alpha*sizeof(int));
    }
    if(other.matkgh)
    {
        int matkghCount = num_alpha + num_beta + dim;
        matkgh = new double*[matkghCount];
        for(int i=0; i<matkghCount; i++)
        {
            matkgh[i] = new double[matkghCount];
            memcpy(matkgh[i], other.matkgh[i], matkghCount*sizeof(double));
        }
    }


	tar.resize(other.tar.size());
	for(unsigned int i=0;i<tar.size();i++)
	{
		tar[i].targ = new double[dim];
		for(unsigned int k=0;k<dim;k++)
			tar[i].targ[k] = other.tar[i].targ[k];

		tar[i].traj.resize(other.tar[i].traj.size());
		for(unsigned int j=0;j<tar[i].traj.size();j++)
		{
			tar[i].traj[j].nPoints = other.tar[i].traj[j].nPoints;

			tar[i].traj[j].y = new int[other.tar[i].traj[j].nPoints];
			for(unsigned int k=0;k<tar[i].traj[j].nPoints;k++)
				tar[i].traj[j].y[k] = other.tar[i].traj[j].y[k];

			tar[i].traj[j].coords = new double*[other.tar[i].traj[j].nPoints];
			tar[i].traj[j].vel = new double*[other.tar[i].traj[j].nPoints];
			for(unsigned int k=0;k<tar[i].traj[j].nPoints;k++)
			{
				tar[i].traj[j].coords[k] = new double[dim];
				tar[i].traj[j].vel[k] = new double[dim];
			}

			for(unsigned int k=0;k<tar[i].traj[j].nPoints;k++)
				for(unsigned int l=0;l<dim;l++)
				{
					tar[i].traj[j].coords[k][l] = other.tar[i].traj[j].coords[k][l];
					tar[i].traj[j].vel[k][l] = other.tar[i].traj[j].vel[k][l];
				}
		}

	}

	return *this;
}

void asvmdata::preprocess(unsigned int tclass)
{

	target_class = tclass;
	double temp2;

	for(unsigned int i=0; i< tar.size(); i++)
		for(unsigned int j=0; j< tar[i].traj.size(); j++)
			for(unsigned  int k=0; k< tar[i].traj[j].nPoints ; k++)
				tar[i].traj[j].y[k] = (i==tclass ? +1:-1);

	for(unsigned int i=0; i< tar.size(); i++)
		for(unsigned int j=0; j< tar[i].traj.size();j++)
		{
			for(unsigned int k=0; k< tar[i].traj[j].nPoints-1; k++)
			{

				for(unsigned int l=0; l< dim; l++)
				{
					tar[i].traj[j].vel[k][l] =  tar[i].traj[j].coords[k+1][l] -  tar[i].traj[j].coords[k][l];
				}
				temp2 = norm( tar[i].traj[j].vel[k],  dim);
				for(unsigned int l=0; l< dim; l++)
				{
					if(fabs(temp2) > VEL_NORM_TOL)
						tar[i].traj[j].vel[k][l] /= temp2;
					else
						tar[i].traj[j].vel[k][l] = 0.0;
				}
			}

			for(unsigned int l=0; l< dim; l++)
			{
				tar[i].traj[j].vel[ tar[i].traj[j].nPoints-1][l] = 0.0;
			}
		}

	int tmp = 0;
	for(unsigned int i=0;i<tar.size();i++)
		tmp += tar[i].traj.size() ;

	num_alpha = totalpoints() - tmp ;
	num_beta = tar[target_class].classPoints() -  tar[target_class].traj.size();

//	alpha_indices = new unsigned int[num_alpha];
//	beta_indices = new unsigned int[num_beta];

	int count=0;
	labels = new int[num_alpha];

	for(unsigned int i=0;i<tar.size();i++)
		for(unsigned int j=0;j<tar[i].traj.size();j++)
			for(unsigned int k=0;k<tar[i].traj[j].nPoints-1;k++)
			{
				labels[count++] = tar[i].traj[j].y[k];
			}


	updateModulationKernel();
}


/*
void asvmdata::preprocess(unsigned int tclass)
{
	target_class = tclass;
	double temp2;

	for(unsigned int i=0; i< tar.size(); i++)
		for(unsigned int j=0; j< tar[i].traj.size(); j++)
			for(unsigned  int k=0; k< tar[i].traj[j].nPoints ; k++)
				tar[i].traj[j].y[k] = (i==tclass ? +1:-1);

	for(unsigned int i=0; i< tar.size(); i++)
		for(unsigned int j=0; j< tar[i].traj.size();j++)
		{
			for(unsigned int k=0; k< tar[i].traj[j].nPoints-1; k++)
			{

				for(unsigned int l=0; l< dim; l++)
				{
					tar[i].traj[j].vel[k][l] =  tar[i].traj[j].coords[k+1][l] -  tar[i].traj[j].coords[k][l];
				}
				temp2 = norm( tar[i].traj[j].vel[k],  dim);
				for(unsigned int l=0; l< dim; l++)
				{
					if(fabs(temp2) > VEL_NORM_TOL)
						tar[i].traj[j].vel[k][l] /= temp2;
					else
						tar[i].traj[j].vel[k][l] = 0.0;
				}
			}

			for(unsigned int l=0; l< dim; l++)
			{
				tar[i].traj[j].vel[ tar[i].traj[j].nPoints-1][l] = 0.0;
			}
		}

	int tmp = 0;
		for(unsigned int i=0;i<tar.size();i++)
			tmp += tar[i].traj.size() - 1;

		num_alpha = totalpoints() - tmp ;
		num_beta = tar[target_class].classPoints() -  tar[target_class].traj.size();

		alpha_indices = new unsigned int[num_alpha];
		beta_indices = new unsigned int[num_beta];

	int count=0;
		int index=0;

		labels = new int[num_alpha];

		for(unsigned int i=0;i<tar.size();i++)
			for(unsigned int j=0;j<tar[i].traj.size();j++)
				for(unsigned int k=0;k<tar[i].traj[j].nPoints;k++)
				{
					if(k == tar[i].traj[j].nPoints-1 && j!=0)
					{}
					else
					{
						alpha_indices[count] = index;
						labels[count++] = tar[i].traj[j].y[k];
					}

					index++;
				}

		count=0;
		index = 0;
		for(unsigned int j=0;j<tar[target_class].traj.size();j++)
		{
			for(unsigned int k=0;k<tar[target_class].traj[j].nPoints;k++)
			{
				if(k != tar[target_class].traj[j].nPoints - 1)
					beta_indices[count++] = index;

				index++;
			}
		}

		updateModulationKernel();
}
 */
void asvmdata::setParams(const char *_kernel_type, double _kernel_width, double _initial_guess)
{
	strcpy( type, _kernel_type);
	lambda = 1.0/(2*pow(_kernel_width,2));
	initial = _initial_guess;
}

void asvmdata::updateModulationKernel()
{

	unsigned int p = num_alpha ;
	unsigned int m = num_beta;
	unsigned int n = dim;

	unsigned int i,j,k;
	unsigned int a,b,c;
	double* temp1 = new double[dim];
	double* temp2 = new double[dim];
	double** temp3 = new double*[dim];
	for(i=0; i<n; i++)
		temp3[i] = new double[dim];


	const char* type1 = type;
	double lambda1 = lambda;

	//calculating full K matrix
	double** K = new double*[p];
	for(i=0; i<p ; i++)
		K[i] = new double[p];

	int l=0, d=0;
	for( i=0; i<tar.size(); i++)
		for( j=0; j<tar[i].traj.size(); j++)
			for( k=0; k<tar[i].traj[j].nPoints-1 ; k++)
			{
				for( a=0; a<tar.size(); a++)
					for( b=0; b<tar[a].traj.size(); b++)
						for(  c=0; c<tar[a].traj[b].nPoints-1 ; c++)
						{
							K[l][d] = tar[i].traj[j].y[k]*tar[a].traj[b].y[c]*
									getkernel(tar[i].traj[j].coords[k], tar[a].traj[b].coords[c], lambda1, type1, dim);
							d++;
						}
				l++;
				d = 0;
			}

	//calculating full G matrix
	l =0;
	d =0;
	double** G = new double*[p];
	for(i=0; i<p; i++)
		G[i] = new double[m];

	for( i=0; i<tar.size(); i++)
		for( j=0; j<tar[i].traj.size(); j++)
			for( k=0; k<tar[i].traj[j].nPoints-1 ; k++)
			{
				for( b=0; b<tar[target_class].traj.size(); b++)
					for( c=0; c<tar[target_class].traj[b].nPoints -1; c++)
					{

						getfirstkernelderivative(tar[i].traj[j].coords[k], tar[target_class].traj[b].coords[c], lambda1, type1, 2, temp1, dim);
						G[l][d] = tar[i].traj[j].y[k]*arraydot(tar[target_class].traj[b].vel[c], temp1, dim);
						d++;
					}
				l++;
				d=0;
			}


	//calculating full Gs matrix
	l=0;
	d=0;
	double** Gs = new double*[p];
	for(i=0; i<p; i++)
		Gs[i] = new double[n];
	for( i=0; i<tar.size(); i++)
		for( j=0; j<tar[i].traj.size(); j++)
			for( k=0; k<tar[i].traj[j].nPoints-1 ; k++)
			{
				for(a=0; a<dim; a++)
				{
					getfirstkernelderivative(tar[i].traj[j].coords[k], tar[target_class].targ, lambda1, type1, 2, temp1, dim);
					for(b=0; b<dim; b++)
					{
						if(b==a)
							temp2[b] = 1;
						else
							temp2[b] = 0;
					}
					Gs[l][a] = tar[i].traj[j].y[k]*arraydot(temp2, temp1, dim);

				}
				l++;
			}

	//calculating full H matrix
	l =0;
	d =0;
	double** H = new double*[m];
	for(i=0; i<m; i++)
		H[i] = new double[m];

	for( j=0; j<tar[target_class].traj.size(); j++)
		for(k=0; k<tar[target_class].traj[j].nPoints-1 ; k++)
		{
			for( b=0; b<tar[target_class].traj.size(); b++)
				for(c=0; c<tar[target_class].traj[b].nPoints-1 ; c++)
				{

					getsecondkernelderivative(tar[target_class].traj[j].coords[k], tar[target_class].traj[b].coords[c], dim, lambda1, type1, temp3);
					VectorMatrixMultipy(tar[target_class].traj[j].vel[k], temp3, temp1, dim, dim);
					H[l][d] = arraydot(tar[target_class].traj[b].vel[c], temp1, dim);
					d++;
				}
			l++;
			d = 0;
		}


	//calculating full Hs matrix
	l =0;
	d =0;
	double** Hs = new double*[m];
	for(i=0; i<m; i++)
		Hs[i] = new double[n];

	for( j=0; j<tar[target_class].traj.size(); j++)
		for(k=0; k<tar[target_class].traj[j].nPoints-1 ; k++)
		{

			for(a=0; a<dim; a++)
			{
				for(b=0; b<dim; b++)
					if(b==a)
						temp2[b] = 1;
					else
						temp2[b] = 0;

				getsecondkernelderivative(tar[target_class].traj[j].coords[k], tar[target_class].targ, dim, lambda1, type1, temp3);
				VectorMatrixMultipy(tar[target_class].traj[j].vel[k], temp3, temp1, dim, dim);
				Hs[l][a] = arraydot(temp2, temp1, dim);

			}
			l++;
		}


	//calculating full Hss matrix
	l =0;
	d =0;
	double** Hss = new double*[n];
	for(i=0; i<n; i++)
		Hss[i] = new double[n];

	for(i=0; i<n; i++)
		for(j=0; j<n; j++)
		{
			for(b=0; b<n; b++)
				if(b==i)
					temp2[b] = 1;
				else
					temp2[b] = 0;
			getsecondkernelderivative(tar[target_class].targ, tar[target_class].targ, dim, lambda1, type1, temp3);
			VectorMatrixMultipy(temp2, temp3, temp1, dim, dim);
			for(b=0; b<n; b++)
				if(b==j)
					temp2[b] = 1;
				else
					temp2[b] = 0;
			Hss[i][j] = arraydot(temp2, temp1, dim);
		}
	/*

		double** tmpkgh = new double*[p+m+n];
		for(i=0; i<p+m+n; i++)
			tmpkgh[i] = new double[p+m+n];

	for(i =0; i< p; i++)
		for(j =0; j< p; j++)
			tmpkgh[i][j] = K[i][j];

	for(i =0; i< p; i++)
		for(j =0; j<m; j++)
		{
			tmpkgh[i][j+ p] = G[i][j];
			tmpkgh[j+p][i] = G[i][j];
		}

	for(i =0; i<m; i++)
		for(j =0; j<m; j++)
			tmpkgh[i+p][j+p] = H[i][j];

	for(i =0; i<p; i++)
		for(j =0; j<n; j++)
		{
			tmpkgh[i][j+p + m] = - Gs[i][j];
			tmpkgh[j+ p + m][i] = - Gs[i][j];
		}


	for(i =0; i< m; i++)
		for(j =0; j< n; j++)
		{
			tmpkgh[i+ p][j+ p + m] = -Hs[i][j];
			tmpkgh[j+ p + m][i+ p] = -Hs[i][j];
		}

	for( i=0; i<dim; i++)
		for( j =0; j<dim; j++)
			tmpkgh[i+ p + m][j+ p + m] = Hss[i][j];
	 */
	unsigned int matkgh_size = num_alpha + num_beta + dim;
	matkgh = new double*[matkgh_size];
	for( i=0; i<matkgh_size; i++)
		matkgh[i] = new double[matkgh_size];
	/*
	//removing unwanted rows, cols
	for( i=0;i<num_alpha;i++)
	{
		for( j=0;j<num_alpha;j++)
			matkgh[i][j] = tmpkgh[alpha_indices[i]][alpha_indices[j]];

		for( j=0;j<num_beta;j++)
			matkgh[i][j+num_alpha] = tmpkgh[alpha_indices[i]][p + beta_indices[j]];

		for( j=0;j<dim;j++)
			matkgh[i][j+num_alpha + num_beta] = tmpkgh[alpha_indices[i]][p + m + j];
	}
	for( i=0;i<num_beta;i++)
	{
		for( j=0;j<num_alpha;j++)
			matkgh[num_alpha + i][j] = tmpkgh[p+beta_indices[i]][alpha_indices[j]];

		for( j=0;j<num_beta;j++)
			matkgh[num_alpha + i][j+num_alpha] = tmpkgh[p+beta_indices[i]][p + beta_indices[j]];

		for( j=0;j<dim;j++)
			matkgh[num_alpha + i][j+num_alpha + num_beta] = tmpkgh[p+beta_indices[i]][p + m + j];
	}
	for( i=0;i<dim;i++)
	{
		for(j=0;j<num_alpha;j++)
			matkgh[num_alpha + num_beta + i][j] = tmpkgh[p+m+i][alpha_indices[j]];

		for(j=0;j<num_beta;j++)
			matkgh[num_alpha +  num_beta + i][j+num_alpha] = tmpkgh[p+m+i][p + beta_indices[j]];

		for(j=0;j<dim;j++)
			matkgh[num_alpha +  num_beta + i][j+num_alpha + num_beta] = tmpkgh[p+m+i][p + m + j];
	}
	 */

	for( i=0;i<num_alpha;i++)
	{
		for( j=0;j<num_alpha;j++)
			matkgh[i][j] = K[i][j];

		for( j=0;j<num_beta;j++)
			matkgh[i][j+num_alpha] = G[i][j];

		for( j=0;j<dim;j++)
			matkgh[i][j+num_alpha + num_beta] = -Gs[i][j];
	}
	for( i=0;i<num_beta;i++)
	{
		for( j=0;j<num_alpha;j++)
			matkgh[num_alpha + i][j] = G[j][i];

		for( j=0;j<num_beta;j++)
			matkgh[num_alpha + i][j+num_alpha] = H[i][j];

		for( j=0;j<dim;j++)
			matkgh[num_alpha + i][j+num_alpha + num_beta] = -Hs[i][j];
	}
	for( i=0;i<dim;i++)
	{
		for(j=0;j<num_alpha;j++)
			matkgh[num_alpha + num_beta + i][j] = -Gs[j][i];

		for(j=0;j<num_beta;j++)
			matkgh[num_alpha +  num_beta + i][j+num_alpha] = -Hs[j][i];

		for(j=0;j<dim;j++)
			matkgh[num_alpha +  num_beta + i][j+num_alpha + num_beta] = Hss[i][j];
	}

}
