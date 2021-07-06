#ifndef VISUALIZATION_IMAGE_TIFF_INCLUDE_TIFF_FRAME_HPP_
#define VISUALIZATION_IMAGE_TIFF_INCLUDE_TIFF_FRAME_HPP_

#include "./tiff_include_common.hpp"

#include "./tiff_helpers.hpp"

namespace Image {

namespace Tiff {

	struct Frame {
		public:
			typedef std::shared_ptr<Frame> Ptr;

		public:
			/// @b Default ctor, which constructs the frame, but doesn't parse the information in it.
			/// @warning If the frame cannot be parsed, throws an exception.
			Frame(const std::string& _v, const tdir_t _cur_directory) noexcept(false);
			/// @b Default dtor, which releases the information required by this frame
			~Frame(void) = default;

			/// @b Returns true if the two frames are 'compatible'.
			/// @details Two frames are considered compatible if they have the same width; the same height, and the same
			/// number of bits per pixel. Anything else is not taken into account, for example if we have one frame
			/// which has two samples per pixel and another which has only one, they are still considered compatible if
			/// they have the same width, height, and BpS.
			bool isCompatibleWith(const Frame& _frame);

			/// @b Returns true if the current frame is 'compatible' with the given width, height, and bits per pixel.
			bool isCompatibleWith(uint32_t w, uint32_t h, uint16_t bps);

			bool hasSameEncoding(const Frame& _frame);

			/// @b Returns the value of the planar configuration of the frame.
			uint16_t planarConfiguration(TIFF* handle = nullptr) const;

			/// @b Returns the value of the photometric interpretation of the frame.
			uint16_t photometricInterpretation(TIFF* handle = nullptr) const;

			/// @b Returns a libTIFF-managed handle to read information from the file
			TIFF* getLibraryHandle(void) const;

			/// @b Reads the width of the frame
			uint32_t width(TIFF* handle = nullptr) const;

			/// @b Reads the height of the frame
			uint32_t height(TIFF* handle = nullptr) const;

			/// @b Read the SampleFormat field in the TIFF IFD.
			uint16_t sampleFormat(TIFF* handle = nullptr) const;

			/// @b Reads the BitsPerSample field in the TIFF IFD.
			uint16_t bitsPerSample(TIFF* handle = nullptr) const;

		protected:
			/// @b Loads information from the TIFF file, in order to parse it efficiently later.
			/// @warning This TIFF frame implementation only supports one sample per pixel, for now.
			void loadTIFFInfo(std::string_view sourceFile) noexcept(false);

			/// @b Returns true if the frame has a valid bits per sample count.
			/// @details The number of bits per sample is defined as 'correct' or 'valid' if all the samples are
			/// expressed using the same number of bits, even for the extra samples given they are present.
			bool fetchSamplesPerPixel(TIFF* frame_handle) const;

		public:
			/// @b The filename of the TIFF file this frame is located in.
			const std::string sourceFile;
			/// @b The directory offset into the file
			const tdir_t directoryOffset;
			/// @b The number of samples per pixel. Contains the original value ([1-3]) with the ExtraSamples added.
			uint16_t samplesPerPixel;
			/// @b The number of rows per strip
			uint32_t rowsPerStrip;
			/// @b The number of strips per image.
			uint64_t stripsPerImage;
	};

} // namespace TIFF

} // namespace Image

#endif // VISUALIZATION_IMAGE_TIFF_INCLUDE_TIFF_FRAME_HPP_
