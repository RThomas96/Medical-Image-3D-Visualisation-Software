#ifndef VISUALIZATION_VIEWER_INCLUDE_VIEWER_STRUCTS_HPP_
#define VISUALIZATION_VIEWER_INCLUDE_VIEWER_STRUCTS_HPP_

#include "../../macros.hpp"
#include "../../grid/include/discrete_grid.hpp"

#include "../../new_grid/include/grid.hpp"

#include <glm/glm.hpp>

#include <QOpenGLFunctions_4_0_Core>

#include <memory>

class Scene; // Fwd-declaration

/// @ingroup graphpipe
/// @brief Simple struct to hold the values passed to glTexImage<n>D() functions.
/// @details Allows to pass a single structure for any type of {1,2,3}D textures when uploaded to OpenGL. The default
/// values of this struct for the texture parameters (the parameters not present in glTexImage() functions) will be
/// the default values of the OpenGL spec.
struct TextureUpload {
	public:
		/// @brief Default constructor for the TextureUpload class
		TextureUpload(void);
		/// @brief Copy constructor for the TextureUpload class
		TextureUpload(const TextureUpload&) = delete;
		/// @brief Move constructor for the TextureUpload class
		TextureUpload(TextureUpload&&) = delete;
		/// @brief Copy operator for the TextureUpload class
		TextureUpload& operator= (const TextureUpload&) = delete;
		/// @brief Move operator for the TextureUpload class
		TextureUpload& operator= (TextureUpload&&) = delete;
		/// @brief Print info about the struct
		void printInfo();
		/// @brief Default destructor
		~TextureUpload();
	public:
		/// @brief Min/mag filters for the texture. X = min filter, Y = mag filter
		glm::vec<2, GLint, glm::defaultp> minmag;
		/// @brief Minimum and maximum values for the LOD (x = min, y = max). Default : {-1000; 1000}.
		glm::vec<2, GLfloat, glm::defaultp> lod;
		/// @brief Wrap parameters for all dimensions.
		glm::vec<3, GLint, glm::defaultp> wrap;
		/// @brief Defines the default swizzle parameter for the texture.
		glm::vec<4, GLint, glm::defaultp> swizzle;
		/// @brief Defines the pixel pack and unpack alignment. X = pack, Y = unpack.
		glm::vec<2, GLint, glm::defaultp> alignment;
		/// @brief The level of the texture uploaded.
		GLint level;
		/// @brief The internal pixel format of the texture uploaded. Ex : GL_RGB, GL_R8UI (...)
		GLint internalFormat;
		/// @brief Texture size, in all dimensions.
		glm::vec<3, GLsizei, glm::defaultp> size;
		/// @brief Pixel format for texture upload. Ex : GL_RED, GL_RGB (...)
		GLenum format;
		/// @brief Pixel type for texture upload. Ex : GL_FLOAT, GL_UNSIGNED_BYTE (...)
		GLenum type;
		/// @brief The data to upload to OpenGL
		const void* data;
};

/// @ingroup graphpipe
/// @brief Helper class regrouping the positions, normals, texture coordinates and indices of a mesh.
/// @details Used to generate the default mesh in the main program. There is multiple index buffers, since
/// we have multiple things to draw, in different ways (this will be uploaded to one VAO only).
struct Mesh {
	public:
		/// @brief Default constructor for the Mesh class.
		Mesh(void);
		/// @brief Copy constructor for the Mesh class.
		Mesh(const Mesh&) = delete;
		/// @brief Move constructor for the Mesh class.
		Mesh(Mesh&&) = delete;
		/// @brief Copy operator for the Mesh class.
		Mesh& operator= (const Mesh&) = delete;
		/// @brief Move operator for the Mesh class.
		Mesh& operator= (Mesh&&) = delete;
		/// @brief Default destructor for the Mesh class.
		~Mesh();
	public:
		/// @brief The positions of the mesh.
		std::vector<glm::vec4> positions;
		/// @brief The normals of each vertex.
		std::vector<glm::vec4> normals;
		/// @brief The texture coordinates of each vertex.
		std::vector<glm::vec3> texture;
		/// @brief The indices used to draw the unit cube used for the program.
		std::vector<unsigned int> indices;
		/// @brief The indices used to draw the cutting planes in the 3D view.
		std::vector<unsigned int> cutting_planes;
		/// @brief The indices used to draw the planes in the planar viewers.
		std::vector<unsigned int> planar_view;
};

/// @ingroup graphpipe
/// @brief Helper class regrouping the data of a tetrahedral mesh.
struct VolMeshData {
	public:
		/// @brief Default constructor for the Mesh class.
		VolMeshData(void);
		/// @brief Copy constructor for the Mesh class.
		VolMeshData(const VolMeshData&) = delete;
		/// @brief Move constructor for the Mesh class.
		VolMeshData(VolMeshData&&) = delete;
		/// @brief Copy operator for the Mesh class.
		VolMeshData& operator= (const VolMeshData&) = delete;
		/// @brief Move operator for the Mesh class.
		VolMeshData& operator= (VolMeshData&&) = delete;
		/// @brief Default destructor for the Mesh class.
		~VolMeshData(void);
	public:
		/// @brief The positions of the mesh.
		std::vector<glm::vec4> positions;
		/// @brief The texture coordinates of each vertex.
		std::vector<glm::vec3> texture;
		/// @brief Stores the indices of vertices needed for a tetrahedron
		std::vector<std::array<std::size_t, 4>> tetrahedra;
		/// @brief Stores the indices of neighboring tetrahedra
		std::vector<std::vector<int>> neighbors;
		/// @brief Per-face normals of each tetrahedron
		std::vector<std::array<glm::vec4, 4>> normals;
};

/// @ingroup graphpipe
/// @brief The VolMesh structure holds all of the texture handles necessary to make our volumetric visualization method
/// work.
/// @details It is used in conjonction with the VolMeshData struct in order to provide a simple and centralized
/// repository for all allocated textures.
struct VolMesh {
	public:
		/// @brief Default constructor for the VolMesh struct.
		VolMesh(void);
		/// @brief Default destructor for the VolMesh struct.
		~VolMesh(void);
		/// @brief Checks if the handles are valid (different from 0) and it has something to draw (tetCount > 0)
		bool isValid(void);
	public:
		/// @brief The texture handle for the visible domains/intensities in the image.
		GLuint visibilityMap;
		/// @brief The texture handle for the vertex positions.
		GLuint vertexPositions;
		/// @brief The texture handle for the vertices' texture coordinates.
		GLuint textureCoordinates;
		/// @brief The texture handle for the tetrahedra neighborhoods.
		GLuint neighborhood;
		/// @brief The texture handle for the per-tetrahedra face normals.
		GLuint faceNormals;
		/// @brief The number of tetrahedra to render once `glDrawElementsInstanced()` is called.
		GLsizei tetrahedraCount;
};

/// @ingroup graphpipe
/// @brief Simple structure merging all resources necessary to view a grid in 3D.
/// @details This structure must be associated to one grid, and one grid only. Thus, the pointer to the grid is defined
/// as `const std::shared_ptr<>` since we want the reference to be held during the lifetime of the object.
struct GridGLView {
	public:
		using Ptr = std::shared_ptr<GridGLView>;
	public:
		/// @brief Default constructor for the grid view. Must be associated to one and only one grid.
		GridGLView(const std::initializer_list<std::shared_ptr<DiscreteGrid>> _g);
		/// @brief Default constructor for two grids.
		explicit GridGLView(const std::shared_ptr<DiscreteGrid>, const std::shared_ptr<DiscreteGrid>);
		/// @brief Copy constructor of the GridGLView struct.
		GridGLView(const GridGLView&) = default;
		/// @brief Move constructor of the GridGLView struct.
		GridGLView(GridGLView&&) = default;
		/// @brief Copy operator of the GridGLView struct.
		GridGLView& operator= (const GridGLView&) = default;
		/// @brief Move operator of the GridGLView struct.
		GridGLView& operator= (GridGLView&&) = default;
		/// @brief Default destructor of the GridGLView struct.
		~GridGLView(void);
	public:
		using data_t = DiscreteGrid::data_t;
		using data_2 = glm::vec<2, data_t, glm::defaultp>;
		using data_3 = glm::vec<3, data_t, glm::defaultp>;
		using color_3 = glm::vec3;
		/// @brief The pointer to the grid we want to show.
		std::vector<std::shared_ptr<DiscreteGrid>> grid;
		/// @brief The texture handle to access the grid data in shaders.
		GLuint gridTexture;
		/// @brief The number of channels contained in the image
		unsigned int nbChannels;
		/// @brief The volumetric mesh handles to use when drawing
		VolMesh volumetricMesh;
		/// @brief The epsilon to provide for the volumetric viewing method
		glm::vec3 defaultEpsilon;
		/// @brief The bounding box's color, as a triplet of normalized values for R, G, and B.
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
};

/// @ingroup graphpipe
/// @brief The colorChannelAttributes_GL struct is mirroring the contents of the similarly-named uniform in GLSL.
/// @details It is defined this way to be simply `memcpy`'d over to the GPU, without any hassle.
struct alignas(32) colorChannelAttributes_GL {
	public:
		typedef glm::tvec2<std::uint32_t> bound_t;
	protected:
		alignas(alignof(std::uint32_t)) std::uint32_t isVisible;
		alignas(alignof(std::uint32_t)) std::uint32_t colorScaleIndex;
		alignas(alignof(bound_t))		bound_t visibleBounds;
		alignas(alignof(bound_t))		bound_t colorScaleBounds;
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
struct NewAPI_GridGLView {
	public:
		using Ptr = std::shared_ptr<NewAPI_GridGLView>;
	public:
		/// @brief Default constructor for the grid view. Must be associated to one and only one grid.
		NewAPI_GridGLView(const Image::Grid::Ptr _g);
		/// @brief Copy constructor of the NewAPI_GridGLView struct.
		NewAPI_GridGLView(const NewAPI_GridGLView&) = default;
		/// @brief Move constructor of the NewAPI_GridGLView struct.
		NewAPI_GridGLView(NewAPI_GridGLView&&) = default;
		/// @brief Copy operator of the NewAPI_GridGLView struct.
		NewAPI_GridGLView& operator= (const NewAPI_GridGLView&) = default;
		/// @brief Move operator of the NewAPI_GridGLView struct.
		NewAPI_GridGLView& operator= (NewAPI_GridGLView&&) = default;
		/// @brief Default destructor of the NewAPI_GridGLView struct.
		~NewAPI_GridGLView(void) = default;

		/// @brief Sets the main color channel for this grid view
		void setMainColorChannel(std::size_t index);
		/// @brief Gets the attributes of the main color channel
		colorChannelAttributes_GL& mainColorChannelAttributes();
	public:
		using data_t = DiscreteGrid::data_t;
		using data_2 = glm::vec<2, data_t, glm::defaultp>;
		using data_3 = glm::vec<3, data_t, glm::defaultp>;
		using color_3 = glm::vec3;
		/// @brief The pointer to the grid we want to show.
		Image::Grid::Ptr grid;
		/// @brief The texture handle to access the grid data in shaders.
		GLuint gridTexture;
		/// @brief The number of channels contained in the image
		unsigned int nbChannels;
		/// @brief The volumetric mesh handles to use when drawing
		VolMesh volumetricMesh;
		/// @brief The epsilon to provide for the volumetric viewing method
		glm::vec3 defaultEpsilon;
		/// @brief The bounding box's color, as a triplet of normalized values for R, G, and B.
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

#endif // VISUALIZATION_VIEWER_INCLUDE_VIEWER_STRUCTS_HPP_
