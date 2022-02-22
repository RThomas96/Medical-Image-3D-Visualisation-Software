#ifndef PLANE_MESH_HPP_
#define PLANE_MESH_HPP_

#include "base_mesh.hpp"
#include <iostream>
//#include <QOpenGLFunctions>

//! \addtogroup geometry
//! @{

class PlaneMesh : BaseMesh {

public:
    PlaneMesh(glm::vec3 p1, glm::vec3 p2, glm::vec3 p3);
    ~PlaneMesh(){};

    void draw();

    void computeNeighborhood() override;
    void computeNormals() override;
    bool getPositionOfRayIntersection(const glm::vec3& origin, const glm::vec3& direction, uint16_t minValue, uint16_t maxValue, glm::vec3& res) const override;
};

//! @}
#endif
