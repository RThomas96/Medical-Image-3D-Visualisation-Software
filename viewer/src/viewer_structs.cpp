#include "../include/viewer_structs.hpp"

#define CASE_GL(x) case x : std::cerr << #x ; break;
#define DEFAULT_GL() default : std::cerr << "<unknown>" ; break;

TextureUpload::TextureUpload(void) {
	// All those values are the initial values of the parameters they represent. Taken from
	// the OpenGL specification, version 4.5.

	// Min and mag filters :
	this->minmag.x = GL_NEAREST_MIPMAP_LINEAR; // Min filter
	this->minmag.y = GL_LINEAR; // Mag filter
	// LOD parameters :
	this->lod.x = -1000.f;
	this->lod.y =  1000.f;
	// Wrap parameters :
	this->wrap.s = GL_REPEAT;
	this->wrap.t = GL_REPEAT;
	this->wrap.p = GL_REPEAT; // The component here, 'P', is named this way in order not to
	// conflict with the 'R' in 'RGBA', but this last parameter is defined as R in the
	// GL docs/spec.

	// Swizzle components :
	this->swizzle.r = GL_RED;
	this->swizzle.g = GL_GREEN;
	this->swizzle.b = GL_BLUE;
	this->swizzle.a = GL_ALPHA;
	// Pack/Unpack pixel alignment :
	this->alignment.x = 4;
	this->alignment.y = 4;

	this->level = 0;
	this->internalFormat = GL_RGBA;
	this->size.x = 0;
	this->size.y = 0;
	this->size.z = 0;
	this->format = GL_RGBA;
	this->type = GL_FLOAT;
	this->data = nullptr;
}

void TextureUpload::printInfo() {
	// Struct :
	//  - minmag
	//  - lod
	//  - wrap
	//  - swizzle
	//  - alignment
	//  - level
	//  - internalformat
	//  - size
	//  - format
	//  - type
	std::cerr << "Info about the struct :" <<"\n";
	std::cerr << "\t- minmag : [";
	switch (this->minmag.x) {
		CASE_GL(GL_NEAREST);
		CASE_GL(GL_LINEAR);
		CASE_GL(GL_NEAREST_MIPMAP_NEAREST);
		CASE_GL(GL_NEAREST_MIPMAP_LINEAR);
		CASE_GL(GL_LINEAR_MIPMAP_NEAREST);
		CASE_GL(GL_LINEAR_MIPMAP_LINEAR);
		DEFAULT_GL();
	} std::cerr << ", ";
	switch (this->minmag.y) {
		CASE_GL(GL_NEAREST);
		CASE_GL(GL_LINEAR);
		DEFAULT_GL();
	}
	std::cerr << "],\n\t- lod:{" << this->lod.x << "," << this->lod.y << "},\n\t- wrap:[";
	switch (this->wrap.x) {
		CASE_GL(GL_CLAMP_TO_EDGE);
		CASE_GL(GL_CLAMP_TO_BORDER);
		CASE_GL(GL_MIRRORED_REPEAT);
		CASE_GL(GL_MIRROR_CLAMP_TO_EDGE);
		CASE_GL(GL_REPEAT);
		DEFAULT_GL();
	} std::cerr << ", ";
	switch (this->wrap.y) {
		CASE_GL(GL_CLAMP_TO_EDGE);
		CASE_GL(GL_CLAMP_TO_BORDER);
		CASE_GL(GL_MIRRORED_REPEAT);
		CASE_GL(GL_MIRROR_CLAMP_TO_EDGE);
		CASE_GL(GL_REPEAT);
		DEFAULT_GL();
	} std::cerr << ", ";
	switch (this->wrap.z) {
		CASE_GL(GL_CLAMP_TO_EDGE);
		CASE_GL(GL_CLAMP_TO_BORDER);
		CASE_GL(GL_MIRRORED_REPEAT);
		CASE_GL(GL_MIRROR_CLAMP_TO_EDGE);
		CASE_GL(GL_REPEAT);
		DEFAULT_GL();
	}
	std::cerr << "],\n\t- swizzle : [";
	switch (this->swizzle.x) {
		CASE_GL(GL_RED);
		CASE_GL(GL_GREEN);
		CASE_GL(GL_BLUE);
		CASE_GL(GL_ALPHA);
		CASE_GL(GL_ZERO);
		CASE_GL(GL_ONE);
		DEFAULT_GL();
	} std::cerr << ", " ;
	switch (this->swizzle.y) {
		CASE_GL(GL_RED);
		CASE_GL(GL_GREEN);
		CASE_GL(GL_BLUE);
		CASE_GL(GL_ALPHA);
		CASE_GL(GL_ZERO);
		CASE_GL(GL_ONE);
		DEFAULT_GL();
	} std::cerr << ", " ;
	switch (this->swizzle.z) {
		CASE_GL(GL_RED);
		CASE_GL(GL_GREEN);
		CASE_GL(GL_BLUE);
		CASE_GL(GL_ALPHA);
		CASE_GL(GL_ZERO);
		CASE_GL(GL_ONE);
		DEFAULT_GL();
	} std::cerr << ", " ;
	switch (this->swizzle.a) {
		CASE_GL(GL_RED);
		CASE_GL(GL_GREEN);
		CASE_GL(GL_BLUE);
		CASE_GL(GL_ALPHA);
		CASE_GL(GL_ZERO);
		CASE_GL(GL_ONE);
		DEFAULT_GL();
	}
	std::cerr << "],\n\t- alignment:{" << this->alignment.x << "," << this->alignment.y <<
		     "}\n\t- level : " << this->level << ",\n\t- internalFormat : ";
	switch (this->internalFormat) {
		CASE_GL(GL_R16UI);
		CASE_GL(GL_R8UI);
		CASE_GL(GL_RG8UI);
		CASE_GL(GL_RG16UI);
		DEFAULT_GL();
	}
	std::cerr << ",\n\t- size:[" << this->size.x << "," << this->size.y << "," <<
		     this->size.z << "],\n\t- format : ";
	switch(this->format) {
		CASE_GL(GL_RED);
		CASE_GL(GL_RED_INTEGER);
		CASE_GL(GL_RG);
		CASE_GL(GL_RG_INTEGER);
		DEFAULT_GL();
	}
	std::cerr << ",\n\t- type : ";
	switch (this->type) {
		CASE_GL(GL_UNSIGNED_BYTE);
		CASE_GL(GL_UNSIGNED_SHORT);
		CASE_GL(GL_BYTE);
		CASE_GL(GL_SHORT);
		DEFAULT_GL();
	}
	std::cerr << '\n';
}

TextureUpload::~TextureUpload() {}

Mesh::Mesh(void) {
	this->positions.clear();
	this->normals.clear();
	this->texture.clear();
	this->indices.clear();
	this->cutting_planes.clear();
	this->planar_view.clear();
}

Mesh::~Mesh(void) {
	this->positions.clear();
	this->normals.clear();
	this->texture.clear();
	this->indices.clear();
	this->cutting_planes.clear();
	this->planar_view.clear();
}

VolMeshData::VolMeshData() {
	this->positions.clear();
	this->texture.clear();
	this->tetrahedra.clear();
	this->neighbors.clear();
	this->normals.clear();
}

VolMeshData::~VolMeshData() {
	this->positions.clear();
	this->texture.clear();
	this->tetrahedra.clear();
	this->neighbors.clear();
	this->normals.clear();
}

VolMesh::VolMesh(void) {
	// All texture handles are at 0 by default (initial value
	// of tex names according to GL spec version 4.5)

	this->visibilityMap = 0;
	this->vertexPositions = 0;
	this->textureCoordinates = 0;
	this->neighborhood = 0;
	this->faceNormals = 0;
	this->tetrahedraCount = 0;
}

bool VolMesh::isValid() {
	return this->visibilityMap != 0 &&
		this->vertexPositions != 0 &&
		this->textureCoordinates != 0 &&
		this->neighborhood != 0 &&
		this->faceNormals != 0 &&
		this->tetrahedraCount > 0;
}

VolMesh::~VolMesh(void) { /* Nothing here for now. */ }

GridGLView::GridGLView(const std::initializer_list<std::shared_ptr<DiscreteGrid>> _g) {
	if (_g.size() == 0) {
		throw std::runtime_error("Cannot create GL view from no grids");
	}
	if (_g.size() > 2) {
		throw std::runtime_error("Cannot create GL view from more than 2 grids");
	}

	std::for_each(_g.begin(),_g.end(), [this](const std::shared_ptr<DiscreteGrid>& _grid){
		this->grid.emplace_back(_grid);
	});
	this->nbChannels = this->grid.size();

	this->gridTexture = 0;
	this->volumetricMesh = {};
	this->boundingBoxColor = glm::vec3(.257, .257, .257);
	this->nbChannels = 1;
	this->defaultEpsilon = glm::vec3(1.5, 1.5, 1.5);
	if (this->grid[0]->getGridReader() != nullptr) {
		this->texBounds0 = this->grid[0]->getGridReader()->getTextureLimits();
		this->colorBounds0 = this->texBounds0;
		if (nbChannels > 1) {
			this->texBounds1 = this->grid[1]->getGridReader()->getTextureLimits();
			this->colorBounds1 = this->texBounds1;
		}
	} else {
		data_2 min(0,0);
		this->texBounds0 = min;
		this->texBounds1 = min;
		this->colorBounds0 = min;
		this->colorBounds1 = min;
	}
}

GridGLView::GridGLView() {
	this->gridTexture = 0;
	this->volumetricMesh = {};
	this->boundingBoxColor = glm::vec3(.257, .257, .257);
	this->nbChannels = 1;
	this->defaultEpsilon = glm::vec3(1.5, 1.5, 1.5);
	data_2 min(0,0);
	this->texBounds0 = min;
	this->texBounds1 = min;
	this->colorBounds0 = min;
	this->colorBounds1 = min;
}

GridGLView::GridGLView(const std::shared_ptr<DiscreteGrid> _red,
			   const std::shared_ptr<DiscreteGrid> _blue) {
	this->grid.emplace_back(_red);
	this->grid.emplace_back(_blue);

	this->gridTexture = 0;
	this->volumetricMesh = {};
	this->boundingBoxColor = glm::vec3(.257, .257, .257);
	this->nbChannels = 1;
	this->defaultEpsilon = glm::vec3(1.5, 1.5, 1.5);
	if (this->grid[0]->getGridReader() != nullptr) {
		this->texBounds0 = this->grid[0]->getGridReader()->getTextureLimits();
		this->colorBounds0 = this->texBounds0;
		if (nbChannels > 1) {
			this->texBounds1 = this->grid[1]->getGridReader()->getTextureLimits();
			this->colorBounds1 = this->texBounds1;
		}
	} else {
		data_2 min(0,0);
		this->texBounds0 = min;
		this->texBounds1 = min;
		this->colorBounds0 = min;
		this->colorBounds1 = min;
	}
}

GridGLView::~GridGLView(void) { /* Nothing here for now. */ }
