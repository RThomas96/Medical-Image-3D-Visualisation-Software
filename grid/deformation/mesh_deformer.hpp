#ifndef MESHDEFORMATOR_HPP_
#define MESHDEFORMATOR_HPP_

#include "AsRigidAsPossible.h"
#include <glm/glm.hpp>
#include <vector>

//! \defgroup deformation Deformation
//! \addtogroup deformation
//! @{

struct BaseMesh;
struct SurfaceMesh;
enum DeformMethod {
    NORMAL,
    WEIGHTED,
    ARAP
};

struct MeshDeformer {
    DeformMethod deformMethod;
    BaseMesh * baseMesh;// TODO: weak pointer

    MeshDeformer(BaseMesh * baseMesh, DeformMethod deformMethod) : baseMesh(baseMesh), deformMethod(deformMethod) {}

    virtual bool hasSelectedPts() = 0;
    virtual void selectPts(const glm::vec3& pt) = 0;
    virtual void deselectPts(const glm::vec3& pt) = 0;
    virtual void deselectAllPts() = 0;

    // Here origin is basically the clicked point
    virtual void movePoint(const glm::vec3& origin, const glm::vec3& target) = 0;
    virtual void movePoints(const std::vector<glm::vec3>& origins, const std::vector<glm::vec3>& targets) = 0;

    virtual ~MeshDeformer(){};
};

struct WeightedMethod : MeshDeformer {
    float radius;
    glm::vec3 originalPoint;
    std::vector<int> selectedPts;

    WeightedMethod(BaseMesh * baseMesh, float radius) : MeshDeformer(baseMesh, DeformMethod::WEIGHTED), radius(radius) {}

    bool hasSelectedPts() override;
    void selectPts(const glm::vec3& pt) override;
    void deselectPts(const glm::vec3& pt) override;
    void deselectAllPts() override;

    void movePoint(const glm::vec3& origin, const glm::vec3& target) override;
    void movePoints(const std::vector<glm::vec3>& origins, const std::vector<glm::vec3>& targets) override;
};

struct NormalMethod : MeshDeformer {
    std::vector<int> selectedPts;

    NormalMethod(BaseMesh * baseMesh) : MeshDeformer(baseMesh, DeformMethod::NORMAL) {}

    bool hasSelectedPts() override;
    void selectPts(const glm::vec3& pt) override;
    void deselectPts(const glm::vec3& pt) override;
    void deselectAllPts() override;

    void movePoint(const glm::vec3& origin, const glm::vec3& target) override;
    void movePoints(const std::vector<glm::vec3>& origins, const std::vector<glm::vec3>& targets) override;
};

struct ARAPMethod : MeshDeformer {
    bool onSurfaceMesh;
    AsRigidAsPossible arap;
    std::vector<int> selectedPts;
    std::vector<bool> handles;

    ARAPMethod(BaseMesh * baseMesh);
    ARAPMethod(SurfaceMesh * surfaceMesh);

    bool hasSelectedPts() override;
    void selectPts(const glm::vec3& pt) override;
    void deselectPts(const glm::vec3& pt) override;
    void deselectAllPts() override;

    void setHandle(int idx);
    void unsetHandle(int idx);

    void movePoint(const glm::vec3& origin, const glm::vec3& target) override;
    void movePoints(const std::vector<glm::vec3>& origins, const std::vector<glm::vec3>& targets) override;

    void fitToPointList(const std::vector<int>& vertices, const std::vector<glm::vec3>& newPositions);
};

//! @}
#endif
