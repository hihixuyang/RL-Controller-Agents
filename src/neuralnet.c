#include "neuralnet.h"

void importNNParams(NNParams * params, FILE * fd)
{
    int state = 1;
    float value;
    char ch;
    int i, j;

    params->theta1  = gsl_matrix_alloc(HL_SIZE, IL_SIZE+1);
    params->theta2  = gsl_matrix_alloc(OL_SIZE, HL_SIZE+1);
    params->mu      = gsl_vector_alloc(IL_SIZE);
    params->SE      = gsl_vector_alloc(IL_SIZE);

    while (!feof(fd) && state <= 4){
        ch = fgetc(fd);

        if (ch == '#'){
            while (fgetc(fd) != '\n');
            continue;
        }
        if (ch == '\n')
            continue;
        if (ch == ' '){
            /* found start of matrix */
            switch (state){
                case 1  :    // Theta 1
                    for (i=0; i<HL_SIZE; i++){
                        for (j=0; j<IL_SIZE+1; j++){
                            fscanf(fd, "%f", &value);
                            gsl_matrix_set(params->theta1, i, j, value);
                            fgetc(fd);
                        }
                        fgetc(fd);
                    }
                    break;
                case 2  :   // Theta 2
                    for (i=0; i<OL_SIZE; i++){
                        for (j=0; j<HL_SIZE+1; j++){
                            fscanf(fd, "%f", &value);
                            gsl_matrix_set(params->theta2, i, j, value);
                            fgetc(fd);
                        }
                        fgetc(fd);
                    }
                    break;
                case 3  :   // mu
                    for (i=0; i<IL_SIZE; i++){
                        fscanf(fd, "%f", &value);
                        gsl_vector_set(params->mu, i, value);
                        fgetc(fd);
                    }
                    break;
                case 4  :   // SE
                    for (i=0; i<IL_SIZE; i++){
                        fscanf(fd, "%f", &value);
                        gsl_vector_set(params->SE, i, value);
                        fgetc(fd);
                    }
                    break;
            }

            state++;
        }
    }
}

float predictStateValue(NNParams * params, int * state, int print)
{
    gsl_vector * x, * h1a, * h1b, *h2;
    float value, sigmoid;
    int maxChoice, i;

    // Neural Network Forward-propogation alg.

    // printf("allocating vectors ...\n"); fflush(stdout);
    x   = gsl_vector_alloc(IL_SIZE+1);
    h1a = gsl_vector_alloc(HL_SIZE);
    h1b = gsl_vector_alloc(HL_SIZE+1);
    h2  = gsl_vector_alloc(OL_SIZE);

    // x = (state - mu) / SE
    // printf("x = (state - mu) / SE ...\n"); fflush(stdout);
    for (i=0; i<IL_SIZE; i++)
        gsl_vector_set(x, i+1, (state[i] - gsl_vector_get(params->mu, i)) / gsl_vector_get(params->SE, i));
    gsl_vector_set(x, 0, 1.0);

    // h1a = Theta1*x
    // printf("h1a = Theta1*x ...\n"); fflush(stdout);
    if (gsl_blas_dgemv(CblasNoTrans, 1.0, params->theta1, x, 0.0, h1a) != 0){
        printf("matrix-vector product error! [h1]\n"); fflush(stdout);
        return 0;
    }

    // h1b = [1 sigmoid(h1a)]
    // printf("h1b = sigmoid(h1a) ...\n"); fflush(stdout);
    for (i=0; i<HL_SIZE; i++){
        sigmoid = 1.0 / (1.0 + exp(-gsl_vector_get(h1a, i)));
        gsl_vector_set(h1b, i+1, sigmoid);
    }
    gsl_vector_set(h1b, 0, 1.0);

    // h2 = Theta2*h1b
    // printf("h2 = Theta1*h1b ...\n"); fflush(stdout);
    if (gsl_blas_dgemv(CblasNoTrans, 1.0, params->theta2, h1b, 0.0, h2) != 0){
        printf("matrix-vector product error! [h2]\n"); fflush(stdout);
        return 0;
    }

    // h2 = sigmoid(h2)
    // printf("h2 = sigmoid(h2) ...\n"); fflush(stdout);
    for (i=0; i<OL_SIZE; i++){
        sigmoid = 1.0 / (1.0 + exp(-gsl_vector_get(h2, i)));
        gsl_vector_set(h2, i, sigmoid);
    }

    // DEBUG
    /*if (print){
        printf("\n");
        for (i=0; i<OL_SIZE; i++)
            printf(" %5.2f", gsl_vector_get(h2, i));
        printf("\n");
    }*/

    // maxChoice = max(h2)
    // printf("maxChoice = max(h2) ...\n"); fflush(stdout);
    maxChoice = 0;
    for (i=1; i<OL_SIZE; i++){
        if (gsl_vector_get(h2, i) > gsl_vector_get(h2, maxChoice))
            maxChoice = i;
    }

    // value is midpoint of range for respective grouping
    // printf("maxChoice --> value ...\n"); fflush(stdout);
    switch (maxChoice){
        case    0: value = -10;  break;
        case    1: value = 5; break;
        case    2: value = 12; break;
        case    3: value = 30;  break;
        default  : value = 0;   break;
    }

    //printf("freeing vectors ...\n"); fflush(stdout);
    gsl_vector_free(x);
    gsl_vector_free(h1a);
    gsl_vector_free(h1b);
    gsl_vector_free(h2);

    // add negative prediction counter-bias to avoid over-optimistic predictions
    return value > 0 ? value/1.5 : value;
}

void destroyNNParams(NNParams * params)
{
    gsl_matrix_free(params->theta1);
    gsl_matrix_free(params->theta2);
    gsl_vector_free(params->mu);
    gsl_vector_free(params->SE);
    free(params);
}
