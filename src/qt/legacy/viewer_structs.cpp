#include "viewer_structs.hpp"

#define CASE_GL(x)       \
	case x:              \
		std::cerr << #x; \
		break;
#define DEFAULT_GL()              \
	default:                      \
		std::cerr << "<unknown>"; \
		break;

TextureUpload::TextureUpload(void) {
	// All those values are the initial values of the parameters they represent. Taken from
	// the OpenGL specification, version 4.5.

	// Min and mag filters :
	this->minmag.x = GL_NEAREST_MIPMAP_LINEAR;	  // Min filter
	this->minmag.y = GL_LINEAR;	   // Mag filter
	// LOD parameters :
	this->lod.x = -1000.f;
	this->lod.y = 1000.f;
	// Wrap parameters :
	this->wrap.s = GL_REPEAT;
	this->wrap.t = GL_REPEAT;
	this->wrap.p = GL_REPEAT;	 // The component here, 'P', is named this way in order not to
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

	this->level			 = 0;
	this->internalFormat = GL_RGBA;
	this->size.x		 = 0;
	this->size.y		 = 0;
	this->size.z		 = 0;
	this->format		 = GL_RGBA;
	this->type			 = GL_FLOAT;
	this->data			 = nullptr;
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
	std::cerr << "Info about the struct :"
			  << "\n";
	std::cerr << "\t- minmag : [";
	switch (this->minmag.x) {
		CASE_GL(GL_NEAREST);
		CASE_GL(GL_LINEAR);
		CASE_GL(GL_NEAREST_MIPMAP_NEAREST);
		CASE_GL(GL_NEAREST_MIPMAP_LINEAR);
		CASE_GL(GL_LINEAR_MIPMAP_NEAREST);
		CASE_GL(GL_LINEAR_MIPMAP_LINEAR);
		DEFAULT_GL();
	}
	std::cerr << ", ";
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
	}
	std::cerr << ", ";
	switch (this->wrap.y) {
		CASE_GL(GL_CLAMP_TO_EDGE);
		CASE_GL(GL_CLAMP_TO_BORDER);
		CASE_GL(GL_MIRRORED_REPEAT);
		CASE_GL(GL_MIRROR_CLAMP_TO_EDGE);
		CASE_GL(GL_REPEAT);
		DEFAULT_GL();
	}
	std::cerr << ", ";
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
	}
	std::cerr << ", ";
	switch (this->swizzle.y) {
		CASE_GL(GL_RED);
		CASE_GL(GL_GREEN);
		CASE_GL(GL_BLUE);
		CASE_GL(GL_ALPHA);
		CASE_GL(GL_ZERO);
		CASE_GL(GL_ONE);
		DEFAULT_GL();
	}
	std::cerr << ", ";
	switch (this->swizzle.z) {
		CASE_GL(GL_RED);
		CASE_GL(GL_GREEN);
		CASE_GL(GL_BLUE);
		CASE_GL(GL_ALPHA);
		CASE_GL(GL_ZERO);
		CASE_GL(GL_ONE);
		DEFAULT_GL();
	}
	std::cerr << ", ";
	switch (this->swizzle.a) {
		CASE_GL(GL_RED);
		CASE_GL(GL_GREEN);
		CASE_GL(GL_BLUE);
		CASE_GL(GL_ALPHA);
		CASE_GL(GL_ZERO);
		CASE_GL(GL_ONE);
		DEFAULT_GL();
	}
	std::cerr << "],\n\t- alignment:{" << this->alignment.x << "," << this->alignment.y << "}\n\t- level : " << this->level << ",\n\t- internalFormat : ";
	switch (this->internalFormat) {
		CASE_GL(GL_R16UI);
		CASE_GL(GL_R8UI);
		CASE_GL(GL_RG8UI);
		CASE_GL(GL_RG16UI);
		DEFAULT_GL();
	}
	std::cerr << ",\n\t- size:[" << this->size.x << "," << this->size.y << "," << this->size.z << "],\n\t- format : ";
	switch (this->format) {
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

TextureUpload::~TextureUpload() {
}

SimpleVolMesh::SimpleVolMesh(void) {
	this->positions.clear();
	this->normals.clear();
	this->texture.clear();
	this->indices.clear();
	this->cutting_planes.clear();
	this->planar_view.clear();
}

SimpleVolMesh::~SimpleVolMesh(void) {
	this->positions.clear();
	this->normals.clear();
	this->texture.clear();
	this->indices.clear();
	this->cutting_planes.clear();
	this->planar_view.clear();
}

VolMesh::VolMesh(void) {
	// All texture handles are at 0 by default (initial value
	// of tex names according to GL spec version 4.5)

	this->visibilityMap		 = 0;
	this->vertexPositions	 = 0;
	this->textureCoordinates = 0;
	this->neighborhood		 = 0;
	this->faceNormals		 = 0;
	this->tetrahedraCount	 = 0;
}

bool VolMesh::isValid() {
	return this->visibilityMap != 0 &&
		   this->vertexPositions != 0 &&
		   this->textureCoordinates != 0 &&
		   this->neighborhood != 0 &&
		   this->faceNormals != 0 &&
		   this->tetrahedraCount > 0;
}

VolMesh::~VolMesh(void) { /* Nothing here for now. */
}

//GridGLView::GridGLView(Grid * _grid) {
//	this->grid			   = _grid;
//    // TODO: add multiChannel grid
//	//this->nbChannels	   = this->grid->getVoxelDimensionality();
//	this->mainColorChannel			= 1;
//
//	// Fill with default attributes
//}
//
//void GridGLView::setMainColorChannel(std::size_t color_channel) {
//	assert((color_channel < 3) && "color channel was not under 3");
//
//	this->mainColorChannel = color_channel;
//}

ColorChannelAttributes_GL::ColorChannelAttributes_GL() {
	this->isVisible		   = true;
	this->colorScaleIndex  = 0;
	auto min			   = std::numeric_limits<std::uint16_t>::min();
	auto max			   = std::numeric_limits<std::uint16_t>::max();
	this->visibleBounds	   = bound_t(min, max);
	this->colorScaleBounds = bound_t(min, max);
}

void ColorChannelAttributes_GL::toggleVisible() {
	if (this->isVisible > 0) {
		this->isVisible = 0;
		return;
	} else {
		this->isVisible = 1;
	}
}

void ColorChannelAttributes_GL::setVisible(bool v) {
	this->isVisible = (v) ? 1 : 0;
	return;
}

void ColorChannelAttributes_GL::setHidden(bool v) {
	this->isVisible = (v) ? 0 : 1;
	return;
}

std::uint32_t ColorChannelAttributes_GL::getVisibility() const {
	return this->isVisible;
}

void ColorChannelAttributes_GL::setColorScale(std::uint32_t new_color_scale_index) {
	this->colorScaleIndex = new_color_scale_index;
}

std::uint32_t ColorChannelAttributes_GL::getColorScale() const {
	return this->colorScaleIndex;
}

void ColorChannelAttributes_GL::setMinVisible(bound_t::value_type _new_min) {
	this->visibleBounds.x = _new_min;
	return;
}

void ColorChannelAttributes_GL::setMaxVisible(bound_t::value_type _new_max) {
	this->visibleBounds.y = _new_max;
	return;
}

void ColorChannelAttributes_GL::setMinColorScale(bound_t::value_type _new_min) {
	this->colorScaleBounds.x = _new_min;
	return;
}

void ColorChannelAttributes_GL::setMaxColorScale(bound_t::value_type _new_max) {
	this->colorScaleBounds.y = _new_max;
	return;
}

ColorChannelAttributes_GL::bound_t ColorChannelAttributes_GL::getVisibleRange() const {
	return this->visibleBounds;
}

ColorChannelAttributes_GL::bound_t ColorChannelAttributes_GL::getColorRange() const {
	return this->colorScaleBounds;
}
