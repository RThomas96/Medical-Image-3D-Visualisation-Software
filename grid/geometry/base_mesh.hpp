#ifndef BASE_MESH_HPP_
#define BASE_MESH_HPP_

#include <glm/glm.hpp>
#include <iostream>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/io.hpp>
#include <vector>
#include <ctime>
#include <chrono>

struct History {
    std::chrono::time_point<std::chrono::system_clock> timer;
    bool isActive;
    int currentState;
    std::vector<std::vector<glm::vec3>> history;

    History(const std::vector<glm::vec3> initialPoints): history({initialPoints}), currentState(0), isActive(true), timer(std::chrono::system_clock::now()) {}

    void activate() { this->isActive = true; }
    void deactivate() { this->isActive = false; }

    bool reset(std::vector<glm::vec3>& res) {
        if(currentState == 0) {
            std::cout << "WARNING: no operations, nothing to do" << std::endl;
            return false;
        }

        this->currentState = 0;
        res = history.front();
        return true;
    }

    bool undo(std::vector<glm::vec3>& res) {
        if(currentState == 0) {
            std::cout << "WARNING: can't undo, no more operations" << std::endl;
            return false;
        }
        
        this->currentState -= 1;
        res = history[currentState];
        return true;
    }

    bool redo(std::vector<glm::vec3>& res) {
        if(currentState == history.size()-1) {
            std::cout << "WARNING: can't redo, no more operations" << std::endl;
            return false;
        }
        
        this->currentState += 1;
        res = history[currentState];
        return true;
    }

    void addStep(const std::vector<glm::vec3>& points, bool useTimer = true) {
        if(!this->isActive)
            return;
        if(useTimer && std::chrono::duration<double>(std::chrono::system_clock::now() - this->timer).count() < 0.25)
            return;
        this->timer = std::chrono::system_clock::now();
        if(this->currentState < this->history.size()-1) {
            int nbStateToDelete = (this->history.size()-1) - this->currentState;
            for(int i = 0; i < nbStateToDelete; ++i)
                this->history.pop_back();
        }
        std::cout << "Add operation in history [" << points.size() << "]" << std::endl;
        this->currentState += 1;
        this->history.push_back(points);
    }
};

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
    MeshDeformer * normalDeformer;// Move the mesh points using the normal strategy
    History * history;// Allow to undo/redo between BaseMesh vertices positions
    std::array<glm::vec3, 3> coordinate_system;

    glm::vec3 getDimensions() const;
    //int getIdxOfClosestPoint(const glm::vec3& p) const;

    BaseMesh();

    void updatebbox();
    std::vector<glm::vec3>& getMeshPositions();
    const std::vector<glm::vec3>& getVertices() const { return this->vertices; };

    // Functions to interact with the mesh
    void setNormalDeformationMethod();

    void scaleToBBox(const glm::vec3& bbMin, const glm::vec3& bbMax);
    bool isInBBox(const glm::vec3& p) const;

    void addStateToHistory(bool useTimer = false);

    glm::vec3 getOrigin() const;
    virtual void translate(const glm::vec3& vec);
    virtual void rotate(const glm::mat3& transf);
    virtual void scale(const glm::vec3& scale);
    virtual void setOrigin(const glm::vec3& origin);

    int getNbVertices() const;
    const glm::vec3& getVertice(int i) const;
    const glm::vec3& getVerticeNormal(int i) const;

    void drawNormals() const;

    virtual void movePoint(const int& origin, const glm::vec3& target);
    virtual void movePoints(const std::vector<int>& origins, const std::vector<glm::vec3>& targets);
    void movePoints(const std::vector<glm::vec3>& targets);

    // Quick hack for mixing normal and arap method...
    bool useNormal = false;
    virtual void replacePoint(const int& origin, const glm::vec3& target) { this->history->deactivate(); this->useNormal=true; this->movePoint(origin, target); this->useNormal=false; this->history->activate();};
    virtual void replacePoints(const std::vector<int>& origins, const std::vector<glm::vec3>& targets) { this->history->deactivate(); this->useNormal=true; this->movePoints(origins, targets); this->useNormal=false; this->history->activate();};
    virtual void replacePoints(const std::vector<glm::vec3>& targets) { this->history->deactivate(); this->useNormal=true; this->movePoints(targets); this->useNormal=false; this->history->activate();};

    virtual void setARAPDeformationMethod() = 0;
    virtual bool getPositionOfRayIntersection(const glm::vec3& origin, const glm::vec3& direction, uint16_t minValue, uint16_t maxValue, const glm::vec3& planePos, glm::vec3& res) const = 0;
    virtual void computeNeighborhood() = 0;
    virtual void computeNormals() = 0;
    virtual ~BaseMesh(){};
};

//! @}

#endif
