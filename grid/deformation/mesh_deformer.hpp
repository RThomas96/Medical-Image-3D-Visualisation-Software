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
enum class DeformMethod;

struct MeshDeformer {
    DeformMethod deformMethod;
    BaseMesh * baseMesh;// TODO: weak pointer

    MeshDeformer(BaseMesh * baseMesh, DeformMethod deformMethod) : baseMesh(baseMesh), deformMethod(deformMethod) {}

    // Here origin is basically the clicked point
    virtual void movePoint(int origin, const glm::vec3& target) = 0;
    virtual void movePoints(const std::vector<int>& origins, const std::vector<glm::vec3>& targets) = 0;

    virtual ~MeshDeformer(){};
};

struct NormalMethod : MeshDeformer {
    NormalMethod(BaseMesh * baseMesh);

    void movePoint(int origin, const glm::vec3& target) override;
    void movePoints(const std::vector<int>& origins, const std::vector<glm::vec3>& targets) override;
};

struct ARAPMethod : MeshDeformer {
    bool onSurfaceMesh;
    AsRigidAsPossible arap;
    std::vector<bool> handles;

    ARAPMethod(BaseMesh * baseMesh);
    ARAPMethod(SurfaceMesh * surfaceMesh);

    void setHandle(int idx);
    void unsetHandle(int idx);

    void movePoint(int origin, const glm::vec3& target) override;
    void movePoints(const std::vector<int>& origins, const std::vector<glm::vec3>& targets) override;

    void fitToPointList(const std::vector<int>& vertices, const std::vector<glm::vec3>& newPositions);
    void initARAP();
};

//! @}
#endif
