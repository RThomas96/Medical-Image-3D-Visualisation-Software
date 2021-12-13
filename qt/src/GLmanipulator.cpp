
#include "../include/GLmanipulator.hpp"

void UITool::GL::MeshManipulator::prepare() {

    // Initialize the texture paramters for upload
    this->texParams.minmag.x = GL_NEAREST;
    this->texParams.minmag.y = GL_NEAREST;
    this->texParams.lod.y    = -1000.f;
    this->texParams.wrap.s   = GL_CLAMP;
    this->texParams.wrap.t   = GL_CLAMP;

    this->texParams.internalFormat = GL_RGB32F;
    this->texParams.size.y 		   = 1;
    this->texParams.size.z 		   = 1;
    this->texParams.format 		   = GL_RGB;
    this->texParams.type     	   = GL_FLOAT;

    // TODO: copy here
    std::vector<glm::vec3> allPositions;
    this->meshManipulator->getAllPositions(allPositions);

    this->texParams.data   = allPositions.data();
    this->texParams.size.x = allPositions.size();;

    this->tex = this->sceneGL->uploadTexture1D(this->texParams);

    // Store all these states in the VAO
    this->sceneGL->glBindVertexArray(this->vao);

    GLuint vboId = this->vboVertices;
    GLuint iboId = this->vboIndices;

    this->sceneGL->glBindBuffer(GL_ARRAY_BUFFER, vboId);
    this->sceneGL->glBufferData(GL_ARRAY_BUFFER,
            this->manipulatorMesh.getInterleavedVertexSize(),
            this->manipulatorMesh.getInterleavedVertices(),
            GL_STATIC_DRAW);

    this->sceneGL->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboId);
    this->sceneGL->glBufferData(GL_ELEMENT_ARRAY_BUFFER,
            this->manipulatorMesh.getIndexSize(),
            this->manipulatorMesh.getIndices(),
            GL_STATIC_DRAW);

    this->sceneGL->glBindVertexArray(0);
}

void UITool::GL::MeshManipulator::draw(GLfloat* mvMat, GLfloat* pMat, GLfloat* mMat) {
    auto getUniform = [&](const char* name) -> GLint {
        GLint g = this->sceneGL->glGetUniformLocation(this->program, name);
        return g;
    };

    if(!this->displayed)
        return;

    this->sceneGL->glUseProgram(this->program);

    GLint location_mMat = getUniform("mMat");
    GLint location_vMat = getUniform("vMat");
    GLint location_pMat = getUniform("pMat");
    GLint location_tex  = getUniform("positions");

    this->sceneGL->glUniformMatrix4fv(location_mMat, 1, GL_FALSE, mMat);
    this->sceneGL->glUniformMatrix4fv(location_vMat, 1, GL_FALSE, mvMat);
    this->sceneGL->glUniformMatrix4fv(location_pMat, 1, GL_FALSE, pMat);

    // Update the manipulators positions stored in the texture
    std::vector<glm::vec3> allPositions;
    this->meshManipulator->getAllPositions(allPositions);

    this->texParams.data   = allPositions.data();
    this->texParams.size.x = allPositions.size();;

    this->tex = this->sceneGL->uploadTexture1D(this->texParams);

    std::size_t tex = 0;
    this->sceneGL->glActiveTexture(GL_TEXTURE0 + tex);
	this->sceneGL->glBindTexture(GL_TEXTURE_1D, this->tex);
    this->sceneGL->glUniform1i(location_tex, tex);
    tex++;

    this->sceneGL->glBindVertexArray(this->vao);

    this->sceneGL->glEnableVertexAttribArray(0);
    this->sceneGL->glBindBuffer(GL_ARRAY_BUFFER, this->vboVertices);
    this->sceneGL->glVertexAttribPointer(0,   3, GL_FLOAT, false, this->manipulatorMesh.getInterleavedStride(), (void*)0);

    this->sceneGL->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->vboIndices);

    // For wireframe
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    this->sceneGL->glDrawElementsInstanced(GL_TRIANGLES,
            this->manipulatorMesh.getIndexCount(),
            GL_UNSIGNED_INT,
            (void*)0, this->meshManipulator->getNbManipulators());

    this->sceneGL->glBindVertexArray(0);

    // Unbind program, buffers and VAO :
    this->sceneGL->glDisableVertexAttribArray(0);
    this->sceneGL->glDisableVertexAttribArray(1);
    this->sceneGL->glBindBuffer(GL_ARRAY_BUFFER, 0);
    this->sceneGL->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    this->sceneGL->glUseProgram(0);
}
