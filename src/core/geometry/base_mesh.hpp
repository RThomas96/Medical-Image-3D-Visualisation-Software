#ifndef BASE_MESH_HPP_
#define BASE_MESH_HPP_

#include <glm/glm.hpp>
#include <iostream>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/io.hpp>
#include <vector>
#include <ctime>
#include <chrono>

//! \defgroup geometry Geometry
//! @brief 
//! All classes that manage meshes.
//! Main classes are:
//! - Grid
//! - BaseMesh
//! - SurfaceMesh
//! - TetMesh
//! - GraphMesh

//! \addtogroup geometry
//! @{

//! @brief Store a stack of vertices positions to undo/redo operations.
struct History {
    std::chrono::time_point<std::chrono::system_clock> timer;
    bool isActive;
    int currentState;
    std::vector<std::vector<glm::vec3>> vertexHistory;
    //! @brief Store the 3D guizmo orientations
    std::vector<std::array<glm::vec3, 3>> coordinateHistory;

    History(const std::vector<glm::vec3> initialPoints, std::array<glm::vec3, 3>& coordinate): vertexHistory({initialPoints}), coordinateHistory({coordinate}), currentState(0), isActive(true), timer(std::chrono::system_clock::now()) {}

    void activate() { this->isActive = true; }
    void deactivate() { this->isActive = false; }

    bool reset(std::vector<glm::vec3>& res, std::array<glm::vec3, 3>& coordinate) {
        if(currentState == 0) {
            std::cout << "WARNING: no operations, nothing to do" << std::endl;
            return false;
        }

        this->currentState = 0;
        res = vertexHistory.front();
        coordinate = coordinateHistory.front();
        return true;
    }

    bool undo(std::vector<glm::vec3>& res, std::array<glm::vec3, 3>& coordinate) {
        if(currentState == 0) {
            std::cout << "WARNING: can't undo, no more operations" << std::endl;
            return false;
        }
        
        this->currentState -= 1;
        res = vertexHistory[currentState];
        coordinate = coordinateHistory[currentState];
        return true;
    }

    bool redo(std::vector<glm::vec3>& res, std::array<glm::vec3, 3>& coordinate) {
        if(currentState == vertexHistory.size()-1) {
            std::cout << "WARNING: can't redo, no more operations" << std::endl;
            return false;
        }
        
        this->currentState += 1;
        res = vertexHistory[currentState];
        coordinate = coordinateHistory[currentState];
        return true;
    }

    void addStep(const std::vector<glm::vec3>& points, const std::array<glm::vec3, 3>& coordinate, bool useTimer = true) {
        if(!this->isActive)
            return;
        if(useTimer && std::chrono::duration<double>(std::chrono::system_clock::now() - this->timer).count() < 0.25)
            return;
        this->timer = std::chrono::system_clock::now();
        if(this->currentState < this->vertexHistory.size()-1) {
            int nbStateToDelete = (this->vertexHistory.size()-1) - this->currentState;
            for(int i = 0; i < nbStateToDelete; ++i) {
                this->vertexHistory.pop_back();
                this->coordinateHistory.pop_back();
            }
        }
        std::cout << "Add operation in history [" << points.size() << "]" << std::endl;
        this->currentState += 1;
        this->vertexHistory.push_back(points);
        this->coordinateHistory.push_back(coordinate);
    }
};

    class Manipulator;

//! @brief A point cloud with normals, a bounding box and an history.
//! This is an interface that contains all data and functions common to all meshes, this way the scene can apply function to all meshes indifferently.
class BaseMesh {

protected:
    std::vector<glm::vec3> vertices;
    
public:
    BaseMesh();

    std::vector<glm::vec3> verticesNormals;
    std::vector<glm::vec3> texCoord;// These are normalised coordinates

    glm::vec3 bbMin;
    glm::vec3 bbMax;

    //! @brief Store a stack of vertices positions to undo/redo operations.
    History * history;
    //! @brief Add the current vertices positions to the history. It also store coordinate_system as the 3D Guizmo orientation.
    //! @param useTimer If this option is True, the state is added only if 0.25sec is elapsed from the last state insertion. This option is no longer used as the insertion of a state is directly managed by MeshManipulator .
    void addStateToHistory(bool useTimer = false);

    //! @brief Model matrix of the mesh. WARNING: this attribute is currently used by the History only to store the 3D guizmo orientation. 
    //! When the user undo or redo an operation he expect the 3D guizmo to restore its orientation.
    //! So to store the 3D guizmo orientation coordinate_system attribute should be filled with the current 3D guizmo orientation before calling addStateToHistory() .
    //! \todo This attribute is incorrectly named, this is not the coordinates_system but the current 3D guizmo orientation. Moreover it do not belong to this class.
    std::array<glm::vec3, 3> coordinate_system;

    glm::vec3 getDimensions() const;

    void updatebbox();


    int getNbVertices() const;
    const glm::vec3& getVertice(int i) const;
    const std::vector<glm::vec3>& getVertices() const { return this->vertices; };
    const glm::vec3& getVerticeNormal(int i) const;
    glm::vec3 getOrigin() const;

    virtual void translate(const glm::vec3& vec);
    virtual void rotate(const glm::mat3& transf);
    virtual void scale(const glm::vec3& scale);
    virtual void setOrigin(const glm::vec3& origin);

    //! @brief Get the position of the intersection between a ray and the mesh. See Grid::getPositionOfRayIntersection().
    virtual bool getPositionOfRayIntersection(const glm::vec3& origin, const glm::vec3& direction, const std::vector<bool>& visibilityMap, const glm::vec3& planePos, glm::vec3& res) const = 0;

    virtual void movePoints(const std::vector<int>& origins, const std::vector<glm::vec3>& targets);
    void movePoints(const std::vector<glm::vec3>& targets);

    /***/

    virtual void computeNormals() = 0;
    virtual ~BaseMesh(){};
};
//! @}

#endif
