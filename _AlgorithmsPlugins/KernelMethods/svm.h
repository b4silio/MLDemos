/*********************************************************************
Copyright (c) 2000-2010 Chih-Chung Chang and Chih-Jen Lin
All rights reserved.
Code Modified in 2010 by Basilio Noris

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:

1. Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.

3. Neither name of copyright holders nor the names of its contributors
may be used to endorse or promote products derived from this software
without specific prior written permission.


THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*********************************************************************/
#ifndef _LIBSVM_H
#define _LIBSVM_H

#ifdef __cplusplus
extern "C" {
#endif

struct svm_node
{
	int index;
	double value;
};

struct svm_problem
{
	int l;
	double *y;
	struct svm_node **x;
};

enum { C_SVC, NU_SVC, ONE_CLASS, EPSILON_SVR, NU_SVR };	/* svm_type */
enum { LINEAR, POLY, RBF, SIGMOID, RBFWEIGH, RBFWMATRIX, PRECOMPUTED }; /* kernel_type */

struct svm_parameter
{
	int svm_type;
	int kernel_type;
	int degree;				/* for poly */
	double gamma;			/* for poly/rbf/sigmoid */
	double coef0;			/* for poly/sigmoid */
	double* kernel_weight;	/* for rbfweight/rbfwmatrix */
	int kernel_dim;			/* for rbfweight/rbfwmatrix */
	bool normalizeKernel;
	double kernel_norm;

	/* these are for training only */
	double cache_size;		/* in MB */
	double eps;				/* stopping criteria */
	double C;				/* for C_SVC, EPSILON_SVR and NU_SVR */
	int nr_weight;			/* for C_SVC */
	int *weight_label;		/* for C_SVC */
	double* weight;			/* for C_SVC */
	double nu;				/* for NU_SVC, ONE_CLASS, and NU_SVR */
	double p;				/* for EPSILON_SVR */
	int shrinking;			/* use the shrinking heuristics */
	int probability;		/* do probability estimates */
};

//
// svm_model
//
struct svm_model
{
	svm_parameter param;	// parameter
	int nr_class;			// number of classes, = 2 in regression/one class svm
	int l;					// total #SV
	svm_node **SV;			// SVs (SV[l])
	double **sv_coef;		// coefficients for SVs in decision functions (sv_coef[k-1][l])
	double *rho;			// constants in decision functions (rho[k*(k-1)/2])
	double *probA;			// pairwise probability information
	double *probB;
	double *eps;

	// for classification only

	int *label;				// label of each class (label[k])
	int *nSV;				// number of SVs for each class (nSV[k])
							// nSV[0] + nSV[1] + ... + nSV[k-1] = l
	// XXX
	int free_sv;			// 1 if svm_model is created by svm_load_model
							// 0 if svm_model is created by svm_train
};


typedef float Qfloat;
typedef signed char schar;
//
// Kernel evaluation
//
// the static method k_function is for doing single kernel evaluation
// the constructor of Kernel prepares to calculate the l*l kernel matrix
// the member function get_Q is for getting one column from the Q Matrix
//
class Q_Matrix {
public:
	virtual Qfloat *get_Q(int column, int len) const = 0;
	virtual Qfloat *get_QD() const = 0;
	virtual void swap_index(int i, int j) const = 0;
	virtual ~Q_Matrix() {}
};

//
// Kernel Cache
//
// l is the number of total data items
// size is the cache size limit in bytes
//
class Cache
{
public:
	Cache(int l,long int size);
	~Cache();

	// request data [0,len)
	// return some position p where [p,len) need to be filled
	// (p >= len if nothing needs to be filled)
	int get_data(const int index, Qfloat **data, int len);
	void swap_index(int i, int j);	// future_option
private:
	int l;
	long int size;
	struct head_t
	{
		head_t *prev, *next;	// a cicular list
		Qfloat *data;
		int len;		// data[0,len) is cached in this entry
	};

	head_t *head;
	head_t lru_head;
	void lru_delete(head_t *h);
	void lru_insert(head_t *h);
};

class Kernel: public Q_Matrix {
public:
	Kernel(int l, svm_node * const * x, const svm_parameter& param);
	virtual ~Kernel();

	static double k_function(const svm_node *x, const svm_node *y,
		const svm_parameter& param);
	virtual Qfloat *get_Q(int column, int len) const = 0;
	virtual Qfloat *get_QD() const = 0;
	virtual void swap_index(int i, int j) const;

protected:

	double (Kernel::*kernel_function)(int i, int j) const;

private:
	const svm_node **x;
	double *x_square;
	double *kernel_weight;
	int dim;

	// svm_parameter
	const int kernel_type;
	const int degree;
	const double gamma;
	const double coef0;
	double kernel_norm;

	static double dot(const svm_node *px, const svm_node *py);
	static double dot(const svm_node *px, const svm_node *py, const double *weight);
	static double matrix(const svm_node *px, const svm_node *py, const double *W, int dim);
	double kernel_linear(const int i, const int j) const;
	double kernel_poly(const int i, const int j) const;
	double kernel_rbf(const int i, const int j) const;
	double kernel_rbf_weight(const int i, const int j) const;
	double kernel_rbf_w(const int i, const int j) const;
	double kernel_sigmoid(const int i, const int j) const;
	double kernel_precomputed(const int i, const int j) const;
};

struct	svm_model *svm_train(const struct svm_problem *prob, const struct svm_parameter *param);
void		svm_cross_validation(const struct svm_problem *prob, const struct svm_parameter *param, int nr_fold, double *target);
void		svm_leave_one_in(const struct svm_problem *prob, const struct svm_parameter *param, int nr_fold, double *errors);
void		svm_leave_one_out(const struct svm_problem *prob, const struct svm_parameter *param, int nr_fold, double *errors);

int		svm_save_model(const char *model_file_name, const struct svm_model *model);
struct	svm_model *svm_load_model(const char *model_file_name);

int		svm_save_model_binary(const char *model_file_name, const struct svm_model *model);
struct	svm_model *svm_load_model_binary(const char *model_file_name);

int		svm_get_svm_type(const struct svm_model *model);
int		svm_get_nr_class(const struct svm_model *model);
void		svm_get_labels(const struct svm_model *model, int *label);
double	svm_get_svr_probability(const struct svm_model *model);
double svm_get_dual_objective_function(const struct svm_model *svm);

void		svm_predict_values(const struct svm_model *model, const struct svm_node *x, double* dec_values);
double	svm_predict(const struct svm_model *model, const struct svm_node *x);
void		svm_predict_votes(const struct svm_model *model, const struct svm_node *x, double *votes);
double	svm_predict_probability(const struct svm_model *model, const struct svm_node *x, double* prob_estimates);

void		svm_destroy_model(struct svm_model *model);
void		svm_destroy_param(struct svm_parameter *param);

const char *svm_check_parameter(const struct svm_problem *prob, const struct svm_parameter *param);
int		svm_check_probability_model(const struct svm_model *model);

#ifdef __cplusplus
}
#endif

#endif /* _LIBSVM_H */
