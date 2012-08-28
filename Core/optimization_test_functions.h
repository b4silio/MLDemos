
/*
//---------------- Author: Francesco Castellini* and Annalisa Riccardi** ---------------//
//------- Affiliation: -----------------------------------------------------------------//
//--------------* Dipartimento di Ingegneria Aerospaziale, Politecnico di Milano -------//
//--------------**Centrum for Industrial Mathematics, University of Bremen -------------//
//--------E-mail: castellfr@gmail.com, nina1983@gmail.com ------------------------------//

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>. 
*/

#include <Eigen/Core>

using namespace Eigen;
using namespace std;

static const double Pi() {return 3.14159265358979323846; };

//SINGLE OBJECTIVE OPTIMIZATION PROBLEMS (see http://www.geatbx.com/docu/fcnindex-01.html#P129_5426 for mathematical definition)

// Ackley's Path function
// f10(x)=-a·exp(-b·sqrt(1/n·sum(x(i)^2)))-exp(1/n·sum(cos(c·x(i))))+a+exp(1)
// a=20; b=0.2; c=2·pi; i=1:n; -32.768<=x(i)<=32.768.
Eigen::VectorXd ackley(Eigen::VectorXd& x)
{
	Eigen::VectorXd y(1);
	double a=20, b=0.2, c=2*Pi();
	int nvars = x.size();

	double sumX = 0;
	for(int i=0; i<nvars; i++) sumX += x(i)*x(i);

	double sumCos = 0;
	for(int i=0; i<nvars; i++) sumCos += cos(c*x(i));

	y[0]=-a*exp(-b*sqrt(1/nvars*sumX))-exp(1./nvars*sumCos)+a+exp(1.);

	return y;
}

// Six-Humps function
// fSixh(x1,x2)=(4-2.1·x1^2+x1^4/3)·x1^2+x1·x2+(-4+4·x2^2)·x2^2
//   -3<=x1<=3, -2<=x2<=2.
Eigen::VectorXd sixhump(Eigen::VectorXd& x)
{
	Eigen::VectorXd y(1);
	int nvars = x.size();
	if (nvars < 2) return y;
	double x1 = x[0];
	double x2 = x[1]*0.6;
	double value =  (4-2.1*(x1*x1) + x1*x1*x1*x1/3)*(x1*x1) + x1*x2 + (-4 + 4*x2*x2)*(x2*x2);
	y[0] = value;
	return y;
}

// 1. Rastragin multi-modal function, uniformly distributed local minima, -5.12<=x(i)<=5.12, global minimum: f(x)=0; x(i)=0, i=1:n.
// Function checked!
Eigen::VectorXd rastragin(Eigen::VectorXd& x)
{
	Eigen::VectorXd y(1);

	int nvars = x.size();
	y[0]=10*nvars; 
	for(int i=0;i<nvars;i++)
		y[0]+= ( x(i)*x(i)-10*cos(2*Pi()*x(i)) );

	return y;
}

// 2. Schwefel's multi-modal function, deceptive, -500<=x(i)<=500, global minimum: f(x)=-n�418.9829; x(i)=420.9687, i=1:n. 
// Function checked!
Eigen::VectorXd schwefel(Eigen::VectorXd& x)
{
	Eigen::VectorXd y(1);

	int nvars = x.size();
	y[0]=0.0; 
	for(int i=0;i<nvars;i++)
		y[0]+= ( -x(i)*sin(sqrt(fabs(x(i)))) );

	return y;
}

// 3. Griewangk multi-modal function with 163^nvars optima, only one is global
// Function checked!
Eigen::VectorXd griewangk(Eigen::VectorXd& x)
{
	//sleep(1);	//sleep for 10 ms
	Eigen::VectorXd y(1);
	double prod;
	int i;
	int nvars = x.size();
	
	y[0]=0;
	prod=1;
	for(i=0;i<nvars;i++)
		{
		prod=prod*cos(x[i]/sqrt((double)(i+1)));
		y[0]=y[0]+pow(x[i],2)/4000;
		}
	y[0]=y[0]-prod + 1;

	return y;
}

// 4. Griewangk multi-modal function with 163^nvars optima, only one is global, with linear constraint
// Function checked!
Eigen::VectorXd griewangk_constrained(Eigen::VectorXd& x)
{
	Eigen::VectorXd y(2);
	double prod;
	int i;
	int nvars = x.size();
	
	y[0]=0;
	y[1]=0;
	prod=1;
	for(i=0;i<nvars;i++)
		{
		prod=prod*cos(x[i]/sqrt((double)(i+1)));
		y[0]=y[0]+pow(x[i],2)/4000;
		}
	y[0]=y[0]-prod + 1;

	for(i=0;i<nvars;i++)
	{
	y[1]+=x[i];
	}

	return y;
}


// 5. Easy 1D-1var function with isolated optimum
// Function checked!

Eigen::VectorXd f_1disolated(Eigen::VectorXd& x)
{
	Eigen::VectorXd y(1);
	y[0] = 2 - exp(-pow(((x[0]-0.2)/0.004),2)) - 0.8*exp(-pow(((x[0]-0.6)/0.4),2));
	return y;
}


// 6. Easy 1D-1var function with isolated optimum
// Function checked!

Eigen::VectorXd f_1disolated2(Eigen::VectorXd& x)
{
	Eigen::VectorXd y(1);
	y[0] = 5 - exp(-pow(((x[0]-0.2)/0.004),2)) - 0.8*exp(-pow(((x[0]-0.6)/0.4),2))- exp(-pow(((x[1]-0.3)/0.003),2)) - 0.8*exp(-pow(((x[1]-0.1)/0.2),2));
	return y;
}


//BI-OBJECTIVE OPTIMIZATION PROBLEMS
Eigen::VectorXd t1(Eigen::VectorXd& x)
//INPUT: x: array with n decision variables (from 2 to inf)
{
	//sleep(1);	//sleep for 10 ms
	Eigen::VectorXd y(2);
	double g, h, alfa;
	int i;
	int nvars = x.size();
	g=1;
	alfa=0.25;
	y[0]=x[0];
	for(i=1;i<nvars;i++)
		g=g+(i+1)*x[i]/(nvars-1);
	if((y[0]/(5*g))<1)
		h=1-pow((y[0]/(5*g)),alfa);
	else
		h=0;
	y[1]=g*h;

	return y;
}

Eigen::VectorXd t2(Eigen::VectorXd& x)
//INPUT: x: array with n decision variables (from 2 to inf)
{
	Eigen::VectorXd y(2);
	double g, h, alfa;
	int i;
	int nvars = x.size();

	g=1;
	alfa=3;

	y[0]=x[0];

	for(i=1;i<nvars;i++)
		g=g+(i+1)*x[i]/(nvars-1);

	if((y[0]/(5*g))<1)
		h=1-pow((y[0]/(5*g)),alfa);
	else
		h=0;


	y[1]=g*h;
	
	return y;
}

Eigen::VectorXd t3(Eigen::VectorXd& x)
{
	Eigen::VectorXd y(2);
	double g, h, alfa;

	y[0]=4*x[0];

	if (x[1]<=0.4)
		g=4-3*exp(-pow(((x[1]-0.2)/0.02),2));
	else
		g=4-2*exp(-pow(((x[1]-0.7)/0.2),2));

	alfa=0.25+3.75*(g-1);

	if (y[0]/g<1)
		h=1-pow((y[0]/g),alfa);
	else
		h=0;

	y[1]=g*h;
	
	return y;

}

Eigen::VectorXd t4(Eigen::VectorXd& x)
//INPUT: x: array with n decision variables (from 2 to inf)
{
	Eigen::VectorXd y(2);
	double g, h, alfa, q;
	int i;
	int nvars = x.size();

	g=1;
	alfa=4;
	q=2;

	y[0]=x[0];

	for(i=1;i<nvars;i++)
		g=g+(i+1)*x[i]/(nvars-1);

	h=2-pow((y[0]/(5*g)),alfa)-(y[0]/(5*g))*sin(2*Pi()*q*y[0]);

	y[1]=g*h;

	return y;

}

Eigen::VectorXd t5(Eigen::VectorXd& x)
//INPUT: x: array with n decision variables (from 2 to inf)
{
	Eigen::VectorXd y(2);
	double g, h, alfa, prod;
	int i;
	int nvars = x.size();

	g=2;
	alfa=4;
	prod=1;

	y[0]=x[0];

	for(i=1;i<nvars;i++)		//g is Griewangk 1D function with n-1 variables
		{
		prod=prod*cos(x[i]/sqrt(i+1.0));
		g=g+pow(x[i],2)/4000;
		}
	g=g-prod;

	if((y[0]/(5*g))<1)
		h=1-pow((y[0]/(5*g)),alfa);
	else
		h=0;

	y[1]=g*h;

	return y;

}

Eigen::VectorXd t6(Eigen::VectorXd& x)
//INPUT: x: array with n decision variables (from 2 to inf)
//		 m: number of decision variables concerning function f1

{
	Eigen::VectorXd y(2);
	double g, h, alfa, sum;
	int i;
	int m = 15; //HARD CODED!!!!!!!!!!!!!
	int nvars = x.size();
//	printf("%d %d",n,m);

	g=1;
	alfa=0.25;
	sum=0;

	for(i=0;i<m;i++)
		sum+=pow(x[i],0.3);

	y[0]=pow(fabs(cos(2*Pi()*sum)),0.3);

	for(i=m;i<nvars;i++)
		g+=9*pow(x[i],0.3)/(nvars-m);

	if((y[0]/g)<1)
		h=1-pow((y[0]/g),alfa);
	else
		h=0;

	y[1]=g*h;

	return y;

}

Eigen::VectorXd t7(Eigen::VectorXd& x)
//INPUT: x: array with 80 bits (0 or 1) --> 11 vars
//Parameters(fixed): 	n1 bits for x1, n2 bits for x2,...x11

{
	Eigen::VectorXd y(2);
	Eigen::VectorXd X(x.size());
	double g, h;
	int i, j, n, n1, n2;

	g=0;
	n=11;
	n1=30;
	n2=5;

	int *u = new int[n];

	for(i=0; i<x.size(); i++){
		if(x[i]<0.5) X[i] = 0.0;
		else X[i] = 1.0;
	}
	for(i=0;i<n;i++)
		u[i]=0;			//inizializing unitations

	for(i=0;i<n1;i++)
		u[0]+=X[i];		//computing uintation of x1

	for(i=1;i<n;i++)
		for(j=n1+(i-1)*n2;j<n1+i*n2;j++)
			u[i]+=X[j];	//computing uintation of x2,...,xn

	y[0]=1+u[0];

	for(i=1;i<n;i++)	//computing g
		if(u[i]==n2)
			g+=1;
		else
			g+=(2+u[i]);
	h=1/y[0];
	y[1]=g*h;

	delete[] u;

	return y;
}

// problem t3 with linear constraint
Eigen::VectorXd t3c1(Eigen::VectorXd& x)
{
	Eigen::VectorXd y(3);
	double g, h, alfa;

	y[0]=4*x[0];

	if (x[1]<=0.4)
		g=4-3*exp(-pow(((x[1]-0.2)/0.02),2));
	else
		g=4-2*exp(-pow(((x[1]-0.7)/0.2),2));

	alfa=0.25+3.75*(g-1);

	if (y[0]/g<1)
		h=1-pow((y[0]/g),alfa);
	else
		h=0;

	y[1]=g*h;
	
	y[2]=x[0]+x[1]-0.3;

	return y;

}

//problem t3 with non linear constraint
Eigen::VectorXd t3c2(Eigen::VectorXd& x)
{
	Eigen::VectorXd y(3);
	double g, h, alfa;

	y[0]=4*x[0];

	if (x[1]<=0.4)
		g=4-3*exp(-pow(((x[1]-0.2)/0.02),2));
	else
		g=4-2*exp(-pow(((x[1]-0.7)/0.2),2));

	alfa=0.25+3.75*(g-1);

	if (y[0]/g<1)
		h=1-pow((y[0]/g),alfa);
	else
		h=0;

	y[1]=g*h;
	
	y[2]=exp(-(x[0]-0.15)*(x[0]-0.15)/(0.03*0.03))+x[1]-0.3;

	return y;

}

//problem t3 with non-linear constraint with disconnected feasible search space
Eigen::VectorXd t3c3(Eigen::VectorXd& x)
{
	Eigen::VectorXd y(4);
	double g, h, alfa;

	y[0]=4*x[0];

	if (x[1]<=0.4)
		g=4-3*exp(-pow(((x[1]-0.2)/0.02),2));
	else
		g=4-2*exp(-pow(((x[1]-0.7)/0.2),2));

	alfa=0.25+3.75*(g-1);

	if (y[0]/g<1)
		h=1-pow((y[0]/g),alfa);
	else
		h=0;

	y[1]=g*h;
	

	double a,b,c,d,e,f,i,l;
	a=1;
	b=0;
	c=1;
	d=0;
	e=-1.0;//-0.045;
	f=1;
	g=0.3;
	h=0.25;
	i=0.6;
	l=-0.05;

	y[2]=a*pow(x[0]-b,2)+c*pow(x[1]-d,2)+e;
	y[3]=f*pow(x[0]-g,2)+h*pow(x[1]-i,2)+l;

	return y;

}

//Example for BBWORHP. x(1) is Integer and the global Optimum is (0.5, 1) with Objective Value 2.
Eigen::VectorXd BB_1(Eigen::VectorXd& x)
{
	Eigen::VectorXd y(3);
	
	y[0]= 2.0*x(0) + x(1);
	y[1] = 1.25 - x(0)*x(0) - x(1);
	y[2] = x(0) + x(1);

	return y;
}

//global Optimum (27, x1, 27, 78, x4) with value -32217.4 
Eigen::VectorXd BB_2(Eigen::VectorXd& x)
{
	Eigen::VectorXd y(4);
		double  a1 = 85.334407,
			a2 = 0.0056858,
			a3 = 0.0006262,
			a4 = 0.0022053,
			a5 = 80.51249,
			a6 = 0.0071317,
			a7 = 0.0029955,
			a8 = 0.0021813,
			a9 = 9.300961,
			a10 = 0.0047026,
			a11 = 0.0012547,
			a12 = 0.0019085;
	y[0]= 5.357854*pow(x[0],2) + 0.835689*x[2]*x[3] + 37.29329*x[3] - 40792.141;
	y[1] = a1 + a2*x[2]*x[4] + a3*x[1]*x[3] - a4*x[0]*x[2];
	y[2] = a5 + a6*x[2]*x[4] + a7*x[3]*x[4] + a8*pow(x[0],2) - 90.0;
	y[3] = a9 + a10*x[0]*x[2] + a11*x[0]*x[3] + a12*x[0]*x[1] - 20.0;
	return y;
}

//Global Optimum (0.2, 1.280624, 1.954483, 1, 0, 0, 1) with value 3.557463
Eigen::VectorXd BB_3(Eigen::VectorXd& x)
{
	Eigen::VectorXd y(10);
	
	y[0]= pow(x[3]-1.0,2) + pow(x[4]-1.0,2) + pow(x[5]-1.0,2) - log(x[6]+1.0) + pow(x[0]-1.0,2) + pow(x[1]-2.0,2) + pow(x[2]-3.0,2);
	y[1] = x[0] + x[1] + x[2] + x[3] + x[4] + x[5];
	y[2] = pow(x[0],2) + pow(x[1],2) + pow(x[2],2) + pow(x[5],2);
	y[3] = x[0] + x[3];
	y[4] = x[1] + x[4];
	y[5] = x[2] + x[5];
	y[6] = x[0] + x[6];
	y[7] = pow(x[1],2) + pow(x[4],2);
	y[8] = pow(x[2],2) + pow(x[5],2);
	y[9] = pow(x[2],2) + pow(x[4],2);

	return y;
}
