
#include "../utils/GLUtilityMethods.h"
#include "drawable.hpp"

UITool::GL::Drawable::Drawable()
{
    this->sphereRatio = 0.012;
    this->sphereRadius = 2.;
    this->linesRatio = 0.006;
    this->linesRadius = 1.;
}

//void UITool::GL::Drawable::draw(GLfloat* mvMat, GLfloat* pMat, GLfloat* mMat, const glm::vec3& planeDisplacement) {
//    if(!this->graph)
//        return;
//    glPolygonMode( GL_FRONT_AND_BACK , GL_FILL );
//    glColor3f(0.2,0.2,0.9);
//    glClear(GL_DEPTH_BUFFER_BIT);
//    glEnable(GL_LIGHTING);
//    glEnable(GL_DEPTH_TEST);
//    glEnable(GL_DEPTH);
//    std::vector<glm::vec3> positions = this->graph->getVertices();
//    for(const auto& position : positions) {
//        BasicGL::drawSphere(position.x, position.y, position.z, this->sphereRadius, 15,15);
//    }
//
//    glLineWidth(this->linesRadius);
//    glDisable(GL_LIGHTING);
//    glBegin(GL_LINES);
//    for(const auto& edge : this->graph->mesh) {
//        glColor3f(1.,0.,0.);
//        glVertex3fv(glm::value_ptr(positions[edge.pointsIdx[0]]));
//        glColor3f(1.,0.,0.);
//        glVertex3fv(glm::value_ptr(positions[edge.pointsIdx[1]]));
//    }
//    glEnd();
//    glDisable(GL_LIGHTING);
//}

void UITool::GL::Drawable::incrementSize(const DrawingPrimitive& object) {
    switch(object) {
        case SPHERE:
            this->sphereRatio += 0.1;
            break;
        case LINE:
            this->linesRatio += 0.1;
            break;
        default:
            break;
    }
}

void UITool::GL::Drawable::decrementSize(const DrawingPrimitive& object) {
    switch(object) {
        case SPHERE:
            this->sphereRatio -= 0.1;
            break;
        case LINE:
            this->linesRatio -= 0.1;
            break;
        default:
            break;
    }
}

void UITool::GL::Drawable::zoom(float sceneRadius) {
    this->sphereRadius = sceneRadius*this->sphereRatio;
    this->linesRadius = sceneRadius*this->linesRatio;
}
