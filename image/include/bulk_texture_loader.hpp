/**********************************************************************
 * FILE : image/include/bulk_texture_loader.hpp
 * AUTH : Thibault de Villèle
 * DATE : 19/02/2020
 * DESC : A bulk image loader, allowing to load a stack of images as a
 *	  3D texture to use later.
 **********************************************************************/

#ifndef IMAGE_INCLUDE_BULK_TEXTURE_LOADER_HPP_
#define IMAGE_INCLUDE_BULK_TEXTURE_LOADER_HPP_

#include <vector>
#include <chrono>
#include <iterator>

#include <QString>

#include "../../TinyTIFF/tinytiffreader.h"

#define DEFAULT_TIMEKEEPING_DURATION std::chrono::duration<double, std::milli>

/**
 * @brief The bulk_texture_loader class allows you to load images from a stack.
 * @details This class loads medical images from a stack, adn returns their
 * data in an array, organized in the same order as the one given by the
 * filesystem. This array can then be used in a 3D texture, using OpenGL's
 * texture bindings.
 * @warning This class assumes every image is the same size : the size of the
 * first one loaded in. It currently DOES NOT check for anything other than the
 * size of the first image. It WILL produce undefined behaviour when given a
 * stack of images of different sizes, or given a directory directly.
 *
 *
 * @todo TODO [Improvement/Future] : allow to give a directory, and get only tif files, leave everyhting else untouched.
 * @todo TODO [Future] : allow to load images of multiple sizes (pad w/r/t the largest)
 * @todo TODO [Future/Maybe] : allow to give directly a vector of filenames (from a JSON most likely) and load those instead
 */
class bulk_texture_loader {
	public:
		bulk_texture_loader();
		~bulk_texture_loader();
		/**
		 * @brief Loads a stack of images directly from a folder.
		 * @details Prompts the user to select images, and proceeds to
		 * load all images. Afterwards, loads them all into memory, and
		 * returns an array representing the images stacked on one another.
		 * @return An array representing the images stacked on one another
		 */
		unsigned char* load_stack_from_folder();
		/**
		 * @brief Returns the image width
		 * @return the image width
		 */
		std::size_t get_image_width() const;
		/**
		 * @brief Returns the image height
		 * @return the image height
		 */
		std::size_t get_image_height() const;
		/**
		 * @brief Returns the number of images loaded (i.e., the depth)
		 * @return the number of images
		 */
		std::size_t get_image_depth() const;
		/**
		 * @brief Get loaded data
		 * @return the data
		 */
		const unsigned char* get_data() const { return this->image_data; }
		/**
		 * @brief Enables or disables downsampling
		 * @param _is_enabled new value of downsampling_enabled
		 */
		void enable_downsampling(bool _is_enabled);
		/**
		 * @brief Frees up the data loaded.
		 */
		void free_data();
	protected:
		/**
		 * @brief Loads a single image from the filesystem
		 * @param absolute_path The path of image to load
		 */
		void load_single_image(QString absolute_path);
		/**
		 * @brief Load all images given by the vector paths
		 * @param absolute_paths The paths if images to load
		 */
		void load_all_images(QStringList absolute_paths);
		/**
		 * @brief Print some basic debug info (time spent [and maybe memoryused ?])
		 */
		void print_debug_info() const;
		/**
		 * @brief Initializes the info on image height/width/depth, and reserves the vector data.
		 * @param test_file The file to get info from.
		 */
		void initialize_image_data(const QString& test_file, int nb_images);
	private:
		/**
		 * @brief raw file opening duration : sum of all file opening durations
		 */
		mutable DEFAULT_TIMEKEEPING_DURATION raw_file_opening_duration;
		/**
		 * @brief raw file opening duration : sum of all file reading durations
		 */
		mutable DEFAULT_TIMEKEEPING_DURATION raw_file_reading_duration;
		/**
		 * @brief raw file copying duration : sum of all copy operation durations
		 */
		mutable DEFAULT_TIMEKEEPING_DURATION raw_file_copying_duration;
		/**
		 * @brief raw file copying duration : sum of all copy operation durations
		 */
		mutable DEFAULT_TIMEKEEPING_DURATION std_data_copying_duration;
		/**
		 * @brief complete stack loading duration : time from original call to end of loading.
		 * WARNING : doesn't take into account the time the user takes to select the files.
		 */
		mutable DEFAULT_TIMEKEEPING_DURATION complete_stack_loading_duration;
		/**
		 * @brief Number of file opening operations
		 */
		mutable uint number_of_file_opening_operations;
		/**
		 * @brief Number of file reading operations
		 */
		mutable uint number_of_file_reading_operations;
		/**
		 * @brief Number of file copying operations
		 */
		mutable uint number_of_file_copying_operations;
		/**
		 * @brief Checks if the image needs to be downsampled when loaded.
		 */
		bool downsampling_enabled;
		/**
		 * @brief Pointer to the tiff file opened.
		 * @note Defined here to optimize the CPU calls by a bit.
		 */
		TinyTIFFReaderFile* tiff_file;
		/**
		 * @brief Image data read by the loader.
		 */
		unsigned char* image_data;
		/**
		 * @brief Storage space for raw data, allowing us to write to it every time and not allocate/free it every iteration.
		 */
		unsigned char* raw_data_storage;
		/**
		 * @brief Pointer to the first empty element in image_data
		 * @warning If image_data is full, this will point to image_data.end()
		 */
		std::size_t end_iterator;
		/**
		 * @brief Width of images
		 */
		std::size_t image_width;
		/**
		 * @brief Height of images
		 */
		std::size_t image_height;
		/**
		 * @brief Number of images loaded
		 */
		std::size_t image_depth;
};

#endif // IMAGE_INCLUDE_BULK_TEXTURE_LOADER_HPP_
