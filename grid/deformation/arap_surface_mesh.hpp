#ifndef ARAP_SURFACE_MESH_HPP_
#define ARAP_SURFACE_MESH_HPP_

#include "base_mesh.hpp"
#include <iostream>

//! \addtogroup deformation
//! @{

class SurfaceMeshARAP : public SurfaceMesh {

public:
    int some;
    int things;
    int usefull;
    int for_arap;


    void movePoint(const glm::vec3& origin, const glm::vec3& target);
};

//! @}
#endif
