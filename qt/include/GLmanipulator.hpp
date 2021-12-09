#ifndef GL_MANIPULATOR_HPP_
#define GL_MANIPULATOR_HPP_

#include "manipulator.hpp"
#include "../../third_party/primitive/Sphere.h"
#include "../../viewer/include/scene.hpp"

#include <QOpenGLContext>
#include <QOpenGLFunctions_3_2_Core>
#include <QOpenGLFunctions_4_0_Compatibility>
#include <QOpenGLFunctions_4_0_Core>

namespace UITool {

/// @defgroup gl GL
/// @brief All classes that interact with OpenGL. Allow a separation between backend and frontend.
namespace GL {

class MeshManipulator : public QOpenGLFunctions_3_2_Core {
public:

    MeshManipulator(Scene * scene, int nbManipulators, float manipulatorRadius = 50.f): manipulatorRadius(manipulatorRadius), manipulatorMesh(Sphere(manipulatorRadius)), scene(scene), meshManipulator(nullptr) {
        this->program = 0;
        this->vao = 0;
        this->vboVertices = 0;
        this->vboIndices = 0;
        this->tex = 0;
        this->displayed = false;
    }

    void initGL(QOpenGLContext * context) {
        // Set the context for later viewers that want to connect to the scene :
        if (context == 0) {
            throw std::runtime_error("Warning : this->context() returned 0 or nullptr !");
        }
        if (context == nullptr) {
            std::cerr << "Warning : Initializing a scene without a valid OpenGL context !" << '\n';
        }
        this->context = context;

        // Get OpenGL functions from the currently bound context :
        if (this->initializeOpenGLFunctions() == false) {
            throw std::runtime_error("Could not initialize OpenGL functions.");
        }
    }

    void prepareSphere();
    void drawSphere(GLfloat* mvMat, GLfloat* pMat, GLfloat* mMat);

    void bind(UITool::MeshManipulator * meshManipulatorToBind) { this->meshManipulator = meshManipulatorToBind; };

    void setProgram(GLuint program) { this->program = program; };
    GLuint getProgram() { return this->program; };

	void setVao(GLuint vao) { this->vao = vao; };
	GLuint getVao() { return this->vao; };

    void setVboVertices(GLuint vboVertices) { this->vboVertices = vboVertices; };
    GLuint getVboVertices() { return this->vboVertices; };

    void setVboIndices(GLuint vboIndices) { this->vboIndices = vboIndices; };
    GLuint getVboIndices() { return this->vboIndices; };

    void setTex(GLuint tex) { this->tex = tex; };
    GLuint getTex() { return this->tex; };

    void toggleDisplay() { this->displayed = !this->displayed; }

    bool isDisplayed() { return this->displayed; }

private:
    UITool::MeshManipulator * meshManipulator;// TODO: shared pointer

    float manipulatorRadius;
    Sphere manipulatorMesh;

    Scene * scene;
	QOpenGLContext* context;

    GLuint program;
	GLuint vao;
    GLuint vboVertices;
    GLuint vboIndices; 
    GLuint tex;

    bool displayed;
};

}

}

#endif
