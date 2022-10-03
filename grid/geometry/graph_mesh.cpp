#include "graph_mesh.hpp"
#include <iostream>
#include <fstream>
#include "../utils/GLUtilityMethods.h"

void GraphMesh::loadMESH(std::string const &filename) {
    std::cout << "Opening " << filename << std::endl;

    std::ifstream myfile;
    myfile.open(filename.c_str());
    if (! myfile.is_open()) {
        std::cout << filename << " cannot be opened" << std::endl;
        return;
    }

    std::string magic_s;
    myfile >> magic_s;
    if (magic_s != "OFF") {
        std::cout << magic_s << " != OFF :   We handle ONLY *.off files." << std::endl;
        myfile.close();
        exit(1);
    }

    int n_vertices, n_faces, n_edges;
    myfile >> n_vertices >> n_faces >> n_edges;

    if (n_edges == 0) {
        std::cout << n_edges << " = 0 : Graph file empty !." << std::endl;
        myfile.close();
        exit(1);
    }

    this->vertices.clear();
    for (int v = 0; v < n_vertices; ++v)
    {
        float x, y, z;
        myfile >> x >> y >> z;
        this->vertices.push_back(glm::vec3(x, y, z));
    }

    for (int v = 0; v < n_edges; ++v)
    {
        int dummy, a, b;
        myfile >> dummy >> a >> b;
        this->mesh.push_back(GraphEdge(a-1, b-1));
    }

    this->updatebbox();
    this->history = new History(this->vertices);
}

void GraphMesh::draw(GLfloat* mvMat, GLfloat* pMat, GLfloat* mMat, const glm::vec3& planeDisplacement) {
    glPolygonMode( GL_FRONT_AND_BACK , GL_FILL );
    glColor3f(0.2,0.2,0.9);
    glClear(GL_DEPTH_BUFFER_BIT);
    glEnable(GL_LIGHTING);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_DEPTH);
    std::vector<glm::vec3> positions = this->getVertices();
    for(const auto& position : positions) {
        BasicGL::drawSphere(position.x, position.y, position.z, this->sphereRadius, 15,15);
    }

    glLineWidth(this->linesRadius);
    glDisable(GL_LIGHTING);
    glBegin(GL_LINES);
    for(const auto& edge : this->mesh) {
        glColor3f(1.,0.,0.);
        glVertex3fv(glm::value_ptr(positions[edge.pointsIdx[0]]));
        glColor3f(1.,0.,0.);
        glVertex3fv(glm::value_ptr(positions[edge.pointsIdx[1]]));
    }
    glEnd();
    glDisable(GL_LIGHTING);
}
