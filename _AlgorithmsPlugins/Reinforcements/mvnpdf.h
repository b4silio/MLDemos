#ifndef MVNPDF_H_
#define MVNPDF_H_

#include <public.h>
#include <mymaths.h>
#include <fgmm/fgmm++.hpp>

float mvnPdf(fvec query, fvec mean, fvec sigma)
{
	// we generate the new data
	gaussian gauss;
	gaussian_init(&gauss,2);
	gauss.mean[0] = mean[0];
	gauss.mean[1] = mean[1];
	gauss.covar->_[0] = sigma[0];
	gauss.covar->_[1] = sigma[1];
	gauss.covar->_[2] = sigma[3];
	invert_covar(&gauss);
	float value = gaussian_pdf(&gauss, &query[0]);
	gaussian_free(&gauss);
	return value;
}

fvec mvnRandN(fvec mean, fvec sigma)
{
	// we generate the new data
	gaussian gauss;
	gaussian_init(&gauss,2);
	gauss.mean[0] = mean[0];
	gauss.mean[1] = mean[1];
	gauss.covar->_[0] = sigma[0];
	gauss.covar->_[1] = sigma[1];
	gauss.covar->_[2] = sigma[3];
	//invert_covar(&gauss);
	smat_cholesky(gauss.covar, gauss.covar_cholesky);
	fvec newSample;
	newSample.resize(2,0);
	gaussian_draw(&gauss, &newSample[0]);
	gaussian_free(&gauss);
	return newSample;
}

#endif /*MVNDPF_H_*/
