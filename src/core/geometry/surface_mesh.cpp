#include "surface_mesh.hpp"
//#include "../deformation/mesh_deformer.hpp"
#include "src/core/drawable/drawable_surface_mesh.hpp"
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

SurfaceMesh::SurfaceMesh(std::string const &filename): DrawableMesh(this) {
    if(filename.substr(filename.find_last_of(".") + 1) == "obj") {
        std::cout << "Loading OBJ" << std::endl;
        this->loadOBJ(filename);
    } else if (filename.substr(filename.find_last_of(".") + 1) == "off") {
        std::cout << "Loading OFF" << std::endl;
        this->loadOFF(filename);
    } else {
        throw std::runtime_error("ERROR: surface mesh loading, format not supported");
    }
    this->arapDeformer = new AsRigidAsPossible();

    this->history = new History(this->vertices, this->coordinate_system);

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
	    glm::vec3 normal  = glm::cross(this->getVertice(t.getVertex(1)) - this->getVertice(t.getVertex(0)), this->getVertice(t.getVertex(2)) - this->getVertice(t.getVertex(0)));
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

void SurfaceMesh::computeNormals() {
    this->computeTriangleNormal();
    this->computeVerticesNormal();
}

SurfaceMesh::~SurfaceMesh() {}

bool SurfaceMesh::getPositionOfRayIntersection(const glm::vec3& origin, const glm::vec3& direction, const std::vector<bool>& visibilityMap, const glm::vec3& planePos, glm::vec3& res) const {
    // Not implemented yet
    std::cout << "Cast ray not implemented yet for Surface mesh" << std::endl;
    return false;
}

void SurfaceMesh::saveOFF(std::string const & filename) {
    std::ofstream myfile;
    myfile.open(filename.c_str());
    if (!myfile.is_open())
    {
        std::cout << filename << " cannot be opened for saving" << std::endl;
        return;
    }

    myfile << "OFF" << std::endl;
    myfile << (this->vertices.size()) << " " << (this->triangles.size()) << " 0" << std::endl;

    for( unsigned int v = 0 ; v < this->vertices.size() ; ++v )
    {
        myfile << (this->vertices[v][0]) << " " << (this->vertices[v][1]) << " " << (this->vertices[v][2]) << std::endl;
    }

    for( unsigned int t = 0 ; t < this->triangles.size() ; ++t )
    {
        myfile << "3 " << (this->triangles[t][0]) << " " << (this->triangles[t][1]) << " " << (this->triangles[t][2]) << std::endl;
    }

    myfile.close();
}

void SurfaceMesh::initARAPDeformer() {
    std::vector<Vec3D<float>> ptsAsVec3D;
    for(int i = 0; i < this->mesh->getNbVertices(); ++i) {
        glm::vec3 pt = this->mesh->getVertice(i);
        ptsAsVec3D.push_back(Vec3D(pt[0], pt[1], pt[2]));
    }
    this->arapDeformer->clear();
    this->arapDeformer->init(ptsAsVec3D, dynamic_cast<SurfaceMesh*>(this->mesh)->getTriangles());
}

void SurfaceMesh::deformARAP(std::vector<glm::vec3>& positions) {
        std::vector<Vec3D<float>> ptsAsVec3D;
        for(int i = 0; i < positions.size(); ++i) {
            glm::vec3 pt = positions[i];
            ptsAsVec3D.push_back(Vec3D(pt[0], pt[1], pt[2]));
        }

        this->arapDeformer->compute_deformation(ptsAsVec3D);

        for(int i = 0; i < positions.size(); ++i)
            positions[i] = glm::vec3(ptsAsVec3D[i][0], ptsAsVec3D[i][1], ptsAsVec3D[i][2]);
        this->movePoints(positions);
}

void SurfaceMesh::setHandlesARAP(const std::vector<bool>& handles) {
    this->arapDeformer->setHandles(handles);
}
