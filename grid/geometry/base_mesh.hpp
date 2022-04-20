#ifndef BASE_MESH_HPP_
#define BASE_MESH_HPP_

#include <glm/glm.hpp>
#include <iostream>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/io.hpp>
#include <vector>

struct MeshDeformer;
struct NormalMethod;
struct ARAPMethod;

namespace UITool {
    class Manipulator;
}

enum class DeformMethod {
    NORMAL,
    WEIGHTED,
    ARAP
};

//! \defgroup geometry Geometry
//! \addtogroup geometry
//! @{

class BaseMesh {

public:
    friend MeshDeformer;
    friend NormalMethod;
    friend ARAPMethod;
protected:
    std::vector<glm::vec3> vertices;
    
public:

    std::vector<glm::vec3> verticesNormals;
    std::vector<glm::vec3> texCoord;// These are normalised coordinates

    glm::vec3 bbMin;
    glm::vec3 bbMax;

    MeshDeformer * meshDeformer;// It will move the mesh points using certain strategies

    glm::vec3 getDimensions() const;
    int getIdxOfClosestPoint(const glm::vec3& p) const;

    BaseMesh();

    void updatebbox();
    std::vector<glm::vec3>& getMeshPositions();
    const std::vector<glm::vec3>& getVertices() const { return this->vertices; };

    // Functions to interact with the mesh
    void setNormalDeformationMethod();

    void scaleToBBox(const glm::vec3& bbMin, const glm::vec3& bbMax);

    glm::vec3 getOrigin() const;
    virtual void translate(const glm::vec3& vec);
    virtual void rotate(const glm::mat3& transf);
    virtual void scale(const glm::vec3& scale);
    virtual void setOrigin(const glm::vec3& origin);

    int getNbVertices() const;
    const glm::vec3& getVertice(int i) const;
    const glm::vec3& getVerticeNormal(int i) const;

    void drawNormals() const;

    virtual void movePoint(const glm::vec3& origin, const glm::vec3& target);
    virtual void movePoints(const std::vector<glm::vec3>& origins, const std::vector<glm::vec3>& targets);
    virtual void setARAPDeformationMethod() = 0;
    virtual bool getPositionOfRayIntersection(const glm::vec3& origin, const glm::vec3& direction, uint16_t minValue, uint16_t maxValue, const glm::vec3& planePos, glm::vec3& res) const = 0;
    virtual void computeNeighborhood() = 0;
    virtual void computeNormals() = 0;
    virtual ~BaseMesh(){};
};

//! @}

#endif
