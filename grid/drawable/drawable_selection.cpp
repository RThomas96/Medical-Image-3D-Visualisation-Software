
#include "drawable_selection.hpp"
#include "../../qt/scene.hpp"

UITool::GL::Selection::Selection(ShaderCompiler::GLFunctions* gl, const glm::vec3& p1, const glm::vec3& p2) : sceneGL(gl) {
    this->program	       = 0;
    this->vao		       = 0;
    this->vboVertices      = 0;
	this->vboNormals       = 0;
    this->vboIndices       = 0;

    this->p0 = glm::vec3(0., 0., 0.);
    this->p1 = glm::vec3(0., 0., 0.);
    this->p2 = glm::vec3(0., 0., 0.);
    this->p3 = glm::vec3(0., 0., 0.);

    this->color = glm::vec4(1., 0., 0., 0.5);
}

void UITool::GL::Selection::prepare() {
	// Store all these states in the VAO
	this->sceneGL->glBindVertexArray(this->vao);

	GLuint vboId = this->vboVertices;
	GLuint iboId = this->vboIndices;

    float vertices[] = {
        p0[0], p0[1], p0[2],
        p1[0], p1[1], p1[2],
        p2[0], p2[1], p2[2],
        p3[0], p3[1], p3[2],
    };

	this->sceneGL->glBindBuffer(GL_ARRAY_BUFFER, vboId);
	this->sceneGL->glBufferData(GL_ARRAY_BUFFER,
	  sizeof(vertices),
	  vertices,
	  GL_STATIC_DRAW);

    unsigned int id[] = {0, 1, 3, 1, 2, 3};

	this->sceneGL->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboId);
	this->sceneGL->glBufferData(GL_ELEMENT_ARRAY_BUFFER,
	  sizeof(id),
	  id,
	  GL_STATIC_DRAW);

	this->sceneGL->glBindVertexArray(0);
}

void UITool::GL::Selection::draw(GLfloat* mvMat, GLfloat* pMat, GLfloat* mMat) {
	auto getUniform = [&](const char* name) -> GLint {
		GLint g = this->sceneGL->glGetUniformLocation(this->program, name);
		return g;
	};

	this->sceneGL->glUseProgram(this->program);

	GLint location_mMat = getUniform("mMat");
	GLint location_vMat = getUniform("vMat");
	GLint location_pMat = getUniform("pMat");
	GLint location_color = getUniform("color");

	this->sceneGL->glUniformMatrix4fv(location_mMat, 1, GL_FALSE, mMat);
	this->sceneGL->glUniformMatrix4fv(location_vMat, 1, GL_FALSE, mvMat);
	this->sceneGL->glUniformMatrix4fv(location_pMat, 1, GL_FALSE, pMat);
	this->sceneGL->glUniform4fv(location_color, 1, glm::value_ptr(this->color));

	// For wireframe
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	this->sceneGL->glBindVertexArray(this->vao);

	this->sceneGL->glEnableVertexAttribArray(0);
	this->sceneGL->glBindBuffer(GL_ARRAY_BUFFER, this->vboVertices);
	this->sceneGL->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*) 0);

	this->sceneGL->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->vboIndices);

    //glClear(GL_COLOR_BUFFER_BIT);
	this->sceneGL->glDrawElements(GL_TRIANGLES,
	  6,
	  GL_UNSIGNED_INT,
	  (void*) 0);

	this->sceneGL->glBindVertexArray(0);

	// Unbind program, buffers and VAO :
	this->sceneGL->glDisableVertexAttribArray(0);
	this->sceneGL->glBindBuffer(GL_ARRAY_BUFFER, 0);
	this->sceneGL->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	this->sceneGL->glUseProgram(0);
}

void UITool::GL::Selection::setSelectionBB(const glm::vec3& p0, const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3) {
    this->p0 = p0;
    this->p1 = p1;
    this->p2 = p2;
    this->p3 = p3;
}
