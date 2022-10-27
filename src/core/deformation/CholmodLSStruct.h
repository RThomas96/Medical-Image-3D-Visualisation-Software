#ifndef CHOLMODLSSTRUCT_H
#define CHOLMODLSSTRUCT_H

#include <QTime>
//#include "cholmod.h"
#include <suitesparse/cholmod.h>
#include <QTime>

class CholmodLSStruct
{
    // cholmod stuff:
    cholmod_common _c;
    cholmod_triplet *_triplet;

    int *_rowPtrA;
    int *_colPtrA;
    double *_valuePtrA;

    cholmod_sparse *_At;
    cholmod_factor *_L;
    cholmod_sparse* _A;
    cholmod_dense* _Atb;
    cholmod_sparse* _AtA;

    cholmod_dense *_b;
    double *_valuePtrB;

    cholmod_dense* _x;
    double * _x_data;

    int _nb_non_zeros_in_A , _Arows , _Acols , _bcols;
    // b has _Arows rows , and _bcols columns;

    bool allocated , factorized;

private:
    inline void resetDimensions()
    {
        setDimensions( -1 , -1 , -1 );
    }
    inline void resetNNZinA()
    {
        _nb_non_zeros_in_A = -1;
    }

public:
    CholmodLSStruct() : _nb_non_zeros_in_A(-1) , _Arows(-1) , _Acols(-1) , _bcols(-1) , allocated(false) , factorized(false) {}


    void start()
    {
        // create A and factorize it:
        cholmod_start(&_c);
    }
    void setDimensions( int rA , int cA , int cB )
    {
        _Arows = rA;
        _Acols = cA;
        _bcols = cB;
    }
    void specifyNonZeroInA( int nnz )
    {
        _nb_non_zeros_in_A = nnz;
    }

    void addValueInA( int r , int c , double v )
    {
        const int i = _triplet->nnz;
        _rowPtrA[i] = r;
        _colPtrA[i] = c;
        _valuePtrA[i] = v;
        _triplet->nnz++;
    }
    void setValueInB( int r , int c , double v )
    {
        // Cholmod specs : Entry in row i and column j is located in x [i+j*d]
        _valuePtrB[ r + c * _Arows ] = v;
    }

    void allocate()
    {
        if( allocated ) return;

        _triplet = cholmod_allocate_triplet(
                    _Arows ,
                    _Acols ,
                    _nb_non_zeros_in_A ,
                    0, CHOLMOD_REAL, &_c);

        _rowPtrA = (int*)_triplet->i;
        _colPtrA = (int*)_triplet->j;
        _valuePtrA = (double*)_triplet->x;

        _b = cholmod_zeros(_Arows, _bcols, CHOLMOD_REAL, &_c);
        _valuePtrB = (double*)_b->x;

        _Atb = cholmod_allocate_dense(_Acols, _bcols, _bcols * _Acols, CHOLMOD_REAL, &_c);

        allocated = true;
        factorized = false;
    }
    void free()
    {
        if( !allocated ) return;

        cholmod_free_triplet(&_triplet,&_c);

        if( factorized )
            cholmod_free_sparse(&_A,&_c);

        cholmod_free_sparse(&_At,&_c);
        cholmod_free_sparse(&_AtA, &_c);
        cholmod_free_factor(&_L,&_c);

        cholmod_free_dense(&_b,&_c);

        allocated = false;
        factorized = false;

        resetDimensions();
        resetNNZinA();
    }

    void freeSolution()
    {
        cholmod_free_dense(&_x, &_c);
        _x_data = NULL;
    }

    double getSolutionValue( int r , int c )
    {
        return _x_data[r + c * _Acols];
    }

    void factorize()
    {
        cholmod_print_triplet (_triplet, "triplet", &_c);

        // Careful : in this function, there is an allocation, you need to take care of the memory
        _A   = cholmod_triplet_to_sparse(_triplet, _triplet->nnz, &_c);

        _At  = cholmod_transpose(_A, 1, &_c);
        _AtA = cholmod_ssmult(_At, _A, 0, 1, 1, &_c);
        _AtA->stype = 1;
        _L = cholmod_analyze(_AtA, &_c);
        cholmod_factorize(_AtA, _L, &_c);

        factorized = true;
    }
    void solve( bool coutResult = true )
    {
        QTime timer_solve;
        timer_solve.start();

        double alpha[] = {1, 1};
        double beta[] = {0, 0};
        cholmod_sdmult(_At, 0, alpha, beta, _b, _Atb, &_c);
        _x = cholmod_solve(CHOLMOD_A, _L, _Atb, &_c);

        if( coutResult )
            std::cout << "cholmod solve took " << timer_solve.elapsed() << " ms" << std::endl;

        // get result:
        _x_data = (double*)(_x->x);
        // Do not forget to free x each time!
    }

    // you should start your structure at the beginning of your program,
    // then allocate once and for all with the correct _rows, _cols and _nb_non_zeros_in_A,
    // then once set the values in A, and factorize it,
    // then set each time the values in b, and solve it, and free the result each time
};

#endif // CHOLMODLSSTRUCT_H
