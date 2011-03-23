#include "SEDS.h"
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

//constructor
SEDS::SEDS()
{
	Options.max_iter = 10000;
	Options.tol_stopping = 1e-10;
	Options.tol_mat_bias = 1e-14;
	Options.cons_penalty = 1e4;
	Options.perior_opt = 1;
	Options.mu_opt = 1;
	Options.sigma_x_opt = 1;
	Options.display = 1;
	Options.objective = 1; //i.e. using likelihood
	d = 0;
	nData = 0;
}


/* Parsing the input commands to the solver */
bool SEDS::Parse_Input(int argc, char **argv, char** file_data, char** file_model, char** file_output)
{
	if ((argc-1)%2!=0 && argc > 1 && strcmp(argv[1],"-h")){
		cout << "Improper number of input arguments!" << endl;
		cout << "Leaving optimization.!" << endl;
		return 0;
	}

	for (int i=1; i < argc; i++){
		if (!strcmp(argv[i],"-dfile")) //name of the data file to be passed to the solver
			*file_data = argv[i+1];
		else if (!strcmp(argv[i],"-mfile"))// name of the file contains the model's initial guess to be passed to the solver
			*file_model = argv[i+1];
		else if (!strcmp(argv[i],"-ofile"))// name of the file contains the model's initial guess to be passed to the solver
			*file_output = argv[i+1];
		else if (!strcmp(argv[i],"-i"))  // an integer defining the maximum number of iterations
			Options.max_iter = atoi(argv[i+1]);
		else if (!strcmp(argv[i],"-p")) // a double variable defining the penalty value
			Options.cons_penalty = atof(argv[i+1]);
		else if (!strcmp(argv[i],"-t")) // a double variable defining an additive bias term on Sigmas
			Options.tol_mat_bias = atof(argv[i+1]);
		else if (!strcmp(argv[i],"-s")) // a double variable defining the stopping threshold
			Options.tol_stopping = atof(argv[i+1]);
		else if (!strcmp(argv[i],"-o")){// defining the objective function
			if (!strcmp(argv[i+1],"mse"))
				Options.objective = 0;
		}
		else if (!strcmp(argv[i],"-op"))// a 0 or 1 integer defining whether Prior should be optimized
			Options.perior_opt = atoi(argv[i+1]) > 0;
		else if (!strcmp(argv[i],"-om"))// a 0 or 1 integer defining whether Mu should be optimized
			Options.mu_opt = atoi(argv[i+1]) > 0;
		else if (!strcmp(argv[i],"-os"))// a 0 or 1 integer defining whether Sigma should be optimized
			Options.sigma_x_opt = atoi(argv[i+1]) > 0;
		else if (!strcmp(argv[i],"-d"))// a 0 or 1 integer defining whether to display results on the screen
			Options.display = atoi(argv[i+1]) > 0;
		else if (!strcmp(argv[i],"-h")){// displaying help

			 cout << "\n SEDS optimization toolbox. This function finds an optimal value of" << endl;
			 cout << " a Gaussian Mixture Model under the constraint of ensuring its global" << endl;
			 cout << " asymptotic stability." << endl;
			 cout << " " << endl;
             cout << " Syntax " << endl;
             cout << "     (Working Folder)$ ./optimization_SEDS -dfile <datafile> " << endl;
             cout << "                       -mfile <modelfile>  -ofile <outputfile>" << endl;
             cout << "                        [optional commands]" << endl;
             cout << " " << endl;
             cout << " " << endl;
             cout << " Inputs -----------------------------------------------------------------" << endl;
             cout << " " << endl;
             cout << "   o -dfile:  Data file contains demonstration datapoints." << endl;
             cout << "              The package supports both binary and text formats." << endl;
             cout << "              For binary files use the file extension .bin and for" << endl;
             cout << "              text files use .txt" << endl;
             cout << " " << endl;
             cout << "              - If the data file is binary, the structure of the file is " << endl;
             cout << "                     <d (int)> <nData (int)>" << endl;
             cout << "                     <Data(1,:) array of nData (double)>" << endl;
             cout << "                          ..." << endl;
             cout << "                     <Data(2*d,:) array of nData (double)>" << endl;
             cout << " " << endl;
             cout << "              - If the file is in the text format, the structure of the file is " << endl;
             cout << "                Each line has 2*d elements and corresponds to each datapoint" << endl;
             cout << "                For more detailed information see the file 'Communicate2Exe.m'" << endl;
             cout << " " << endl;
             cout << " " << endl;
             cout << "   o -mfile:    Model file contains an initial guess for the model." << endl;
             cout << "                The package supports both binary and text format." << endl;
             cout << "                For binary files use the file extension .bin and for" << endl;
             cout << "                text files use .txt" << endl;
             cout << " " << endl;
             cout << "              - If the model file is binary, the structure of the file is " << endl;
             cout << "                     <d (int)> <K (int)> <Priors array of K (double)>" << endl;
             cout << "                     <Mu(1,:) array of K (double)> ... <Mu(2*d,:) array of K (double)>" << endl;
             cout << "                     <Sigma(1,:,1) array of 2*d (double)> ... <Sigma(2*d,:,1) array of 2*d (double)>" << endl;
             cout << "                         ..." << endl;
             cout << "                     <Sigma(1,:,K) array of 2*d (double)> ... <Sigma(2*d,:,K) array of 2*d (double)>" << endl;
             cout << " " << endl;
             cout << "              - If the file is in the text format, the structure of the file is " << endl;
             cout << "                     First Line:         d" << endl;
             cout << "                     Second Line:         K" << endl;
             cout << "                     Third Line:         Priors" << endl;
             cout << "                     Next 2*d Lines:    Mu" << endl;
             cout << "                     Next 2*d Lines:    Sigma(:,:,1)" << endl;
             cout << "                         ..." << endl;
             cout << "                     Next 2*d Lines:    Sigma(:,:,K)" << endl;
             cout << " " << endl;
             cout << "                For more detailed information see the file 'Communicate2Exe.m'" << endl;
             cout << " " << endl;
             cout << "   o -ofile:    Name of the output file. The obtained optimal value for the GMM" << endl;
             cout << "                will be saved as a text file based on the following order:" << endl;
             cout << "                     First Line:         d" << endl;
             cout << "                     Second Line:         K" << endl;
             cout << "                     Third Line:         Priors" << endl;
             cout << "                     Next 2*d Lines:    Mu" << endl;
             cout << "                     Next 2*d Lines:    Sigma(:,:,1)" << endl;
             cout << "                         ..." << endl;
             cout << "                     Next 2*d Lines:    Sigma(:,:,K)" << endl;
             cout << " " << endl;
             cout << "                For more detailed information see the file 'Communicate2Exe.m'" << endl;
             cout << " " << endl;
             cout << " " << endl;
             cout << " Optional Commands-------------------------------------------------------" << endl;
             cout << "       -t:    a very small positive scalar to avoid instabilities in " << endl;
             cout << "              Gaussian kernel [default: 10^-15]" << endl;
             cout << " " << endl;
             cout << "       -s:    A small positive scalar defining the stoppping tolerance for " << endl;
             cout << "              the optimization solver [default: 10^-10]" << endl;
             cout << " " << endl;
             cout << "       -i:    maximum number of iteration for the solver [default: i_max=1000]" << endl;
             cout << " " << endl;
             cout << "       -p:    Most of the time, it is preferable to transform a constrained " << endl;
             cout << "              optimization problem into an unconstrained one by penalizing " << endl;
             cout << "              if the constrains are violated. 'cons_penalty' should be a " << endl;
             cout << "              big value in comparison to the order of magnitude of data." << endl;
             cout << "              If you wish to solve the real unconstrained problem, set the" << endl;
             cout << "              value of 'cons_penalty' to Inf [default: 10^4]" << endl;
             cout << " " << endl;
             cout << "       -o:   'likelihood': use likelihood as criterion to optimize parameters " << endl;
             cout << "              of GMM 'mse': use mean square error as criterion to optimize " << endl;
             cout << "              parameters of GMM " << endl;
             cout << " " << endl;
             cout << "       -d:    An option to control whether the algorithm displays the output" << endl;
             cout << "              of each iterations [default: true]" << endl;
             cout << " " << endl;
             cout << "       -op:   Shall the sover optimize priors? This is an option given to the" << endl;
             cout << "              user if s/he wishes not to optimize the priors [default: true]" << endl;
             cout << " " << endl;
             cout << "       -om:   Shall the sover optimize centers? This is an option given to the" << endl;
             cout << "              user if s/he wishes not to optimize the centers Mu [default: true]" << endl;
             cout << " " << endl;
             cout << "       -os:   Shall the sover optimize Sigma_x? This is an option given to the " << endl;
             cout << "              user if s/he wishes not to optimize the Sigma_x [default: true]" << endl;
             cout << " " << endl;
             cout << " " << endl;
             cout << "   %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%" << endl;
             cout << "   %%%    Copyright (c) 2010 S. Mohammad Khansari-Zadeh, LASA Lab, EPFL,   %%%" << endl;
             cout << "   %%%          CH-1015 Lausanne, Switzerland, http://lasa.epfl.ch         %%%" << endl;
             cout << "   %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%" << endl;
             cout << " " << endl;
             cout << "   The program is free for non-commercial academic use. Please contact the" << endl;
             cout << "   author if you are interested in using the software for commercial purposes." << endl;
             cout << "   The software must not be modified or distributed without prior permission" << endl;
             cout << "   of the authors. Please acknowledge the authors in any academic publications" << endl;
             cout << "   that have made use of this code or part of it. Please use this BibTex" << endl;
             cout << "   reference:" << endl;
             cout << " " << endl;
             cout << "      S. M. Khansari Zadeh and A. Billard, 'Imitation learning of Globally " << endl;
             cout << "      Stable Non-Linear Point-to-Point Robot Motions using Nonlinear" << endl;
             cout << "      Programming', in Proceeding of the 2010 IEEE/RSJ International" << endl;
             cout << "      Conference on Intelligent Robots and Systems (IROS 2010), Taipei," << endl;
             cout << "      Taiwan, October 2010 " << endl;
             cout << " " << endl;
             cout << "    To get latest upadate of the software please visit" << endl;
             cout << "                             http://lasa.epfl.ch/khansari" << endl;
             cout << " " << endl;
             cout << "    Please send your feedbacks or questions to:" << endl;
             cout << "                             mohammad.khansari_at_epfl.ch\n\n" << endl;
            
			return 0;
		}
		else{
			cout << "\nInvalid arguments '" << argv[i] <<"', please try again." << endl;
			cout << "Use -h to get a list of possible commands." << endl;
			cout << "Leaving optimization.!\n" << endl;
			return 0;
		}

		i++;
	}
	
	if (*file_data == NULL){
		cout << "\nThe name of the data file is not passed to the solver." << endl;
		cout << "Use the option -dfile to specify the file name." << endl;
		cout << "Leaving optimization.!\n" << endl;
		return 0;
	}
	if (*file_model == NULL){
		cout << "\nThe name of the model file is not passed to the solver." << endl;
		cout << "Use the option -mfile to specify the file name." << endl;
		cout << "Leaving optimization.!\n" << endl;
		return 0;
	}
	if (*file_output == NULL){
		cout << "\nThe name of the output file is not passed to the solver." << endl;
		cout << "Use the option -ofile to specify the file name." << endl;
		cout << "Leaving optimization.!\n" << endl;
		return 0;
	}
	
	return 1;
}


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
bool SEDS::loadData(const char fileName[], char type)
{
	// Load coefficient of a GMM from a file
	
	if (type == 'b'){ //reading a binary file
		//open the file
		FILE *file = fopen(fileName, "rb");
		if (!file)
		{
			std::cout << "Error: Could not open the file!" << std::endl;
			return false;
		}
		//read from file, get nData etc
		fread(&d, sizeof(int), 1, file);
		fread(&nData, sizeof(int), 1, file);
		
		//read from file, get data matrix
		Data.Resize(2*d,nData);
		fread(Data.Array(), sizeof(REALTYPE), nData*2*d, file);
		fclose(file);
	}
	else{
		std::ifstream file(fileName);
	
		if(!file.is_open()){
			std::cout << "Error: Could not open the file!" << std::endl;
			return false;
		}

		char tmp[1024];
		double valTmp;
		// Get number of row
		while(!file.eof()){
			file.getline(tmp,1024);
			nData++;
			if (nData==1){
				// Get number of columns
				std::istringstream strm;
				strm.str(tmp); 
				while (strm >> valTmp)
					d++;
			}
		}
		nData = nData - 1;
		d = d/2;
		Data.Resize(2*d,nData); // note that Data' is saved in the text file
		file.clear();
		file.seekg(0); // returns to beginning of the file
		for(unsigned int i=0;i<nData;i++){ 
			file.getline(tmp,1024);
			std::istringstream strm;
			strm.str(tmp); 
			for(unsigned int j=0;j<2*d;j++){
				strm >> Data(j,i);
			}
		}
		file.close();
	}
	return true;
}


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
bool SEDS::loadModel(const char fileName[], char type)
{
	// Load coefficient of a GMM from a file
	
	if (type == 'b'){ //reading a binary file
		//open the file
		FILE *file = fopen(fileName, "rb");
		if (!file)
		{
			std::cout << "Error: Could not open the file!" << std::endl;
			return false;
		}
		//read from file, get d and K etc
		fread(&d, sizeof(int), 1, file);
		fread(&K, sizeof(int), 1, file);
		
		//read prior
		Priors.Resize(K);
		fread(Priors.Array(), sizeof(REALTYPE), K, file);
		
		//read Mu
		Mu.Resize(2*d,K);
		fread(Mu.Array(), sizeof(REALTYPE), 2*d*K, file);
		
		//read Sigma
		Sigma = new Matrix[K];
		for (int k = 0; k < K; k++)
		{
			Sigma[k] = Matrix(2*d,2*d);
			fread(Sigma[k].Array(), sizeof(REALTYPE), 4*d*d, file);
		}

		fclose(file);
	}
	else{
		std::ifstream file(fileName);
	
		if(!file.is_open()){
			std::cout << "Error: Could not open the file!" << std::endl;
			return false;
		}
		
		file >> d >> K;

		Priors.Resize(K);
		for (int k = 0; k < K; k++)
			file >> Priors[k];
		
		Mu.Resize(2*d,K);
		for (int i = 0; i < 2*d; i++)
			for (int k = 0; k < K; k++)
				file >> Mu(i,k);
		
		Sigma = new Matrix[K];
		for (int k = 0; k < K; k++)
		{
			Sigma[k] = Matrix(2*d,2*d);
			for (int i = 0; i < 2*d; i++)
				for (int j = 0; j < 2*d; j++)
					file >> Sigma[k](i,j);
		}
		file.close();
	}
	
	return true;
}


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
bool SEDS::saveModel(const char fileName[])
{
	// Save the dataset to a file
	std::ofstream file(fileName);
	
	if(!file){
		std::cout << "Error: Could not open the file!" << std::endl;
		return false;
	}
	
	file << d << endl; //dimension
	file << K << endl << endl; //number of Gaussian
	
	file.precision(10); //setting the precision of writing
	
	for (unsigned int k = 0; k < K; k++)
		file << Priors(k) << " ";
	file << endl << endl;
	
	for (unsigned int i = 0; i < 2*d; i++){
		for (unsigned int k = 0; k < K; k++)
			file << Mu(i,k) << " ";
		file << endl;
	}
	file << endl;
	
	for (unsigned int k = 0; k < K; k++){
		for (unsigned int i = 0; i < 2*d; i++){
			for (unsigned int j = 0; j < 2*d; j++)
				file << Sigma[k](i,j) << " ";
			file << endl;
		}
		file << endl;
	}
	file.close();
	
	return true;
}

void SEDS::preprocess_sigma(){
	for (int k=0; k<K; k++)
	{
		for (int i=0; i<d; i++)
		{
			for (int j=0; j<d; j++)
			{
				if(i==j)
				{

					Sigma[k](i,i) = fabs(Sigma[k](i,i));
					Sigma[k](i+d,i) = -fabs(Sigma[k](i+d,i));
					Sigma[k](i,i+d) = -fabs(Sigma[k](i,i+d));
					Sigma[k](i+d,i+d) = fabs(Sigma[k](i+d,i+d));
				}
				else
				{
					Sigma[k](i,j) = 0.;
					Sigma[k](i+d,j) = 0.;
					Sigma[k](i,j+d) = 0.;
					Sigma[k](i+d,j+d) = 0.;
				}
			}
		}
	}
}

bool SEDS::initialize_value(){
	tmpData = new Matrix[K];
	Pxi = new Vector[K];
	h = new Vector[K];
	h_tmp = new Vector[K];
	prob.Resize(nData);
	Pxi_Priors.Resize(nData); //a vector representing Pxi*Priors
	
	A = new Matrix[K];
	L = new Matrix[K];
	invSigma_x = new Matrix[K];
	Sigma_xdx = new Matrix[K];
	Mu_x = new Vector[K];
	Mu_xd = new Vector[K];
	invSigma = new Matrix[K];
	B_Inv = new Matrix[d];
	
	detSigma.Resize(K);
	detSigma_x.Resize(K);
	rSrs.Resize(2*d,2*d);
	rArs.Resize(d,d);
	rBrs.Resize(d,d);
	rAvrs.Resize(d); //vector form of rArs!!!
	tmp_A.Resize(d,2*d); //will be used for eq to Matlab [eye(2);A(:,:,i)]
	B.Resize(d,d);
	dc.Resize(d*K,K+K*d+K*d*(2*d+1));
	c.Resize(K*d); //a vector representing the constraints
	if (Options.objective)
		tmp_mat.Resize(2*d,nData);
	else
		tmp_mat.Resize(d,nData);
		
	for(int k=0; k<K; k++) {
		Pxi[k] = Vector(nData);
		h[k] = Vector(nData);
		h_tmp[k] = Vector(nData);
		L[k] = Matrix(2*d,2*d);
		invSigma_x[k]=Matrix(d,d);
		Sigma_xdx[k]=Matrix(d,d);
		invSigma[k]=Matrix(2*d,2*d);
		A[k]=Matrix(d,d);
		Mu_x[k] = Vector(d);
		Mu_xd[k] = Vector(d);
	}
	
	if (!Options.objective){ //mse objective function
		X = Data.GetMatrix(0,d-1,0,nData-1);
		Xd = Data.GetMatrix(d,2*d-1,0,nData-1);
		Xd_hat.Resize(d,nData);
		for(int k=0; k<K; k++)
			tmpData[k] = Matrix(d,nData);
		
	}
	else
	{
		for(int k=0; k<K; k++)
			tmpData[k] = Matrix(2*d,nData);
	}
	return true;
}

/* Running optimization solver to find the optimal values for the model.
 * The result will be saved in the variable p
*/
bool SEDS::Optimize(){
	initialize_value();
	preprocess_sigma();
	cout << "sigmas preprocessed!\n";
	for(int k=0; k<K; k++)
	{
		cout << "k: " << k << "\n";
		Sigma[k].Print();
	}
	p = Vector(K+K*d+K*d*(2*d+1)); //a vector of parameters
	int counter = K*d+K;//the index at which sigma should start

	for (int k=0; k<K; k++){
		p(k)=log(Priors(k)); //insert Priors to p
		
		for(int j=0; j<d; j++) //for all dimensions
			p(K+k*d+j)=Mu(j,k); //insert Mu
		
		L[k] = Sigma[k].SCholesky();//set L
		
		for(int j=0; j<2*d; j++){
			p.InsertSubVector(counter,L[k].GetColumn(j),j,2*d-j);
			counter += 2*d-j;
		}
	}
	//now p contains Priors, Mu_x and Sigma. ready to optimize
   
	Matrix B(K*(1+2*d*(d+1)),K*(1+2*d*(d+1)));
	
	//computing the mean of the data 
	Vector mean_Data(2*d);
	mean_Data = Data.SumColumn()/nData;
	
	//computing the variance of Data, will be used for initialization of B
	Vector var_Data(2*d);
	for (int i=0 ; i<2*d ; i++)
		var_Data(i) = ((Data.GetRow(i)-mean_Data(i))^(Data.GetRow(i)-mean_Data(i))).Sum(); //sum the square
	
	var_Data /= (nData-1); //finally normalize to get the N-1-normalized variance (as in matlab)
	
	double meanVar=var_Data.Sum()/(2*d); //take mean of varaince over columns
	
	B=B.Identity()/meanVar; //this causes problems. 
	
	char* str[2];
	str[0] = (char*)"MSE";
	str[1] = (char*)"Likelihood";
	if (Options.display)
	{
		std::cout << std::endl << std::endl;
		std::cout << "%-------------------------------------------------------------------------------------%" << std::endl;
		std::cout << "%          Stable Estimator of Dynamical Systems (SEDS) Optimization Solver           %" << std::endl;
		std::cout << "%       Copyright(c) 2010 S. Mohammad Khansari Zadeh, LASA Lab, EPFL, Lausanne        %" << std::endl;
		std::cout << "%                    Switzerland, http://lasa.epfl.ch/khansari                        %" << std::endl;
		std::cout << "%-------------------------------------------------------------------------------------%" << std::endl;
		std::cout << "\n\nOptimization Algorithm starts ..." << endl;
		printf("Using %s as the objective function ...\n\n",str[Options.objective]);
		
	}

	double c1=0.0001;
	double c2=0.9;
	int j;
	double alpha=0;
	Vector dp(p.Size());
	Vector p_tmp(p.Size());
	Vector dJ(p.Size());
	Vector dJ_new(p.Size());

	double J=Compute_J(dJ,p); //compute dJ and J
	double J_new=0;
	double c1_cond,c2_cond;
	str[0] = (char*)"fail";
	str[1] = (char*)"ok";
	
	for(int i=0; i < Options.max_iter; i++){
		dp=-B*dJ; //set change direction
		j=0;
		c1_cond = dJ*dp;
		c2_cond = dJ*(B*dJ);
		if(c1_cond < 0 && c2_cond> 0){
			while(true)
			{
				alpha = 3*exp(-0.5*j); //set the step-length
				p_tmp = p+dp*alpha; //change p 
				J_new = Compute_J(dJ_new,p_tmp); //get corresponding J and dJ
				j++;
				
				if( (J_new < J + dp*dJ*c1*alpha && dp*alpha*dJ_new>=dp*dJ*c2) || alpha < Options.tol_stopping*Options.tol_stopping || dJ.Norm()<Options.tol_stopping || fabs(c1_cond)<Options.tol_stopping)
					break; //check if good enough, then break infinite loop
			}
		}
		else{
			printf("c1 and c2 conditons not satisfied\n");
			break;
		}
		
		if (Options.display){
			if (i%20 == 0){
				printf("\n                                                    1st Optimality    2nd Optimality\n");
				printf("   Iteration      J           |dJ|       step-size     Condition         Condition\n");
				printf("---------------------------------------------------------------------------------------\n");
			}
			printf("      %-3i     %-10.3f    %-10.3f     %-4.3f          %-4s              %-4s\n",i+1,J,dJ.Norm(),alpha,str[c1_cond < 0],str[c2_cond > 0]);
		}
	
		Compute_Hess(B,dJ_new-dJ,dp*alpha); //update the hessian (result in B)
		p = p + dp*alpha; //update p
		J=J_new;
		dJ=dJ_new;
		if (alpha < Options.tol_stopping*Options.tol_stopping || dJ.Norm()<Options.tol_stopping || fabs(c1_cond)<Options.tol_stopping)
			break; //break before max_iteration if good enough
	}
	
	Unpack_params(p); //forming Priors, Mu, and Sigma from parameters p
	return true;
}


/* This function computes the sensitivity of Cost function w.r.t. optimization parameters.
 * The result is saved in the Vector dJ. The returned value of function is J.
 * Don't mess with this function. Very sensitive and a bit complicated!
*/
double SEDS::Compute_J(Vector& dJ,Vector pp) //compute the objective function and derivative w.r.t parameters (updated in vector dJ) 
{
	
		double J = 0; 
		double sum=0; 
		Vector col(2*d); // a temporary vector needed below
		int lastind=0;
		
		for (int k=0; k<K; k=k++){
			//constructing Priors
			Priors[k]=exp(pp[k]); //extract the Priors from correspondng position in optimization vector
			sum+=Priors(k);
			
			//reconstructing Sigma
			for(int j=0; j<2*d; j++){ //for all dimensions
				col.Zero();
				for(int i=j; i<2*d; i++){
					col(i)=pp(K+K*d+lastind);
					lastind++;
				}
				L[k].SetColumn(col, j);
			}
			Sigma[k]=L[k]*(L[k].Transpose());
			
			for(int i=0; i<2*d;i++)
				Sigma[k](i,i)+=Options.tol_mat_bias;
			
			invSigma_x[k]=Sigma[k].GetMatrix(0,d-1,0,d-1).Inverse(&detSigma_x[k]);   
			Sigma_xdx[k]=Sigma[k].GetMatrix(d,2*d-1,0,d-1);
			
			invSigma[k]=Sigma[k].Inverse(&detSigma(k));
			
			A[k]=Sigma_xdx[k]*invSigma_x[k]; //dynamical system matrix. 
			
			//reconstructing Mu
			Mu_x[k] = pp.GetSubVector(K+k*d,d);
			Mu_xd[k] = A[k]*Mu_x[k]; //proagate the centers through the dynamical system
			for (int i=0; i<d; i++)
			{
				Mu(i,k) = Mu_x[k](i);
				Mu(i+d,k) = Mu_xd[k](i);
			}
		}
		Priors /= sum; //normalizing Priors
		
		//computing likelihood:
		double prob_tmp;
		Pxi_Priors.Zero();
			
		
		for (int k=0; k<K; k++){
			int d_tmp;
			double tmp_den;
			REALTYPE *p_X;
			
			if (Options.objective){ //likelihod
				tmp_den = sqrt(pow(2*M_PI,2*d)*fabs(detSigma[k])+DBL_MIN);
				d_tmp = 2*d;
				p_X = Data.Array();
			}
			else{ //mse
				tmp_den = sqrt(pow(2*M_PI,d)*fabs(detSigma_x[k])+DBL_MIN);
				d_tmp = d;
				p_X = X.Array();
			}
			
			REALTYPE *p_tmpData = tmpData[k].Array();
			for(int j=0; j<d_tmp; j++){
				double tmp_dbl = Mu(j,k);
				for(int i=0; i<nData; i++)
					*p_tmpData++ = (*p_X++) - tmp_dbl;
			}
			
			if (Options.objective){ //likelihod
				tmp_mat = invSigma[k]*tmpData[k];
			}
			else{ //mse
				tmp_mat = invSigma_x[k]*tmpData[k];
			}
			
			REALTYPE *p_tmp_mat = tmp_mat.Array();
			REALTYPE *p_Pxi = Pxi[k].Array();
			REALTYPE *p_Pxi_Priors = Pxi_Priors.Array();
			p_tmpData = tmpData[k].Array();
			prob.Zero();
			
			for(int i=0; i<d_tmp; i++){
				REALTYPE *p_prob = prob.Array();
				for(int j=0; j<nData; j++){
					if (i<d_tmp-1){
						*p_prob++ += (*p_tmp_mat++) * (*p_tmpData++);
					}
					else{
						*p_prob += (*p_tmp_mat++) * (*p_tmpData++);
						if (*p_prob > 200) //to avoid numerical issue
							*p_prob = 200;
						else if (*p_prob < -200) //to avoid numerical issue
							*p_prob = -200;
							
						*p_Pxi = exp(-0.5*(*p_prob++))/tmp_den;
						*(p_Pxi_Priors++) += (*p_Pxi++)*Priors[k];
					}
				}
			}
			/*
			for(int j=0; j<d; j++)
				tmpData[k].SetRow(Mu(j,k),j);
			
			tmpData[k]=X-tmpData[k]; // remove centers from data
			
			prob = ((invSigma_x[k]*tmpData[k])^tmpData[k]).SumRow();//cf the expontential in gaussian pdf
				
			double tmp_den = sqrt(pow(2*M_PI,d)*fabs(detSigma_x[k])+DBL_MIN);
				
			for(int j=0; j<nData; j++){
				if (prob[j] > 200) //to avoid numerical issue
					prob[j] = 200;
				else if (prob[j] < -200) //to avoid numerical issue
					prob[j] = -200;
					
				Pxi[k][j] = exp(-0.5*prob[j])/tmp_den;
				Pxi_Priors[j] += Pxi[k][j]*Priors[k];
			}
			*/ 
		}
		
		//computing GMR
		if (!Options.objective){ //mse
			for (int k=0; k<K; k++){
				
				tmp_mat = A[k]*X;
				REALTYPE *p_tmp_mat = tmp_mat.Array();
				REALTYPE *p_Xd_hat = Xd_hat.Array();
				REALTYPE *p_Pxi = Pxi[k].Array();
				REALTYPE *p_Pxi_Priors = Pxi_Priors.Array();
				
				for(int i=0; i<d; i++)
				{
					REALTYPE *p_h = h[k].Array();

					for(int j=0; j<nData; j++){
						if (i==0)
							*p_h = *(p_Pxi++)/(*(p_Pxi_Priors++)) * Priors[k];
						if (k==0)
							*(p_Xd_hat++) = *(p_h++) * (*p_tmp_mat++);
						else
							*(p_Xd_hat++) += *(p_h++) * (*p_tmp_mat++);
					}
				}
				
				/*
				h[k] = Pxi[k]*Priors[k]/Pxi_Priors;
				if (k==0)
					Xd_hat = Vector_Multiply(tmp_mul,h[k])^(A[k]*X);
				else
					Xd_hat += Vector_Multiply(tmp_mul,h[k])^(A[k]*X);
				*/
			}
		}
		
		for(int i=0; i<d; i++)
			tmp_A(i,i) = 1; 
		
		double det_term;
		double term;
		
		Vector dJ_dMu_k(d);
		
		int i_c;
		dJ.Zero();
		for(int k=0; k<K; k++){
			//sensitivity wrt Priors
			if (Options.perior_opt)
			{
				if (Options.objective){ //likelihood
					REALTYPE *p_h = h[k].Array();
					REALTYPE *p_Pxi = Pxi[k].Array();
					REALTYPE *p_Pxi_Priors = Pxi_Priors.Array();
					sum = 0;
					
					for (int j=0; j<nData; j++){
						*p_h = (*p_Pxi++)/(*p_Pxi_Priors++)*Priors[k];
						sum += (*p_h++) - Priors[k];
					}
					
					dJ[k] = -exp(pp(k))/Priors[k]*sum;
				
					/*
					h[k] = Pxi[k]*Priors[k]/Pxi_Priors;
					dJ(k)=-exp(pp(k))/Priors[k]*((h[k]-Priors[k]).Sum());
					*/
				}
				else{ //mse
					
					sum = 0;
					double tmp_dbl;
					tmp_mat = A[k]*X;
					h_tmp[k].Zero(); //??
					REALTYPE *p_tmp_mat = tmp_mat.Array();
					REALTYPE *p_Xd_hat = Xd_hat.Array();
					REALTYPE *p_Xd = Xd.Array();
					
					for (int i=0; i<d; i++){
						REALTYPE *p_h_tmp = h_tmp[k].Array();
						REALTYPE *p_h = h[k].Array();
						for (int j=0; j<nData; j++){
							tmp_dbl = *(p_h++) * (*(p_tmp_mat++)-*p_Xd_hat) * (*(p_Xd_hat++)-*(p_Xd++));
							*(p_h_tmp++) += tmp_dbl;
							sum += tmp_dbl;
						}
					}
					dJ[k] = exp(pp[k])/Priors[k]*sum;
					
					/*
					h_tmp[k] = h[k]^(((A[k]*X-Xd_hat)^(Xd_hat-Xd)).SumRow()); //This vector is common in all dJ computation.
																			  //Thus, I defined it as a variable to save some computation power
					dJ(k)= exp(pp(k))/Priors[k]*h_tmp[k].Sum();	//derivative of priors(k) w.r.t. p(k)
					*/
				}
				
			}
			
			if (Options.mu_opt)
			{
				if (Options.objective){ //likelihood
					tmp_A.InsertSubMatrix(0,d,A[k].Transpose(),0,d,0,d); // eq to Matlab [eye(2) A(:,:,i)']
					dJ_dMu_k=-((tmp_A*invSigma[k])*tmpData[k])*h[k];
					dJ.InsertSubVector(K+k*d,dJ_dMu_k,0,d);
				}
				else{ //mse
					
					tmp_mat = invSigma_x[k]*tmpData[k];
					REALTYPE *p_tmp_mat = tmp_mat.Array();
					REALTYPE *p_dJ = &dJ(K+k*d);
					
					for (int i=0; i<d; i++){
						REALTYPE *p_h_tmp = h_tmp[k].Array();
						for (int j=0; j<nData; j++)
							*p_dJ += *(p_tmp_mat++) * (*(p_h_tmp++));
						p_dJ++;
					}
						
					/*
					dJ_dMu_k = (tmpData[k]*invSigma_x[k]).Transpose()*h_tmp[k];
					dJ.InsertSubVector(K+k*d,dJ_dMu_k,0,d);
					*/
				}
			}
			
			//sensitivity wrt sigma
			i_c=0;
			
			if (Options.objective) //likelihood
				det_term = (detSigma(k)<0) ? -1:1; //det_term=sign(det_term)
			else
				det_term = (detSigma_x(k)<0) ? -1:1; //det_term=sign(det_term)
			
			for (int i=0;i<2*d;i++){
				for (int j=i;j<2*d;j++){
					i_c = i_c + 1;
					if (Options.sigma_x_opt || i>=d || j>=d)
					{
						rSrs = rSrs *0;
						rSrs(j,i)=1;
						rSrs = rSrs*L[k].Transpose() + L[k]*rSrs.Transpose();
						
						if (Options.objective) {//likelihood
							rAvrs = (-A[k] * rSrs.GetMatrix(0,d-1,0,d-1)+ rSrs.GetMatrix(d,2*d-1,0,d-1))*invSigma_x[k] * Mu_x[k];
							tmp_mat = (invSigma[k]*(rSrs*invSigma[k]))*tmpData[k];
							double tmp_dbl = (-0.5)*det_term*(invSigma[k]*rSrs).Trace();
							Vector tmp_vec = invSigma[k].GetMatrix(0,2*d-1,d,2*d-1)*rAvrs;
							
							REALTYPE *p_tmp_mat = tmp_mat.Array();
							REALTYPE *p_tmpData = tmpData[k].Array();
							sum = 0;
							
							for (int i=0; i<2*d; i++){
								REALTYPE *p_h = h[k].Array();
								for (int j=0; j<nData; j++){
									sum -= (0.5 * (*p_tmp_mat++) * (*p_tmpData) +
												  (i==0)*tmp_dbl + //derivative with respect to det Sigma which is in the numenator
												  (*p_tmpData++) * tmp_vec[i]) * (*p_h++); //since Mu_xi_d = A*Mu_xi, thus we should consider its effect here
								}
							}
							dJ(K+K*d+k*d*(2*d+1)+i_c-1) = sum;
							
							/*
							Vector tmp_vec = (((invSigma[k]*(rSrs*invSigma[k]))*tmpData[k])^tmpData[k]).SumRow();
							
							dJ(K+K*d+k*d*(2*d+1)+i_c-1) = ( tmp_vec*0.5 +
									(-0.5)*det_term*(invSigma[k]*rSrs).Trace() + //derivative with respect to det Sigma which is in the numenator
									tmpData[k].Transpose()*(invSigma[k].GetMatrix(0,2*d-1,d,2*d-1)*rAvrs)) //since Mu_xi_d = A*Mu_xi, thus we should consider its effect here
									*h[k]*(-1);
							*/ 
						}
						else { //mse
							tmp_mat = (-A[k]*rSrs.GetMatrix(0,d-1,0,d-1) + rSrs.GetMatrix(d,2*d-1,0,d-1))*invSigma_x[k]*X;
							Matrix tmp_mat2 = (invSigma_x[k]*rSrs.GetMatrix(0,d-1,0,d-1)*invSigma_x[k])*tmpData[k];
							double tmp_dbl = -det_term*(invSigma_x[k]*rSrs.GetMatrix(0,d-1,0,d-1)).Trace();
							
							REALTYPE *p_tmp_mat = tmp_mat.Array();
							REALTYPE *p_tmp_mat2 = tmp_mat2.Array();
							REALTYPE *p_tmpData = tmpData[k].Array();
							REALTYPE *p_Xd_hat = Xd_hat.Array();
							REALTYPE *p_Xd = Xd.Array();
							sum = 0;
							
							for (int i=0; i<d; i++){
								REALTYPE *p_h_tmp = h_tmp[k].Array();
								REALTYPE *p_h = h[k].Array();
								for (int j=0; j<nData; j++){
									sum += (*p_h++) * (*p_tmp_mat++) * ((*p_Xd_hat++) - (*p_Xd++)) +  //derivative of A
										   0.5 * (*p_h_tmp++) * ((*p_tmp_mat2++) * (*p_tmpData++) + (i == 0)*tmp_dbl); //derivative with respect to det Sigma which is in the numenator
																									//the above term (i==d-1) is just to sum temp_dbl once
								}
							}
							dJ(K+K*d+k*d*(2*d+1)+i_c-1) = sum;
															  
							/*
							dJ(K+K*d+k*d*(2*d+1)+i_c-1) =  (
								(h[k]^((((-A[k]*rSrs.GetMatrix(0,d-1,0,d-1) + rSrs.GetMatrix(d,2*d-1,0,d-1))*invSigma_x[k]*X)^(Xd_hat-Xd)).SumRow())) +//derivative of A
								((((invSigma_x[k]*rSrs.GetMatrix(0,d-1,0,d-1)*invSigma_x[k]*tmpData[k])^tmpData[k]).SumRow()  + //derivative w.r.t. Sigma in exponential
								     -det_term*(invSigma_x[k]*rSrs.GetMatrix(0,d-1,0,d-1)).Trace())^h_tmp[k]/2) //derivative with respect to det Sigma which is in the numenator
								).Sum();
							*/
						}
					}
				}
			}
		}
		
		int i1_tmp;
		double tmp_detB;
		Vector detB(d);
		c.Zero();
		//constraints
		for (int k=0; k<K; k++)//for all states
		{
			i1_tmp = 1; 
			B = (A[k]+A[k].Transpose())/2; //define B

			//computing the constraints (this part is basicly an exact rewrite of the matlab version)
			for (int i=0; i<d; i++) //for all dimensions
			{
				B_Inv[i]=B.GetMatrix(0,i,0,i).Inverse(&tmp_detB);//get inverse and determinants of minors
				
				if (i1_tmp*tmp_detB+pow(Options.tol_mat_bias,(double)(i+1))/d > 0)
					c(k*d+i) = i1_tmp*tmp_detB+pow(Options.tol_mat_bias,(double)(i+1))/d;

				//computing the sensitivity of the constraints to the parameters
				i_c = 0;
				for (int ii=0;ii<d;ii++){
					for (int jj=ii;jj<2*d;jj++){
						if (Options.sigma_x_opt || ii>=d || jj>=d)
						{
							rSrs = rSrs * 0;
							rSrs (jj,ii) = 1;
							rSrs = rSrs*L[k].Transpose() + L[k]*rSrs.Transpose();
							rArs = (-A[k] * rSrs.GetMatrix(0,d-1,0,d-1) + rSrs.GetMatrix(d,2*d-1,0,d-1)) * invSigma_x[k];
							rBrs = (rArs + rArs.Transpose())/2;
							if (i==0)
								term = rBrs(0,0);
							else
								term = (B_Inv[i]*rBrs.GetMatrix(0,i,0,i)).Trace()*tmp_detB;
							
							dc(k*d+i,K + d*K + k*d*(2*d+1)+i_c) = i1_tmp*term;
						}
						i_c = i_c +1;
					}
				}
				i1_tmp *= -1; //used to alternate sign over iterations
			}
		}
		
		//calc J
		if (Options.objective) {//likelihood
			REALTYPE *p_Pxi_Priors = Pxi_Priors.Array();
			for(int i=0; i<nData ; i++)
				J -= log(*p_Pxi_Priors++);
		}
		else{
			REALTYPE *p_Xd_hat = Xd_hat.Array();
			REALTYPE *p_Xd = Xd.Array();
			for(int i=0; i<d ; i++)
				for(int j=0; j<nData ; j++)
					J += 0.5 * ((*p_Xd_hat) - (*p_Xd)) * ((*p_Xd_hat++) - (*p_Xd++));
			//J = 0.5*((Xd_hat-Xd)^(Xd_hat-Xd)).Sum();
		}
		
		J = J/nData + Options.cons_penalty*(c*c);
		dJ = dJ/nData + (dc.Transpose()*c)*2*Options.cons_penalty; //calc dJ
		return J;
}


/*Updates the estimated value of inverse of Hessian at each iteration.
 * The function is written based on BFGS method.
*/
bool SEDS::Compute_Hess(Matrix &B, Vector gamma, Vector delta){ //updates the hessian, B
	double gBg=gamma*(B*gamma);
	double dg=delta*gamma;
	Matrix dd=Vector_Multiply(delta,delta);
	Matrix gg=Vector_Multiply(gamma,gamma);
	Vector tmp1=(delta/dg-B*gamma/gBg);
	B = B + dd/dg - B * gg * B /gBg +   Vector_Multiply(tmp1,tmp1)*gBg;
	return true;
}


/* Transforming the vector of optimization's parameters into a GMM model.*/
bool SEDS::Unpack_params(Vector pp){ //this is used to unpack the parameters in p after optimization, to reconstruct the 
//GMM-parameters in their ususal form: Priors, Mu and Sigma. 
	
	int lastind=0;
	double sum=0;
	Vector Mu_x(d),Mu_xd(2*d),col(2*d);
	Matrix* A=new Matrix[K];
	
	for (int k=0; k<K; k=k++){
		
		//Constructin Priors
		Priors(k)=exp(pp(k)); //extract the Priors from correspondng position in optimization vector
		sum+=Priors(k);
		
		//constructing Sigma
		for(int j=0; j<2*d; j++){ //for all dimensions
			col.Zero();
			for(int i=j; i<2*d; i++){
				col(i)=pp(K+K*d+lastind);
				lastind++;
			}
			L[k].SetColumn(col, j);
		}
		Sigma[k]=Matrix(2*d,2*d);
		Sigma[k]=L[k]*(L[k].Transpose());
		for(int i=0; i<2*d;i++)
			Sigma[k](i,i)+=Options.tol_mat_bias;
		
		A[k]=Matrix(d,d);
		A[k]=Sigma[k].GetMatrix(d,2*d-1,0,d-1)*Sigma[k].GetMatrix(0,d-1,0,d-1).Inverse(); //dynamical system matrix. 
			
		//reconstructing Mu
		Mu_x = pp.GetSubVector(K+k*d,d);
		Mu_xd = A[k]*Mu_x; //proagate the centers through the dynamical system
		
		for (int i=0; i<d; i++)
		{
			Mu(i,k) = Mu_x(i);
			Mu(i+d,k) = Mu_xd(i);
		}
	}
	
	Priors /= sum; //Normalizing Priors
	
	if (Options.display)
		CheckConstraints(A);
		
	return true;
}

/* checking if every thing goes well. Sometimes if the parameter
 * 'Options.cons_penalty' is not big enough, the constrains may be violated.
 * Then this function notifies the user to increase 'Options.cons_penalty'.
*/
bool SEDS::CheckConstraints(Matrix * A){
	bool ver=true;
	Vector eigvals;
	Matrix eigvects;
	Vector Mu_k(2*d);
	Matrix B(d,d);
	for(int k=0; k<K; k++)
	{
		B = (A[k]+A[k].Transpose())/2;
		B.EigenValuesDecomposition(eigvals,eigvects,100);
		for(int i=0; i<eigvals.Size(); i++){
			if(eigvals(i) > 0){
				if (ver)
				{
					cout << endl;
					cout<<"Optimization did not reach to an optimal point."<<endl;
					cout<<"Some constraints were slightly violated."<<endl;
					cout<<"The error may be due to change of hard constraints to soft constrints."<<endl;
					cout<<"To handle this error, increase the value of 'cons_penalty'"<<endl;
					cout<<"and re-run the optimization."<<endl<<endl;
					cout<<"Output error for debugging purpose:"<<endl;
				}
				cout<< "k = " << k << "  ;  err = " << eigvals(i) << endl;
				ver=false;
			}
		}
	}
	
	if(ver)
		cout<<"Optimization finished succesfully!"<<endl;
	cout << endl;
	
	return true;
}

// Computes x1^T * x2
Matrix SEDS::Vector_Multiply(Vector &x1, Vector &x2)
{
	int rows=x1.Size();
	int cols=x2.Size();
	Matrix output(rows,cols);
	
	REALTYPE *p_x1 = x1.Array();
	REALTYPE *p_out = output.Array();
	
	for (int i=0; i<rows; i++)
	{
		REALTYPE *p_x2 = x2.Array();
		for (int j=0; j<cols; j++)
			*(p_out++) = *p_x1 * (*(p_x2++));
		p_x1++;
	}
			
	return output;
}
