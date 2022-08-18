
#include "drawable_manipulator.hpp"

UITool::GL::MeshManipulator::MeshManipulator(SceneGL* sceneGL, BaseMesh * mesh, const std::vector<glm::vec3>& positions) : planeViewRadius(1.), manipulatorMesh(Sphere(1.)), sceneGL(sceneGL), meshManipulator(new UITool::DirectManipulator(mesh, positions)) 
{
    //QObject::connect(dynamic_cast<QObject*>(this->meshManipulator), &UITool::MeshManipulator::needRedraw, this, &UITool::GL::MeshManipulator::prepare);
    QObject::connect(dynamic_cast<QObject*>(this->meshManipulator), SIGNAL(needRedraw()), this, SLOT(prepare()));// This syntax is needed to cast an interface
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
    this->meshManipulatorType = UITool::MeshManipulatorType::DIRECT;
}

void UITool::GL::MeshManipulator::prepare() {
    if(!this->meshManipulator)
        return;
	// TODO: copy here
	std::vector<glm::vec3> allPositions;
	this->meshManipulator->getAllPositions(allPositions);

	this->texParams.data   = allPositions.data();
	this->texParams.size.x = allPositions.size();

    glDeleteTextures(1, &this->tex);
	this->tex = this->sceneGL->uploadTexture1D(this->texParams);

	// TODO: copy here
	std::vector<bool> rawToDisplay;
    this->meshManipulator->getManipulatorsToDisplay(rawToDisplay);

	std::vector<glm::vec3> toDisplay;
    for(int i = 0; i < rawToDisplay.size(); ++i)
        if(rawToDisplay[i])
            toDisplay.push_back(glm::vec3(1., 1., 1.));
        else
            toDisplay.push_back(glm::vec3(0., 0., 0.));

	this->texParamsVisible.data   = toDisplay.data();
	this->texParamsVisible.size.x = toDisplay.size();

    glDeleteTextures(1, &this->visible);
	this->visible = this->sceneGL->uploadTexture1D(this->texParamsVisible);

	// TODO: copy here
	std::vector<State> rawState;
    this->meshManipulator->getManipulatorsState(rawState);

	std::vector<glm::vec3> state;
    for(int i = 0; i < rawState.size(); ++i) {
        int value = int(rawState[i]);
        state.push_back(glm::vec3(value, value, value));
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

void UITool::GL::MeshManipulator::draw(GLfloat* mvMat, GLfloat* pMat, GLfloat* mMat, const glm::vec3& planeDisplacement) {
    if(!this->meshManipulator)
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
	this->meshManipulator->getAllPositions(allPositions);
    //std::cout << allPositions[0] << std::endl;

    /***/
	std::vector<bool> rawToDisplay;
    this->meshManipulator->getManipulatorsToDisplay(rawToDisplay);

	std::vector<glm::vec3> toDisplay;
    for(int i = 0; i < rawToDisplay.size(); ++i) {
        if(rawToDisplay[i]) {
            toDisplay.push_back(glm::vec3(1., 1., 1.));
            //if(planeDisplacement.x-15 < allPositions[i].x &&
            //   planeDisplacement.y-15 < allPositions[i].y &&
            //   planeDisplacement.z-15 < allPositions[i].z)
            //    toDisplay.push_back(glm::vec3(1., 1., 1.));
            //else
            //    toDisplay.push_back(glm::vec3(0., 0., 0.));
        } else {
            toDisplay.push_back(glm::vec3(0., 0., 0.));
        }
    }

    /***/
	std::vector<State> rawState;
    this->meshManipulator->getManipulatorsState(rawState);

	std::vector<glm::vec3> state;
    for(int i = 0; i < rawState.size(); ++i) {
        int value = int(rawState[i]);
        state.push_back(glm::vec3(value, value, value));
    }

    if(this->needPreview) {
        allPositions.push_back(this->previewPosition);
        toDisplay.push_back(glm::vec3(1., 1., 1.));
        state.push_back(glm::vec3(float(State::HIGHLIGHT), float(State::HIGHLIGHT), float(State::HIGHLIGHT)));
    }

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

    if(this->meshManipulator->kid_manip && this->meshManipulator->kid_manip->isVisible) {
        this->meshManipulator->kid_manip->draw();
    }
}

void UITool::GL::MeshManipulator::toggleActivation() {
	//this->meshManipulator->setActivation(!this->meshManipulator->isActive());
    //this->displayWireframe = this->meshManipulator->isWireframeDisplayed();
    this->displayWireframe = false;
}

void UITool::GL::MeshManipulator::createNewMeshManipulator(BaseMesh * mesh, Scene * scene, MeshManipulatorType type) {
    this->manipulatorRatio = 0.002;
    this->kidRatio = 0.25;
    //scene->updateSceneRadius();
    if(this->meshManipulator) {
        UITool::CompManipulator * previousManipulator = dynamic_cast<UITool::CompManipulator*>(this->meshManipulator);
        if(previousManipulator) {
            // The previous manipulator was a comp manipulator
            std::cout << "Save register manipulator selected points" << std::endl;
            this->persistantRegistrationToolPreviousPoints = previousManipulator->previousPositions;
            this->persistantRegistrationToolSelectedPoints = previousManipulator->selectedPoints;
            this->persistantRegistrationToolSessions.push_back(previousManipulator->selectedPoints.size());
            for(auto i : this->persistantRegistrationToolSessions)
                std::cout << i << std::endl;
        }
    }

    this->displayWireframe = false;// Because wathever the manipulator created it is not activated at creation
    if(this->meshManipulator) {
        delete this->meshManipulator;
        this->meshManipulator = nullptr;
    }

    if(!mesh)
        return;

    const std::vector<glm::vec3>& positions = mesh->getMeshPositions();

    this->meshManipulatorType = type;
    if(type == MeshManipulatorType::DIRECT) {
        this->meshManipulator = new UITool::DirectManipulator(mesh, positions);
        this->manipulatorRatio = 0.005;
        this->updateManipulatorRadius(scene->getSceneRadius());
    } else if(type == MeshManipulatorType::FREE) {
        this->meshManipulator = new UITool::FreeManipulator(mesh, positions);
    } else if(type == MeshManipulatorType::POSITION) {
        this->meshManipulator = new UITool::PositionManipulator(mesh, positions);
        this->kidRatio = 0.45;
        this->updateManipulatorRadius(scene->getSceneRadius());
        //scene->updateSceneRadius();
    } else if(type == MeshManipulatorType::REGISTRATION) {
        this->meshManipulator = new UITool::CompManipulator(mesh, positions);
    } else if(type == MeshManipulatorType::ARAP) {
        this->meshManipulator = new UITool::ARAPManipulator(mesh, positions);
        this->manipulatorRatio = 0.006;
    } else if(type == MeshManipulatorType::SLICE) {
        this->meshManipulator = new UITool::SliceManipulator(mesh, positions);
        this->manipulatorRatio = 0.006;
        this->kidRatio = 0.45;
    } else if(type == MeshManipulatorType::FIXED_REGISTRATION) {
        if(scene->grids.size() > 0) {
            this->meshManipulator = new UITool::FixedRegistrationManipulator(mesh, scene->grids[0]->grid, positions);
            this->manipulatorRatio = 0.015;
            //scene->updateSceneRadius();
        } else {
            this->meshManipulator = new UITool::PositionManipulator(mesh, positions);
            std::cout << "No grid available to use the register tool" << std::endl;
            return;
        }
    }
    if(this->meshManipulator) {
        this->prepare();
        QObject::connect(dynamic_cast<QObject*>(this->meshManipulator), SIGNAL(needChangeCursor(UITool::CursorType)), scene, SLOT(changeCursor(UITool::CursorType)));
        QObject::connect(dynamic_cast<QObject*>(this->meshManipulator), SIGNAL(needChangeCursorInPlanarView(UITool::CursorType)), scene, SLOT(changeCursorInPlanarView(UITool::CursorType)));
        QObject::connect(dynamic_cast<QObject*>(this->meshManipulator), SIGNAL(needChangeSelectedPoints(std::pair<int,glm::vec3>)), scene, SLOT(changeSelectedPoint(std::pair<int,glm::vec3>)));
        QObject::connect(dynamic_cast<QObject*>(this->meshManipulator), SIGNAL(needUpdateSceneCenter()), scene, SLOT(updateSceneCenter()));
        // Scene->MeshManipulator
        QObject::connect(scene, SIGNAL(keyPressed(QKeyEvent*)), dynamic_cast<QObject*>(this->meshManipulator), SLOT(keyPressed(QKeyEvent*)));
        QObject::connect(scene, SIGNAL(keyReleased(QKeyEvent*)), dynamic_cast<QObject*>(this->meshManipulator), SLOT(keyReleased(QKeyEvent*)));
        QObject::connect(scene, SIGNAL(mousePressed(QMouseEvent*)), dynamic_cast<QObject*>(this->meshManipulator), SLOT(mousePressed(QMouseEvent*)));
        QObject::connect(scene, SIGNAL(mouseReleased(QMouseEvent*)), dynamic_cast<QObject*>(this->meshManipulator), SLOT(mouseReleased(QMouseEvent*)));
        if(type == MeshManipulatorType::SLICE) {
            QObject::connect(scene, SIGNAL(planesMoved(const glm::vec3&)), dynamic_cast<SliceManipulator*>(this->meshManipulator), SLOT(movePlanes(const glm::vec3&)));
            //dynamic_cast<SliceManipulator*>(this->meshManipulator)->selectionRadius = this->planeViewRadius;
        }
        // Scene->MeshManipulator->Selection
        QObject::connect(scene, SIGNAL(keyPressed(QKeyEvent*)), dynamic_cast<QObject*>(&this->meshManipulator->selection), SLOT(keyPressed(QKeyEvent*)));
        QObject::connect(scene, SIGNAL(keyReleased(QKeyEvent*)), dynamic_cast<QObject*>(&this->meshManipulator->selection), SLOT(keyReleased(QKeyEvent*)));

        // MeshManipulator->DrawableMeshManipulator
        QObject::connect(dynamic_cast<QObject*>(this->meshManipulator), SIGNAL(needRedraw()), this, SLOT(prepare()));
        QObject::connect(dynamic_cast<QObject*>(this->meshManipulator), SIGNAL(needSendTetmeshToGPU()), scene, SLOT(sendFirstTetmeshToGPU()));
        //QObject::connect(dynamic_cast<QObject*>(this->meshManipulator), SIGNAL(needChangeKidManipulatorRadius(float)), this, SLOT(setKidRadius(float)));

        // MeshManipulator->DrawableSelection
        QObject::connect(&this->meshManipulator->selection, &UITool::Selection::needToRedrawSelection, scene, &Scene::redrawSelection);

        this->toggleActivation();
    }
    // MeshManipulator->Scene
}


void UITool::GL::MeshManipulator::updateManipulatorRadius(float sceneRadius) {
    this->manipulatorRadius = sceneRadius*this->manipulatorRatio;
    this->kidRadius = sceneRadius*this->kidRatio;
    this->manipulatorMesh = Sphere(this->manipulatorRadius);
    this->prepare();
    if(this->meshManipulator && this->meshManipulator->kid_manip) {
        this->meshManipulator->kid_manip->setDisplayScale(this->kidRadius);
    }
}
