#pragma once

#include <glm/glm.hpp>
#include <vector>

struct ProjectedPoint {
    glm::vec3 position;
    glm::vec3 normal;
    float curvature;
};

/**
 * @brief Project one point using APSS.
 *
 * @param inputPoint The point to project 
 * @param positions The vertices of the mesh
 * @param normals  The normals of the mesh
 * @param knn_indices The indices of the k nearest neighbors of `inputPoint`
 * @param knn_squared_distances The squared distances to the knn
 * @return the result of the projection 
*/
ProjectedPoint apss(const glm::vec3& inputPoint, 
                    const std::vector<glm::vec3>& positions, 
                    const std::vector<glm::vec3>& normals,
                    const std::vector<float>& knn_indices,// nth closest points from inputPoint
                    const std::vector<float>& knn_squared_distances);// same but distances
