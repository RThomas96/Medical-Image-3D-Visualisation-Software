#include "../include/color_scale_base.hpp"

#include "../../viewer/include/scene.hpp"
#include "../../viewer/include/viewer_structs.hpp"

#include <chrono>

namespace Color {

	ColorScaleBase::ColorScaleBase(std::size_t target_sample_count) {
		using namespace std::chrono_literals;
		using time_t  = std::decay_t<decltype(this->last_update_host)>;
		using clock_t = time_t::clock;

		this->cs_type	= ColorScale_t::Generic;
		this->name_user = "generic_color_scale";
		// By default, always set the device update time before the host update, to force the device upload of data.
		// However, this need for an update/upload will be silently ignored since we're just in the constructor.
		this->last_update_host	 = clock_t::now();
		this->last_update_device = time_t(this->last_update_host - 1s);

		this->texture_handle = GL_INVALID_VALUE;
		this->sample_count	 = target_sample_count;

		// Connect the sample count change to the internal data update :
		// (we want to change the texture with new sample count when it changes)
		QObject::connect(this, &ColorScaleBase::sampleCountChanged, this, &ColorScaleBase::dataChanged);
		// Connect the host update signal :
		QObject::connect(this, &ColorScaleBase::dataChanged, this, &ColorScaleBase::internal_data_update);
		// Connect the device upload signal :
		QObject::connect(this, &ColorScaleBase::dataUploaded, this, &ColorScaleBase::after_update);
	}

	ColorScaleBase::~ColorScaleBase() {
		// Disconnect all signals laid out in the ctor, in reverse order just to be nicer.
		QObject::disconnect(this, &ColorScaleBase::dataUploaded, this, &ColorScaleBase::after_update);
		QObject::disconnect(this, &ColorScaleBase::dataChanged, this, &ColorScaleBase::internal_data_update);
		QObject::disconnect(this, &ColorScaleBase::sampleCountChanged, this, &ColorScaleBase::dataChanged);
	}

	std::size_t ColorScaleBase::getSampleCount() const {
		return this->sample_count;
	}

	void ColorScaleBase::setSampleCount(std::size_t _new_sample_count) {
		// don't accept sample counts less than 2
		if (_new_sample_count <= 1) {
			return;
		}
		this->sample_count = _new_sample_count;
		emit this->sampleCountChanged();
	}

	std::string ColorScaleBase::getName() const {
		return this->name_user;
	}

	void ColorScaleBase::setName(std::string _new_user_name) {
		if (_new_user_name.empty()) {
			return;
		}
		this->name_user = _new_user_name;
		emit this->nameChanged();
	}

	void ColorScaleBase::uploadData(Scene *f) {
		// Previous data was here, need to re-upload it :
		if (this->texture_handle != GL_INVALID_VALUE && this->texture_handle > 0) {
			f->glDeleteTextures(1, &this->texture_handle);
		}

		// Prepare the upload parameters :
		TextureUpload colorScaleUploadParameters;
		colorScaleUploadParameters.minmag.x	 = GL_LINEAR;
		colorScaleUploadParameters.minmag.y	 = GL_LINEAR;
		colorScaleUploadParameters.lod.y	 = -1000.f;
		colorScaleUploadParameters.wrap.x	 = GL_CLAMP_TO_EDGE;
		colorScaleUploadParameters.wrap.y	 = GL_CLAMP_TO_EDGE;
		colorScaleUploadParameters.wrap.z	 = GL_CLAMP_TO_EDGE;
		colorScaleUploadParameters.swizzle.r = GL_RED;
		colorScaleUploadParameters.swizzle.g = GL_GREEN;
		colorScaleUploadParameters.swizzle.b = GL_BLUE;
		colorScaleUploadParameters.swizzle.a = GL_ONE;

		colorScaleUploadParameters.level		  = 0;
		colorScaleUploadParameters.internalFormat = GL_RGB;
		colorScaleUploadParameters.size.x		  = static_cast<int>(this->sample_count);
		colorScaleUploadParameters.size.y		  = 1;
		colorScaleUploadParameters.size.z		  = 1;
		colorScaleUploadParameters.format		  = GL_RGB;
		colorScaleUploadParameters.type			  = GL_FLOAT;

		auto color_data					= this->getColorScale();
		colorScaleUploadParameters.data = color_data.data();

		this->texture_handle = f->uploadTexture1D(colorScaleUploadParameters);
		if (f->get_context()->isValid()) {
		}

		if (this->texture_handle != 0) {
			emit this->dataUploaded();
		}
	}

	GLuint ColorScaleBase::textureHandle() const {
		// If not created, return the default texture name to bind nothing to the GLSL program :
		if (this->texture_handle == GL_INVALID_VALUE) {
			return 0;
		}
		return this->texture_handle;
	}

	bool ColorScaleBase::needsUpdate() const {
		return this->last_update_device < this->last_update_host;
	}

	void ColorScaleBase::internal_data_update(void) {
		using clock_t		   = std::decay_t<decltype(this->last_update_host)>::clock;
		this->last_update_host = clock_t::now();
	}

	void ColorScaleBase::after_update(void) {
		using clock_t			 = std::decay_t<decltype(this->last_update_host)>::clock;
		this->last_update_device = clock_t::now();
	}

}	 // namespace Color
