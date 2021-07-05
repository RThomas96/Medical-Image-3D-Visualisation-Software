#ifndef	VISUALIAZTION_IMAGE_TIFF_INCLUDE_TEMPLATED_BACKEND_IMPL_HPP_
#define	VISUALIAZTION_IMAGE_TIFF_INCLUDE_TEMPLATED_BACKEND_IMPL_HPP_

#ifndef  VISUALIAZTION_IMAGE_TIFF_INCLUDE_TEMPLATED_BACKEND_HPP_
#include "./templated_backend.hpp"		// Included between header guards to have code completion in any QtCreator.
#endif

namespace Image {

template <typename unsupported_element_t>
TIFFBackendDetail<unsupported_element_t>::TIFFBackendDetail(uint32_t w, uint32_t h, std::size_t _dim) {
	UNUSED(w); UNUSED(h); UNUSED(_dim);
	throw std::runtime_error("Error : unsupported type passed to the TIFFBackendDetail constructor.");
}

template <>
TIFFBackendDetail<std::uint8_t>::TIFFBackendDetail(uint32_t w, uint32_t h, std::size_t _dim) {
	this->images.clear();

	this->resolution.x = w;
	this->resolution.y = h;
	this->resolution.z = 0;
	this->voxel_dimensionality = _dim;

	this->bitsPerSample = 8;
	this->sampleFormat = SAMPLEFORMAT_UINT;
	this->voxel_dimensions = glm::vec3{1.f, 1.f, 1.f};
	#ifndef NDEBUG
	std::cerr << "Creating a TIFF backend detail of type " << pnt(std::uint8_t) << '\n';
	#endif
}

template <>
TIFFBackendDetail<std::uint16_t>::TIFFBackendDetail(uint32_t w, uint32_t h, std::size_t _dim) {
	this->images.clear();

	this->resolution.x = w;
	this->resolution.y = h;
	this->resolution.z = 0;
	this->voxel_dimensionality = _dim;

	this->bitsPerSample = 16;
	this->sampleFormat = SAMPLEFORMAT_UINT;
	this->voxel_dimensions = glm::vec3{1.f, 1.f, 1.f};
	#ifndef NDEBUG
	std::cerr << "Creating a TIFF backend detail of type " << pnt(std::uint16_t) << '\n';
	#endif
}

template <>
TIFFBackendDetail<std::uint32_t>::TIFFBackendDetail(uint32_t w, uint32_t h, std::size_t _dim) {
	this->images.clear();

	this->resolution.x = w;
	this->resolution.y = h;
	this->resolution.z = 0;
	this->voxel_dimensionality = _dim;

	this->bitsPerSample = 32;
	this->sampleFormat = SAMPLEFORMAT_UINT;
	this->voxel_dimensions = glm::vec3{1.f, 1.f, 1.f};
	#ifndef NDEBUG
	std::cerr << "Creating a TIFF backend detail of type " << pnt(std::uint32_t) << '\n';
	#endif
}

template <>
TIFFBackendDetail<std::uint64_t>::TIFFBackendDetail(uint32_t w, uint32_t h, std::size_t _dim) {
	this->images.clear();

	this->resolution.x = w;
	this->resolution.y = h;
	this->resolution.z = 0;
	this->voxel_dimensionality = _dim;

	this->bitsPerSample = 64;
	this->sampleFormat = SAMPLEFORMAT_UINT;
	this->voxel_dimensions = glm::vec3{1.f, 1.f, 1.f};
	#ifndef NDEBUG
	std::cerr << "Creating a TIFF backend detail of type " << pnt(std::uint64_t) << '\n';
	#endif
}

template <>
TIFFBackendDetail<std::int8_t>::TIFFBackendDetail(uint32_t w, uint32_t h, std::size_t _dim) {
	this->images.clear();

	this->resolution.x = w;
	this->resolution.y = h;
	this->resolution.z = 0;
	this->voxel_dimensionality = _dim;

	this->bitsPerSample = 8;
	this->sampleFormat = SAMPLEFORMAT_INT;
	this->voxel_dimensions = glm::vec3{1.f, 1.f, 1.f};
	#ifndef NDEBUG
	std::cerr << "Creating a TIFF backend detail of type " << pnt(std::int8_t) << '\n';
	#endif
}

template <>
TIFFBackendDetail<std::int16_t>::TIFFBackendDetail(uint32_t w, uint32_t h, std::size_t _dim) {
	this->images.clear();

	this->resolution.x = w;
	this->resolution.y = h;
	this->resolution.z = 0;
	this->voxel_dimensionality = _dim;

	this->bitsPerSample = 16;
	this->sampleFormat = SAMPLEFORMAT_INT;
	this->voxel_dimensions = glm::vec3{1.f, 1.f, 1.f};
	#ifndef NDEBUG
	std::cerr << "Creating a TIFF backend detail of type " << pnt(std::int16_t) << '\n';
	#endif
}

template <>
TIFFBackendDetail<std::int32_t>::TIFFBackendDetail(uint32_t w, uint32_t h, std::size_t _dim) {
	this->images.clear();

	this->resolution.x = w;
	this->resolution.y = h;
	this->resolution.z = 0;
	this->voxel_dimensionality = _dim;

	this->bitsPerSample = 32;
	this->sampleFormat = SAMPLEFORMAT_INT;
	this->voxel_dimensions = glm::vec3{1.f, 1.f, 1.f};
	#ifndef NDEBUG
	std::cerr << "Creating a TIFF backend detail of type " << pnt(std::int32_t) << '\n';
	#endif
}

template <>
TIFFBackendDetail<std::int64_t>::TIFFBackendDetail(uint32_t w, uint32_t h, std::size_t _dim) {
	this->images.clear();

	this->resolution.x = w;
	this->resolution.y = h;
	this->resolution.z = 0;
	this->voxel_dimensionality = _dim;

	this->bitsPerSample = 64;
	this->sampleFormat = SAMPLEFORMAT_INT;
	this->voxel_dimensions = glm::vec3{1.f, 1.f, 1.f};
	#ifndef NDEBUG
	std::cerr << "Creating a TIFF backend detail of type " << pnt(std::int64_t) << '\n';
	#endif
}

template <>
TIFFBackendDetail<float>::TIFFBackendDetail(uint32_t w, uint32_t h, std::size_t _dim) {
	this->images.clear();

	this->resolution.x = w;
	this->resolution.y = h;
	this->resolution.z = 0;
	this->voxel_dimensionality = _dim;

	this->bitsPerSample = 32;
	this->sampleFormat = SAMPLEFORMAT_IEEEFP;
	this->voxel_dimensions = glm::vec3{1.f, 1.f, 1.f};
	#ifndef NDEBUG
	std::cerr << "Creating a TIFF backend detail of type " << pnt(float) << '\n';
	#endif
}

template <>
TIFFBackendDetail<double>::TIFFBackendDetail(uint32_t w, uint32_t h, std::size_t _dim) {
	this->images.clear();

	this->resolution.x = w;
	this->resolution.y = h;
	this->resolution.z = 0;
	this->voxel_dimensionality = _dim;

	this->bitsPerSample = 64;
	this->sampleFormat = SAMPLEFORMAT_IEEEFP;
	this->voxel_dimensions = glm::vec3{1.f, 1.f, 1.f};
	#ifndef NDEBUG
	std::cerr << "Creating a TIFF backend detail of type " << pnt(double) << '\n';
	#endif
}

template <typename element_t> typename TIFFBackendDetail<element_t>::Ptr
TIFFBackendDetail<element_t>::createBackend(uint32_t width, uint32_t height, std::size_t _dim) {
	return Ptr(new TIFFBackendDetail<pixel_t>(width, height, _dim));
}

}

#endif //  VISUALIAZTION_IMAGE_TIFF_INCLUDE_TEMPLATED_BACKEND_IMPL_HPP_
