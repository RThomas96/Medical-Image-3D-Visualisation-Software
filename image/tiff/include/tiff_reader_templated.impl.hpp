#ifndef VISUALIAZTION_IMAGE_TIFF_INCLUDE_TEMPLATED_BACKEND_IMPL_HPP_
#define VISUALIAZTION_IMAGE_TIFF_INCLUDE_TEMPLATED_BACKEND_IMPL_HPP_

#ifndef VISUALIAZTION_IMAGE_TIFF_INCLUDE_TEMPLATED_BACKEND_HPP_
#include "./tiff_reader_templated.hpp"	  // Included between header guards to have code completion in QtCreator.
#endif

#include <iomanip>

namespace Image {

	namespace Tiff {

		template <typename unsupported_element_t>
		TIFFReaderTemplated<unsupported_element_t>::TIFFReaderTemplated(uint32_t w, uint32_t h, std::size_t _dim, uint32_t z, glm::vec3 voxel_dim) {
			UNUSED(w);
			UNUSED(h);
			UNUSED(_dim);
			throw std::runtime_error("Error : unsupported type passed to the TIFFReaderTemplated constructor.");
		}
		template <>
		TIFFReaderTemplated<std::uint8_t>::TIFFReaderTemplated(uint32_t w, uint32_t h, std::size_t _dim, uint32_t z, glm::vec3 voxel_dim) {
			this->images.clear();

			this->resolution.x		   = w;
			this->resolution.y		   = h;
			this->resolution.z		   = z;
			this->voxel_dimensionality = _dim;

			this->internal_data_type = ImageDataType::Unsigned | ImageDataType::Bit_8;

			this->bitsPerSample	   = 8;
			this->sampleFormat	   = SAMPLEFORMAT_UINT;
			this->voxel_dimensions = voxel_dim;
#ifndef NDEBUG
			std::cerr << "Creating a TIFF backend detail of type " << pnt(std::uint8_t) << '\n';
#endif
		}

		template <>
		TIFFReaderTemplated<std::uint16_t>::TIFFReaderTemplated(uint32_t w, uint32_t h, std::size_t _dim, uint32_t z, glm::vec3 voxel_dim) {
			this->images.clear();

			this->resolution.x		   = w;
			this->resolution.y		   = h;
			this->resolution.z		   = z;
			this->voxel_dimensionality = _dim;

			this->internal_data_type = ImageDataType::Unsigned | ImageDataType::Bit_16;

			this->bitsPerSample	   = 16;
			this->sampleFormat	   = SAMPLEFORMAT_UINT;
			this->voxel_dimensions = voxel_dim;
#ifndef NDEBUG
			std::cerr << "Creating a TIFF backend detail of type " << pnt(std::uint16_t) << '\n';
#endif
		}

		template <>
		TIFFReaderTemplated<std::uint32_t>::TIFFReaderTemplated(uint32_t w, uint32_t h, std::size_t _dim, uint32_t z, glm::vec3 voxel_dim) {
			this->images.clear();

			this->resolution.x		   = w;
			this->resolution.y		   = h;
			this->resolution.z		   = z;
			this->voxel_dimensionality = _dim;

			this->internal_data_type = ImageDataType::Unsigned | ImageDataType::Bit_32;

			this->bitsPerSample	   = 32;
			this->sampleFormat	   = SAMPLEFORMAT_UINT;
			this->voxel_dimensions = voxel_dim;
#ifndef NDEBUG
			std::cerr << "Creating a TIFF backend detail of type " << pnt(std::uint32_t) << '\n';
#endif
		}

		template <>
		TIFFReaderTemplated<std::uint64_t>::TIFFReaderTemplated(uint32_t w, uint32_t h, std::size_t _dim, uint32_t z, glm::vec3 voxel_dim) {
			this->images.clear();

			this->resolution.x		   = w;
			this->resolution.y		   = h;
			this->resolution.z		   = z;
			this->voxel_dimensionality = _dim;

			this->internal_data_type = ImageDataType::Unsigned | ImageDataType::Bit_64;

			this->bitsPerSample	   = 64;
			this->sampleFormat	   = SAMPLEFORMAT_UINT;
			this->voxel_dimensions = voxel_dim;
#ifndef NDEBUG
			std::cerr << "Creating a TIFF backend detail of type " << pnt(std::uint64_t) << '\n';
#endif
		}

		template <>
		TIFFReaderTemplated<std::int8_t>::TIFFReaderTemplated(uint32_t w, uint32_t h, std::size_t _dim, uint32_t z, glm::vec3 voxel_dim) {
			this->images.clear();

			this->resolution.x		   = w;
			this->resolution.y		   = h;
			this->resolution.z		   = z;
			this->voxel_dimensionality = _dim;

			this->internal_data_type = ImageDataType::Signed | ImageDataType::Bit_8;

			this->bitsPerSample	   = 8;
			this->sampleFormat	   = SAMPLEFORMAT_INT;
			this->voxel_dimensions = voxel_dim;
#ifndef NDEBUG
			std::cerr << "Creating a TIFF backend detail of type " << pnt(std::int8_t) << '\n';
#endif
		}

		template <>
		TIFFReaderTemplated<std::int16_t>::TIFFReaderTemplated(uint32_t w, uint32_t h, std::size_t _dim, uint32_t z, glm::vec3 voxel_dim) {
			this->images.clear();

			this->resolution.x		   = w;
			this->resolution.y		   = h;
			this->resolution.z		   = z;
			this->voxel_dimensionality = _dim;

			this->internal_data_type = ImageDataType::Signed | ImageDataType::Bit_16;

			this->bitsPerSample	   = 16;
			this->sampleFormat	   = SAMPLEFORMAT_INT;
			this->voxel_dimensions = voxel_dim;
#ifndef NDEBUG
			std::cerr << "Creating a TIFF backend detail of type " << pnt(std::int16_t) << '\n';
#endif
		}

		template <>
		TIFFReaderTemplated<std::int32_t>::TIFFReaderTemplated(uint32_t w, uint32_t h, std::size_t _dim, uint32_t z, glm::vec3 voxel_dim) {
			this->images.clear();

			this->resolution.x		   = w;
			this->resolution.y		   = h;
			this->resolution.z		   = z;
			this->voxel_dimensionality = _dim;

			this->internal_data_type = ImageDataType::Signed | ImageDataType::Bit_32;

			this->bitsPerSample	   = 32;
			this->sampleFormat	   = SAMPLEFORMAT_INT;
			this->voxel_dimensions = voxel_dim;
#ifndef NDEBUG
			std::cerr << "Creating a TIFF backend detail of type " << pnt(std::int32_t) << '\n';
#endif
		}

		template <>
		TIFFReaderTemplated<std::int64_t>::TIFFReaderTemplated(uint32_t w, uint32_t h, std::size_t _dim, uint32_t z, glm::vec3 voxel_dim) {
			this->images.clear();

			this->resolution.x		   = w;
			this->resolution.y		   = h;
			this->resolution.z		   = z;
			this->voxel_dimensionality = _dim;

			this->internal_data_type = ImageDataType::Signed | ImageDataType::Bit_64;

			this->bitsPerSample	   = 64;
			this->sampleFormat	   = SAMPLEFORMAT_INT;
			this->voxel_dimensions = voxel_dim;
#ifndef NDEBUG
			std::cerr << "Creating a TIFF backend detail of type " << pnt(std::int64_t) << '\n';
#endif
		}

		template <>
		TIFFReaderTemplated<float>::TIFFReaderTemplated(uint32_t w, uint32_t h, std::size_t _dim, uint32_t z, glm::vec3 voxel_dim) {
			this->images.clear();

			this->resolution.x		   = w;
			this->resolution.y		   = h;
			this->resolution.z		   = z;
			this->voxel_dimensionality = _dim;

			this->internal_data_type = ImageDataType::Floating | ImageDataType::Bit_32;

			this->bitsPerSample	   = 32;
			this->sampleFormat	   = SAMPLEFORMAT_IEEEFP;
			this->voxel_dimensions = voxel_dim;
#ifndef NDEBUG
			std::cerr << "Creating a TIFF backend detail of type " << pnt(float) << '\n';
#endif
		}

		template <>
		TIFFReaderTemplated<double>::TIFFReaderTemplated(uint32_t w, uint32_t h, std::size_t _dim, uint32_t z, glm::vec3 voxel_dim) {
			this->images.clear();

			this->resolution.x		   = w;
			this->resolution.y		   = h;
			this->resolution.z		   = z;
			this->voxel_dimensionality = _dim;

			this->internal_data_type = ImageDataType::Floating | ImageDataType::Bit_64;

			this->bitsPerSample	   = 64;
			this->sampleFormat	   = SAMPLEFORMAT_IEEEFP;
			this->voxel_dimensions = voxel_dim;
#ifndef NDEBUG
			std::cerr << "Creating a TIFF backend detail of type " << pnt(double) << '\n';
#endif
		}

		template <typename element_t>
		TIFFReaderTemplated<element_t>::~TIFFReaderTemplated() {
			this->cleanResources();
		}

		template <typename element_t>
		typename TIFFReaderTemplated<element_t>::Ptr
		  TIFFReaderTemplated<element_t>::createBackend(uint32_t width, uint32_t height, std::size_t _dim, uint32_t z, glm::vec3 voxel_dim) {
			return Ptr(new TIFFReaderTemplated<pixel_t>(width, height, _dim, z, voxel_dim));
		}

		template <typename element_t>
		void TIFFReaderTemplated<element_t>::cleanResources() {
			// Clear cache and loaded image data :
			this->cachedSlices.clearCache();
			this->images.clear();
			// Reset all variables to either their original state, or a known error value (like voxdims = -1) :
			this->internal_data_type   = ImageDataType::Unknown;
			this->resolution		   = svec3{0, 0, 0};
			this->voxel_dimensions	   = glm::vec3{-1.f, -1.f, -1.f};
			this->voxel_dimensionality = 0;
			this->bitsPerSample		   = 0;
			this->sampleFormat		   = SAMPLEFORMAT_VOID;
			this->samplesPerPixel	   = 0;
		}

		template <typename element_t>
		void TIFFReaderTemplated<element_t>::compute_stack_basename(void) {
			if (this->images.empty()) {
				this->stack_base_name = "<undefined>";
			}
			this->stack_base_name = "default_grid_name";
		}

		template <typename element_t>
		std::size_t TIFFReaderTemplated<element_t>::loadSlice(std::size_t slice_idx) {
			if (slice_idx >= this->images.size()) {
				throw std::runtime_error("Tried to load past-the-end image.");
			}

			// If the image is already loaded, return its index :
			if (this->cachedSlices.hasData(slice_idx)) {
				std::cerr << "Already cached previously\n";
				return this->cachedSlices.findIndex(slice_idx);
			}

			// Image dimensions :
			std::size_t imgsize = this->resolution.x * this->voxel_dimensionality * this->resolution.y;

			// Typedef of cache-able data :
			using ImagePtr = typename cache_t::data_t_ptr;	  // So : std::shared_ptr< std::vector<img_t> >

			// Read all values for all frames at index slice_idx :
			std::vector<pixel_t> framedata(this->resolution.x * this->resolution.y);
			// The target vector to combine into :
			ImagePtr full_image = std::make_shared<std::vector<pixel_t>>(framedata.size() * this->voxel_dimensionality);

			// Result for libTIFF operations. For most ops, returns 1 on success.
			int result = 1;

#warning Only works for single-channel images. Should take into account the nb of tiff frames, not voxel dimensionality
			// Iterate on all planes :
			for (std::size_t i = 0; i < this->voxel_dimensionality; ++i) {
				// Get the right frame :
				const Frame::Ptr& frame = this->images[slice_idx][i];
				// Open the file, and set it to the right directory :
				TIFF* file = frame->getLibraryHandle();
				if (file == nullptr) {
					throw std::runtime_error("Could not set the directory of file " + frame->sourceFile);
				}

				// Read all strips :
				for (uint64_t i = 0; i < frame->stripsPerImage; ++i) {
					tsize_t readPixelSize = 0;	  // bytes to read from file
					if (i == frame->stripsPerImage - 1) {
						// The last strip is handled differently than the rest. Fewer bytes should be read.
						// compute the remaining rows, to get the number of bytes :
						tsize_t last_strip_row_count = this->resolution.y - (frame->stripsPerImage - 1) * frame->rowsPerStrip;
						if (last_strip_row_count == 0) {
							TIFFClose(file);
							throw std::runtime_error("Last strip was 0 rows tall ! Something went wrong beforehand.");
						}
						readPixelSize = last_strip_row_count * this->resolution.x;
					} else {
						readPixelSize = this->resolution.x * frame->rowsPerStrip;	 // strip size, in pixels
					}

					tsize_t readBytesSize  = readPixelSize * (this->bitsPerSample / 8);	   // strip size, in bytes
					tsize_t stripPixelSize = this->resolution.x * frame->rowsPerStrip;	  // The size of a strip, in rows

					tmsize_t read = TIFFReadEncodedStrip(file, i, framedata.data() + i * stripPixelSize, readBytesSize);
					if (read < 0) {
						// print width for numbers :
						std::size_t pwidth = static_cast<std::size_t>(std::ceil(std::log10(static_cast<double>(frame->stripsPerImage))));
						std::cerr << "[DEBUG]\t\t(" << std::setw(pwidth) << i << "/" << frame->stripsPerImage << ") ERROR:";
						std::cerr << "Could not read strip " << i << " from the frame.\n";
					}
				}

				// Close the tiff file (mostly to free up one file descriptor) :
				TIFFClose(file);

				// Copy the strips' data to the full image :
				typename std::vector<pixel_t>::iterator full_img_pos = full_image->begin() + i;	   // begin + current color plane queried
				std::for_each(framedata.cbegin(), framedata.cend(), [this, &full_img_pos](const pixel_t pixval) -> void {
					*full_img_pos = pixval;
					full_img_pos += this->voxel_dimensionality;
				});
			}

			return this->cachedSlices.loadData(slice_idx, full_image);
		}

		template <typename element_t>
		template <typename out_t>
		bool TIFFReaderTemplated<element_t>::template_tiff_read_sub_region(svec3 origin, svec3 size, std::vector<out_t>& values) {
			/// @brief Const iterator type for the cached data, which does not modify the data itself
			using cache_iterator_t = typename cache_t::data_t_ptr::element_type::const_iterator;
			/// @brief Iterator type for the target data
			using target_iterator_t = typename std::vector<out_t>::iterator;

			// ensure we have the right size for the buffer, fill it with 0s for now :
			values.resize(size.x * size.y * size.z * this->voxel_dimensionality);
			std::fill(values.begin(), values.end(), pixel_t(0));

			//
			// Check for outliers :
			// If origin's coordinates are bigger than this stack's dimensions, the whole subregion will be outside.
			// Simply fill the vector with null values and return
			//
			if (origin.x >= this->resolution.x || origin.y >= this->resolution.y || origin.z >= this->images.size()) {
				return true;
			}

			/// @brief Beginning of slices to load and cache
			std::size_t src_slice_begin = origin.z;
			/// @brief end of slices to cache or end of slices available
			std::size_t src_slice_end = (src_slice_begin + size.z >= this->images.size()) ?
											this->images.size() :
											src_slice_begin + size.z;

			// the number of slices which will be read by the first for-loop :
			std::size_t tgt_slices_readable = src_slice_end - src_slice_begin;

			/// @brief Index of the last line we can read from the source buffer
			std::size_t src_height_idx_end = (origin.y + size.y >= this->resolution.y) ?
												 this->resolution.y - origin.y :
												 size.y;

			/// @brief the number of lines that can be read from the source buffer :
			std::size_t src_height_readable = src_height_idx_end - origin.y;

			/// @brief total length of a line in the source
			std::size_t src_line_size = this->resolution.x * this->voxel_dimensionality;
			/// @brief beginning of a line to read from the source buffer
			std::size_t src_line_idx_begin = origin.x * this->voxel_dimensionality;
			/// @brief amount of values to read into the target buffer from the source
			std::size_t src_line_idx_end = (src_line_idx_begin + size.x * this->voxel_dimensionality >= src_line_size) ?
											   src_line_size :
											   src_line_idx_begin + size.x * this->voxel_dimensionality;

			/// @brief Line length in the buffer to write to
			std::size_t target_line_length = size.x * this->voxel_dimensionality;
			/// @brief image length in the buffer to write to
			std::size_t target_image_length = size.y * target_line_length;

			std::size_t y = 0, z = 0;

			// Iterate on slices :
			for (z = 0; z < tgt_slices_readable; ++z) {
				// load and cache the slice to load in memory (or fetch it directly if already cached) :
				std::size_t cache_idx = this->loadSlice(src_slice_begin + z);
				// get img data from cache :
				typename cache_t::data_t_ptr image_data = this->cachedSlices.getDataIndexed(cache_idx);
				// read all lines we _can_ from the source :
				for (y = 0; y < src_height_readable; ++y) {
					// Figure out the right location in the source buffer :
					cache_iterator_t begin = image_data->cbegin() + (origin.y + y) * src_line_size + src_line_idx_begin;
					// Read <source begin> + [size to read | rest of source line] :
					cache_iterator_t end = image_data->cbegin() + (origin.y + y) * src_line_size + src_line_idx_end;
					// beginning in target buffer :
					target_iterator_t target_begin = values.begin() + z * target_image_length + y * target_line_length;
					// Copy a whole line at once :
					std::copy(begin, end, target_begin);
				}
			}
			// don't need to pad the remaining slices (if any) because of the first call to fill()
			return true;
		}

		template <typename element_t>
		template <typename range_t>
		bool TIFFReaderTemplated<element_t>::template_tiff_get_sub_range_values(std::size_t channel, glm::tvec2<range_t>& range) {
			// If channel unavailable, return false and leave the ranges uninitialized.
			if (channel >= this->voxel_dimensionality) {
				return false;
			}
			// If the channel is available, return the value ranges :
			range = this->value_ranges[channel];
			return true;
		}

		template <typename element_t>
		void TIFFReaderTemplated<element_t>::parse_info_in_separate_thread(ThreadedTask::Ptr _task,
		  const std::vector<std::vector<std::string>>& user_filenames) {
			// Should be checked in the launcher of this thread, but just to be sure :
			if (user_filenames.empty()) {
				_task->pushMessage("No filenames were provided.");
				_task->end(false);
				return;
			}

			_task->setState(TaskState::Launched);

			{
				{
					{	 // Check there are no empty vectors in the provided filenames :
						bool files_valid			 = true;
						std::size_t integer_iterator = 0;
						std::for_each(user_filenames.begin(), user_filenames.end(),
						  [&files_valid, &_task, &integer_iterator](const std::vector<std::string>& v) -> void {
							  if (v.empty()) {
								  files_valid = false;
								  _task->pushMessage(std::string("Error : stack number ") + std::to_string(integer_iterator) + std::string(" of"
																																		   "filenames was empty. Please remove it and retry parsing the files."));
							  }
							  integer_iterator++;
						  });
						// if anything happened return with a failure state :
						if (not files_valid) {
							_task->end(false);
							this->cleanResources();
							return;
						}
					}
				}
			}

			// Used to check all frames are compatible with each other, and with the current object
			Frame::Ptr reference_frame					= nullptr;
			std::uint16_t incremental_samples_per_pixel = 0;

			{
				{
					{	 // Check current object is of right type
						try {
							reference_frame = std::make_shared<Frame>(user_filenames[0][0], 0);
						} catch (std::exception& e) {
							_task->pushMessage(std::string("Error : could not create a reference frame from the first file to parse the "
														   "rest of the files. Erorr message : \n") +
											   e.what());
							_task->end(false);
							this->cleanResources();
							return;
						}
						// if not compatible, stop parsing files !
						if (not this->is_frame_compatible_with_backend(_task, reference_frame)) {
							_task->pushMessage("Error : given frames are not compatible with the currently created implementation.");
							_task->end(false);
							this->cleanResources();
							return;
						}
					}
				}
			}

			incremental_samples_per_pixel += reference_frame->samplesPerPixel;
			std::cerr << "Reference frame had " << incremental_samples_per_pixel << " samples\n";
			std::cerr << "Stack count : " << user_filenames.size() << '\n';

			{
				{
					{	 // Check the same number of files are here (per stack of filenames).
						// If there's only one do nothing. Otherwise :
						if (user_filenames.size() > 1) {
							// Otherwise, check each additionnal stack and see if they have the same # of files :
							bool valid			  = true;
							std::size_t ref_count = user_filenames[0].size();	 // reference count

							// Note : could find any stack with diff nb of files with std::find[firstt_of|last_of]() but this way I can log
							// which ones are not compatible and tell the user exactly what needs to change in its input :
							for (std::size_t i = 1; i < user_filenames.size(); i++) {
								if (user_filenames[i].size() != ref_count) {
									// signal it's not valid, but don't stop here.
									valid = false;
									_task->pushMessage(
									  std::string("Stack of files number <") + std::to_string(i) + std::string("> does not contain the"
																											   " requisite amount of filenames (got ") +
									  std::to_string(user_filenames[i].size()) + std::string(", instead of the expected ") + std::to_string(ref_count) + std::string(")."));
								}
								if (not user_filenames.empty()) {
									try {
										Frame::Ptr additional_frame = std::make_shared<Frame>(user_filenames[i][0], 0);
										incremental_samples_per_pixel += additional_frame->samplesPerPixel;
										std::cerr << "Additional frames on stack " << i << " have " << additional_frame->samplesPerPixel << " additionnal samples\n";
									} catch (std::exception& _e) {
										std::cerr << "Error : exception thrown when creating a frame in stack " << i << '\n';
									}
								}
							}

							// after all stacks are done, stop here if an error occured.
							if (not valid) {
								this->cleanResources();
								_task->end(false);
								return;
							}
						}
					}
				}
			}

			this->samplesPerPixel = incremental_samples_per_pixel;
			this->value_ranges.resize(this->samplesPerPixel);

			// Preliminary parsing of the file stack is done :
			_task->setSteps(user_filenames[0].size());
			_task->setState(TaskState::Running);

			// The number of file stacks :
			std::size_t file_channels = user_filenames.size();

			{
				{
					{	 // Then, check the files are the 'same' (same encoding [bitsperpixel, samplesperpixel] and size and frame count)
						for (std::size_t file_index = 0; file_index < user_filenames[0].size(); ++file_index) {
							// Get the number of available directories :
							tdir_t current_dir_count = countDirectories(std::string_view(user_filenames[0][file_index]));
							// Check all other files in the different stacks have the same number of directories :
							for (std::size_t c = 1; c < file_channels; ++c) {
								tdir_t temp_dir_count = countDirectories(std::string_view(user_filenames[c][file_index]));
								if (temp_dir_count != current_dir_count) {
									_task->pushMessage(
									  std::string("Error : file \"") + user_filenames[c][file_index] + std::string("\" does not have the"
																												   " expected count of TIFF images (expected ") +
									  std::to_string(current_dir_count) + std::string(", "
																					  "but got ") +
									  std::to_string(temp_dir_count) + std::string(" instead)."));
									this->cleanResources();
									_task->end(false);
									return;
								}
							}

							// Now, we _know_ all files have the same number of directories.
							// Iterate, and build the image representations out of the directories :
							for (std::size_t dir = 0; dir < current_dir_count; ++dir) {
								// Create a container for the image :
								TIFFImage current_image		   = TIFFImage();
								Tiff::Frame::Ptr current_frame = nullptr;
								// offset to apply when getting/setting color channels of the grid from the context of a single frame
								std::size_t channel_offset = 0;

								// iterate on all file stacks :
								for (std::size_t c = 0; c < file_channels; ++c) {
									try {
										// Attempt to create a frame from this file, at directory index 'dir'
										current_frame = std::make_shared<Tiff::Frame>(user_filenames[c][file_index], dir);
									} catch (std::exception e) {
										_task->pushMessage(
										  std::string("Error : parsing file \"") + user_filenames[c][file_index] + std::string("produced"
																															   "an error at directory ") +
										  std::to_string(dir) + std::string(". Error message : ") + e.what());
										this->cleanResources();
										_task->end(false);
										return;
									}

									// Check if the frame is compatible with the reference frame :
									if (not reference_frame->isCompatibleWith(*current_frame)) {
										_task->pushMessage(
										  std::string("Error : frame ") + std::to_string(dir) + std::string(" of file ") +
										  user_filenames[c][file_index] + std::string(" is not compatible with the rest of the frames."));
										this->cleanResources();
										_task->end(false);
										return;
									}

									// push the full slice onto the stack :
									current_image.push_back(current_frame);

									// update ranges for each sample, in case they exist in the metadata :
									for (std::size_t s = 0; s < current_frame->samplesPerPixel; ++s) {
										glm::tvec2<pixel_t> frame_range{};
										// try to read from the frame :
										current_frame->getMinMaxSample(s, frame_range);
										// update values for the sample :
										this->value_ranges[channel_offset + s] = glm::tvec2<pixel_t>{
										  glm::min(frame_range.x, this->value_ranges[channel_offset + s].x),
										  glm::max(frame_range.y, this->value_ranges[channel_offset + s].y)};
									}

									// next frame will set different channels' data :
									channel_offset += current_frame->samplesPerPixel;
								}

								// push current image to the stack :
								this->images.push_back(current_image);
							}

							_task->advance();
						}
					}
				}
			}

			_task->setState(TaskState::Finishing);

			// Values already set in the ctor based on TIFF's capabilities and provided info :
			// - resolution (on X and Y)
			// - voxel dimensionality (supposed to be equal to samplesperpixel BUT CHECKED)
			// - internal_data_type (based on template parameters)
			// - bitsPerSample (based on template parameters)
			// - sampleFormat (based on template parameters)
			// - voxel_dimensions (TIFF doesn't carry any physical information)

			// Still to define :
			// - images (done above)
			// - filenames
			// - samplesPerPixel
			// - stack_base_name
			// - resolution (on Z)

			// set stack depth :
			this->resolution.z = this->images.size();

			// Copy filenames to local storage and then compute basename of this stack :
			this->filenames = user_filenames;
			this->compute_stack_basename();
			// Reset cache :
			this->cachedSlices.clearCache();

			// All directories are compatible and at least one frame is available. Count total # of samples :
			std::size_t total_samples = 0;
			for (std::size_t i = 0; i < file_channels; ++i) {
				total_samples += this->images[0][i]->samplesPerPixel;
			}
			if (total_samples != this->voxel_dimensionality) {
				std::cerr << "Warning : samples per pixel (computed) and voxel dimensionality (provided) are different. "
						  << "Overwriting previous value (" << this->voxel_dimensionality << " != " << total_samples << ")\n";
				this->voxel_dimensionality = total_samples;
			}
			if (total_samples != this->samplesPerPixel) {
				std::cerr << "Warning : previous incremental count of samples per pixel is erroneous (" << this->samplesPerPixel
						  << " != " << total_samples << ")\n";
				this->samplesPerPixel = total_samples;
			}

			// Finish the current task :
			_task->end();
			return;
		}

		template <typename element_t>
		bool TIFFReaderTemplated<element_t>::is_frame_compatible_with_backend(ThreadedTask::Ptr& _task,
		  Frame::Ptr& reference_frame) const {
			// Open the file :
			TIFF* lib_handle = reference_frame->getLibraryHandle();

			bool valid					  = true;
			std::uint16_t bits_per_sample = reference_frame->bitsPerSample(lib_handle);
			std::uint16_t sample_format	  = reference_frame->sampleFormat(lib_handle);
			std::uint32_t width			  = reference_frame->width(lib_handle);
			std::uint32_t height		  = reference_frame->height(lib_handle);

			if (this->sampleFormat != sample_format) {
				_task->pushMessage("Error : sample format was not identical between reference frame and current TIFF backend.");
				valid = false;
			}
			if (this->bitsPerSample != bits_per_sample) {
				_task->pushMessage("Error : bits/sample was not identical between reference frame and current TIFF backend.");
				valid = false;
			}
			if (this->resolution.x != width) {
				_task->pushMessage("Error : frame width was not identical between reference frame and current TIFF backend.");
				valid = false;
			}
			if (this->resolution.y != height) {
				_task->pushMessage("Error : frame height was not identical between reference frame and current TIFF backend.");
				valid = false;
			}

			TIFFClose(lib_handle);
			return valid;
		}

	}	 // namespace Tiff

}	 // namespace Image

#endif	  //  VISUALIAZTION_IMAGE_TIFF_INCLUDE_TEMPLATED_BACKEND_IMPL_HPP_
