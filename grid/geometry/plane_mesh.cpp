#include "plane_mesh.hpp"
#include <QOpenGLFunctions>

PlaneMesh::PlaneMesh(glm::vec3 p1, glm::vec3 p2, glm::vec3 p3) {

}

void PlaneMesh::draw() {
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_DEPTH);

    glClear(GL_COLOR_BUFFER_BIT);

    glBegin(GL_TRIANGLES);

    glColor4f(1.,0.,0., 1.);

    glVertex3f(0.,0.,0.);
    glVertex3f(1.,0.,0.);
    glVertex3f(0.,0.,1.);

    glVertex3f(1.,0.,0.);
    glVertex3f(0.,0.,1.);
    glVertex3f(1.,0.,1.);

    glEnd();

    glFlush();

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_DEPTH);
}

void PlaneMesh::computeNeighborhood() {

}

void PlaneMesh::computeNormals() {

}

bool PlaneMesh::getPositionOfRayIntersection(const glm::vec3& origin, const glm::vec3& direction, uint16_t minValue, uint16_t maxValue, glm::vec3& res) const {
    return false;
}
