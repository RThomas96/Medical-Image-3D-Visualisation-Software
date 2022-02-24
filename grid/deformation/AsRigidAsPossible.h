#ifndef ASRIGIDASPOSSIBLE_H
#define ASRIGIDASPOSSIBLE_H

#include "../utils/Vec3D.h"
#include "../utils/Edge.h"
#include "../utils/Triangle.h"
#include "gsl/gsl_linalg.h"

#include <suitesparse/cholmod.h>


class AsRigidAsPossible
{
public:
    AsRigidAsPossible();

    ~AsRigidAsPossible();
    void init( const std::vector<Vec3Df> & _vertices, const std::vector< Triangle > & _triangles);
    void init( const std::vector<Vec3Df> & _vertices, const std::vector< std::vector <int> > & _triangles );

    void setVertices( const std::vector<Vec3Df> & _vertices );

    inline std::vector<Vec3Df> & getVertices(){ return vertices; }
    inline const std::vector<Vec3Df> getVertices() const { return vertices; }

    inline std::vector< bool > & getHandles(){ return handles; }
    inline const std::vector< bool > & getHandles() const { return handles; }

    void setHandles(const std::vector< bool > & _handles);
    void compute_deformation(std::vector<Vec3Df> & positions);

    void setIterationNb(unsigned int itNb){ iterationNb = itNb; }
    unsigned int getIterationNb(){ return iterationNb; }

    //void draw();

    void clear();

protected:

    void compute_product_and_sum( gsl_matrix * R, const Vec3Df & point, Vec3Df & result );
    void compute_S( gsl_matrix * S , unsigned int vi, const std::vector<Vec3Df> & pdef);
    void compute_R( gsl_matrix * R , gsl_matrix * U, gsl_matrix * V );
    float compute_determinant( gsl_matrix * M );
    void singular_value_decomposition( gsl_matrix * matrix, gsl_matrix * U, gsl_matrix * V );
    void factorize_cholmod_A_system();void add_A_coeff( const int row , const int col , const double value );
    void update_A_coeff( const int i , const double value );
    void set_b_value( const int i , const Vec3Df & value );
    cholmod_dense* solve_cholmod();
    void allocates_cholmod_A_and_b(  );
    void fill_cholmod_A(  );
    void setDefaultRotations();

    int constrainedNb;
    // PARTIE CHOLMOD , ininteressante //
    int _rows;
    int _cols;
    cholmod_triplet *_triplet;
    int *_rowPtrA;
    int *_colPtrA;
    double *_valuePtrA;

    cholmod_sparse *_At;

    cholmod_factor *_L;

    cholmod_dense *_b;
    double *_valuePtrB;

    cholmod_common _c;

    int _nb_non_zeros_in_A;

    unsigned int step;

    bool data_loaded;
    /////////////////////////////////////
    unsigned int iterationNb;
    std::vector< Vec3Df > vertices;
    CotangentWeights edgesWeightMap;
    std::map<Edge, Vec3Df, compareEdge> bij;
    std::vector< bool > handles;
    std::vector< std::vector<unsigned int> > oneRing;
    std::vector<gsl_matrix *> R;
    std::vector<float> sumWij;

};

#endif // ASRIGIDASPOSSIBLE_H

