#include "eigen_pca.h"

int main(int, char *[])
{
	unsigned int dimSpace = 10; // dimension space
	unsigned int m = 100;   // dimension of each point
	unsigned int n = 100;  // number of points 
	// this is a toy example, set some randomness in the origanl point space
	MatrixXd dataPoints = MatrixXd::Random(m, n);  // matrix (m x n)
	PCA p;

	p.kernel_pca(dataPoints, dimSpace);

	std::cout << p.get();

	std::string a;
	std::cin >> a;
};
