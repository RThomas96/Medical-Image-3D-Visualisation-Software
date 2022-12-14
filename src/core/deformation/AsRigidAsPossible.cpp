#include "AsRigidAsPossible.h"
#include "math.h"
//#include "GLUtilityMethods.h"
#include <gsl/gsl_blas.h>
#include <algorithm>

AsRigidAsPossible::AsRigidAsPossible()
{
    iterationNb = 5;
    data_loaded = false;
}

AsRigidAsPossible::~AsRigidAsPossible(){

    if(data_loaded){
        cholmod_free_triplet(&_triplet, &_c);
        cholmod_free_sparse(&_At, &_c);
        cholmod_free_factor(&_L, &_c);
        cholmod_free_dense(&_b, &_c);

        for(unsigned int i = 0 ; i < R.size() ; i++ )
            gsl_matrix_free (R[i]);
        R.clear();

        delete [] _rowPtrA;
        delete [] _colPtrA;
        delete [] _valuePtrA;
        delete [] _valuePtrB;
    }
}

void AsRigidAsPossible::clear(){

    if(data_loaded){
        cholmod_free_triplet(&_triplet, &_c);
        cholmod_free_sparse(&_At, &_c);
        cholmod_free_factor(&_L, &_c);
        cholmod_free_dense(&_b, &_c);

        for(unsigned int i = 0 ; i < R.size() ; i++ )
            gsl_matrix_free (R[i]);
        R.clear();

        delete [] _rowPtrA;
        delete [] _colPtrA;
        delete [] _valuePtrA;
        delete [] _valuePtrB;

        vertices.clear();
        edgesWeightMap.clear();
        bij.clear();
        handles.clear();
        oneRing.clear();

        sumWij.clear();
        data_loaded = false;
    }

}

void AsRigidAsPossible::init( const std::vector<Vec3Df> & _vertices, const std::vector< std::vector <int> > & _triangles ){
    std::vector<Triangle> triangles (_triangles.size());
    for(unsigned int i = 0 ; i < _triangles.size() ; i++ ){
        triangles[i] = Triangle(_triangles[i][0], _triangles[i][1], _triangles[i][2]);
    }
    
    init(_vertices, triangles);
}

void AsRigidAsPossible::init( const std::vector<Vec3Df> & _vertices, const std::vector< Triangle > & _triangles ){

    vertices = _vertices;
    
    edgesWeightMap.clear();
    bij.clear();
    
    oneRing.clear();
    oneRing.resize (vertices.size ());
    handles.clear();
    handles.resize(vertices.size(), false);
    constrainedNb = 0;

    for( unsigned int i = 0 ; i < _triangles.size() ; i ++ ){
        
        const Triangle & triangle = _triangles[i];
        
        Vec3Df p1 = vertices[triangle.getVertex(0)];
        Vec3Df p2 = vertices[triangle.getVertex(1)];
        Vec3Df p3 = vertices[triangle.getVertex(2)];

        float e1sq = ( p2 - p1 ).getSquaredLength();
        float e2sq = ( p2 - p3 ).getSquaredLength();
        float e3sq = ( p3 - p1 ).getSquaredLength();

        float e1e2 = Vec3Df::dotProduct( p2 - p1 , p3 - p2 );
        float e2e3 = Vec3Df::dotProduct( p3 - p1 , p3 - p2 );
        float e3e1 = Vec3Df::dotProduct( p2 - p1 , p3 - p1 );

        float cot1 = e1sq / ( 2.f * sqrt( ( e2sq * e3sq / ( e2e3 * e2e3 ) ) - 1.f ) );
        float cot2 = e2sq / ( 2.f * sqrt( ( e1sq * e3sq / ( e3e1 * e3e1 ) ) - 1.f ) );
        float cot3 = e3sq / ( 2.f * sqrt( ( e2sq * e1sq / ( e1e2 * e1e2 ) ) - 1.f ) );



        Edge edge1( triangle.getVertex(0) , triangle.getVertex(1) );
        Edge edge2( triangle.getVertex(1) , triangle.getVertex(2) );
        Edge edge3( triangle.getVertex(2) , triangle.getVertex(0) );

        CotangentWeights::iterator it = edgesWeightMap.find(edge1);
        if( it != edgesWeightMap.end() ){
            it->second = (it->second + cot1)/2.;
            bij[it->first] = (vertices[it->first.v[1]] - vertices[it->first.v[0]]) * it->second/2.;
        } else {
            edgesWeightMap[edge1] = cot1;
            bij[edge1] = (vertices[edge1.v[1]] - vertices[edge1.v[0]]) * cot1/2.;
        }

        it = edgesWeightMap.find(edge2);
        if( it != edgesWeightMap.end() ){
            it->second = (it->second + cot2)/2.;
            bij[it->first] = (vertices[it->first.v[1]] - vertices[it->first.v[0]]) * it->second/2.;
        } else {
            edgesWeightMap[edge2] = cot2;
            bij[edge2] = (vertices[edge2.v[1]] - vertices[edge2.v[0]]) * cot2/2.;
        }

        it = edgesWeightMap.find(edge3);
        if( it != edgesWeightMap.end() ){
            it->second = (it->second + cot3)/2.;
            bij[it->first] = (vertices[it->first.v[1]] - vertices[it->first.v[0]]) * it->second/2.;
        } else {
            edgesWeightMap[edge3] = cot3;
            bij[edge3] = (vertices[edge3.v[1]] - vertices[edge3.v[0]]) * cot3/2.;
        }

        /*

        Vec3Df e[3] = { vertices[ triangle.getVertex(1) ] - vertices[ triangle.getVertex(0) ],
                            vertices[ triangle.getVertex(2) ] - vertices[ triangle.getVertex(1) ],
                            vertices[ triangle.getVertex(0) ] - vertices[ triangle.getVertex(2) ]};

        for( int v = 0 ; v < 3 ; v++ )
            e[v].normalize();



            Edge edge( triangle.getVertex(v) , triangle.getVertex( (v+1)%3 ) );
            float w = 1. / tan( acos( dofProduct( -1.*e[ (v+1)%3 ], e[ (v+2)%3 ] ) ) );
            
            CotangentWeights::iterator it = edgesWeightMap.find(edge);
            if( it != edgesWeightMap.end() ){
                w = ( w + it->second )/2. ;
                it->second = w;
                bij[it->first] = (vertices[it->first.v[0]] - vertices[it->first.v[1]]) * it->second/2.;
            } else {
                edgesWeightMap[edge] = w;
            }
*/
        for( int v = 0 ; v < 3 ; v++ ){
            unsigned int vj = triangle.getVertex (v);
            for (unsigned int k = 1; k < 3; k++) {
                unsigned int vk = triangle.getVertex ((v+k)%3);

                if (std::find (oneRing[vj].begin(), oneRing[vj].end(), vk) == oneRing[vj].end ())
                    oneRing[vj].push_back (vk);
            }

        }
    }
    
    setDefaultRotations();
}

void AsRigidAsPossible::setHandles(const std::vector< bool > & _handles){

    handles = _handles;
    
    constrainedNb = 0;
    for( unsigned int i = 0 ; i < handles.size() ; i ++ )
        if( handles[i] ) constrainedNb++;

    if( constrainedNb == 0 ) return;

    cholmod_start(&_c);
    allocates_cholmod_A_and_b();
    fill_cholmod_A();
    factorize_cholmod_A_system();
}


void AsRigidAsPossible::setDefaultRotations(){

    for(unsigned int i = 0 ; i < R.size() ; i++ )
        gsl_matrix_free (R[i]);
    R.clear();
    R.resize(vertices.size());

    for( unsigned int i = 0 ; i < vertices.size() ; i ++ ){

        R[i] = gsl_matrix_alloc(3, 3);

        gsl_matrix_set_identity( R[i] );

    }

}

void AsRigidAsPossible::compute_deformation(std::vector<Vec3Df> & positions){
    
    if( constrainedNb == 0 ) {
        return;
    }


    int nb_found = 0;
    for( unsigned int i = 0 ; i < vertices.size() ; i ++ ){
        if( handles[i] ){
            set_b_value( vertices.size() + nb_found, sumWij[i] * positions[i]);
            nb_found++;
        }
    }

    gsl_matrix * M = gsl_matrix_alloc(3, 3);
    gsl_matrix * S = gsl_matrix_alloc(3, 3);

    gsl_matrix * U = gsl_matrix_alloc(3, 3);
    gsl_matrix * V = gsl_matrix_alloc(3, 3);

    step = 0;
    while(step < iterationNb){
        for( unsigned int i = 0 ; i < vertices.size() ; i ++ ){
            //   if( !handles[i] ){

            Vec3Df p (0.,0.,0.);

            for( unsigned int v = 0 ; v < oneRing[i].size() ; v++ ){
                gsl_matrix_set_zero (M);

                unsigned int j = oneRing[i][v];
                Edge e(i,j);

                gsl_matrix_add(M , R[i]);
                gsl_matrix_add(M , R[j]);
                float prod = -1.;
                if(j < i) prod = 1.;
                compute_product_and_sum( M, bij[e]*prod, p );
            }

            set_b_value( i, p);
            //    }
        }


        cholmod_dense* x = solve_cholmod();

        double * data = (double *)x->x;
        for(unsigned int i = 0 ; i < vertices.size() ; i ++ ){
            if ( !handles[i] ){
                positions[i][0] = data[i + _cols*0];
                positions[i][1] = data[i + _cols*1];
                positions[i][2] = data[i + _cols*2];
            }
        }


        for(unsigned int i = 0 ; i < vertices.size() ; i ++ ){
            compute_S( S , i, positions );
            singular_value_decomposition( S, U, V );
            compute_R( R[i] , U, V );
        }
        step ++;
    }
    //compute_guess()
    
    gsl_matrix_free( M );
    gsl_matrix_free( S );

    gsl_matrix_free( U );
    gsl_matrix_free( V );

}

void AsRigidAsPossible::compute_product_and_sum( gsl_matrix * R, const Vec3Df & point, Vec3Df & result ){

    result[0] += point[0]*gsl_matrix_get( R, 0 , 0 ) + point[1]*gsl_matrix_get( R, 0 , 1 )+ point[2]*gsl_matrix_get( R, 0 , 2 );
    result[1] += point[0]*gsl_matrix_get( R, 1 , 0 ) + point[1]*gsl_matrix_get( R, 1 , 1 )+ point[2]*gsl_matrix_get( R, 1 , 2 );
    result[2] += point[0]*gsl_matrix_get( R, 2 , 0 ) + point[1]*gsl_matrix_get( R, 2 , 1 )+ point[2]*gsl_matrix_get( R, 2 , 2 );

}

void AsRigidAsPossible::compute_S( gsl_matrix * S , unsigned int vi, const std::vector<Vec3Df> & verticesp){

    std::vector<unsigned int> & Ni = oneRing[vi];

    gsl_matrix_set_zero ( S );
    
    for (unsigned int j = 0; j < Ni.size(); j++){
        Vec3Df eij = vertices[Ni[j]] -vertices[vi];
        Vec3Df eijp = verticesp[Ni[j]] -verticesp[vi];

        Edge e ( vi , Ni[j] );
        float wij = edgesWeightMap[e];

        for( int k = 0 ; k < 3  ; ++k )
            for( int l = 0 ; l < 3 ; ++l )
                gsl_matrix_set(S, k , l , gsl_matrix_get(S,k,l) + wij * eij[k] * eijp[l] );
    }
}


void AsRigidAsPossible::singular_value_decomposition( gsl_matrix * matrix, gsl_matrix * U, gsl_matrix * V ){

    gsl_vector * s = gsl_vector_alloc(3), * work= gsl_vector_alloc(3);

    gsl_matrix_memcpy( U, matrix );
    gsl_linalg_SV_decomp (U, V, s, work);

    gsl_vector_free(s);
    gsl_vector_free(work);
    
}


void AsRigidAsPossible::compute_R( gsl_matrix * R , gsl_matrix * U, gsl_matrix * V ){
    
    
    gsl_matrix * C = gsl_matrix_alloc ( 3, 3 );
    gsl_blas_dgemm( CblasNoTrans, CblasTrans, 1.0, V, U, 0.0, C );
    
    float det = compute_determinant(C);
    if(det < 0.)
        det = -1.;
    else
        det = 1.;
    
    gsl_matrix_set_identity (C);
    gsl_matrix_set( C, 2, 2, det );
    
    gsl_matrix * T = gsl_matrix_alloc ( 3, 3 );
    gsl_blas_dgemm( CblasNoTrans, CblasTrans, 1.0, C, U, 0.0, T );
    
    gsl_blas_dgemm( CblasNoTrans, CblasNoTrans, 1.0, V, T, 0.0, R );
    
    gsl_matrix_free(C);
    gsl_matrix_free(T);

}

float AsRigidAsPossible::compute_determinant( gsl_matrix * M ){
    float a = gsl_matrix_get( M, 0, 0); float b = gsl_matrix_get( M, 0, 1); float c = gsl_matrix_get( M, 0, 2);
    float d = gsl_matrix_get( M, 1, 0); float e = gsl_matrix_get( M, 1, 1); float f = gsl_matrix_get( M, 1, 2);
    float g = gsl_matrix_get( M, 2, 0); float h = gsl_matrix_get( M, 2, 1); float i = gsl_matrix_get( M, 2, 2);
    
    return a*(e*i - h*f) + b*(g*f - d*i) + c*(d*h - g*e);
}

void AsRigidAsPossible::factorize_cholmod_A_system()
{
    cholmod_print_triplet (_triplet, "triplet", &_c);
    cholmod_sparse* A   = cholmod_triplet_to_sparse(_triplet, _triplet->nnz, &_c);
    
    _At  = cholmod_transpose(A, 1, &_c);
    
    cholmod_sparse* AtA = cholmod_ssmult(_At, A, 0, 1, 1, &_c);
    AtA->stype = 1;
    
    _L = cholmod_analyze(AtA, &_c);
    
    cholmod_factorize(AtA, _L, &_c);
}


void AsRigidAsPossible::add_A_coeff( const int row , const int col , const double value )
{
    const int i = _triplet->nnz;
    _rowPtrA[i] = row;
    _colPtrA[i] = col;
    _valuePtrA[i] = value;
    _triplet->nnz++;
}

void AsRigidAsPossible::update_A_coeff( const int i , const double value )
{
    _valuePtrA[i] = value;
    // DANGEREUX , ne faire que si on connait parfaitement la structure de la matrice (ce qui est le cas ici, si on se debrouille bien ...)
}

void AsRigidAsPossible::set_b_value( const int i , const Vec3Df & value )
{
    _valuePtrB[i + 0 * _rows] = value[0];
    _valuePtrB[i + 1 * _rows] = value[1];
    _valuePtrB[i + 2 * _rows] = value[2];
}

cholmod_dense* AsRigidAsPossible::solve_cholmod()
{
    cholmod_dense* x;
    double alpha[] = {1, 1};
    double beta[] = {0, 0};
    
    cholmod_dense* Atb = cholmod_allocate_dense(_cols, 3, 3 * _cols, CHOLMOD_REAL, &_c);
    
    cholmod_sdmult(_At, 0, alpha, beta, _b, Atb, &_c);
    
    x = cholmod_solve(CHOLMOD_A, _L, Atb, &_c);
    
    cholmod_free_dense(&Atb, &_c);
    
    return x;
}


void AsRigidAsPossible::allocates_cholmod_A_and_b(  )
{
    // POUR PARAMETRER LE SOLVEUR :
    // Vous devez dire au moment de l'allocation (dans les lignes suivantes) le nombre d'equations que vous allez ajouter dans la matrice A,
    // ainsi que le nombre de valeurs non nulles que chaque terme d'energie implique dans la matrice A.
    //
    // Vous devez donc changer les lignes ci dessous , dans la partie ALLOCATION de la PARAMETRISATION du solveur, ainsi qu'ajouter les termes
    // dans le solveur dans la fonction fill_cholmod_A_and_b() , ou vous devrez mettre en face les equations implicites dans A et le terme de droite
    // dans b.
    
    
    // 1) PARAMETRISATION SOLVEUR , PARTIE ALLOCATION :
    
    _cols = vertices.size();
    _rows = _cols + constrainedNb;
    
    _nb_non_zeros_in_A = _cols;
    for( unsigned int i = 0 ; i < oneRing.size(); i++ ){
        //  if( !handles[i] )
        _nb_non_zeros_in_A += oneRing[i].size();
    }

    _nb_non_zeros_in_A += constrainedNb;
    
    // std::cout << _rows << " lines " << _cols << " colones " << _nb_non_zeros_in_A << " non zero " << std::endl;
    // FIN DE LA PARTIE ALLOCATION DE LA PARAMETRISATION DU SOLVEUR , ne touchez a rien d'autre en dessous ,
    // allez voir directement dans la fonction fill_cholmod_A_and_b()
    
    
    
    // X etant l'inconnue du least squares system
    
    _triplet = cholmod_allocate_triplet(
            _rows ,
            _cols ,
            _nb_non_zeros_in_A,
            0, CHOLMOD_REAL, &_c);
    
    _rowPtrA = (int*)_triplet->i;
    _colPtrA = (int*)_triplet->j;
    _valuePtrA = (double*)_triplet->x;

    _b = cholmod_zeros(_rows, 3, CHOLMOD_REAL, &_c);
    _valuePtrB = (double*)_b->x;

    //  std::cout << "allocated " << std::endl;
}

void AsRigidAsPossible::fill_cholmod_A(  )
{
    

    sumWij.clear();
    sumWij.resize(vertices.size(), 0.);
    for( unsigned int i = 0; i < vertices.size() ; i++ ){
        float sum = 1.;
        //   if( !handles[i] ){
        sum  = 0.;
        for( unsigned int j = 0; j < oneRing[i].size() ; j++ ){
            Edge e( i, oneRing[i][j] );
            float wij = edgesWeightMap[e];
            if( i == e.v[0] )
                add_A_coeff( i, e.v[1], -wij );
            else
                add_A_coeff( i, e.v[0], -wij );
            sum += wij;
        }
        //    }
        add_A_coeff( i, i, sum );
        sumWij[i] = sum;
    }

    int nb_found = 0;
    for( unsigned int i = 0; i < vertices.size() ; i++ ){
        if( handles[i] ){
            add_A_coeff( vertices.size() + nb_found, i, sumWij[i] );
            ++nb_found;
        }
    }
    
}

//void AsRigidAsPossible::draw(){
//    for( unsigned int i = 0 ; i < vertices.size() ; i++ ){
//        Vec3Df & center = vertices[i];
//
//        glColor3f(.3,0.3,0.3);
//        float radius = 1.;
//        if( handles.size() >0 && handles[i] ){
//            radius *= 3;
//            glColor3f(0.,0.8,0.);
//        }
//
//        BasicGL::drawSphere( center[0], center[1], center[2], radius, 30, 30 );
//    }
//}
