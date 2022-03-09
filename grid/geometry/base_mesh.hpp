#ifndef BASE_MESH_HPP_
#define BASE_MESH_HPP_

#include <glm/glm.hpp>
#include <iostream>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/io.hpp>
#include <vector>

struct MeshDeformer;
namespace UITool {
    class Manipulator;
}

//! \defgroup geometry Geometry
//! \addtogroup geometry
//! @{

class BaseMesh : public QObject {
    Q_OBJECT
public:

    std::vector<glm::vec3> vertices;
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

    // Functions to interact with the mesh
    void setNormalDeformationMethod();
    void setWeightedDeformationMethod(float radius);
    void selectPts(const glm::vec3& pt);
    void deselectPts(const glm::vec3& pt);
    void deselectAllPts();

    glm::vec3 getOrigin();
    virtual void translate(const glm::vec3& vec);
    virtual void rotate(const glm::mat3& transf);
    virtual void scale(const glm::vec3& scale);
    virtual void setOrigin(const glm::vec3& origin);

    int getNbVertices() const;
    const glm::vec3& getVertice(int i) const;
    const glm::vec3& getVerticeNormal(int i) const;

    void drawNormals() const;

    virtual void movePoint(const glm::vec3& origin, const glm::vec3& target);
    virtual void setARAPDeformationMethod() = 0;
    virtual bool getPositionOfRayIntersection(const glm::vec3& origin, const glm::vec3& direction, uint16_t minValue, uint16_t maxValue, glm::vec3& res) const = 0;
    virtual void computeNeighborhood() = 0;
    virtual void computeNormals() = 0;
    virtual ~BaseMesh(){};
};

//! @}

#endif
