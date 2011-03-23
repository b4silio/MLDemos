#ifndef SEDS_h
	#define SEDS_h
	#include <stdio.h>
	#include <sstream>
	#include <fstream>
	#include <float.h>
	#include "MathLib/MathLib.h"
	using namespace MathLib;

	class SEDS {
	public:
		Vector Priors,p; //Priors of GMM, a vector containing the optimization variable
		MathLib::Matrix Data,Mu,*Sigma; //Data=matrix containing demonstrations, Mu=Centers of GMM, Sigma=Covariance matrices of GMM
		int nData,d,K; //nData=number of datapoints, d=dimension ,K=number of Gaussian components
		
		struct options{  //A struct containing all passed options by user
			double tol_mat_bias;// = 10^-18;  //constant added to diagonal of covarince to avoid numerical instability
			double cons_penalty;// = 10^2; //penalty for contraints
			double tol_stopping;//=10^-10; //threshold rate of change for optimum
			int max_iter;// = 1000; //maximum amount of iterations allowed
			int perior_opt;// = 1; //estimate Priors:1 do not estimate Priors:0
			bool mu_opt; // = 1; //estimate Mu:1 do not estimate Mu:0
			bool sigma_x_opt;// = 1; //estimate Sigma:1 do not estimate Sigma:0
			bool objective; //determines if MSE or likelihood is to be used =true for mean square error and =false for likelihood
			bool display; //print output?
		} Options;
		
		
		//constructor
		SEDS();
		
		/* Parsing the input commands to the solver */
		bool Parse_Input(int argc, char **argv, char** file_data, char** file_model, char** file_output);
		

		/* Loading demonstration detapoint
		 * fileName: name of the file containing datapoints
		 * For binary files use the file extension .bin and for
		 * text files use .txt
		 * 
		 * 		 - If the data file is binary, the structure of the file is 
		 * 				 <d (int)> <nData (int)> <Data(1,:) array of nData (double)> ... <Data(2*d,:) array of nData (double)>
		 * 
		 * 		 - If the file is in the text format, the structure of the file is 
		 * 				 Each line has 2*d elements and corresponds to each datapoint
		 * For more detailed information see the file 'Communicate2Exe.m'
		*/
		bool loadData(const char fileName[], char type = 't');
		
		
		
		/* Loading initial guess of the model
		 * fileName: name of the file containing the model
		 * For binary files use the file extension .bin and for
		 * text files use .txt
		 * 
		 * 	 - If the model file is binary, the structure of the file is 
		 *  	 <d (int)> <K (int)> <Priors array of K (double)>
		 * 		 <Mu(1,:) array of K (double)> ... <Mu(2*d,:) array of K (double)>
		 * 		 <Sigma(1,:,1) array of 2*d (double)> ... <Sigma(2*d,:,1) array of 2*d (double)>
		 * 			  ...
		 * 		 <Sigma(1,:,K) array of 2*d (double)> ... <Sigma(2*d,:,K) array of 2*d (double)>
		 * 
		 * 	 - If the file is in the text format, the structure of the file is 
		 * 		 First Line: 		d
		 * 		 Second Line: 		K
		 * 		 Third Line: 		Priors
		 * 		 Next 2*d Lines:	Mu
		 * 		 Next 2*d Lines:	Sigma(:,:,1)
		 * 			 ...
		 * 		 Next 2*d Lines:	Sigma(:,:,K)
		 * 
		 * For more detailed information see the file 'Communicate2Exe.m'
		*/
		bool loadModel(const char fileName[], char type = 't');
		
		
		
		/* Saving the optimal obtained model from SEDS
		 * fileName: name of the file to save the model.
		 * The model will be saved in text format based on the following strcuture:
		 * 
		 * 				 First Line: 		d
		 * 				 Second Line: 		K
		 * 				 Third Line: 		Priors
		 * 				 Next 2*d Lines:	Mu
		 * 				 Next 2*d Lines:	Sigma(:,:,1)
		 * 				 ...
		 * 				 Next 2*d Lines:	Sigma(:,:,K)
		 * 
		 * For more detailed information see the file 'Communicate2Exe.m'
		*/
		bool saveModel(const char fileName[]);
		
		
		
		/* Running optimization solver to find the optimal values for the model.
		 * The result will be saved in the variable p
		*/
		bool Optimize();
		
protected:
		//These are temporary variable that are used during optimization
		Vector detSigma_x,detSigma;
		MathLib::Matrix X,Xd,Xd_hat,*Sigma_x,*Sigma_xdx,*L,*tmpData, *A, *invSigma, *invSigma_x, tmp_A, B, *B_Inv, dc;
		MathLib::Matrix rSrs, rArs, rBrs, tmp_mat;
		Vector prob, *Pxi, *h_tmp, *h, *Mu_x, *Mu_xd, rAvrs, c;
		Vector Pxi_Priors; //a vector representing Pxi*Priors
		
		bool initialize_value();

		/* This function ensures that the initial guess of sigma satisfies the stability conditions
	   */
		void preprocess_sigma();


		/* This function computes the sensitivity of Cost function w.r.t. optimization parameters.
		 * The result is saved in the Vector dJ. The returned value of function is J.
		 * Don't mess with this function. Very sensitive and a bit complicated!
		*/
		double Compute_J(Vector &dJ,Vector p);
		
		/*Updates the estimated value of inverse of Hessian at each iteration.
		 * The function is written based on BFGS method.
		*/
		bool Compute_Hess(MathLib::Matrix &B, Vector gamma, Vector delta);
		
		/* Transforming the vector of optimization's parameters into a GMM model.*/
		bool Unpack_params(Vector pp); //when optimization is done, use this to correctly extract Priors, mu and sigma for model
		
		
		/* checking if every thing goes well. Sometimes if the parameter
		 * 'Options.cons_penalty' is not big enough, the constrains may be violated.
		 * Then this function notifies the user to increase 'Options.cons_penalty'.
		*/
		bool CheckConstraints(MathLib::Matrix * A); // to verify that the solution is legit
		
		// Computes x1^T * x2
		MathLib::Matrix Vector_Multiply(Vector &x1, Vector &x2);
	};
#endif
