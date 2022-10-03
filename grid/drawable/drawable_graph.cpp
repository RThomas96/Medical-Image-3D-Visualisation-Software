
#include "../utils/GLUtilityMethods.h"
#include "drawable_graph.hpp"
#include "grid/geometry/graph_mesh.hpp"
#include "../../qt/scene.hpp"
#include <GL/gl.h>

UITool::GL::Graph::Graph(SceneGL* sceneGL, GraphMesh * mesh) : manipulatorMesh(Sphere(1.)), sceneGL(sceneGL), graph(mesh)
{
    QObject::connect(dynamic_cast<QObject*>(this->graph), SIGNAL(needRedraw()), this, SLOT(prepare()));// This syntax is needed to cast an interface
    this->program	       = 0;
    this->vao		       = 0;
    this->vboVertices      = 0;
    this->vboIndices       = 0;
    this->tex		       = 0;
    this->visible          = 0;
    this->state            = 0;
    this->displayWireframe = false;
    
    this->texParams.minmag.x = GL_NEAREST;
    this->texParams.minmag.y = GL_NEAREST;
    this->texParams.lod.y	 = -1000.f;
    this->texParams.wrap.s	 = GL_CLAMP;
    this->texParams.wrap.t	 = GL_CLAMP;
    
    this->texParams.internalFormat = GL_RGB32F;
    this->texParams.size.y		   = 1;
    this->texParams.size.z		   = 1;
    this->texParams.format		   = GL_RGB;
    this->texParams.type		   = GL_FLOAT;
    
    this->texParamsVisible.minmag.x = GL_NEAREST;
    this->texParamsVisible.minmag.y = GL_NEAREST;
    this->texParamsVisible.lod.y	 = -1000.f;
    this->texParamsVisible.wrap.s	 = GL_CLAMP;
    this->texParamsVisible.wrap.t	 = GL_CLAMP;
    
    this->texParamsVisible.internalFormat = GL_RGB32F;
    this->texParamsVisible.size.y		   = 1;
    this->texParamsVisible.size.z		   = 1;
    this->texParamsVisible.format		   = GL_RGB;
    this->texParamsVisible.type		   = GL_FLOAT;
    
    this->texParamsState.minmag.x = GL_NEAREST;
    this->texParamsState.minmag.y = GL_NEAREST;
    this->texParamsState.lod.y	 = -1000.f;
    this->texParamsState.wrap.s	 = GL_CLAMP;
    this->texParamsState.wrap.t	 = GL_CLAMP;
    
    this->texParamsState.internalFormat = GL_RGB32F;
    this->texParamsState.size.y		   = 1;
    this->texParamsState.size.z		   = 1;
    this->texParamsState.format		   = GL_RGB;
    this->texParamsState.type		   = GL_FLOAT;

    this->lightPosition = glm::vec3(500., 500., 500.);
    this->manipulatorRatio = 0.012;
}

void UITool::GL::Graph::prepare() {
}

void UITool::GL::Graph::draw(GLfloat* mvMat, GLfloat* pMat, GLfloat* mMat, const glm::vec3& planeDisplacement) {
    if(!this->graph)
        return;
    glPolygonMode( GL_FRONT_AND_BACK , GL_FILL );
    glColor3f(0.2,0.2,0.9);
    glClear(GL_DEPTH_BUFFER_BIT);
    glEnable(GL_LIGHTING);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_DEPTH);
    std::vector<glm::vec3> positions = this->graph->getVertices();
    for(const auto& position : positions) {
        BasicGL::drawSphere(position.x, position.y, position.z, this->manipulatorRadius, 15,15);
    }

    glLineWidth(this->manipulatorRadius/2.);
    glDisable(GL_LIGHTING);
    glBegin(GL_LINES);
    for(const auto& edge : this->graph->mesh) {
        glColor3f(1.,0.,0.);
        glVertex3fv(glm::value_ptr(positions[edge.pointsIdx[0]]));
        glColor3f(1.,0.,0.);
        glVertex3fv(glm::value_ptr(positions[edge.pointsIdx[1]]));
    }
    glEnd();
    glDisable(GL_LIGHTING);
}

void UITool::GL::Graph::updateManipulatorRadius(float sceneRadius) {
    this->manipulatorRadius = sceneRadius*this->manipulatorRatio;
    this->manipulatorMesh = Sphere(this->manipulatorRadius);
    this->prepare();
}
