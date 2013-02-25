#include "SEDS.h"
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

double NLOpt_Compute_J(unsigned nPar, const double *x, double *grad, void *f_data)
{
    SEDS *seds = (SEDS *) f_data;

    Vector p(nPar),dJ(nPar);
    p.Set(x,nPar);

    double J = seds->Compute_J(p, dJ);
    if(grad)
    {
        for (int i=0; i<nPar; i++)
            grad[i] = dJ[i];
    }

    double J_tmp = 1e20;
    if (seds->displayData.size()>0)
        J_tmp = seds->displayData.back();

    J_tmp = min(J,J_tmp);
    seds->displayData.push_back(J_tmp);
#ifdef USEQT
    // we paint the data
    seds->PaintData(seds->displayData);
#endif

    return J;
}

void NLOpt_Constraint(unsigned nCtr, double *result, unsigned nPar, const double* x, double* grad, void* f_data)
{
    SEDS *seds = (SEDS *) f_data;

    Vector c(nCtr);
    Matrix dc(nCtr,nPar);
    seds->Compute_Constraints(c, dc, false);

    for (int i=0; i<nCtr; i++){
        result[i] = c[i];
        if(grad)
        {
            for (int j=0; j<nPar; j++)
                grad[i*nPar+j] = dc(i,j);
        }
    }
}

//constructor
SEDS::SEDS()
{
    Options.max_iter = 1000;
    Options.tol_stopping = 1e-4;
    Options.delta = 1.0e-5;
    Options.tol_mat_bias = 1e-12;
    Options.perior_opt = 1;
    Options.mu_opt = 1;
    Options.sigma_x_opt = 1;
    Options.display = 1;
    Options.objective = 1; //i.e. using likelihood
    Options.eps_margin = 1e-4;
    Options.SEDS_Ver = 2;
    d = 0;
    nData = 0;
#ifdef USEQT
    displayLabel = 0;
#endif
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
        nData = 0;
        d = 0;
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

        d /= 2;

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

        //        d /= 2; // correction by Manuel Muehlig

        Priors.Resize(K);
        for (int k = 0; k < K; k++)
            file >> Priors[k];

        Mu.Resize(2*d,K);
        for (int i = 0; i < 2*d; i++)
            for (int k = 0; k < K; k++) // loop order swapped, correction by Manuel Muehlig
                file >> Mu(i,k);

        Sigma = new Matrix[K];
        for (int k = 0; k < K; k++)
        {
            Sigma[k] = Matrix(2*d,2*d);
            for (int i = 0; i < 2*d; i++)
                for (int j = 0; j < 2*d; j++)
                    file >> Sigma[k](i,j);
        }

        endpoint.resize(2*d);
        for (unsigned int i = 0; i < 2*d; i++)
        {
            file >> endpoint[i];
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

    for (unsigned int i = 0; i < 2*d; i++)
    {
        file << endpoint[i] << " ";
    }
    file << endl;
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
    invSigma_x = new Matrix[K];
    Sigma_xdx = new Matrix[K];
    Mu_x = new Vector[K];
    Mu_xd = new Vector[K];
    invSigma = new Matrix[K];
    B_Inv = new Matrix[d];

    detSigma.Resize(K);
    detSigma_x.Resize(K);
    rArs.Resize(d,d);
    rBrs.Resize(d,d);
    rAvrs.Resize(d); //vector form of rArs!!!
    tmp_A.Resize(d,2*d); //will be used for eq to Matlab [eye(2);A(:,:,i)]
    B.Resize(d,d);

    sum_dp.Resize(K);

    if (Options.objective)
        tmp_mat.Resize(2*d,nData);
    else
        tmp_mat.Resize(d,nData);

    for(int k=0; k<K; k++) {
        Pxi[k] = Vector(nData);
        h[k] = Vector(nData);
        h_tmp[k] = Vector(nData);
        invSigma_x[k]=Matrix(d,d);
        Sigma_xdx[k]=Matrix(d,d);
        invSigma[k]=Matrix(2*d,2*d);
        A[k]=Matrix(d,d);
        Mu_x[k] = Vector(d);
        Mu_xd[k] = Vector(d);
    }

    if (Options.objective){ //likelihood objective function
        rSrs.Resize(2*d,2*d);
        L = new Matrix[K];
        for(int k=0; k<K; k++){
            tmpData[k] = Matrix(2*d,nData);
            L[k] = Matrix(2*d,2*d);
        }
    }else{ //mse objective function
        rSrs.Resize(d,d);
        Sigma_x = new Matrix[K];
        L_x = new Matrix[K];
        X = Data.GetMatrix(0,d-1,0,nData-1);
        Xd = Data.GetMatrix(d,2*d-1,0,nData-1);
        Xd_hat.Resize(d,nData);
        for(int k=0; k<K; k++){
            tmpData[k] = Matrix(d,nData);
            L_x[k] = Matrix(d,d);
            Sigma_x[k] = Matrix(d,d);
        }
    }

    C_Lyapunov.Resize(d,d);
    for (int i=0;i<d;i++)
        C_Lyapunov(i,i) = 1;
    return true;
}

//QPixmap pm(320,240);
//QLabel lbl;
void SEDS::PaintData(std::vector<float> data)
{
#ifndef USEQT
    return;
#else
    if(!displayLabel) return;
    int w = displayLabel->width();
    int h = displayLabel->height();
    QPixmap pm(w,h);
    //QBitmap bitmap(w,h);
    //bitmap.clear();
    //pm.setMask(bitmap);
    pm.fill(Qt::transparent);
    QPainter painter(&pm);

    int cnt = data.size();
    int pad = 4;
    QPointF oldPoint;
    double minVal = FLT_MAX;
    double maxVal = -FLT_MAX;
    for(int i=0; i< data.size(); i++)
    {
        if(minVal > data[i]) minVal = data[i];
        if(maxVal < data[i]) maxVal = data[i];
    }
    if (minVal == maxVal)
    {
        minVal = 0;
    }

    painter.setBrush(Qt::NoBrush);
    painter.setPen(QPen(QColor(200,200,200), 0.5));
    int steps = 3;
    for(int i=0; i<=steps; i++)
    {
        painter.drawLine(QPoint(0, i/(float)steps*(h-2*pad) + pad), QPoint(w, i/(float)steps*(h-2*pad) + pad)); // horizontal
    }
    steps = steps*w/h;
    for(int i=0; i<=steps; i++)
    {
        painter.drawLine(QPoint(i/(float)steps*w, 0), QPoint(i/(float)steps*w, h)); // vertical
    }
    painter.setRenderHint(QPainter::Antialiasing);

    painter.setPen(QPen(Qt::black, 1.5));
    for(int i=0; i< data.size(); i++)
    {
        float value = data[i];
        if (value != value) continue;
        float x = i/(float)cnt*w;
        float y = (1 - (value-minVal)/(maxVal - minVal)) * (float)(h-2*pad) + pad;
        QPointF point(x, y);
        if(i) painter.drawLine(oldPoint, point);
        //painter.drawEllipse(point, 3, 3);
        oldPoint = point;
    }
    painter.setPen(QPen(Qt::black, 0.5));
    painter.setBrush(QColor(255,255,255,200));
    painter.drawRect(QRect(190,5,100,45));
    painter.setPen(QPen(Qt::black, 1));
    QFont font = painter.font();
    font.setPointSize(8);
    painter.setFont(font);
    painter.drawText(QPointF(w*2/3, 12), QString("J_0: %1").arg(data[0]));
    painter.drawText(QPointF(w*2/3, 22), QString("J_F: %1").arg(data[data.size()-1]));
    displayLabel->setPixmap(pm);
    qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
#endif
}


/* Running optimization solver to find the optimal values for the model.
 * The result will be saved in the variable p
*/
bool SEDS::Optimize(){
    displayData.clear();
    /*string str = "test.txt";
    loadModel(str.c_str());
    str = "data.txt";
    loadData(str.c_str());*/
    initialize_value();
    preprocess_sigma();

    if (K==1)
        Options.perior_opt = false;

    if (Options.objective){
        nPar = Options.perior_opt*K + Options.mu_opt*K*d + Options.sigma_x_opt*K*d*(d+1) + K*d*d + (Options.SEDS_Ver-1)*d*d;
        p.Resize(nPar); //a vector of parameters
        GMM_2_Parameters_Likelihood(p);
    }else{
        nPar = Options.perior_opt*K + Options.mu_opt*K*d + Options.sigma_x_opt*K*d*(d+1)/2 + K*d*d + (Options.SEDS_Ver-1)*d*d;
        p.Resize(nPar); //a vector of parameters
        GMM_2_Parameters_MSE(p);
    }
    nCtr = K*d + (Options.SEDS_Ver-1)*d;

    //-running NLOpt--------------------------------------------------------------------------------------
    //double lb[2] = { -HUGE_VAL, 0 }; // lower bounds
    nlopt::opt opt(Options.optimizationType, nPar); // algorithm and dimensionality
    //nlopt_set_lower_bounds(opt, lb);
    opt.set_min_objective(NLOpt_Compute_J, this);

    //nlopt_add_inequality_constraint(opt, myconstraint, &data[0], 1e-8);
    //nlopt_add_inequality_constraint(opt, myconstraint, &data[1], 1e-8);

    opt.set_xtol_rel(Options.tol_stopping);
    opt.set_maxeval(Options.max_iter);

    std::vector<double> vec_tol(nCtr);
    for (int i=0; i<nCtr; i++)
        vec_tol[i] = Options.tol_stopping;

    opt.add_inequality_mconstraint(NLOpt_Constraint, this, vec_tol);

    double minf; // the minimum objective value, upon return

    std::vector<double> p_std(nPar);
    for (int i=0; i<nPar; i++)
        p_std[i] = p[i];

    double min0 = Compute_J(p);

    int nlopt_result = opt.optimize(p_std, minf);
    if ( nlopt_result < 0) {
        printf("nlopt failed!\n");
    }
    else {
        for (int i=0; i<nPar; i++)
            p[i] = p_std[i];
        printf("Decrease the objective function from %0.10g to %0.10g\n", min0, minf);
    }
    printf("exit flag = %d\n", nlopt_result);
    //--------------------------------------------------------------------------------------------


    //forming Priors, Mu, and Sigma from parameters p
    if (Options.objective)
        Parameters_2_GMM_Likelihood(p);
    else{
        Parameters_2_GMM_MSE(p);
        Priors /= Priors.Sum();
        for (int k=0;k<K;k++){
            for (int i=0;i<d;i++){
                for (int j=0;j<d;j++){
                    Sigma[k](i,j) = Sigma_x[k](i,j);
                    Sigma[k](i+d,j) = Sigma_xdx[k](i,j);
                    Sigma[k](j,i+d) = Sigma_xdx[k](i,j); // corrected version from Manuel Muehlig
                    //Sigma[k](j,i+d) = Sigma_xdx[k](j,i);
                }
            }
        }
    }

    CheckConstraints(A);
    return true;
}

/* This function computes the sensitivity of Cost function w.r.t. optimization parameters.
 * The result is saved in the Vector dJ. The returned value of function is J.
 * Don't mess with this function. Very sensitive and a bit complicated!
*/
double SEDS::Compute_J(Vector pp, Vector& dJ) //compute the objective function and derivative w.r.t parameters (updated in vector dJ)
{
    double J = Compute_J(pp); //computing the cost function

    int counter_mu = Options.perior_opt*K; //the index at which mu should start
    int counter_sigma = counter_mu + Options.mu_opt*K*d; //the index at which sigma should start
    int counter_A = counter_sigma + Options.sigma_x_opt*K*d*(d+1)/2; //the index at which A should start


    for(int i=0; i<d; i++)
        tmp_A(i,i) = 1;

    double det_term;
    double term,sum;
    Vector dJ_dMu_k(d);
    int ind_start,ind_max_col;

    if (Options.sigma_x_opt){
        ind_start = 0;
        ind_max_col = 2*d;
    }else{
        ind_start = d;
        ind_max_col = d;
    }

    dJ.Zero();
    for(int k=0; k<K; k++){
        //sensitivity wrt Priors
        if (Options.perior_opt && Options.objective){ //likelihood
            dJ[k] = -exp(-pp[k])*Priors[k]*sum_dp[k];
            /*
            h[k] = Pxi[k]*Priors[k]/Pxi_Priors;
            dJ(k)=-exp(pp(k))/Priors[k]*((h[k]-Priors[k]).Sum());
            */
        }else if (!Options.objective){
            sum = 0;
            double tmp_dbl;
            h_tmp[k].Zero();
            tmp_mat = A[k]*X;
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
            if (Options.perior_opt)
                dJ[k] = exp(-pp[k])*Priors[k]*sum;
            /*
                h_tmp[k] = h[k]^(((A[k]*X-Xd_hat)^(Xd_hat-Xd)).SumRow()); //This vector is common in all dJ computation.
                Thus, I defined it as a variable to save some computation power
                dJ(k)= h_tmp[k].Sum();	//derivative of priors(k) w.r.t. p(k)
                */
        }

        if (Options.mu_opt)
        {
            if (Options.objective){ //likelihood
                tmp_A.InsertSubMatrix(0,d,A[k].Transpose(),0,d,0,d); // eq to Matlab [eye(2) A(:,:,i)']
                dJ_dMu_k=-((tmp_A*invSigma[k])*tmpData[k])*h[k];
                dJ.InsertSubVector(counter_mu,dJ_dMu_k,0,d);
                counter_mu += d;
            }
            else{ //mse
                tmp_mat = invSigma_x[k]*tmpData[k];
                REALTYPE *p_tmp_mat = tmp_mat.Array();
                REALTYPE *p_dJ = &dJ(counter_mu);

                for (int i=0; i<d; i++){
                    REALTYPE *p_h_tmp = h_tmp[k].Array();
                    for (int j=0; j<nData; j++)
                        *p_dJ += *(p_tmp_mat++) * (*(p_h_tmp++));
                    p_dJ++;
                    counter_mu++;
                }

                /*
                dJ_dMu_k = (tmpData[k]*invSigma_x[k]).Transpose()*h_tmp[k];
                dJ.InsertSubVector(K+k*d,dJ_dMu_k,0,d);
                */
            }
        }

        //sensitivity wrt sigma
        if (Options.objective) //likelihood
            det_term = (detSigma(k)<0) ? -1:1; //det_term=sign(det_term)
        else
            det_term = (detSigma_x(k)<0) ? -1:1; //det_term=sign(det_term)

        if (Options.objective){ //likelihood
            for (int i=0;i<ind_max_col;i++){
                int j = (Options.sigma_x_opt) ? i:d;
                while (j<2*d){
                    rSrs.Zero();
                    rSrs(j,i)=1;
                    rSrs = rSrs*L[k].Transpose() + L[k]*rSrs.Transpose();

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
                    dJ(counter_sigma) = sum;
                    counter_sigma++;
                    j++;

                    /*
                    Vector tmp_vec = (((invSigma[k]*(rSrs*invSigma[k]))*tmpData[k])^tmpData[k]).SumRow();

                    dJ(K+K*d+k*d*(2*d+1)+i_c-1) = ( tmp_vec*0.5 +
                                                  (-0.5)*det_term*(invSigma[k]*rSrs).Trace() + //derivative with respect to det Sigma which is in the numenator
                                                  tmpData[k].Transpose()*(invSigma[k].GetMatrix(0,2*d-1,d,2*d-1)*rAvrs)) //since Mu_xi_d = A*Mu_xi, thus we should consider its effect here
                                                  *h[k]*(-1);
                    */
                }
            }
        }else{ //mse
            for (int i=0;i<d;i++){
                for (int j=0;j<d;j++){
                    if (Options.sigma_x_opt && j>=i){
                        rSrs.Zero();
                        rSrs(j,i)=1;
                        rSrs = rSrs*L_x[k].Transpose() + L_x[k]*rSrs.Transpose();

                        tmp_mat = (invSigma_x[k]*rSrs*invSigma_x[k])*tmpData[k];
                        double tmp_dbl = -(invSigma_x[k]*rSrs).Trace();

                        REALTYPE *p_tmp_mat = tmp_mat.Array();
                        REALTYPE *p_tmpData = tmpData[k].Array();
                        sum = 0;

                        for (int ii=0; ii<d; ii++){
                            REALTYPE *p_h_tmp = h_tmp[k].Array();
                            for (int jj=0; jj<nData; jj++){
                                sum +=  (*p_h_tmp++) * ((*p_tmp_mat++) * (*p_tmpData++) //derivative w.r.t. Sigma in exponential
                                                        + (ii == 0)*tmp_dbl); //derivative with respect to det Sigma which is in the numenator

                                //the above term (i==0) is just to sum temp_dbl once
                            }
                        }
                        dJ(counter_sigma) = 0.5*sum;
                        counter_sigma++;

                        /*
                        dJ(K+K*d+k*d*(2*d+1)+i_c-1) =  0.5*(
                                                        ((((invSigma_x[k]*rSrs*invSigma_x[k]*tmpData[k])^tmpData[k]).SumRow()  + //derivative w.r.t. Sigma in exponential
                                                        -det_term*(invSigma_x[k]*rSrs.GetMatrix(0,d-1,0,d-1)).Trace())^h_tmp[k]/2) //derivative with respect to det Sigma which is in the numenator
                                                         ).Sum();
                        */
                    }

                    sum = 0;
                    rSrs.Zero();
                    rSrs(j,i)=1;
                    tmp_mat = rSrs*X;
                    REALTYPE *p_tmp_mat = tmp_mat.Array();
                    REALTYPE *p_Xd_hat = Xd_hat.Array();
                    REALTYPE *p_Xd = Xd.Array();

                    for (int ii=0; ii<d; ii++){
                        REALTYPE *p_h = h[k].Array();
                        for (int jj=0; jj<nData; jj++){
                            sum += *(p_h++) * (*(p_tmp_mat++)) * (*(p_Xd_hat++) - *(p_Xd++));
                        }
                    }
                    dJ(counter_A) = sum;
                    counter_A++;
                    // dJ(counter_A) = sum(sum((rSrs*x).*dJdxd).*h(:,k)');  %derivative of A

                }
            }
        }
    }

    dJ /= nData;
    return J;
}

/* This function computes the sensitivity of Cost function w.r.t. optimization parameters.
 * The result is saved in the Vector dJ. The returned value of function is J.
 * Don't mess with this function. Very sensitive and a bit complicated!
*/
double SEDS::Compute_J(Vector pp){

    double J = 0;
    if (Options.objective){
        Parameters_2_GMM_Likelihood(pp);
    }else{
        Parameters_2_GMM_MSE(pp);
    }

    //computing likelihood:
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
            Pxi[k][j] = exp(-0.5*prob[j])/tmp_den;
            Pxi_Priors[j] += Pxi[k][j]*Priors[k];
        }
        */
    }

    //computing GMR
    if (Options.objective){ //likelihood
        for (int k=0; k<K; k++){
            REALTYPE *p_h = h[k].Array();
            REALTYPE *p_Pxi = Pxi[k].Array();
            REALTYPE *p_Pxi_Priors = Pxi_Priors.Array();
            sum_dp[k] = 0;

            for (int j=0; j<nData; j++){
                *p_h = (*p_Pxi++)/(*p_Pxi_Priors++)*Priors[k];
                sum_dp[k] += (*p_h++) - Priors[k];
            }
        }
    }else{
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
    J /= nData;
    return J;
}

/* Computing the stability constraints and their derivatives */
void SEDS::Compute_Constraints(Vector &c, Matrix &dc, bool used_for_penalty){
    int i1_tmp;
    double tmp_detB, term;
    int counter_sigma = Options.perior_opt*K + Options.mu_opt*K*d; //the index at which sigma should start
    int counter_A = counter_sigma + Options.sigma_x_opt*K*d*(d+1)/2; //the index at which A should start
    int counter_C = counter_A + K*d*d; //the index at which C_Lyapunov should start
    if (Options.objective) //likelihood
        counter_C += Options.sigma_x_opt*K*d*(d+1)/2; //because likelihood has more variable than mse

    c.Zero();
    dc.Zero();

    int ind_start,ind_max_col;
    if (Options.sigma_x_opt){
        ind_start = 0;
        ind_max_col = 2*d;
    }else{
        ind_start = d;
        ind_max_col = d;
    }

    if (Options.constraintCriterion){ //Principal Minor
        //constraints
        for (int k=0; k<K; k++)//for all states
        {
            i1_tmp = 1;
            B = C_Lyapunov*A[k]+A[k].Transpose()*C_Lyapunov; //define B

            //computing the constraints (this part is basicly an exact rewrite of the matlab version)
            for (int i=0; i<d; i++) //for all dimensions
            {
                B_Inv[i]=B.GetMatrix(0,i,0,i).Inverse(&tmp_detB);//get inverse and determinants of minors

                if (!used_for_penalty || i1_tmp*tmp_detB+Options.eps_margin > 0)
                    c(k*d+i) = i1_tmp*tmp_detB + Options.eps_margin;

                //computing the sensitivity of the constraints to the parameters
                if (Options.objective){ //likelihood
                    int i_c = Options.sigma_x_opt*k*d*(d+1) + k*d*d;
                    for (int ii=0;ii<ind_max_col;ii++){
                        int jj = (Options.sigma_x_opt) ? ii:d;
                        while (jj<2*d){
                            rSrs.Zero();
                            rSrs(jj,ii) = 1;
                            rSrs = rSrs*L[k].Transpose() + L[k]*rSrs.Transpose();
                            rArs = (-A[k] * rSrs.GetMatrix(0,d-1,0,d-1) + rSrs.GetMatrix(d,2*d-1,0,d-1)) * invSigma_x[k];
                            rBrs = C_Lyapunov*rArs + rArs.Transpose()*C_Lyapunov;

                            if (i==0)
                                term = rBrs(0,0);
                            else
                                term = (B_Inv[i]*rBrs.GetMatrix(0,i,0,i)).Trace()*tmp_detB;

                            dc(k*d+i, counter_sigma + i_c) = i1_tmp*term;

                            if (Options.SEDS_Ver == 2 && jj>=d && ii<d){
                                //derivative with respect to the Lyapunov stuffs
                                rArs.Zero(); //in fact it is rCrc, but to avoid defining an extra variable, I used rArs
                                rArs(ii,jj-d) = 1;
                                rBrs = rArs*A[k]+A[k].Transpose()*rArs;

                                if (i==0)
                                    term = rBrs(0,0);
                                else
                                    term = (B_Inv[i]*rBrs.GetMatrix(0,i,0,i)).Trace()*tmp_detB;

                                dc(k*d+i,counter_C + ii*d+(jj-d)) = i1_tmp*term;
                            }

                            jj++;
                            i_c++;
                        }
                    }
                }else{ //mse
                    for (int ii=0; ii<=i; ii++){
                        for (int jj=0; jj<=i; jj++){
                            rArs.Zero();
                            rArs(jj,ii) = 1;
                            rBrs = C_Lyapunov*rArs + rArs.Transpose()*C_Lyapunov;

                            if (i==0)
                                term = rBrs(0,0);
                            else
                                term = (B_Inv[i]*rBrs.GetMatrix(0,i,0,i)).Trace()*tmp_detB;

                            dc(k*d+i,counter_A + k*d*d + ii*d + jj) = i1_tmp*term;

                            if (Options.SEDS_Ver == 2){
                                //derivative with respect to the Lyapunov stuffs
                                rArs.Zero(); //in fact it is rCrc, but to avoid defining an extra variable, I used rArs
                                rArs(ii,jj) = 1;
                                rBrs = rArs*A[k]+A[k].Transpose()*rArs;

                                if (i==0)
                                    term = rBrs(0,0);
                                else
                                    term = (B_Inv[i]*rBrs.GetMatrix(0,i,0,i)).Trace()*tmp_detB;

                                dc(k*d+i,counter_C + ii*d+jj) = i1_tmp*term;
                            }
                        }
                    }
                }
                i1_tmp *= -1; //used to alternate sign over iterations
            }
        }
    }else{ //eigenvalue
        Vector eigVal(d), eigVal_new(d);
        Matrix M_tmp(d,d),eigVec(d,d);

        for (int k=0; k<K; k++)//for all states
        {
            B = C_Lyapunov*A[k]+A[k].Transpose()*C_Lyapunov; //define B
            B.EigenValuesDecomposition(eigVal,eigVec,100);
            eigVal.Sort();
            for (int i=0; i<d;i++){
                if (!used_for_penalty || eigVal[i] + Options.eps_margin>0)
                    c(k*d + i) = eigVal[i] + Options.eps_margin;
            }

            if (Options.objective){ //likelihood
                int i_c = Options.sigma_x_opt*k*d*(d+1) + k*d*d;
                for (int ii=0;ii<ind_max_col;ii++){
                    int jj = (Options.sigma_x_opt) ? ii:d;
                    while (jj<2*d){
                        rSrs = L[k];
                        rSrs(jj,ii) += Options.delta;
                        rSrs = rSrs*(rSrs.Transpose());
                        M_tmp = rSrs.GetMatrix(d,2*d-1,0,d-1)*rSrs.GetMatrix(0,d-1,0,d-1).Inverse();
                        B = C_Lyapunov*M_tmp + M_tmp.Transpose()*C_Lyapunov;
                        B.EigenValuesDecomposition(eigVal_new,eigVec,100);
                        eigVal_new.Sort();
                        eigVal_new -= eigVal;
                        eigVal_new /= Options.delta;

                        for (int i=0; i<d;i++)
                            dc(k*d+i,counter_sigma + i_c) = eigVal_new[i];


                        if (Options.SEDS_Ver == 2 && jj>=d && ii<d){
                            //derivative with respect to the Lyapunov stuffs
                            M_tmp = C_Lyapunov;
                            M_tmp(ii,jj-d) += Options.delta;
                            M_tmp = M_tmp*A[k]+A[k].Transpose()*M_tmp;
                            M_tmp.EigenValuesDecomposition(eigVal_new,eigVec,100);
                            eigVal_new.Sort();
                            eigVal_new -= eigVal;
                            eigVal_new /= Options.delta;

                            for (int i=0; i<d;i++)
                                dc(k*d+i,counter_C + ii*d+(jj-d)) = eigVal_new[i];
                        }

                        jj++;
                        i_c++;
                    }
                }
            }else{ //mse
                for (int ii=0;ii<d;ii++){
                    for (int jj=0;jj<d;jj++){
                        M_tmp = A[k];
                        M_tmp(jj,ii) += Options.delta;
                        M_tmp = C_Lyapunov*M_tmp+M_tmp.Transpose()*C_Lyapunov;
                        M_tmp.EigenValuesDecomposition(eigVal_new,eigVec,100);
                        eigVal_new.Sort();
                        eigVal_new -= eigVal;
                        eigVal_new /= Options.delta;

                        for (int i=0; i<d;i++)
                            dc(k*d+i,counter_A) = eigVal_new[i];

                        counter_A++;

                        if (Options.SEDS_Ver == 2){
                            //derivative with respect to the Lyapunov stuffs
                            M_tmp = C_Lyapunov;
                            M_tmp(ii,jj) += Options.delta;
                            M_tmp = M_tmp*A[k]+A[k].Transpose()*M_tmp;
                            M_tmp.EigenValuesDecomposition(eigVal_new,eigVec,100);
                            eigVal_new.Sort();
                            eigVal_new -= eigVal;
                            eigVal_new /= Options.delta;

                            for (int i=0; i<d;i++)
                                dc(k*d+i,counter_C + ii*d+jj) = eigVal_new[i];
                        }
                    }
                }
            }
        }
    }


    //constraint on positive definiteness of C_Lyapunov
    if (Options.SEDS_Ver == 2){
        Vector eigVal(d), eigVal_new(d);
        Matrix M_tmp(d,d),eigVec(d,d);

        C_Lyapunov.EigenValuesDecomposition(eigVal,eigVec,100);
        eigVal.Sort();
        for (int i=0; i<d;i++){
            if (!used_for_penalty || eigVal[i] + Options.eps_margin>0)
                c(K*d + i) = -eigVal[i] + Options.eps_margin;
        }

        //derivative of the constraint on positive definiteness of C_Lyapunov
        for (int ii=0;ii<d;ii++){
            for (int jj=0;jj<d;jj++){
                M_tmp = C_Lyapunov;
                M_tmp(jj,ii) += Options.delta;
                M_tmp(ii,jj) += Options.delta;
                M_tmp.EigenValuesDecomposition(eigVal_new,eigVec,100);
                eigVal_new.Sort();
                eigVal_new -= eigVal;
                eigVal_new /= Options.delta;

                for (int i=0; i<d;i++)
                    dc(K*d+i,counter_C) = -eigVal_new[i];

                counter_C++;
            }
        }
    }
}

/* Computing the stability constraints */
void SEDS::Compute_Constraints(Vector &c){
    int i1_tmp;
    double tmp_detB;
    Vector eigVal(d);
    Matrix eigVec(d,d);
    c.Zero();

    //constraints
    for (int k=0; k<K; k++)//for all states
    {
        i1_tmp = 1;
        B = A[k]+A[k].Transpose(); //define B

        if (Options.constraintCriterion){ //principal minor
            //computing the constraints (this part is basicly an exact rewrite of the matlab version)
            for (int i=0; i<d; i++) //for all dimensions
            {
                B_Inv[i]=B.GetMatrix(0,i,0,i).Inverse(&tmp_detB);//get inverse and determinants of minors
                c(k*d+i) = i1_tmp*tmp_detB+pow(Options.tol_mat_bias,(double)(i+1)/d);
                i1_tmp *= -1; //used to alternate sign over iterations
            }
        }else{ //eigenvalue
            B.EigenValuesDecomposition(eigVal,eigVec,100);
            eigVal.Sort();
            c.SetSubVector(k*d,eigVal);
        }
    }
}

/* Transforming the GMM model into the vector of optimization's parameters.*/
bool SEDS::GMM_2_Parameters_Likelihood(Vector &p){ //this is used to transform GMM to the optimization parameters
    int counter_mu = Options.perior_opt*K; //the index at which mu should start
    int counter_sigma = counter_mu + Options.mu_opt*K*d; //the index at which sigma should start
    int counter_C = counter_sigma + K*d*d + Options.sigma_x_opt*K*d*(d+1);

    for (int k=0; k<K; k++){
        if (Options.perior_opt)
            p[k] = -log(1.0/Priors[k]-1.0); //insert Priors to p

        if (Options.mu_opt){
            for(int j=0; j<d; j++) //for all dimensions
                p(counter_mu+k*d+j)=Mu(j,k); //insert Mu
        }else{
            Mu_x[k] = Mu.GetColumn(k).GetSubVector(0,d);
        }

        Sigma[k].Cholesky(L[k]);//set L

        if (Options.sigma_x_opt){
            for(int j=0; j<2*d; j++){
                p.InsertSubVector(counter_sigma,L[k].GetColumn(j),j,2*d-j);
                counter_sigma += 2*d-j;
            }
        }else{
            for(int j=0; j<d; j++){
                p.InsertSubVector(counter_sigma,L[k].GetColumn(j),d,d);
                counter_sigma += d;
            }
        }
    }

    if (Options.SEDS_Ver == 2){
        Vector tmp;
        tmp.Set(C_Lyapunov.Array(),d*d);
        tmp /= 2;
        p.SetSubVector(counter_C,tmp);
    }
    return true;
}

/* Transforming the vector of optimization's parameters into a GMM model.*/
bool SEDS::Parameters_2_GMM_Likelihood(Vector pp){ //this is used to unpack the parameters in p after optimization, to reconstruct the
    //GMM-parameters in their ususal form: Priors, Mu and Sigma.

    double sum=0;
    Vector col(2*d); // a temporary vector needed below

    int counter_mu = Options.perior_opt*K; //the index at which mu should start
    int counter_sigma = counter_mu + Options.mu_opt*K*d; //the index at which sigma should start
    int counter_C = counter_sigma + K*d*d + Options.sigma_x_opt*K*d*(d+1);

    for (int k=0; k<K; k=k++){
        //constructing Priors
        if (Options.perior_opt){
            Priors[k] = 1.0/(1.0+exp(-pp[k])); //extract the Priors from correspondng position in optimization vector
            sum += Priors[k];
        }

        //reconstructing Sigma
        if (Options.sigma_x_opt){
            for(int j=0; j<2*d; j++){ //for all dimensions
                col.Zero();
                for(int i=j; i<2*d; i++){
                    col(i)=pp(counter_sigma);
                    counter_sigma++;
                }
                L[k].SetColumn(col, j);
            }
        }else{
            for(int j=0; j<d; j++){ //for all dimensions
                for(int i=0; i<d; i++){
                    L[k](i+d,j) = pp(counter_sigma);
                    counter_sigma++;
                }
            }
        }

        Sigma[k]=L[k]*(L[k].Transpose());

        for(int i=0; i<2*d;i++)
            Sigma[k](i,i)+=Options.tol_mat_bias;

        invSigma[k]=Sigma[k].Inverse(&detSigma(k));
        invSigma_x[k]=Sigma[k].GetMatrix(0,d-1,0,d-1).Inverse(&detSigma_x[k]);
        Sigma_xdx[k]=Sigma[k].GetMatrix(d,2*d-1,0,d-1);
        A[k]=Sigma_xdx[k]*invSigma_x[k]; //dynamical system matrix.

        if (Options.mu_opt){ //reconstructing Mu
            Mu_x[k] = pp.GetSubVector(K+k*d,d);
        }
        Mu_xd[k] = A[k]*Mu_x[k]; //proagate the centers through the dynamical system
        for (int i=0; i<d; i++)
        {
            if (Options.mu_opt){
                Mu(i,k) = Mu_x[k](i);
            }
            Mu(i+d,k) = Mu_xd[k](i);
        }
    }
    if (Options.perior_opt)
        Priors /= sum; //normalizing Priors

    if (Options.SEDS_Ver == 2){
        C_Lyapunov.Set(pp.GetSubVector(counter_C,d*d).Array(),d,d);
        C_Lyapunov = C_Lyapunov + C_Lyapunov.Transpose();
    }

    return true;
}

/* Transforming the GMM model into the vector of optimization's parameters.*/
bool SEDS::GMM_2_Parameters_MSE(Vector &p){ //this is used to transform GMM to the optimization parameters

    int counter_mu = Options.perior_opt*K; //the index at which mu should start
    int counter_sigma = counter_mu + Options.mu_opt*K*d; //the index at which sigma should start
    int counter_A = counter_sigma + Options.sigma_x_opt*K*d*(d+1)/2; //the index at which A should start
    int counter_C = counter_A + K*d*d;


    for (int k=0; k<K; k++){
        if (Options.perior_opt)
            p[k] = -log(1.0/Priors[k]-1.0); //insert Priors to p

        if (Options.mu_opt){
            for(int j=0; j<d; j++) //for all dimensions
                p(counter_mu+k*d+j)=Mu(j,k); //insert Mu
        }else{
            Mu_x[k] = Mu.GetColumn(k).GetSubVector(0,d);
        }

        Sigma_x[k] = Sigma[k].GetMatrix(0,d-1,0,d-1);
        invSigma_x[k]=Sigma_x[k].Inverse(&detSigma_x[k]);
        Sigma_xdx[k] = Sigma[k].GetMatrix(d,2*d-1,0,d-1);
        A[k] = Sigma_xdx[k]*invSigma_x[k];
        Sigma_x[k].Cholesky(L_x[k]);//set L_x

        for(int j=0; j<d; j++){
            if (Options.sigma_x_opt){
                p.InsertSubVector(counter_sigma,L_x[k].GetColumn(j),j,d-j);
                counter_sigma += d-j;
            }

            p.SetSubVector(counter_A,A[k].GetColumn(j));
            counter_A += d;
        }
    }

    if (Options.SEDS_Ver == 2){
        Vector tmp;
        tmp.Set(C_Lyapunov.Array(),d*d);
        tmp /= 2;
        p.SetSubVector(counter_C,tmp);
    }

    return true;
}

/* Transforming the vector of optimization's parameters into a GMM model.*/
bool SEDS::Parameters_2_GMM_MSE(Vector pp){ //this is used to unpack the parameters in p after optimization, to reconstruct the
    //GMM-parameters in their ususal form: Priors, Mu and Sigma.

    Vector col(d); // a temporary vector needed below
    int counter_mu = Options.perior_opt*K; //the index at which mu should start
    int counter_sigma = counter_mu + Options.mu_opt*K*d; //the index at which sigma should start
    int counter_A = counter_sigma + Options.sigma_x_opt*K*d*(d+1)/2; //the index at which A should start
    int counter_C = counter_A + K*d*d;

    for (int k=0; k<K; k=k++){
        //constructing Priors
        if (Options.perior_opt)
            Priors[k] = 1.0/(1.0+exp(-pp[k]));

        //reconstructing Sigma
        for(int j=0; j<d; j++){ //for all dimensions
            col.Zero();
            for(int i=0; i<d; i++){
                if (i>=j && Options.sigma_x_opt){
                    col(i)=pp(counter_sigma);
                    counter_sigma++;
                }
                A[k](i,j) = pp(counter_A);
                counter_A++;
            }
            if (Options.sigma_x_opt)
                L_x[k].SetColumn(col, j);
        }

        if (Options.sigma_x_opt){
            Sigma_x[k]=L_x[k]*(L_x[k].Transpose());
            for(int i=0; i<d;i++)
                Sigma_x[k](i,i)+=Options.tol_mat_bias;
            invSigma_x[k]=Sigma_x[k].Inverse(&detSigma_x[k]);
        }

        Sigma_xdx[k] = A[k]*Sigma_x[k];

        if (Options.mu_opt)
            Mu_x[k] = pp.GetSubVector(counter_mu+k*d,d); //reconstructing Mu

        Mu_xd[k] = A[k]*Mu_x[k]; //proagate the centers through the dynamical system
        for (int i=0; i<d; i++)
        {
            if (Options.mu_opt)
                Mu(i,k) = Mu_x[k](i);
            Mu(i+d,k) = Mu_xd[k](i);
        }
    }

    if (Options.SEDS_Ver == 2){
        C_Lyapunov.Set(pp.GetSubVector(counter_C,d*d).Array(),d,d);
        C_Lyapunov = C_Lyapunov + C_Lyapunov.Transpose();
    }
    return true;
}

/* checking if every thing goes well. Sometimes if the parameter
 * 'Options.cons_penalty' is not big enough, the constrains may be violated.
 * Then this function notifies the user to increase 'Options.cons_penalty'.
*/
bool SEDS::CheckConstraints(Matrix * A){
    Vector eigvals;
    Matrix eigvects;
    Matrix B(d,d);
    c.Zero();
    int nCtrViolated = 0;
    QString str("");

    for(int k=0; k<K; k++)
    {
        B = C_Lyapunov*A[k]+A[k].Transpose()*C_Lyapunov;
        B.EigenValuesDecomposition(eigvals,eigvects,100);
        for(int i=0; i<d; i++){
            if(eigvals(i) > 0){
                if (nCtrViolated == 0)
                {
                    cout << endl;
                    cout<<"Optimization did not finish successfully. Some constraints were violated."<<endl;
                    cout<<"The error may be due to change of hard constraints to soft constrints."<<endl;
                    cout<<"To handle this error, increase the value of 'cons_penalty' and re-run the"<<endl;
                    cout<<"optimization. Output error for debugging purpose:"<<endl<<endl;
                    str.sprintf("%s","Optimization did not finish successfully. Some constraints were violated. "
                                "The error may be due to change of hard constraints to soft constrints. "
                                "To handle this error, increase the value of 'Constraint Penalty' and re-run the "
                                "optimization.\nOutput error for debugging purpose:");
                }
                cout<< "k = " << k << "  ;  err = " << eigvals(i) << endl;
                nCtrViolated++;
                str.sprintf("%s %.3f",str.toStdString().c_str(),eigvals(i));
            }
        }
    }

    C_Lyapunov.Print();
    C_Lyapunov.EigenValuesDecomposition(eigvals,eigvects,100);
    for(int i=0; i<d; i++){
        if(eigvals(i) < 0){
            if (nCtrViolated == 0)
            {
                cout << endl;
                cout<<"Optimization did not finish successfully. Some constraints were violated."<<endl;
                cout<<"The error may be due to change of hard constraints to soft constrints."<<endl;
                cout<<"To handle this error, increase the value of 'cons_penalty' and re-run the"<<endl;
                cout<<"optimization. Output error for debugging purpose:"<<endl<<endl;
                str.sprintf("%s","Optimization did not finish successfully. Some constraints were violated. "
                            "The error may be due to change of hard constraints to soft constrints. "
                            "To handle this error, increase the value of 'Constraint Penalty' and re-run the "
                            "optimization.\nOutput error for debugging purpose:");
            }
            cout<< "C_Lyapunov ;  err = " << eigvals(i) << endl;
            nCtrViolated++;
            str.sprintf("%s %.3f",str.toStdString().c_str(),eigvals(i));
        }
    }

    if (nCtrViolated == 0)
        cout<<"Optimization finished succesfully!"<<endl;
    else{
        QMessageBox *qmes = new QMessageBox ();
        qmes->addButton("Ok",QMessageBox::AcceptRole);
        qmes->setWindowTitle("Error!");
        str.sprintf("%s. In total %d constraints were violated.",str.toStdString().c_str(),nCtrViolated);
        qmes->setText(str);
        qmes->show();
    }

    return true;
}
