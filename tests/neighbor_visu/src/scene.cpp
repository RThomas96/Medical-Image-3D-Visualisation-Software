#include "../include/scene.hpp"

#include <glm/gtx/transform.hpp>

// TODO : change the inspectorPosNormlized by inspectorSizeNormalized and inspectorPosition
// TODO : check we passed all attributes to the shader
// TODO	: allow for the texture cube to be drawn
// TODO : test the class

Scene::Scene(void) {
	this->vao = nullptr;
	this->program = nullptr;
	this->drawRealVoxelSize = false;
	this->textureHandle = 0;
	this->pMatrixLocation = 0;
	this->vMatrixLocation = 0;
	this->mMatrixLocation = 0;
	this->transformationMatrixLocation = 0;
	this->texDataLocation = 0;
	this->texOffsetLocation = 0;
	this->inspectorTexSizeLocation = 0;
	this->positionNormalized = glm::vec3(.0f,.0f,.0f);
	this->gridWidth = 0;
	this->gridHeight = 0;
	this->gridDepth = 0;
	this->neighborWidth = 0;
	this->neighborHeight= 0;
	this->neighborDepth = 0;
	this->isInitialized = false;
	this->loader = new bulk_texture_loader();
	this->transposeMatrices = GL_FALSE;
}

Scene::~Scene(void) {
	delete this->vao;
	delete this->program;

	glDeleteTextures(1, &this->textureHandle);
}

void Scene::initGl(std::size_t _x, std::size_t _y, std::size_t _z) {
	if (this->isInitialized == true) { return; }
	this->isInitialized = true;

	glewExperimental = false;
	if (glewInit() != GLEW_OK) {
		std::cerr << "Could not start GLEW" << '\n';
		exit(EXIT_FAILURE);
	}

	std::size_t pos;
	VBOCreator vert = {pos, 4, GL_FLOAT};
	VBOCreator uvco = {pos, 2, GL_FLOAT};
	VBOCreator elem = {pos, 1, GL_UNSIGNED_INT};
	this->vao = new VAOObject(VBOTypes::Vertex, vert, VBOTypes::TexCoords, uvco, VBOTypes::IndexBuffer, elem);

	// Create the vertex, fragment shaders and program
	std::shared_ptr<ShaderObject> vSha = std::make_shared<ShaderObject>("./base_scene_vshader.glsl", GL_VERTEX_SHADER);
	std::shared_ptr<ShaderObject> fSha = std::make_shared<ShaderObject>("./base_scene_fshader.glsl", GL_FRAGMENT_SHADER);
	this->program = new ProgramObject(vSha, nullptr, fSha);
	this->program->linkShaders();

	this->generateGrid(_x,_y,_z);
	this->queryImage();

	this->program->use();
	this->pMatrixLocation = this->program->getUniformLocation("pMatrix");
	this->vMatrixLocation = this->program->getUniformLocation("vMatrix");
	this->mMatrixLocation = this->program->getUniformLocation("mMatrix");
	this->transformationMatrixLocation = this->program->getUniformLocation("transformationMatrix");
	this->texDataLocation = this->program->getUniformLocation("texData");
	this->texOffsetLocation = this->program->getUniformLocation("texOffset");
	this->inspectorTexSizeLocation = this->program->getUniformLocation("inspectorTexSize");
}

void Scene::loadImage(std::size_t i, std::size_t j, std::size_t k, const unsigned char *pData) {
	if (pData == nullptr) { this->loadEmptyImage(); }

	this->gridWidth = i;
	this->gridHeight= j;
	this->gridDepth = k;

	if (this->textureHandle == 0) {
		glGenTextures(1, &this->textureHandle);
	}
	glBindTexture(GL_TEXTURE_3D, this->textureHandle);

	// Set nearest neighbor :
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	// Set the texture upload to not generate mimaps :
	glTexParameterf(GL_TEXTURE_3D, GL_TEXTURE_MAX_LOD, static_cast<GLfloat>(-1000.f));
	// Stop once UV > 1 or < 0
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	// Swizzle G/B to R value, to save data upload
	GLint swizzleMask[] = {GL_RED, GL_RED, GL_RED, GL_ONE};
	glTexParameteriv(GL_TEXTURE_3D, GL_TEXTURE_SWIZZLE_RGBA, swizzleMask);

	glTexImage3D(
		GL_TEXTURE_3D,		// GLenum : Target
		static_cast<GLint>(0),	// GLint  : Level of detail of the current texture (0 = original)
		GL_RED,			// GLint  : Number of color components in the picture. Here grayscale so GL_RED
		static_cast<GLsizei>(i),// GLsizei: Image width
		static_cast<GLsizei>(j),// GLsizei: Image height
		static_cast<GLsizei>(k),// GLsizei: Image depth (number of layers)
		static_cast<GLint>(0),	// GLint  : Border. This value MUST be 0.
		GL_RED,			// GLenum : Format of the pixel data
		GL_UNSIGNED_BYTE,	// GLenum : Type (the data type as in uchar, uint, float ...)
		pData			// void*  : Data to load into the buffer
	);
}

void Scene::queryImage(void) {
	const unsigned char* image = this->loader->load_stack_from_folder();
	std::size_t i = this->loader->get_image_width();
	std::size_t j = this->loader->get_image_height();
	std::size_t k = this->loader->get_image_depth();

	this->loadImage(i, j, k, image);
}

void Scene::reloadShaders(void) const {
	if (this->program != nullptr) {
		this->program->reloadShaders();
	}
}

void Scene::drawRealSpace(GLfloat mvMat[], GLfloat pMat[], bool bDrawWireframe) const {
	if (this->program == nullptr || this->vao == nullptr) { std::cerr << "Nothing here " << std::endl; return; }

	this->program->use();
	this->vao->bind();
	this->vao->enableVertexAttributes();
	if (bDrawWireframe) {
		// Set the mode to wireframe :
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}

	//glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_3D, this->textureHandle);
	this->program->setUniform1ui("texData", 0); // set texture sampler in texture slot 0

	glm::mat4 transfoMat = glm::mat4();
	if (this->drawRealVoxelSize) {
		transfoMat *= glm::scale(glm::vec3(0.39, 0.39, 1.927));
	}
	glm::vec3 posAbsolute(static_cast<float>(this->gridWidth) * this->positionNormalized.x,
			      static_cast<float>(this->gridHeight)* this->positionNormalized.y,
			      static_cast<float>(this->gridDepth) * this->positionNormalized.z);
	transfoMat *= glm::translate(posAbsolute);

	this->program->setUniformMatrix4fv("transformationMatrix", 1, this->transposeMatrices, &transfoMat[0][0]);

	this->program->setUniformMatrix4fv("mvMatrix", 1, this->transposeMatrices, mvMat);
	this->program->setUniformMatrix4fv("pMatrix", 1, this->transposeMatrices, pMat);
	this->program->setUniform3fv("texOffset", 1, &this->positionNormalized[0]);

	this->vao->draw();

	this->vao->disableVertexAttributes();
	this->vao->unBind();
	this->program->release();
	// DISABLE VAO
}

void Scene::drawInitialSpace(GLfloat mvMat[], GLfloat pMat[], bool bDrawWireframe) const {
	this->program->use();
	this->vao->bind();
	this->vao->enableVertexAttributes();

	if (bDrawWireframe) {
		// Set the mode to wireframe :
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}

	glm::mat4 transfoMat = glm::mat4(1.0);
	this->program->setUniformMatrix4fv("transformationMatrix", 1, GL_FALSE, &transfoMat[0][0]);
	this->program->setUniformMatrix4fv("mvMatrix", 1, GL_FALSE, mvMat);
	this->program->setUniformMatrix4fv("pMatrix", 1, GL_FALSE, pMat);
	this->vao->draw();

	this->vao->disableVertexAttributes();
	this->vao->unBind();
	glUseProgram(0);
}

void Scene::generateGrid(std::size_t _x, std::size_t _y, std::size_t _z) {
	std::vector<glm::vec4> vertPos;
	std::vector<glm::vec3> vertTex;
	std::vector<GLuint> vertIdx;
	this->neighborWidth = _x;
	this->neighborHeight = _y;
	this->neighborDepth = _z;

	// Create the grid, in raw form :
	for (std::size_t i = 0; i <= _z; ++i) {
		for (std::size_t j = 0; j <= _y; ++j) {
			for (std::size_t k = 0; k <= _x; ++k) {
				vertPos.push_back(glm::vec4(static_cast<float>(k),static_cast<float>(j),static_cast<float>(i), 1.f));
				float xtex = static_cast<float>(k) / static_cast<float>(_x);
				float ytex = static_cast<float>(j) / static_cast<float>(_y);
				float ztex = static_cast<float>(i) / static_cast<float>(_z);
				vertTex.push_back(glm::vec3(xtex,ytex,ztex));
			}
		}
	}

	// Create the index buffer used to draw it in order :
	// (see notes for reference about draw order)
	for (std::size_t i = 0; i < _z; ++i) {
		for (std::size_t j = 0; j < _y; ++j) {
			for (std::size_t k = 0; k < _x; ++k) {
				/**
				 * Name references :
				 *     a-------b-->+k
				 *    /|      /|
				 *   c-------d |
				 *  /| |     | |
				 *+j | |e----|-|f
				 *   |/|     |/
				 *   g-------h
				 *     V
				 *    +i
				 */
				GLuint a = static_cast<GLuint>((k+0) + (j+0)*_x + (i+0)*_x*_y);
				GLuint b = static_cast<GLuint>((k+1) + (j+0)*_x + (i+0)*_x*_y);
				GLuint c = static_cast<GLuint>((k+0) + (j+1)*_x + (i+0)*_x*_y);
				GLuint d = static_cast<GLuint>((k+1) + (j+1)*_x + (i+0)*_x*_y);
				GLuint e = static_cast<GLuint>((k+0) + (j+0)*_x + (i+1)*_x*_y);
				GLuint f = static_cast<GLuint>((k+1) + (j+0)*_x + (i+1)*_x*_y);
				GLuint g = static_cast<GLuint>((k+0) + (j+1)*_x + (i+1)*_x*_y);
				GLuint h = static_cast<GLuint>((k+1) + (j+1)*_x + (i+1)*_x*_y);

				// Draw the half of the cube atteinable from the position a :
				// Faces :	   |    1   |    2   |    3   |    4   |    5   |   6   |
				GLuint faceIdx1[]= {a, d, c, a, b, c, e, b, a, e, f, b, g, e, a, g, a, c};
				// Draw the other side :
				// Faces :	   |    1   |    2   |    3   |    4   |    5   |   6   |
				GLuint faceIdx2[]= {h, e, g, h, f, e, h, g, c, h, c, d, h, d, b, h, d, f};

				// Insert them into the vector :
				vertIdx.insert(vertIdx.end(), faceIdx1, faceIdx1+18);
				vertIdx.insert(vertIdx.end(), faceIdx2, faceIdx2+18);
			}
		}
	}

	// Upload vertex positions :
	this->vao->setVBOData<VBOTypes::Vertex>(vertPos.size(), 4, vertPos.size()*sizeof(glm::vec4), vertPos.data(), GL_STATIC_DRAW, GL_FLOAT);
	// Upload texture coordinates :
	this->vao->setVBOData<VBOTypes::TexCoords>(vertTex.size(), 3, vertTex.size()*sizeof(glm::vec3), vertTex.data(), GL_STATIC_DRAW, GL_FLOAT);
	// Upload the index buffer :
	this->vao->setVBOData<VBOTypes::IndexBuffer>(vertIdx.size(), 1, vertIdx.size()*sizeof(GLuint), vertIdx.data(), GL_STATIC_DRAW, GL_UNSIGNED_INT);

	std::cerr << "Inserted " << vertIdx.size() << " elements into element buffer. " << '\n';
}

void Scene::toggleRealVoxelSize() {
	this->drawRealVoxelSize = !this->drawRealVoxelSize;
}

void Scene::loadEmptyImage() {
	if (this->textureHandle == 0) {
		glGenTextures(1, &this->textureHandle);
	}

	std::size_t i = 10;
	std::size_t j = 10;
	std::size_t k = 10;

	const char* pData = static_cast<char*>(calloc(i*j*k, sizeof(char)));

	this->gridWidth = i;
	this->gridHeight= j;
	this->gridDepth = k;


	glBindTexture(GL_TEXTURE_3D, this->textureHandle);

	// Set nearest neighbor :
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	// Set the texture upload to not generate mimaps :
	glTexParameterf(GL_TEXTURE_3D, GL_TEXTURE_MAX_LOD, static_cast<GLfloat>(-1000.f));
	// Stop once UV > 1 or < 0
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	// Swizzle G/B to R value, to save data upload
	GLint swizzleMask[] = {GL_RED, GL_RED, GL_RED, GL_ONE};
	glTexParameteriv(GL_TEXTURE_3D, GL_TEXTURE_SWIZZLE_RGBA, swizzleMask);

	glTexImage3D(
		GL_TEXTURE_3D,		// GLenum : Target
		static_cast<GLint>(0),	// GLint  : Level of detail of the current texture (0 = original)
		GL_RED,			// GLint  : Number of color components in the picture. Here grayscale so GL_RED
		static_cast<GLsizei>(i),// GLsizei: Image width
		static_cast<GLsizei>(j),// GLsizei: Image height
		static_cast<GLsizei>(k),// GLsizei: Image depth (number of layers)
		static_cast<GLint>(0),	// GLint  : Border. This value MUST be 0.
		GL_RED,			// GLenum : Format of the pixel data
		GL_UNSIGNED_BYTE,	// GLenum : Type (the data type as in uchar, uint, float ...)
		pData			// void*  : Data to load into the buffer
	);

	return;
}

glm::vec3 Scene::getSceneBoundaries() const {
	return glm::vec3(
		static_cast<float>(std::max(this->gridWidth, this->neighborWidth)),
		static_cast<float>(std::max(this->gridHeight, this->neighborHeight)),
		static_cast<float>(std::max(this->gridDepth, this->neighborDepth))
	);
}
