
#include "drawable_graph.hpp"
#include "grid/geometry/graph_mesh.hpp"
#include "../../qt/scene.hpp"

UITool::GL::Graph::Graph(SceneGL* sceneGL, GraphMesh * mesh) : manipulatorMesh(Sphere(1.)), sceneGL(sceneGL), graph(mesh)
{
    QObject::connect(dynamic_cast<QObject*>(this->graph), SIGNAL(needRedraw()), this, SLOT(prepare()));// This syntax is needed to cast an interface
    this->program	       = 0;
    this->vao		       = 0;
    this->vboVertices      = 0;
    this->vboIndices       = 0;
    this->tex		       = 0;
    this->visible          = 0;
    this->state            = 0;
    this->displayWireframe = false;
    
    this->texParams.minmag.x = GL_NEAREST;
    this->texParams.minmag.y = GL_NEAREST;
    this->texParams.lod.y	 = -1000.f;
    this->texParams.wrap.s	 = GL_CLAMP;
    this->texParams.wrap.t	 = GL_CLAMP;
    
    this->texParams.internalFormat = GL_RGB32F;
    this->texParams.size.y		   = 1;
    this->texParams.size.z		   = 1;
    this->texParams.format		   = GL_RGB;
    this->texParams.type		   = GL_FLOAT;
    
    this->texParamsVisible.minmag.x = GL_NEAREST;
    this->texParamsVisible.minmag.y = GL_NEAREST;
    this->texParamsVisible.lod.y	 = -1000.f;
    this->texParamsVisible.wrap.s	 = GL_CLAMP;
    this->texParamsVisible.wrap.t	 = GL_CLAMP;
    
    this->texParamsVisible.internalFormat = GL_RGB32F;
    this->texParamsVisible.size.y		   = 1;
    this->texParamsVisible.size.z		   = 1;
    this->texParamsVisible.format		   = GL_RGB;
    this->texParamsVisible.type		   = GL_FLOAT;
    
    this->texParamsState.minmag.x = GL_NEAREST;
    this->texParamsState.minmag.y = GL_NEAREST;
    this->texParamsState.lod.y	 = -1000.f;
    this->texParamsState.wrap.s	 = GL_CLAMP;
    this->texParamsState.wrap.t	 = GL_CLAMP;
    
    this->texParamsState.internalFormat = GL_RGB32F;
    this->texParamsState.size.y		   = 1;
    this->texParamsState.size.z		   = 1;
    this->texParamsState.format		   = GL_RGB;
    this->texParamsState.type		   = GL_FLOAT;

    this->lightPosition = glm::vec3(500., 500., 500.);
    this->manipulatorRatio = 0.006;
}

void UITool::GL::Graph::prepare() {
    if(!this->graph)
        return;
	// TODO: copy here
	std::vector<glm::vec3> allPositions;
    allPositions = this->graph->getVertices();

	this->texParams.data   = allPositions.data();
	this->texParams.size.x = allPositions.size();

    glDeleteTextures(1, &this->tex);
	this->tex = this->sceneGL->uploadTexture1D(this->texParams);

	// TODO: copy here
	std::vector<bool> rawToDisplay;
    //this->graph->getManipulatorsToDisplay(rawToDisplay);

	std::vector<glm::vec3> toDisplay;
    for(int i = 0; i < allPositions.size(); ++i)
        toDisplay.push_back(glm::vec3(1., 1., 1.));

	this->texParamsVisible.data   = toDisplay.data();
	this->texParamsVisible.size.x = toDisplay.size();

    glDeleteTextures(1, &this->visible);
	this->visible = this->sceneGL->uploadTexture1D(this->texParamsVisible);

	// TODO: copy here
    //std::vector<State> rawState;
    //this->graph->getManipulatorsState(rawState);

	std::vector<glm::vec3> state;
    for(int i = 0; i < allPositions.size(); ++i) {
        //int value = int(rawState[i]);
        //state.push_back(glm::vec3(value, value, value));
        state.push_back(glm::vec3(0, 0, 0));
    }

	this->texParamsState.data   = state.data();
	this->texParamsState.size.x = state.size();

    glDeleteTextures(1, &this->state);
	this->state = this->sceneGL->uploadTexture1D(this->texParamsState);

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

void UITool::GL::Graph::draw(GLfloat* mvMat, GLfloat* pMat, GLfloat* mMat, const glm::vec3& planeDisplacement) {
    if(!this->graph)
        return;
	auto getUniform = [&](const char* name) -> GLint {
		GLint g = this->sceneGL->glGetUniformLocation(this->program, name);
		return g;
	};

    //std::cout << planeDisplacement << std::endl;
	//if (!this->displayed)
	//	return;

	this->sceneGL->glUseProgram(this->program);

	GLint location_mMat     = getUniform("mMat");
	GLint location_vMat     = getUniform("vMat");
	GLint location_pMat     = getUniform("pMat");
	GLint location_tex	    = getUniform("positions");
	GLint location_visible	= getUniform("visible2");
	GLint location_state	= getUniform("state");
	GLint location_light	= getUniform("lightPosition");

	this->sceneGL->glUniformMatrix4fv(location_mMat, 1, GL_FALSE, mMat);
	this->sceneGL->glUniformMatrix4fv(location_vMat, 1, GL_FALSE, mvMat);
	this->sceneGL->glUniformMatrix4fv(location_pMat, 1, GL_FALSE, pMat);
	this->sceneGL->glUniform3fv(location_light, 1, glm::value_ptr(this->lightPosition));

	// Update the manipulators positions stored in the texture
	std::vector<glm::vec3> allPositions;
    allPositions = this->graph->getVertices();
    //this->meshManipulator->getAllPositions(allPositions);
    //std::cout << allPositions[0] << std::endl;

    /***/
	std::vector<bool> rawToDisplay;
    //this->graph->getManipulatorsToDisplay(rawToDisplay);

	std::vector<glm::vec3> toDisplay;
    for(int i = 0; i < allPositions.size(); ++i) {
            toDisplay.push_back(glm::vec3(1., 1., 1.));
            //if(planeDisplacement.x-15 < allPositions[i].x &&
            //   planeDisplacement.y-15 < allPositions[i].y &&
            //   planeDisplacement.z-15 < allPositions[i].z)
            //    toDisplay.push_back(glm::vec3(1., 1., 1.));
            //else
            //    toDisplay.push_back(glm::vec3(0., 0., 0.));
    }

    /***/
    //std::vector<State> rawState;
    //this->graph->getManipulatorsState(rawState);

	std::vector<glm::vec3> state;
    for(int i = 0; i < allPositions.size(); ++i) {
        //int value = int(rawState[i]);
        //state.push_back(glm::vec3(value, value, value));
        state.push_back(glm::vec3(0., 0., 0.));
    }

    //if(this->needPreview) {
    //    allPositions.push_back(this->previewPosition);
    //    toDisplay.push_back(glm::vec3(1., 1., 1.));
    //    state.push_back(glm::vec3(float(State::HIGHLIGHT), float(State::HIGHLIGHT), float(State::HIGHLIGHT)));
    //}

	this->texParams.data   = allPositions.data();
	this->texParams.size.x = allPositions.size();

	this->texParamsVisible.data   = toDisplay.data();
	this->texParamsVisible.size.x = toDisplay.size();

	this->texParamsState.data   = state.data();
	this->texParamsState.size.x = state.size();

    glDeleteTextures(1, &this->tex);
	this->tex = this->sceneGL->uploadTexture1D(this->texParams);
    glDeleteTextures(1, &this->visible);
	this->visible = this->sceneGL->uploadTexture1D(this->texParamsVisible);
    glDeleteTextures(1, &this->state);
	this->state = this->sceneGL->uploadTexture1D(this->texParamsState);
    /***/

	std::size_t tex = 0;
	this->sceneGL->glActiveTexture(GL_TEXTURE0 + tex);
	this->sceneGL->glBindTexture(GL_TEXTURE_1D, this->tex);
	this->sceneGL->glUniform1i(location_tex, tex);
	tex++;

	this->sceneGL->glActiveTexture(GL_TEXTURE0 + tex);
	this->sceneGL->glBindTexture(GL_TEXTURE_1D, this->visible);
	this->sceneGL->glUniform1i(location_visible, tex);
	tex++;

	this->sceneGL->glActiveTexture(GL_TEXTURE0 + tex);
	this->sceneGL->glBindTexture(GL_TEXTURE_1D, this->state);
	this->sceneGL->glUniform1i(location_state, tex);
	tex++;

	this->sceneGL->glBindVertexArray(this->vao);

	this->sceneGL->glEnableVertexAttribArray(0);
	this->sceneGL->glBindBuffer(GL_ARRAY_BUFFER, this->vboVertices);
	this->sceneGL->glVertexAttribPointer(0, 3, GL_FLOAT, false, this->manipulatorMesh.getInterleavedStride(), (void*) 0);

	this->sceneGL->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->vboIndices);

	// For wireframe
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glClear(GL_COLOR_BUFFER_BIT);
	this->sceneGL->glDrawElementsInstanced(GL_TRIANGLES,
	  this->manipulatorMesh.getIndexCount(),
	  GL_UNSIGNED_INT,
	  (void*) 0, allPositions.size());

	this->sceneGL->glBindVertexArray(0);

	// Unbind program, buffers and VAO :
	this->sceneGL->glDisableVertexAttribArray(0);
	this->sceneGL->glDisableVertexAttribArray(1);
	this->sceneGL->glBindBuffer(GL_ARRAY_BUFFER, 0);
	this->sceneGL->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	this->sceneGL->glUseProgram(0);

    //if(this->graph->kid_manip && this->graph->kid_manip->isVisible) {
    //    this->graph->kid_manip->draw();
    //}
}

void UITool::GL::Graph::updateManipulatorRadius(float sceneRadius) {
    this->manipulatorRadius = sceneRadius*this->manipulatorRatio;
    this->manipulatorMesh = Sphere(this->manipulatorRadius);
    this->prepare();
}
