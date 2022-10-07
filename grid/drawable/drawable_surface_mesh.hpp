#ifndef VISUALISATION_MESHES_DRAWABLE_SURFACE_MESH_HPP_
#define VISUALISATION_MESHES_DRAWABLE_SURFACE_MESH_HPP_

//#include "../geometry/surface_mesh.hpp"

#include "../../legacy/meshes/drawable/shaders.hpp"
#include "qobjectdefs.h"

#include <QOpenGLContext>
#include <QOpenGLExtraFunctions>
#include <QOpenGLFunctions>
#include <QOpenGLFunctions_3_2_Compatibility>
#include <QOpenGLFunctions_3_2_Core>

#include <glm/glm.hpp>
#include <glm/vector_relational.hpp>

#include <iostream>
#include <memory>
#include <vector>

#include <memory>
#include <vector>

class SurfaceMesh;

class DrawableMesh {
public:
    DrawableMesh(SurfaceMesh * mesh): gl(nullptr), mesh(mesh){};
    virtual ~DrawableMesh() = default;

    void initializeGL(ShaderCompiler::GLFunctions* functions);

    void draw(GLfloat *proj_mat, GLfloat *view_mat, const glm::vec4& camera, const glm::vec3& planePosition);

    glm::vec4 color;
    glm::vec3 lightPosition;


protected:

    SurfaceMesh * mesh;

    ShaderCompiler::GLFunctions* gl;

    GLuint program;

	GLuint vao;
	GLuint vbo_vertices;
	GLuint vbo_normals;
	GLuint vbo_indices;
};

#endif	  // VISUALISATION_MESHES_DRAWABLE_MESH_HPP_
