#ifndef VISUALIZATION_VIEWER_INCLUDE_VIEWER_STRUCTS_HPP_
#define VISUALIZATION_VIEWER_INCLUDE_VIEWER_STRUCTS_HPP_

#include "../../macros.hpp"
#include "../../grid/include/discrete_grid.hpp"

#include <glm/glm.hpp>

#include <QOpenGLFunctions_4_0_Core>

#include <memory>

class Scene; // Fwd-declaration

/// @b Simple struct to hold the values passed to glTexImage<n>D() functions.
/// @details Allows to pass a single structure for any type of {1,2,3}D textures when uploaded to OpenGL. The default
/// values of this struct for the texture parameters (the parameters not present in glTexImage() functions) will be
/// the default values of the OpenGL spec.
struct TextureUpload {
	public:
		/// @b Default constructor for the TextureUpload class
		TextureUpload(void);
		/// @b Copy constructor for the TextureUpload class
		TextureUpload(const TextureUpload&) = delete;
		/// @b Move constructor for the TextureUpload class
		TextureUpload(TextureUpload&&) = delete;
		/// @b Copy operator for the TextureUpload class
		TextureUpload& operator= (const TextureUpload&) = delete;
		/// @b Move operator for the TextureUpload class
		TextureUpload& operator= (TextureUpload&&) = delete;
		/// @b Default destructor
		~TextureUpload();
	public:
		/// @b Min/mag filters for the texture. X = min filter, Y = mag filter
		glm::vec<2, GLint, glm::defaultp> minmag;
		/// @b Minimum and maximum values for the LOD (x = min, y = max). Default : {-1000; 1000}.
		glm::vec<2, GLfloat, glm::defaultp> lod;
		/// @b Wrap parameters for all dimensions.
		glm::vec<3, GLint, glm::defaultp> wrap;
		/// @b Defines the default swizzle parameter for the texture.
		glm::vec<4, GLint, glm::defaultp> swizzle;
		/// @b Defines the pixel pack and unpack alignment. X = pack, Y = unpack.
		glm::vec<2, GLint, glm::defaultp> alignment;
		/// @b The level of the texture uploaded.
		GLint level;
		/// @b The internal pixel format of the texture uploaded. Ex : GL_RGB, GL_R8UI (...)
		GLint internalFormat;
		/// @b Texture size, in all dimensions.
		glm::vec<3, GLsizei, glm::defaultp> size;
		/// @b Pixel format for texture upload. Ex : GL_RED, GL_RGB (...)
		GLenum format;
		/// @b Pixel type for texture upload. Ex : GL_FLOAT, GL_UNSIGNED_BYTE (...)
		GLenum type;
		/// @b The data to upload to OpenGL
		const void* data;
};

/// @b Helper class regrouping the positions, normals, texture coordinates and indices of a mesh.
/// @details Used to generate the default mesh in the main program. There is multiple index buffers, since
/// we have multiple things to draw, in different ways (this will be uploaded to one VAO only).
struct Mesh {
	public:
		/// @b Default constructor for the Mesh class.
		Mesh(void);
		/// @b Copy constructor for the Mesh class.
		Mesh(const Mesh&) = delete;
		/// @b Move constructor for the Mesh class.
		Mesh(Mesh&&) = delete;
		/// @b Copy operator for the Mesh class.
		Mesh& operator= (const Mesh&) = delete;
		/// @b Move operator for the Mesh class.
		Mesh& operator= (Mesh&&) = delete;
		/// @b Default destructor for the Mesh class.
		~Mesh();
	public:
		/// @b The positions of the mesh.
		std::vector<glm::vec4> positions;
		/// @b The normals of each vertex.
		std::vector<glm::vec4> normals;
		/// @b The texture coordinates of each vertex.
		std::vector<glm::vec3> texture;
		/// @b The indices used to draw the unit cube used for the program.
		std::vector<unsigned int> indices;
		/// @b The indices used to draw the cutting planes in the 3D view.
		std::vector<unsigned int> cutting_planes;
		/// @b The indices used to draw the planes in the planar viewers.
		std::vector<unsigned int> planar_view;
};

/// @b Helper class regrouping the data of a tetrahedral mesh.
struct VolMeshData {
	public:
		/// @b Default constructor for the Mesh class.
		VolMeshData(void);
		/// @b Copy constructor for the Mesh class.
		VolMeshData(const VolMeshData&) = delete;
		/// @b Move constructor for the Mesh class.
		VolMeshData(VolMeshData&&) = delete;
		/// @b Copy operator for the Mesh class.
		VolMeshData& operator= (const VolMeshData&) = delete;
		/// @b Move operator for the Mesh class.
		VolMeshData& operator= (VolMeshData&&) = delete;
		/// @b Default destructor for the Mesh class.
		~VolMeshData(void);
	public:
		/// @b The positions of the mesh.
		std::vector<glm::vec4> positions;
		/// @b The texture coordinates of each vertex.
		std::vector<glm::vec3> texture;
		/// @b Stores the indices of vertices needed for a tetrahedron
		std::vector<std::array<std::size_t, 4>> tetrahedra;
		/// @b Stores the indices of neighboring tetrahedra
		std::vector<std::vector<int>> neighbors;
		/// @b Per-face normals of each tetrahedron
		std::vector<std::array<glm::vec4, 4>> normals;
};

struct VolMesh {
	public:
		/// @b Default constructor for the VolMesh struct.
		VolMesh(void);
		/// @b Default destructor for the VolMesh struct.
		~VolMesh(void);
		/// @b Checks if the handles are valid (different from 0) and it has something to draw (tetCount > 0)
		bool isValid(void);
	public:
		/// @b The texture handle for the visible domains/intensities in the image.
		GLuint visibilityMap;
		/// @b The texture handle for the vertex positions.
		GLuint vertexPositions;
		/// @b The texture handle for the vertices' texture coordinates.
		GLuint textureCoordinates;
		/// @b The texture handle for the tetrahedra neighborhoods.
		GLuint neighborhood;
		/// @b The texture handle for the per-tetrahedra face normals.
		GLuint faceNormals;
		/// @b The number of tetrahedra to render once `glDrawElementsInstanced()` is called.
		GLsizei tetrahedraCount;
};


/// @b Simple structure merging all resources necessary to view a grid in 3D.
/// @details This structure must be associated to one grid, and one grid only. Thus, the pointer to the grid is defined
/// as `const std::shared_ptr<>` since we want the reference to be held during the lifetime of the object.
struct GridGLView {
	public:
		/// @b Default constructor for the grid view. Must be associated to one and only one grid.
		GridGLView(const std::shared_ptr<DiscreteGrid>& _g);
		/// @b Copy constructor of the GridGLView struct.
		GridGLView(const GridGLView&) = default;
		/// @b Move constructor of the GridGLView struct.
		GridGLView(GridGLView&&) = default;
		/// @b Copy operator of the GridGLView struct.
		GridGLView& operator= (const GridGLView&) = default;
		/// @b Move operator of the GridGLView struct.
		GridGLView& operator= (GridGLView&&) = default;
		/// @b Default destructor of the GridGLView struct.
		~GridGLView(void);
	public:
		/// @b The pointer to the grid we want to show.
		std::shared_ptr<DiscreteGrid> grid;
		/// @b The texture handle to access the grid data in shaders.
		GLuint gridTexture;
		/// @b The volumetric mesh handles to use when drawing
		VolMesh volumetricMesh;
		/// @b The bounding box's color, as a triplet of normalized values for R, G, and B.
		glm::vec3 boundingBoxColor;
		/// @b The epsilon to provide for the volumetric viewing method
		glm::vec3 defaultEpsilon;
		/// @b The number of channels contained in the image
		unsigned int nbChannels;
};

#endif // VISUALIZATION_VIEWER_INCLUDE_VIEWER_STRUCTS_HPP_
