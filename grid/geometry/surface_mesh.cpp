#include "surface_mesh.hpp"
#include "../deformation/mesh_deformator.hpp"
#include <fstream>
#include <QOpenGLFunctions>
#include <cfloat>
#include <memory>
#include <glm/gtx/string_cast.hpp> 

void SurfaceMesh::loadOBJ(std::string const &filename) {
    std::cout << "Opening " << filename << std::endl;

    std::ifstream file;
    file.open(filename.c_str());
    if (! file.is_open()) {
        std::cout << filename << " cannot be opened" << std::endl;
        return;
    }

    this->vertices.clear();
    this->triangles.clear();
    std::string id;
    while (file >> id) {
        if(id == "v") {
            float x, y, z;
            file >> x >> y >> z;
            this->vertices.push_back(glm::vec3(x, y+1, z+2));
            //std::cout << x << " " << y << " " << z << std::endl;
        } else if (id == "f") {
            unsigned int v1, v2, v3;
            file >> v1 >> v2 >> v3;
            this->triangles.push_back(Triangle(v1-1, v2-1, v3-1));
            //std::cout << v1 << " " << v2 << " " << v3 << std::endl;
        } else {
            std::cout << "G line" << std::endl;
            std::string dummy;
            std::getline(file, dummy);
            std::cout << dummy << std::endl;
        }
    }
    file.close();
}

void SurfaceMesh::loadOFF(std::string const &filename) {
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

    int n_vertices, n_faces, dummy_int;
    myfile >> n_vertices >> n_faces >> dummy_int;

    this->vertices.clear();
    for (int v = 0; v < n_vertices; ++v)
    {
        float x, y, z;
        myfile >> x >> y >> z;
        this->vertices.push_back(glm::vec3(x, y, z));
    }

    triangles.clear();
    for (int f = 0; f < n_faces; ++f)
    {
        int n_vertices_on_face;
        myfile >> n_vertices_on_face;
        if (n_vertices_on_face == 3)
        {
            unsigned int v1, v2, v3;
            myfile >> v1 >> v2 >> v3;
            triangles.push_back(Triangle(v1, v2, v3));
        } else if (n_vertices_on_face == 4)
        {
            unsigned int v1, v2, v3, v4;

            myfile >> v1 >> v2 >> v3 >> v4;
            triangles.push_back(Triangle(v1, v2, v3));
            triangles.push_back(Triangle(v1, v3, v4));
        } else
        {
            std::cout << "We handle ONLY *.off files with 3 or 4 vertices per face" << std::endl;
            myfile.close();
            exit(1);
        }
    }

}

SurfaceMesh::SurfaceMesh(std::string const &filename) {
    if(filename.substr(filename.find_last_of(".") + 1) == "obj") {
        std::cout << "Loading OBJ" << std::endl;
        this->loadOBJ(filename);
    } else if (filename.substr(filename.find_last_of(".") + 1) == "off") {
        std::cout << "Loading OFF" << std::endl;
        this->loadOFF(filename);
    }

    this->updatebbox();

    this->computeTriangleNormal();
    this->computeVerticesNormal();
}

SurfaceMesh::SurfaceMesh(const std::vector<glm::vec3>& vertices, const std::vector<Triangle>& triangles) {
    for(int i = 0; i < vertices.size(); ++i)
        this->vertices.push_back(vertices[i]);

    for(int i = 0; i < triangles.size(); ++i)
        this->triangles.push_back(triangles[i]);

    this->updatebbox();

    this->computeTriangleNormal();
    this->computeVerticesNormal();
}

void SurfaceMesh::computeTriangleNormal() {
	this->normals.clear();
	this->normals.resize(this->triangles.size(), glm::vec3(0., 0., 0.));
	for (unsigned int i = 0; i < triangles.size(); i++) {
	    const Triangle& t = triangles[i];
	    //glm::vec3 normal  = glm::cross(vertices[t.getVertex(1)] - vertices[t.getVertex(0)], vertices[t.getVertex(2)] - vertices[t.getVertex(0)]);
	    glm::vec3 normal  = glm::cross(this->getWorldVertice(t.getVertex(1)) - this->getWorldVertice(t.getVertex(0)), this->getWorldVertice(t.getVertex(2)) - this->getWorldVertice(t.getVertex(0)));
	    this->normals[i] = glm::normalize(normal);
	}
}

void SurfaceMesh::computeVerticesNormal() {
	this->verticesNormals.clear();
	this->verticesNormals.resize(this->vertices.size(), glm::vec3(0., 0., 0.));

	for (unsigned int t = 0; t < triangles.size(); ++t)
	{
		glm::vec3 const& tri_normal = normals[t];

		this->verticesNormals[triangles[t].getVertex(0)] += tri_normal;
		this->verticesNormals[triangles[t].getVertex(1)] += tri_normal;
		this->verticesNormals[triangles[t].getVertex(2)] += tri_normal;
	}

	for (unsigned int v = 0; v < this->verticesNormals.size(); ++v)
	{
		this->verticesNormals[v] = glm::normalize(this->verticesNormals[v]);
	}
}

void SurfaceMesh::glTriangle(unsigned int i) {
	const Triangle& t = this->triangles[i];
	for (int j = 0; j < 3; j++) {
		glm::vec3 n = this->verticesNormals[t.getVertex(j)];
		glm::vec3 v = this->vertices[t.getVertex(j)];

		glNormal3f(n.x, n.y, n.z);
		glVertex3f(v.x, v.y, v.z);
	}
}

void SurfaceMesh::draw() {
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_DEPTH);

	glBegin(GL_TRIANGLES);

	for (unsigned int i = 0; i < this->triangles.size(); i++) {
	    glColor3f(1., 0., 0.);
		glTriangle(i);
	}

	glEnd();

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_DEPTH);
}

void SurfaceMesh::computeNormals() {
    this->computeTriangleNormal();
    this->computeVerticesNormal();
}

void SurfaceMesh::computeNeighborhood() {}

SurfaceMesh::~SurfaceMesh() {delete this->meshDeformator;}

bool SurfaceMesh::getPositionOfRayIntersection(const glm::vec3& origin, const glm::vec3& direction, uint16_t minValue, uint16_t maxValue, glm::vec3& res) const {
    // Not implemented yet
    std::cout << "Cast ray not implemented yet for Surface mesh" << std::endl;
    return false;
}
