#include "surface_mesh.hpp"
#include "../deformation/mesh_deformer.hpp"
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
    } else {
        throw std::runtime_error("ERROR: surface mesh loading, format not supported");
    }

    this->history = new History(this->vertices);

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

    //GLfloat model[16];
    //glGetFloatv(GL_MODELVIEW_MATRIX, model);

    //glPushMatrix();
    //glLoadMatrixf(glm::value_ptr(glm::make_mat4(model) * this->getModelMatrix()));

    //glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );

	glBegin(GL_TRIANGLES);

	for (unsigned int i = 0; i < this->triangles.size(); i++) {
	    glColor3f(1., 0., 0.);
		glTriangle(i);
	}

	glEnd();

    glPopMatrix();

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_DEPTH);
}

void SurfaceMesh::computeNormals() {
    this->computeTriangleNormal();
    this->computeVerticesNormal();
}

void SurfaceMesh::computeNeighborhood() {}

SurfaceMesh::~SurfaceMesh() {delete this->meshDeformer;}

bool SurfaceMesh::getPositionOfRayIntersection(const glm::vec3& origin, const glm::vec3& direction, uint16_t minValue, uint16_t maxValue, const glm::vec3& planePos, glm::vec3& res) const {
    // Not implemented yet
    std::cout << "Cast ray not implemented yet for Surface mesh" << std::endl;
    return false;
}

void SurfaceMesh::setARAPDeformationMethod() {
    if(this->meshDeformer->deformMethod != DeformMethod::ARAP) {
        delete this->meshDeformer;
        this->meshDeformer = new ARAPMethod(this);
    }
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

void getSortedTriangles(SurfaceMesh * mesh, const glm::vec3& cam, std::vector<GLuint>& indices) {
    std::vector<std::pair<float, int>> dists;
    dists.reserve(mesh->getTriangles().size());
    for(int i = 0; i < mesh->getTriangles().size(); ++i) {
        glm::vec3 center(0., 0., 0.);
        for(int j = 0; j < 3; ++j)
            center += mesh->getVertice(mesh->getTriangles()[i][j]);
        dists.push_back(std::make_pair(glm::distance(center/3.f, cam), i));
    }
    struct less_than_key {
        inline bool operator() (const std::pair<float, int>& struct1, const std::pair<float, int>& struct2) {
            return (struct1.first > struct2.first);
        }
    };
    std::sort(dists.begin(), dists.end(), less_than_key());
    indices.reserve(mesh->getNbVertices());
    for(int i = 0; i < dists.size(); ++i) {
        for(int j = 0; j < 3; ++j)
            indices.push_back(mesh->triangles[dists[i].second][j]);
    }
}

void SurfaceMesh::draw(GLfloat *proj_mat, GLfloat *view_mat, const glm::vec4& camera, const glm::vec3& planePosition) {
    if(!this->gl) {
        std::cout << "WARNING: OpenGL functions not initialized" << std::endl;
        return;
    }
    this->gl->glUseProgram(this->program);

    GLint location_proj		    = this->gl->glGetUniformLocation(this->program, "proj");
    GLint location_view		    = this->gl->glGetUniformLocation(this->program, "view");
    GLint location_model	    = this->gl->glGetUniformLocation(this->program, "model");
    GLint location_camera_pos   = this->gl->glGetUniformLocation(this->program, "camera_pos");
    GLint location_color        = this->gl->glGetUniformLocation(this->program, "objectColor");
    GLint location_light        = this->gl->glGetUniformLocation(this->program, "lightPosition");
    GLint location_plane        = this->gl->glGetUniformLocation(this->program, "planePosition");

    this->gl->glUniformMatrix4fv(location_proj, 1, GL_FALSE, proj_mat);
    this->gl->glUniformMatrix4fv(location_view, 1, GL_FALSE, view_mat);
    this->gl->glUniformMatrix4fv(location_model, 1, GL_FALSE, glm::value_ptr(glm::mat4(1.f)));
    this->gl->glUniform4fv(location_camera_pos, 1, glm::value_ptr(camera));
    this->gl->glUniform4fv(location_color, 1, glm::value_ptr(this->color));
    this->gl->glUniform3fv(location_light, 1, glm::value_ptr(this->lightPosition));
    this->gl->glUniform3fv(location_plane, 1, glm::value_ptr(planePosition));

    // vertex buffer :
    auto vertices = this->getVertices();
    this->gl->glDeleteBuffers(1, &this->vbo_vertices);
    this->gl->glGenBuffers(1, &this->vbo_vertices);
    this->gl->glBindBuffer(GL_ARRAY_BUFFER, this->vbo_vertices);
    this->gl->glBufferData(GL_ARRAY_BUFFER, vertices.size() * 3 * sizeof(GLfloat), vertices.data(), GL_STATIC_DRAW);
    this->gl->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void *) 0);
    this->gl->glEnableVertexAttribArray(0);
    auto normals  = this->getVertexNormals();
    this->gl->glDeleteBuffers(1, &this->vbo_normals);
    this->gl->glGenBuffers(1, &this->vbo_normals);
    this->gl->glBindBuffer(GL_ARRAY_BUFFER, this->vbo_normals);
    this->gl->glBufferData(GL_ARRAY_BUFFER, normals.size() * 3 * sizeof(GLfloat), normals.data(), GL_STATIC_DRAW);
    this->gl->glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void *) 0);
    this->gl->glEnableVertexAttribArray(1);

    std::vector<GLuint> final_order;
    getSortedTriangles(this, camera, final_order);
    this->gl->glDeleteBuffers(1, &this->vbo_indices);
    this->gl->glGenBuffers(1, &this->vbo_indices);
    this->gl->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->vbo_indices);
    this->gl->glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLsizeiptr>(final_order.size()) * sizeof(GLuint), final_order.data(), GL_STATIC_DRAW);

    /***/

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_CULL_FACE);

    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glLineWidth(1.0f);
    this->gl->glUniform4fv(location_color, 1, glm::value_ptr(glm::vec4(0.6, 0.6, 0.6, 1)));
    this->gl->glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(this->getTriangles().size() * 3), GL_UNSIGNED_INT, 0);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glDisable(GL_CULL_FACE);

    glEnable(GL_FLAT);
    glShadeModel(GL_FLAT);
    this->gl->glUniform4fv(location_color, 1, glm::value_ptr(this->color));
    this->gl->glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(this->getTriangles().size() * 3), GL_UNSIGNED_INT, 0);
    glEnable(GL_SMOOTH);
    glShadeModel(GL_SMOOTH);

    this->gl->glUseProgram(0);
}
