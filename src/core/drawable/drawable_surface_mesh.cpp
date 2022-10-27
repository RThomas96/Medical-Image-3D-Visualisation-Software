#include "drawable_surface_mesh.hpp"
#include "../geometry/surface_mesh.hpp"

#include <glm/gtc/type_ptr.hpp>

#include <memory>

void DrawableMesh::initializeGL(ShaderCompiler::GLFunctions *functions) {
    this->gl = functions;
    // Create the right shader program :
    auto compiler = std::make_unique<ShaderCompiler>(functions);
    compiler->vertexShader_file("../shaders/base_mesh.vert").fragmentShader_file("../shaders/base_mesh.frag");
    if (compiler->compileShaders()) {
        this->program = compiler->programName();
    } else {
        std::cerr << "Error while building shaders for drawable mesh.\n" << compiler->errorString() << "\n";
    }

    this->color = glm::vec4(1., 0., 0., 1.);
    this->lightPosition = glm::vec3(500., 500., 500.);
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

void DrawableMesh::draw(GLfloat *proj_mat, GLfloat *view_mat, const glm::vec4& camera, const glm::vec3& planePosition, const glm::vec3& planeDirection) {
    if(!this->gl) {
        std::cout << "WARNING: OpenGL functions not initialized" << std::endl;
        return;
    }
    this->gl->glUseProgram(this->program);

    GLint location_proj		       = this->gl->glGetUniformLocation(this->program, "proj");
    GLint location_view		       = this->gl->glGetUniformLocation(this->program, "view");
    GLint location_model	       = this->gl->glGetUniformLocation(this->program, "model");
    GLint location_camera_pos      = this->gl->glGetUniformLocation(this->program, "camera_pos");
    GLint location_color           = this->gl->glGetUniformLocation(this->program, "objectColor");
    GLint location_light           = this->gl->glGetUniformLocation(this->program, "lightPosition");
    GLint location_plane           = this->gl->glGetUniformLocation(this->program, "planePosition");
    GLint location_plane_direction = this->gl->glGetUniformLocation(this->program, "planeDirection");

    this->gl->glUniformMatrix4fv(location_proj, 1, GL_FALSE, proj_mat);
    this->gl->glUniformMatrix4fv(location_view, 1, GL_FALSE, view_mat);
    this->gl->glUniformMatrix4fv(location_model, 1, GL_FALSE, glm::value_ptr(glm::mat4(1.f)));
    this->gl->glUniform4fv(location_camera_pos, 1, glm::value_ptr(camera));
    this->gl->glUniform4fv(location_color, 1, glm::value_ptr(this->color));
    this->gl->glUniform3fv(location_light, 1, glm::value_ptr(this->lightPosition));
    this->gl->glUniform3fv(location_plane, 1, glm::value_ptr(planePosition));
    this->gl->glUniform3fv(location_plane_direction, 1, glm::value_ptr(planeDirection));

    // vertex buffer :
    auto vertices = mesh->getVertices();
    this->gl->glDeleteBuffers(1, &this->vbo_vertices);
    this->gl->glGenBuffers(1, &this->vbo_vertices);
    this->gl->glBindBuffer(GL_ARRAY_BUFFER, this->vbo_vertices);
    this->gl->glBufferData(GL_ARRAY_BUFFER, vertices.size() * 3 * sizeof(GLfloat), vertices.data(), GL_STATIC_DRAW);
    this->gl->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void *) 0);
    this->gl->glEnableVertexAttribArray(0);
    auto normals  = mesh->getVertexNormals();
    this->gl->glDeleteBuffers(1, &this->vbo_normals);
    this->gl->glGenBuffers(1, &this->vbo_normals);
    this->gl->glBindBuffer(GL_ARRAY_BUFFER, this->vbo_normals);
    this->gl->glBufferData(GL_ARRAY_BUFFER, normals.size() * 3 * sizeof(GLfloat), normals.data(), GL_STATIC_DRAW);
    this->gl->glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void *) 0);
    this->gl->glEnableVertexAttribArray(1);

    std::vector<GLuint> final_order;
    getSortedTriangles(mesh, camera, final_order);
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
    this->gl->glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(mesh->getTriangles().size() * 3), GL_UNSIGNED_INT, 0);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glDisable(GL_CULL_FACE);

    glEnable(GL_FLAT);
    glShadeModel(GL_FLAT);
    this->gl->glUniform4fv(location_color, 1, glm::value_ptr(this->color));
    this->gl->glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(mesh->getTriangles().size() * 3), GL_UNSIGNED_INT, 0);
    glEnable(GL_SMOOTH);
    glShadeModel(GL_SMOOTH);

    this->gl->glUseProgram(0);
}
