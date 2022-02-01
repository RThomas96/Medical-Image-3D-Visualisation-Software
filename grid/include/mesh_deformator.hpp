#ifndef MESHDEFORMATOR_HPP_
#define MESHDEFORMATOR_HPP_

#include <glm/glm.hpp>
#include <vector>

struct BaseMesh;
enum DeformMethod {
    NORMAL,
    WEIGHTED
};

struct MeshDeformator {
    DeformMethod deformMethod;
    BaseMesh * baseMesh;

    MeshDeformator(BaseMesh * baseMesh, DeformMethod deformMethod) : baseMesh(baseMesh), deformMethod(deformMethod) {}

    virtual bool hasSelectedPts() = 0;
    virtual void selectPts(const glm::vec3& pt) = 0;
    virtual void deselectPts(const glm::vec3& pt) = 0;
    virtual void deselectAllPts() = 0;

    // Here origin is basically the clicked point
    virtual void movePoint(const glm::vec3& origin, const glm::vec3& target) = 0;

    virtual ~MeshDeformator(){};
};

struct WeightedMethod : MeshDeformator {
    float radius;
    glm::vec3 originalPoint;
    std::vector<int> selectedPts;

    WeightedMethod(BaseMesh * baseMesh, float radius) : MeshDeformator(baseMesh, DeformMethod::WEIGHTED), radius(radius) {}

    bool hasSelectedPts() override;
    void selectPts(const glm::vec3& pt) override;
    void deselectPts(const glm::vec3& pt) override;
    void deselectAllPts() override;

    void movePoint(const glm::vec3& origin, const glm::vec3& target) override;
};

struct NormalMethod : MeshDeformator {
    std::vector<int> selectedPts;

    NormalMethod(BaseMesh * baseMesh) : MeshDeformator(baseMesh, DeformMethod::NORMAL) {}

    bool hasSelectedPts() override;
    void selectPts(const glm::vec3& pt) override;
    void deselectPts(const glm::vec3& pt) override;
    void deselectAllPts() override;

    void movePoint(const glm::vec3& origin, const glm::vec3& target) override;
};

#endif
