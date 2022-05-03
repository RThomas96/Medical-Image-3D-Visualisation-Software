#ifndef TETRAHEDRALMESH_HPP_
#define TETRAHEDRALMESH_HPP_

#include "../utils/Octree.h"
#include "base_mesh.hpp"
//#include "../include/mesh_deformer.hpp"

struct MeshDeformer;

//! \addtogroup geometry
//! @{

struct Tetrahedron {
    glm::vec3 * points[4];// Optionnal, this data can be deleted and computeBary, isInTet and baryToWord function moved out in the TetMesh class
    glm::vec4 normals[4];

    int pointsIdx[4];
    int neighbors[4];

    Tetrahedron();

    Tetrahedron(glm::vec3* a, glm::vec3* b, glm::vec3* c, glm::vec3* d);

    void setIndices(int a, int b, int c, int d);

    glm::vec4 computeBaryCoord(const glm::vec3& p);

    bool isInTetrahedron(const glm::vec3& p) const;

    glm::vec3 baryToWorldCoord(const glm::vec4& coord);

    void computeNormals();

    int getPointIndex(int faceIdx, int ptIdxInFace) const;

    void getCentroid(glm::vec3& centroid) const;
};

struct QuickTetSearcher;
class TetMesh : public BaseMesh {

public:

    QuickTetSearcher * quickTetSearcher;
    std::vector<Tetrahedron> mesh;
    glm::vec3 nbTetra;

    TetMesh();

    virtual void loadMESH(std::string const &filename);

    bool isEmpty() const;
    void buildGrid(const glm::vec3& nbCube, const glm::vec3& sizeCube, const glm::vec3& origin);

    void computeNeighborhood() override;
    void computeNormals() override;

    // Specific to Tethrahedal mesh
    Tetrahedron getTetra(int idx) const;
    int inTetraIdx(const glm::vec3& p) const;
    //glm::vec3 getCoordInInitial(const TetMesh& initial, glm::vec3 p) const;
    bool getCoordInInitial(const TetMesh& initial, const glm::vec3& p, glm::vec3& out) const;

    void setARAPDeformationMethod() override;
    bool getPositionOfRayIntersection(const glm::vec3& origin, const glm::vec3& direction, uint16_t minValue, uint16_t maxValue, const glm::vec3& planePos, glm::vec3& res) const override;

    ~TetMesh();

private:
    // This function is private because it doesn't update fields nbTetra, bbMin and bbMax
    // Thus it can only be used in buildGrid function
    void decomposeAndAddCube(std::vector<glm::vec3*> pts, const std::vector<int>& ptsIdx);
    std::vector<glm::vec3*> insertCubeIntoPtGrid(std::vector<glm::vec3> cubePts, glm::vec3 indices, std::vector<glm::vec3>& ptGrid, std::vector<int>& ptIndices);
    int from3DTo1D(const glm::vec3& p) const;
};

struct QuickTetSearcher {
    bool upToDate;

    Octree * octree;
    OctreePoint * octreePoints;

    TetMesh * mesh;

    QuickTetSearcher(TetMesh * mesh): mesh(mesh), upToDate(false), octree(nullptr), octreePoints(nullptr) {
        this->update();
    }

    void update() {
        if(!this->upToDate) {
            mesh->updatebbox();
            int nbTet = this->mesh->mesh.size();

            delete this->octree;
            delete this->octreePoints;

            //this->octree = new Octree(mesh->bbMin, mesh->bbMax);
            this->octree = new Octree((mesh->bbMax + mesh->bbMin)/2.f, (mesh->bbMax - mesh->bbMin)/2.f);
            this->octreePoints = new OctreePoint[nbTet];

            glm::vec3 centroid(0., 0., 0.);
            for(int i = 0; i < nbTet; ++i) {
                this->mesh->mesh[i].getCentroid(centroid);
                this->octreePoints[i].setIdx(i);
                this->octreePoints[i].setPosition(centroid);
		        this->octree->insert(octreePoints + i);
            }

            this->upToDate = true;
        }
    }

    int getInTetraIdx(const glm::vec3& p) {
        this->update();
        OctreePoint * res = this->octree->getClosestPoint(p);
        if(res) {
            int idx = res->getIdx();
            if(this->mesh->mesh[idx].isInTetrahedron(p)) {
                return idx;
            }

            std::vector<int> tetToCheck;
            tetToCheck.push_back(idx);
            int neighborsToCheck = 5;
            for(int i = 0; i < neighborsToCheck; ++i) {
                std::vector<int> neighbors;
                for(int tetIdx = 0; tetIdx < tetToCheck.size(); ++tetIdx) {
                    for(int nei = 0; nei < 4; ++nei) {
                        if(this->mesh->mesh[tetToCheck[tetIdx]].neighbors[nei] != -1) {
                            neighbors.push_back(this->mesh->mesh[tetToCheck[tetIdx]].neighbors[nei]);
                        }
                    }
                }
                
                tetToCheck.clear();
                for(int tetIdx = 0; tetIdx < neighbors.size(); ++tetIdx) {
                    if(this->mesh->mesh[neighbors[tetIdx]].isInTetrahedron(p)) {
                        return neighbors[tetIdx];
                    } else {
                        tetToCheck.push_back(neighbors[tetIdx]);
                    }
                }
            }
        }
        return -1;
    }
};


//! @}
#endif
