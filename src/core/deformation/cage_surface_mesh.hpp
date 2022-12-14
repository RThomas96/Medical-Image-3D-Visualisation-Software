#ifndef CAGE_HPP_
#define CAGE_HPP_

#include <vector>
#include <queue>
#include <map>
#include <set>
#include <algorithm>
#include <string>
#include <iostream>
#include <fstream>
#include <cfloat>
#include <cmath>

#include "../geometry/surface_mesh.hpp"
#include "CageCoordinates.h"
#include "CellInfo.h"
#include "../geometry/tetrahedral_mesh.hpp"
#include "../utils/BasicPoint.h"

#include "CholmodLSStruct.h"

//! \defgroup deformation Cage system
//! @brief 
//! All classes that manage cages.
//! Main classes are:
//! - Cage
//! - CageMVC
//! - CageGreen
//! - CageGreenLRI

//! \addtogroup deformation
//! @{

//! @brief Interface class for a Cage. A cage is a surface mesh which propagates its deformation to another mesh using cage coordinates.
//! The functions reInitialize(), computeCoordinates() and updateMeshToDeform() are all implemented in each child classes CageMVC, CageGreen and CageGreenLRI, this way the class Cage can call any of these functions whatever the cage type. Cage inherite from SurfaceMesh allowing the Scene to manipulate it exactly like a regular mesh. However, internally CageMVC, CageGreen and CageGreenLRI reimplement the movePoint function to deform the linked mesh.
struct Cage : public SurfaceMesh {

    //! @brief If set to False the cage do not deform the linked mesh.
    bool moveMeshToDeform;

    BaseMesh * meshToDeform;
    std::vector<glm::vec3> originalVertices;

    Cage(std::string const &filename, BaseMesh * meshToDeform) : SurfaceMesh(filename), meshToDeform(meshToDeform), moveMeshToDeform(true) {
        this->originalVertices = this->meshToDeform->getVertices();
    };

    Cage(SurfaceMesh * cage, BaseMesh * meshToDeform) : SurfaceMesh(*cage), meshToDeform(meshToDeform), moveMeshToDeform(true) {
        this->originalVertices = this->meshToDeform->getVertices();
    };
    
    //! @brief Compute cage coordinates, normals and reset the resting position of the cage (assign originalVertices to meshToDeform vertices).
    virtual void reInitialize() = 0;
    //! @brief Compute cage coordinates
    virtual void computeCoordinates() = 0;
    //! @brief Update the vertices position of the linked mesh. 
    virtual void updateMeshToDeform() = 0;

    //! @brief Move all the cage vertices to new positions.
    //! @param cage New positions of the cage vertices.
    void applyCage(const std::vector<glm::vec3>& cage);

    void changeMeshToDeform(BaseMesh * meshToDeform) {
        this->meshToDeform = meshToDeform;
        this->reInitialize();
    }

    void translate(const glm::vec3& vec) override {
        SurfaceMesh::translate(vec);
        if(moveMeshToDeform) {
            this->meshToDeform->translate(vec);
            this->reInitialize();
        }
    }

    void rotate(const glm::mat3& transf) override {
        SurfaceMesh::rotate(transf);
        if(moveMeshToDeform) {
            this->meshToDeform->rotate(transf);
            this->reInitialize();
        }
    }

    void scale(const glm::vec3& scale) override {
        SurfaceMesh::scale(scale);
        if(moveMeshToDeform) {
            this->meshToDeform->scale(scale);
            this->reInitialize();
        }
    }

    void setOrigin(const glm::vec3& origin) override {
        SurfaceMesh::setOrigin(origin);
        if(moveMeshToDeform) {
            this->meshToDeform->setOrigin(origin);
            this->reInitialize();
        }
    }

    void bindMovementWithDeformedMesh() {
        std::cout << "Rebind movement" << std::endl;
        this->moveMeshToDeform = true;
        this->reInitialize();
    }

    void unbindMovementWithDeformedMesh() {
        this->moveMeshToDeform = false;
    }

    virtual ~Cage(){};
};

//! @brief Cage that deform its linked mesh using MVC coordinates.
struct CageMVC : Cage {

    std::vector<std::vector<std::pair<unsigned int , float>>> MVCCoordinates;

    CageMVC(std::string const &filename, BaseMesh * meshToDeform) : Cage(filename, meshToDeform) {
        this->reInitialize();
    };

    CageMVC(SurfaceMesh * cage, BaseMesh * meshToDeform) : Cage(cage, meshToDeform) {
        this->reInitialize();
    };

    void reInitialize() override;
    void movePoints(const std::vector<int>& origins, const std::vector<glm::vec3>& targets) override;
    void computeCoordinates() override;
    void updateMeshToDeform() override;
};

//! @brief Cage that deform its linked mesh using Green coordinates.
//! This cage type cannot work is some vertices of the linked mesh are outside the cage, because Green coordinates are not defined outside the cage, in such scenario a CageGreenLRO should be used.
//! If a CageGreenLRI is created while no vertices are outside the cage, it is automatically detected and the CageGreenLRI become a CageGreen.
//! Therefore a CageGreenLRI should always be used to leverage this automatic detection, as it is done in Scene::openCage() .
struct CageGreen : Cage {

    std::vector<std::vector<double>> phiCoordinates;
    std::vector<std::vector<double>> psiCoordinates;
    std::vector<GreenCoords::GreenScalingFactor<BasicPoint>> scalingFactors;

    std::vector<glm::vec3> initial_cage_vertices;
    std::vector<glm::vec3> initial_cage_triangle_normals;

    CageGreen(std::string const &filename, BaseMesh * meshToDeform) : Cage(filename, meshToDeform) {
        this->reInitialize();
    };

    CageGreen(SurfaceMesh * cage, BaseMesh * meshToDeform) : Cage(cage, meshToDeform) {
        this->reInitialize();
    };

    void reInitialize() override;
    void movePoints(const std::vector<int>& origins, const std::vector<glm::vec3>& targets) override;
    void computeCoordinates() override;
    void updateMeshToDeform() override;

    void update_cage_triangle_scalingFactors();

    void getVertexInitialPosition(int p_idx, glm::vec3& pos) {
        if(p_idx < this->phiCoordinates.size()) {
            pos = glm::vec3(0.,0.,0.);
            for(unsigned int vc = 0 ; vc < initial_cage_vertices.size() ; ++vc)
                pos += static_cast<float>(phiCoordinates[p_idx][vc]) * initial_cage_vertices[vc];
            for(unsigned int tc = 0 ; tc < initial_cage_triangle_normals.size() ; ++tc)
                pos += static_cast<float>(psiCoordinates[p_idx][tc]) * 1.f * initial_cage_triangle_normals[tc];
        }
    }
};

//! @brief Cage that deform its linked mesh using Green coordinates, and that propagates its deformations even for vertices outside the cage using Laplacian.
//! If a CageGreenLRI is created while no vertices are outside the cage, it is automatically detected and the CageGreenLRI become a CageGreen.
//! Therefore a CageGreenLRI should always be used to leverage this automatic detection, as it is done in Scene::openCage() .
//! \todo This cage is not able to deform tetrahedrons with no vertex inside the cage. Only tetrahedrons with one or more vertices inside the cage are correctly deformed.
struct CageGreenLRI : CageGreen {

    // TODO: to fill even before initialization !!!!
    // cage_inlier is OK
    std::vector<CellInfo> tetInfos;
    // Edge = std::pair<int, int>, deux vertex id
    // vector int = l'adresse de tous les tetrahedre qui contiennent cette edge
    std::map<std::pair<int, int>, std::vector<int>> edgeMap;

    /***/
    CholmodLSStruct BasisSolver , VerticesSolver;
    std::vector<Tetrahedron> tetrahedra;
    std::vector<int> orig_tetrahedra_index;
    std::vector<int> handle_tetrahedra;// on connait les deformations, tous les vertex dans la cage
    std::vector<int> unknown_tetrahedra;// un des vertex pas dans la cage
    std::vector<std::pair<int, int>> edges;

    std::vector<int>         verts_mapping_from_solver_to_mesh;
    std::vector<int>         verts_mapping_from_mesh_to_solver;
    std::vector<glm::vec3>   verts_initial_positions;

    std::vector<bool> outlier_vertices;
    std::vector<bool> unknown_vertices;
    std::vector<int> verts_constraints;
    /***/

    bool invertMatrix( const std::vector<glm::vec3> & m, std::vector<glm::vec3> & invM ){
    
        float a = m[ 0 ][ 0 ]; float b = m[ 1 ][ 0 ]; float c = m[ 2 ][ 0 ];
        float d = m[ 0 ][ 1 ]; float e = m[ 1 ][ 1 ]; float f = m[ 2 ][ 1 ];
        float g = m[ 0 ][ 2 ]; float h = m[ 1 ][ 2 ]; float k = m[ 2 ][ 2 ];
    
        float det =  a*(e*k - h*f) + b*(g*f - d*k) + c*(d*h - g*e);
    
        if( det == 0 ) return false;
    
        invM[ 0 ][ 0 ] = (e*k - f*h)/det; invM[ 1 ][ 0 ] = (c*h - b*k)/det; invM[ 2 ][ 0 ] = (b*f - c*e)/det;
        invM[ 0 ][ 1 ] = (f*g - d*k)/det; invM[ 1 ][ 1 ] = (a*k - c*g)/det; invM[ 2 ][ 1 ] = (c*d - a*f)/det;
        invM[ 0 ][ 2 ] = (d*h - e*g)/det; invM[ 1 ][ 2 ] = (g*b - a*h)/det; invM[ 2 ][ 2 ] = (a*e - b*d)/det;
    
        return true;
    }

    void computeBasisTransform(const Tetrahedron& ch, std::vector<glm::vec3>& D) {
        std::vector<glm::vec3> B (3, glm::vec3(0.,0.,0.));
        B[0] = this->originalVertices[ch.pointsIdx[3]] - this->originalVertices[ch.pointsIdx[0]];
        B[1] = this->originalVertices[ch.pointsIdx[1]] - this->originalVertices[ch.pointsIdx[0]];
        B[2] = this->originalVertices[ch.pointsIdx[2]] - this->originalVertices[ch.pointsIdx[0]];

        std::vector<glm::vec3> Bp (3, glm::vec3(0.,0.,0.));
        Bp[0] = *ch.points[3] - *ch.points[0];
        Bp[1] = *ch.points[1] - *ch.points[0];
        Bp[2] = *ch.points[2] - *ch.points[0];

        std::vector<glm::vec3> invB(3, glm::vec3(0.,0.,0.));

        if(this->invertMatrix(B, invB)){
            D[0] = glm::vec3(Bp[0][0]*invB[0][0] + Bp[1][0]*invB[0][1] + Bp[2][0]*invB[0][2],
                    Bp[0][1]*invB[0][0] + Bp[1][1]*invB[0][1] + Bp[2][1]*invB[0][2],
                    Bp[0][2]*invB[0][0] + Bp[1][2]*invB[0][1] + Bp[2][2]*invB[0][2] );

            D[1] = glm::vec3(Bp[0][0]*invB[1][0] + Bp[1][0]*invB[1][1] + Bp[2][0]*invB[1][2],
                    Bp[0][1]*invB[1][0] + Bp[1][1]*invB[1][1] + Bp[2][1]*invB[1][2],
                    Bp[0][2]*invB[1][0] + Bp[1][2]*invB[1][1] + Bp[2][2]*invB[1][2] );

            D[2] = glm::vec3(Bp[0][0]*invB[2][0] + Bp[1][0]*invB[2][1] + Bp[2][0]*invB[2][2],
                    Bp[0][1]*invB[2][0] + Bp[1][1]*invB[2][1] + Bp[2][1]*invB[2][2],
                    Bp[0][2]*invB[2][0] + Bp[1][2]*invB[2][1] + Bp[2][2]*invB[2][2] );
        }
    }

    TetMesh * getMeshToDeform() {
        return dynamic_cast<TetMesh*>(this->meshToDeform);
    }

    void computeBasisTransforms() {
        for(int tet = 0; tet < this->getMeshToDeform()->mesh.size(); ++tet) {
            std::vector<glm::vec3> basis_tr(3, glm::vec3(0.,0.,0.));
            this->computeBasisTransform(this->getMeshToDeform()->mesh[tet], basis_tr);
            this->tetInfos[tet].basis_def = basis_tr;
        }
    }

    void updateMeshToDeform() override {
        CageGreen::updateMeshToDeform();
        if(this->moveMeshToDeform) {
            if( this->outlier_vertices.size() > 0 ){
                std::cout << "Solving LRI..." << std::endl;
                this->computeBasisTransforms();
                this->update_constraints();
            } else {
                std::cout << "Everything is fine :) No need LRI" << std::endl;
            }
        }
    }

    void movePoints(const std::vector<int>& origins, const std::vector<glm::vec3>& targets) override {
        CageGreen::movePoints(origins, targets);
        this->updateMeshToDeform();
    }

    CageGreenLRI(std::string const &filename, TetMesh * meshToDeform) : CageGreen(filename, meshToDeform) {
        this->reInitialize();
    };

    void reInitialize() override {
        this->clear();
        CageGreen::reInitialize();
        /** TetMeshCreator checkAndMarkOutliers  **/
        bool hasOutliers = this->checkAndMarkOutliers();

        if(!hasOutliers) {
            this->outlier_vertices.clear();
            std::cout << "No outliers found, no need LRI" << std::endl;
            return;
        } else {
            this->tetInfos.clear();
            this->tetInfos.resize(this->getMeshToDeform()->mesh.size(), CellInfo());
            for(int i = 0; i < this->getMeshToDeform()->mesh.size(); ++i) {
                this->tetInfos[i].cage_inlier = true;
                for(int v = 0; v < 4; ++v) {
                    if(this->outlier_vertices[this->getMeshToDeform()->mesh[i].pointsIdx[v]]) {
                        this->tetInfos[i].cage_inlier = false;
                        break;
                    }
                }
            }
        }
         /** **/

        /** TetMeshCreator initializeLRISolver  **/
        this->tetrahedra.clear();
        this->tetrahedra.reserve(this->getMeshToDeform()->mesh.size());
        this->handle_tetrahedra.clear();
        this->unknown_tetrahedra.clear();
        this->orig_tetrahedra_index.clear();

        for(int tet = 0; tet < this->getMeshToDeform()->mesh.size(); ++tet) {
            unsigned int index = tetrahedra.size();
            if(this->tetInfos[tet].cage_inlier) {
                for(int i = 0; i < 4; ++i) {
                    const int curNeighTet = this->getMeshToDeform()->mesh[tet].neighbors[i];
                    // Handle tetrahedra = tetra au bord mais avec tous les vertex dans la cage
                    if(curNeighTet != -1 && !this->tetInfos[curNeighTet].cage_inlier) {
                        this->handle_tetrahedra.push_back(index);
                        this->tetrahedra.push_back(this->getMeshToDeform()->mesh[tet]);
                        this->orig_tetrahedra_index.push_back(tet);
                        break;
                    }
                }
            } else {
                // unknown tetrahedra = tetra avec un vertex hors de la cage
                this->tetrahedra.push_back(this->getMeshToDeform()->mesh[tet]);
                this->unknown_tetrahedra.push_back(index);
                this->orig_tetrahedra_index.push_back(tet);
            }
        }

        // Remap neighbors indices
        for(int i = 0; i < tetrahedra.size(); ++i) {
            for(int j = 0; j < 4; ++j) {
                int currentIndex = this->tetrahedra[i].neighbors[j];
                if(currentIndex != -1) {
                    auto it = find(this->orig_tetrahedra_index.begin(), this->orig_tetrahedra_index.end(), currentIndex);
                    if(it != this->orig_tetrahedra_index.end()) {
                        int id = it - this->orig_tetrahedra_index.begin();
                        this->tetrahedra[i].neighbors[j] = id;
                    }
                }
            }
        }

        // this->outlierChecked = true;
        /** **/

        /***/
        /** TetraLRISolver: Load tetrahedra **/
        /***/

        //TODO: bad tricks for easy rename
        this->unknown_vertices = this->outlier_vertices;

        // Triangulation = meshToDeform
        this->collect_edges();
        this->collect_vertices();
        this->collect_constraints_vertices();

        unsigned int neighbor_count = 0 ;
        for( unsigned int i = 0 ; i < unknown_tetrahedra.size() ; i ++ ){
            const Tetrahedron & ch = tetrahedra[unknown_tetrahedra[i]];
            for( int j = 0 ; j < 4 ; j++ ){
                if(ch.neighbors[j] != -1){
                    neighbor_count++;
                }
            }
        }

        BasisSolver.start();
        VerticesSolver.start();

        // set dimensions:
        BasisSolver.setDimensions( unknown_tetrahedra.size() + handle_tetrahedra.size() , unknown_tetrahedra.size() + handle_tetrahedra.size() , 9 );
        VerticesSolver.setDimensions( edges.size() + verts_constraints.size() , verts_mapping_from_solver_to_mesh.size() , 3 );

        // give non zeros in A matrices:
        BasisSolver.specifyNonZeroInA( unknown_tetrahedra.size() + neighbor_count + handle_tetrahedra.size() );
        VerticesSolver.specifyNonZeroInA( 2*edges.size() + verts_constraints.size() );

        // allocate:
        BasisSolver.allocate();
        VerticesSolver.allocate();

        std::vector<float> volumes (this->tetrahedra.size(), 0.);
        for(unsigned int i = 0 ; i < this->tetrahedra.size() ; i ++){
            const Tetrahedron ch = tetrahedra[i];
            const glm::vec3 p0 = this->getMeshToDeform()->getVertice(ch.pointsIdx[0]);
            const glm::vec3 p1 = this->getMeshToDeform()->getVertice(ch.pointsIdx[1]);
            const glm::vec3 p2 = this->getMeshToDeform()->getVertice(ch.pointsIdx[2]);
            const glm::vec3 p3 = this->getMeshToDeform()->getVertice(ch.pointsIdx[3]);

            glm::vec3 e10 = p1 - p0;
            glm::vec3 e20 = p2 - p0;

            glm::vec3 v1 = glm::cross(e10, e20);
            glm::vec3 v2 = (p3 - p0);
            float res = 0;
            for(int i = 0; i < 3; ++i)
                res += v1[i] * v2[i];
            volumes[i] = std::fabs(res)/6.;
        }

        // set values in BasisSolver's A matrix:
        for( unsigned int i = 0 ; i < unknown_tetrahedra.size() ; i ++ ){
            const Tetrahedron & ch = tetrahedra[unknown_tetrahedra[i]];
            std::vector<int> neighbors;
            float v_sum = 0.;
            for( int j = 0 ; j < 4 ; j++ ){
                unsigned int n_index = ch.neighbors[j];// These indices are remaped
                if(n_index != -1){
                    neighbors.push_back(n_index);
                    v_sum += volumes[n_index];
                }
            }
            unsigned int index = unknown_tetrahedra[i];

            BasisSolver.addValueInA( i , index , -1.*volumes[index] );

            for( int j = 0 ; j < neighbors.size() ; j++ ){
                BasisSolver.addValueInA( i , neighbors[j] , (volumes[index]*volumes[ neighbors[j] ])/(double)v_sum );
            }

            for( unsigned int coord = 0 ; coord < 9 ; ++coord )
                BasisSolver.setValueInB( i , coord , 0.0 );
        }

        volumes.clear();

        for( unsigned int i = 0 ; i < handle_tetrahedra.size() ; i ++ ){
            //const Tetrahedron & ch = tetrahedra[handle_tetrahedra[i]];

            unsigned int index = handle_tetrahedra[i];
            BasisSolver.addValueInA( unknown_tetrahedra.size() + i , index , 1.0 );
        }

        // set values in VerticesSolver's A matrix:
        for( unsigned int i = 0 ; i < edges.size() ; i++ ){
            unsigned int id_v0 = verts_mapping_from_mesh_to_solver[edges[i].first];
            unsigned int id_v1 = verts_mapping_from_mesh_to_solver[edges[i].second];
            VerticesSolver.addValueInA( i , id_v1 , 1.0 );
            VerticesSolver.addValueInA( i , id_v0 , -1.0 );
        }
        for( unsigned int i = 0 ; i < verts_constraints.size() ; i++ ){
            VerticesSolver.addValueInA( edges.size() + i , verts_constraints[i] , 1.0 );
        }

        // factorization:
        BasisSolver.factorize();
        VerticesSolver.factorize();

        std::cout << "Solver initialized" << std::endl;
    }

    void collect_edges() {
        edges.clear();
        for(unsigned int i = 0; i < this->tetrahedra.size(); ++i){
            const Tetrahedron& ch = this->tetrahedra[i];
            for( int v = 0 ; v < 4 ; v ++ ){
                for( int vn = v + 1  ; vn < 4 ; vn ++ ){
                    //std::pair<int, int> edge = std::make_pair(ch.getPointIndex(v, vn), ch.getPointIndex(v, (vn+1)%3));
                    std::pair<int, int> edge = std::make_pair(ch.pointsIdx[v], ch.pointsIdx[vn]);
                    if( std::find( edges.begin(), edges.end(), edge ) == edges.end() ){
                        edges.push_back( edge );
                    }

                    int idx1 = ch.pointsIdx[v];
                    int idx2 = ch.pointsIdx[vn];
                    edge = std::make_pair(idx1, idx2);
                    if(this->edgeMap.find(edge) == this->edgeMap.end()) {
                        this->edgeMap[edge] = std::vector<int>{i};
                    } else {
                        this->edgeMap[edge].push_back(i);
                    }

                    std::swap(idx1, idx2);
                    edge = std::make_pair(idx1, idx2);
                    if(this->edgeMap.find(edge) == this->edgeMap.end()) {
                        this->edgeMap[edge] = std::vector<int>{i};
                    } else {
                        this->edgeMap[edge].push_back(i);
                    }
                }
            }
        }
    }

    void collect_vertices() {
        verts_mapping_from_mesh_to_solver.clear();
        verts_mapping_from_mesh_to_solver.resize(this->unknown_vertices.size(), -1);

        verts_mapping_from_solver_to_mesh.clear();
        verts_initial_positions.clear();

        for( unsigned int i = 0 ; i < this->tetrahedra.size() ; i ++ ){
            const Tetrahedron& ch = this->tetrahedra[i];

            for( int v = 0 ; v < 4 ; v ++ ){
                const int& vh = ch.pointsIdx[v];
                const glm::vec3& point = *ch.points[v];
                if( verts_mapping_from_mesh_to_solver[vh] == -1 )
                {
                    verts_mapping_from_mesh_to_solver[vh] = verts_mapping_from_solver_to_mesh.size();
                    verts_mapping_from_solver_to_mesh.push_back(vh);
                    verts_initial_positions.push_back(point);
                }
            }
        }
    }

    void collect_constraints_vertices() {
        std::vector<bool> marked_vertices (unknown_vertices.size(), false);
        this->verts_constraints.clear();

        for( unsigned int i = 0 ; i < this->unknown_tetrahedra.size() ; i ++ ){
            const Tetrahedron & ch = this->tetrahedra[this->unknown_tetrahedra[i]];

            for( int v = 0 ; v < 4 ; v ++ ){
                unsigned int v_index = ch.pointsIdx[v];
                if( !unknown_vertices[ v_index ] && !marked_vertices[ v_index ]) {
                    this->verts_constraints.push_back(this->verts_mapping_from_mesh_to_solver[v_index]);
                    marked_vertices[v_index] = true;
                }
            }
        }
    }

    bool checkAndMarkOutliers(){
        std::cout << "Checking for ouliers..." << std::endl;
        this->updatebbox();
        float error_max = glm::length(this->bbMax - this->bbMin)/1000.;
        this->outlier_vertices.clear();
        this->outlier_vertices.resize(this->meshToDeform->getNbVertices(), false);
        bool outlierDetected = false;
        int outlierCount = 0;
        for(int i = 0; i < this->meshToDeform->getNbVertices(); ++i) {
            glm::vec3 position = this->originalVertices[i];
            glm::vec3 up_position;
            this->getVertexInitialPosition(i, up_position);
            glm::vec3 diff = (position - up_position);
            float norm = diff[0] * diff[0] + diff[1] * diff[1] + diff[2] * diff[2];
            if (!(norm < 0.00001) && (glm::length(diff) > error_max) ){
                this->outlier_vertices[i] = true;
                outlierDetected = true;
                outlierCount++;
            }
        }

        std::cout << outlierCount << "/" << this->meshToDeform->getNbVertices() << " ouliers marked" << std::endl;
        return outlierDetected;
    }

    void update_constraints();

    void clear() {
        for(int i = 0; i < this->tetInfos.size(); ++i)
            this->tetInfos[i].basis_def.clear();
        this->tetInfos.clear();
        this->edgeMap.clear();

        this->BasisSolver.free();
        this->VerticesSolver.free();

        this->tetrahedra.clear();
        this->orig_tetrahedra_index.clear();
        this->handle_tetrahedra.clear();
        this->unknown_tetrahedra.clear();
        this->edges.clear();

        this->verts_mapping_from_solver_to_mesh.clear();
        this->verts_mapping_from_mesh_to_solver.clear();
        this->verts_initial_positions.clear();

        this->outlier_vertices.clear();
        this->unknown_vertices.clear();
        this->verts_constraints.clear();
    }
};

//! @}
#endif
