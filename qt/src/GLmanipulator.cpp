
#include "../include/GLmanipulator.hpp"

void UITool::GL::MeshManipulator::prepareSphere() {
    auto getUniform = [&](const char* name) -> GLint {
        GLint g = glGetUniformLocation(this->program, name);
        return g;
    };

    GLint location_tex = getUniform("positions");

    // Struct to upload the texture to OpenGL :
    TextureUpload texParams = {};

    texParams.minmag.x = GL_NEAREST;
    texParams.minmag.y = GL_NEAREST;
    texParams.lod.y	   = -1000.f;
    texParams.wrap.s   = GL_CLAMP;
    texParams.wrap.t   = GL_CLAMP;

    std::vector<glm::vec3> allPositions;
    this->meshManipulator->getAllPositions(allPositions);
    texParams.internalFormat = GL_RGB32F;
    texParams.size.x		  = allPositions.size();;
    texParams.size.y		  = 1;
    texParams.size.z		  = 1;
    texParams.format		  = GL_RGB;
    texParams.type			  = GL_FLOAT;
    texParams.data			  = allPositions.data();
    //texParams.data			  = controllerPos;
    this->tex    = this->scene->uploadTexture1D(texParams);

    glBindTexture(GL_TEXTURE_1D, 0);
}

void UITool::GL::MeshManipulator::drawSphere(GLfloat* mvMat, GLfloat* pMat, GLfloat* mMat) {
    auto getUniform = [&](const char* name) -> GLint {
        GLint g = glGetUniformLocation(this->program, name);
        return g;
    };

    if(!this->displayed)
        return;

    glUseProgram(this->program);

    //////////////////////
    /* Prepare uniforms */
    //////////////////////

    GLint location_mMat = getUniform("mMat");
    GLint location_vMat = getUniform("vMat");
    GLint location_pMat = getUniform("pMat");
    GLint location_tex = getUniform("positions");

    glUniformMatrix4fv(location_mMat, 1, GL_FALSE, mMat);
    glUniformMatrix4fv(location_vMat, 1, GL_FALSE, mvMat);
    glUniformMatrix4fv(location_pMat, 1, GL_FALSE, pMat);

    //////////////
    /* Texture */
    //////////////

    std::size_t tex = 0;
    glActiveTexture(GL_TEXTURE0 + tex);
    glBindTexture(GL_TEXTURE_1D, this->tex);
    glUniform1i(location_tex, tex);
    tex++;

    //////////////
    /* Bind VAO */
    //////////////

    GLuint vboId = this->vboVertices;
    GLuint iboId = this->vboIndices;

    glBindBuffer(GL_ARRAY_BUFFER, vboId);           // for vertex data
    glBufferData(GL_ARRAY_BUFFER,                   // target
            this->manipulatorMesh.getInterleavedVertexSize(), // data size, # of bytes
            this->manipulatorMesh.getInterleavedVertices(),   // ptr to vertex data
            GL_STATIC_DRAW);                   // usage

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboId);   // for index data
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,           // target
            this->manipulatorMesh.getIndexSize(),             // data size, # of bytes
            this->manipulatorMesh.getIndices(),               // ptr to index data
            GL_STATIC_DRAW);                   // usage


    //////////////////////
    /* Bind to location */
    //////////////////////

    // bind VBOs

    // Encapsulate all this stuff in the VAO
    glBindVertexArray(this->vao);
    int stride = this->manipulatorMesh.getInterleavedStride();     // should be 32 bytes
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, this->vboVertices);
    glVertexAttribPointer(0,   3, GL_FLOAT, false, stride, (void*)0);

    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->vboIndices);
    glVertexAttribPointer(1,   3, GL_FLOAT, false, stride, (void*)(sizeof(float)*3));

    //////////////////

    // For wireframe
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glDrawElementsInstanced(GL_TRIANGLES,
            this->manipulatorMesh.getIndexCount(),
            GL_UNSIGNED_INT,
            (void*)0, this->meshManipulator->getNbManipulators());

    glBindVertexArray(0);
    // deactivate attrib arrays
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);

    // Unbind program, buffers and VAO :
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glUseProgram(0);
}
