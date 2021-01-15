#include "../include/viewer_structs.hpp"

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
	this->wrap.p = GL_REPEAT; // The component here, 'P', is named this way in order not to conflict
	// with the 'R' in 'RGBA', but this last parameter is defined as R in the GL docs/spec.

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
	// All texture handles are at 0 by default (initial value of tex names according to GL spec)

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

GridGLView::GridGLView(const std::shared_ptr<DiscreteGrid>& _g) : grid(_g) {
	this->gridTexture = 0;
	this->volumetricMesh = {};
	this->boundingBoxColor = glm::vec3(.257, .257, .257);
}

GridGLView::~GridGLView(void) { /* Nothing here for now. */ }
