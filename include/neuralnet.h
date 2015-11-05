#ifndef _NEURALNET_HEADER__
#define _NEURALNET_HEADER__

#include "env.h"
#include <math.h>
#include <gsl/gsl_vector.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_blas.h>

#define IL_SIZE             39
#define HL_SIZE             100
#define OL_SIZE             4
#define PRED_COUNTER_BIAS   0

struct NNParams{
    gsl_matrix * theta1;
    gsl_matrix * theta2;
    gsl_vector * mu;
    gsl_vector * SE;
};
typedef struct NNParams NNParams;

void importNNParams(NNParams *, FILE *);

float predictStateValue(NNParams *, int *, int);

void destroyNNParams(NNParams *);

#endif
