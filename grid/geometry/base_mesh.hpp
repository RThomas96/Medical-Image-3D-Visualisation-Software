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
    BaseMesh();

    std::vector<glm::vec3> verticesNormals;
    std::vector<glm::vec3> texCoord;// These are normalised coordinates

    glm::vec3 bbMin;
    glm::vec3 bbMax;

    History * history;// Allow to undo/redo between BaseMesh vertices positions
    std::array<glm::vec3, 3> coordinate_system;

    glm::vec3 getDimensions() const;

    void updatebbox();
    const std::vector<glm::vec3>& getVertices() const { return this->vertices; };

    void addStateToHistory(bool useTimer = false);

    int getNbVertices() const;
    const glm::vec3& getVertice(int i) const;
    const glm::vec3& getVerticeNormal(int i) const;
    glm::vec3 getOrigin() const;

    virtual void translate(const glm::vec3& vec);
    virtual void rotate(const glm::mat3& transf);
    virtual void scale(const glm::vec3& scale);
    virtual void setOrigin(const glm::vec3& origin);

    virtual bool getPositionOfRayIntersection(const glm::vec3& origin, const glm::vec3& direction, const std::vector<bool>& visibilityMap, const glm::vec3& planePos, glm::vec3& res) const = 0;

    virtual void movePoints(const std::vector<int>& origins, const std::vector<glm::vec3>& targets);
    void movePoints(const std::vector<glm::vec3>& targets);

    /***/

    virtual void computeNormals() = 0;
    virtual ~BaseMesh(){};
};

//! @}

#endif
