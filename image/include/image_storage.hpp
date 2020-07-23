#ifndef IMAGE_INCLUDE_IMAGE_STORAGE_HPP_
#define IMAGE_INCLUDE_IMAGE_STORAGE_HPP_

#include "./bulk_texture_loader.hpp"
#include "../../grid/include/bounding_box.hpp"

#include <glm/glm.hpp>

#include <vector>
#include <iostream>

/// TODO (infinite) : make this class templated as to accept different types of data from TIF images. The standard supports uchar, uint16, and floats (IIRC, see paulbourke.net/dataformats/tiff/ for details)
/// TODO (1) : make the image loading and processing happen here, or in a separate private struct, but centralise everything here (don't need 2 classes for the same job ...)
/// TODO (2) : in relation to (1), once it's done, remove the bulk_texture_loader reference.
/// DONE (3) : move the interpolation function from here to TetMesh, we'll need it there to interpolate according to the user's need.
/// TODO (4) : after (1) and maybe (2) are done, make a vector of pair of boundaries for different values of data as such :
///		- std::vector<std::pair<glm::uvec3, glm::uvec3>> :
///			- each index 'i' contains the positions of the bounding boxes for the data, if 'i' is the minimum value in which we are sure there's data.
///		construct it in the loading process : each time we copy a value, we loop from 'value' to 'max_vector_checked_value' and update coordinates with ternary operator
/// TODO (5) : once (4) is done, adapt the code to reflect (4), and remove the following macro :

/// @brief The minimum value at which a point is considered 'data' instead of 'void'
#define MIN_DATA_VALUE 5

typedef glm::vec<3, std::size_t, glm::highp> svec3;
typedef glm::vec<4, std::size_t, glm::highp> svec4;

/// @brief This class holds a texture in memory, for easy management.
/// @details The texture , once loaded is interpreted as a cube of dimensions [imageWidth, imageHeight, imageDepth] (meaning each voxel
/// is assumed to be a cube of side 1.0) in what we call initial space. As such, when querying the voxel index at a given position in
/// real space, we need only to return the truncature of the position given by `pos * real2InitialMatrix`. Additionnaly, a voxel's center
/// is easily defined : `voxelCenterPosition = voxelIndex + [.5, .5, .5]`.
class TextureStorage {

	public:
		/// @brief Default constructor for a stack loader.
		TextureStorage();

		/// @brief Destructor for the stack loader.
		~TextureStorage();

		/// @brief Asks the bulk_texture_loader to load images from the filesystem.
		TextureStorage& loadImages(void);

		/// @brief Enables downsampling on the texture loader
		TextureStorage& enableDownsampling(bool enabled = true) { this->downsampleImages = enabled; return *this; }

		/// @brief get the image specs.
		std::vector<svec3> getImageSpecs() const;

		/// @brief Returns the image size. First width, then height and finally depth (== number of images)
		svec3 getImageSize() const;

		/// @brief Get the min point of the image's data bounding box
		svec3 getImageBoundingBoxMin() const; // See TODO(4) and add argument here (value to get BoundingBox for)

		/// @brief Get the max point of the image's data bounding box
		svec3 getImageBoundingBoxMax() const; // See TODO(4) and add argument here (value to get BoundingBox for)

		/// @brief Gets the position in world space of the minimum point of the image's bounding box
		glm::vec3 getImageBoundingBoxMin_WS() const;

		/// @brief Gets the position in world space of the maximum point of the image's bounding box
		glm::vec3 getImageBoundingBoxMax_WS() const;

		/// @brief Get the image data, in its entirety
		const std::vector<unsigned char>& getData() const;

		/// @brief Gets the value of the texel nearest of the given position (no interpolation).
		unsigned char getTexelValue(glm::vec4 position) const;

		/// @brief Converts a real-space XYZ position to a voxel index within the grid.
		/// @returns A vector containing the IJK indexes of the index within the grid.
		svec3 convertRealSpaceToVoxelIndex(const glm::vec4 position) const;

		/// @brief Converts a real-space XYZ position to an initial-space XYZ position.
		/// @returns A XYZ position in initial space.
		glm::vec4 convertRealSpaceToInitialSpace(const glm::vec4 position) const;

		/// @brief Converts an initial-space XYZ position to a voxel index within the grid.
		/// @returns A vector containing the IJK indexes of the index within the grid.
		svec3 convertInitialSpaceToVoxelIndex(const glm::vec4 position) const;

		/// @brief Converts an initial-space XYZ position to a real-space XYZ position.
		/// @returns A XYZ position in real space.
		glm::vec4 convertInitialSpaceToRealSpace(const glm::vec4 position) const;

		/// @brief Get the matrix to transform from real space to initial space.
		glm::mat4 getRealToInitialMatrix(void) const { return this->real2InitialMatrix; }

		/// @brief Get the matrix to transform from initial space to real space.
		glm::mat4 getInitialToRealMatrix(void) const { return this->initial2RealMatrix; }

		/// @brief Set the transformation matrix to convert from initial space to real space.
		TextureStorage& setInitialToRealMatrix(const glm::mat4 transfoMat);

		/// @brief Gets the bounding box of this object's render bounding box, in world space.
		BoundingBox_General<float> getRenderBB_WS(void);

		/// @brief Resets all data and measurements associated with the texture.
		TextureStorage& resetTexture();
	protected:
		/// @brief Holds the specifications for the image.
		/// @details This is a 3x3 matrix, holding :
		///   - the image size in the first three components,
		///   - the image bounding box min point in the next three components,
		///   - the image bounding box max point in the last three components.
		/// The image bounding box is defined as the bounding box around the parts of the image that actually contain data.
		std::vector<svec3> imageSpecs;

		/// @brief a loader for the images.
		bulk_texture_loader* texLoader;

		/// @brief The image data loaded by the loader.
		std::vector<unsigned char> data;

		/// @brief Should the loader downsample the images while loading them ?
		bool downsampleImages;

		glm::mat4 initial2RealMatrix; ///< The matrix to go from initial space (this stack's space) to real space (the sample's original space).
		glm::mat4 real2InitialMatrix; ///< The matrix to go from real space (the sample's original space) to initial space (this stack's space).

		/// @brief load the imageSpec vector with the values obtained from the bulk_texture_loader
		void loadImageSpecs();
		/// @brief reset the image specs to their original values ([0, 0, 0] pretty much everywhere)
		void resetImageSpecs();
};

#endif // IMAGE_INCLUDE_IMAGE_STORAGE_HPP_
