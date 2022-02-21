#ifndef VISUALIZATION_VIEWER_INCLUDE_VIEWER_STRUCTS_HPP_
#define VISUALIZATION_VIEWER_INCLUDE_VIEWER_STRUCTS_HPP_

//#include "../../grid/include/discrete_grid.hpp"
#include "../../macros.hpp"

#include "../../grid/include/grid.hpp"
#include "../../legacy/image/utils/include/bounding_box.hpp"

#include <glm/glm.hpp>

#include <QOpenGLFunctions_4_0_Core>

#include <memory>

class Scene;	// Fwd-declaration

namespace Image {
/// @brief Old bbox definition
//  TODO: wtf are you doing here
typedef BoundingBox_General<float> bbox_t;
}

/// @ingroup graphpipe
/// @brief Simple struct to hold the values passed to glTexImage<n>D() functions.
/// @details Allows to pass a single structure for any type of {1,2,3}D textures when uploaded to OpenGL. The default
/// values of this struct for the texture parameters (the parameters not present in glTexImage() functions) will be
/// the default values of the OpenGL spec.
struct TextureUpload
{
public:
	TextureUpload(void);
	TextureUpload(const TextureUpload&) = delete;
	TextureUpload(TextureUpload&&) = delete;
	TextureUpload& operator=(const TextureUpload&) = delete;
	TextureUpload& operator=(TextureUpload&&) = delete;
	void printInfo();
	~TextureUpload();

public:
	/// @brief Min/mag filters for the texture. X = min filter, Y = mag filter
	glm::vec<2, GLint, glm::defaultp> minmag;
	/// @brief Minimum and maximum values for the LOD (x = min, y = max). Default : {-1000; 1000}.
	glm::vec<2, GLfloat, glm::defaultp> lod;
	glm::vec<3, GLint, glm::defaultp> wrap;
	glm::vec<4, GLint, glm::defaultp> swizzle;
	glm::vec<2, GLint, glm::defaultp> alignment;
	GLint level;
	GLint internalFormat;
	glm::vec<3, GLsizei, glm::defaultp> size;
	/// @brief Pixel format for texture upload. Ex : GL_RED, GL_RGB (...)
	GLenum format;
	/// @brief Pixel type for texture upload. Ex : GL_FLOAT, GL_UNSIGNED_BYTE (...)
	GLenum type;

	const void* data;
};

/// @ingroup graphpipe
/// @brief Helper class regrouping the positions, normals, texture coordinates and indices of a mesh.
/// @details Used to generate the default mesh in the main program. There is multiple index buffers, since
/// we have multiple things to draw, in different ways (this will be uploaded to one VAO only).
struct SimpleVolMesh
{
public:
	SimpleVolMesh(void);
	SimpleVolMesh(const SimpleVolMesh&) = delete;
	SimpleVolMesh(SimpleVolMesh&&) = delete;
	SimpleVolMesh& operator=(const SimpleVolMesh&) = delete;
	SimpleVolMesh& operator=(SimpleVolMesh&&) = delete;
	~SimpleVolMesh();

public:
	std::vector<glm::vec4> positions;
	std::vector<glm::vec4> normals;
	std::vector<glm::vec3> texture;
	std::vector<unsigned int> indices;
	std::vector<unsigned int> cutting_planes;
	std::vector<unsigned int> planar_view;
};

/// @ingroup graphpipe
/// @brief The VolMesh structure holds all of the texture handles necessary to make our volumetric visualization method
/// work.
/// @details It is used in conjonction with the VolMeshData struct in order to provide a simple and centralized
/// repository for all allocated textures.
struct VolMesh
{
public:
	VolMesh(void);
	~VolMesh(void);
	bool isValid(void);

public:
	GLuint visibilityMap;
	GLuint vertexPositions;
	GLuint textureCoordinates;
	GLuint neighborhood;
	GLuint faceNormals;
	GLsizei tetrahedraCount;
};

/// @ingroup graphpipe
/// @brief The colorChannelAttributes_GL struct is mirroring the contents of the similarly-named uniform in GLSL.
/// @details It is defined this way to be simply `memcpy`'d over to the GPU, without any hassle.
struct alignas(32) colorChannelAttributes_GL
{
public:
	typedef glm::tvec2<std::uint32_t> bound_t;

protected:
	alignas(alignof(std::uint32_t)) std::uint32_t isVisible;
	alignas(alignof(std::uint32_t)) std::uint32_t colorScaleIndex;
	alignas(alignof(bound_t)) bound_t visibleBounds;
	alignas(alignof(bound_t)) bound_t colorScaleBounds;

public:
	colorChannelAttributes_GL(void);
	~colorChannelAttributes_GL(void) = default;

public:
	void toggleVisible();
	void setVisible(bool v = true);
	void setHidden(bool h = true);
	std::uint32_t getVisibility() const;

	void setColorScale(std::uint32_t new_color_scale_index);
	std::uint32_t getColorScale(void) const;

	void setMinVisible(bound_t::value_type _new_min);
	void setMaxVisible(bound_t::value_type _new_max);
	void setMinColorScale(bound_t::value_type _new_min);
	void setMaxColorScale(bound_t::value_type _new_max);
	bound_t getVisibleRange() const;
	bound_t getColorRange() const;
};

/// @ingroup graphpipe
/// @brief Simple structure merging all resources necessary to view a grid in 3D.
/// @details This structure must be associated to one grid, and one grid only. Thus, the pointer to the grid is defined
/// as `const std::shared_ptr<>` since we want the reference to be held during the lifetime of the object.
struct GridGLView
{
public:
	using Ptr = std::shared_ptr<GridGLView>;

public:
	GridGLView(const GridGL * _g);
	GridGLView(const GridGLView&) = default;
	GridGLView(GridGLView&&) = default;
	GridGLView& operator=(const GridGLView&) = default;
	GridGLView& operator=(GridGLView&&) = default;
	~GridGLView(void) = default;

	void setMainColorChannel(std::size_t index);
	colorChannelAttributes_GL& mainColorChannelAttributes();

public:
	using data_t  = double;
	using data_2  = glm::vec<2, data_t, glm::defaultp>;
	using data_3  = glm::vec<3, data_t, glm::defaultp>;
	using color_3 = glm::vec3;

	using int_pair = glm::vec<2, int, glm::defaultp>;

	//Image::Grid::Ptr grid;
    //const Grid * grid;
    const GridGL * grid;

    // WARNING
    // TODO: to fill
    glm::vec3 voxelDimensions;

	GLuint gridTexture;
	unsigned int nbChannels;
	VolMesh volumetricMesh;
	glm::vec3 defaultEpsilon;
	color_3 boundingBoxColor;
	/// @brief The 'base' color for the user-defined color scale
	color_3 color_0;
	/// @brief The 'final' color for the user-defined color scale
	color_3 color_1;
	/// @brief The minimum and maximum texture values to display
	data_2 texBounds0;
	data_2 texBounds1;
	/// @brief The minimum and maximum values of the color scale
	data_2 colorBounds0;
	data_2 colorBounds1;
	/// @brief Handle for the uniform buffer with the channel attributes
	GLuint uboHandle_colorAttributes;
	/// @brief The color channel attributes.
	std::array<colorChannelAttributes_GL, 3> colorChannelAttributes;

protected:
	/// @brief The index of the main color channel.
	/// @details Protected so as not to accidentally set it to something invalid during runtime.
	std::size_t mainColorChannel;
};

#endif	  // VISUALIZATION_VIEWER_INCLUDE_VIEWER_STRUCTS_HPP_
