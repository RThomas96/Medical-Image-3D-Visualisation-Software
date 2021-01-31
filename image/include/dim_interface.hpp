#ifndef IMAGE_INCLUDE_DIM_INTERFACE_HPP_
#define IMAGE_INCLUDE_DIM_INTERFACE_HPP_

#include "./data_interface.hpp"

#include <fstream>

namespace IO {

	template <typename T>
	class DIMInterface : public DataInterface<T> {
		public:
			/// @brief Simple typedef for the current template parameter typename.
			/// @details need to be re-defined in derived classes.
			using data_t = typename DataInterface<T>::data_t;
			/// @brief Pixel type, used for getPixel().
			/// @details Similar to OpenGL, whatever the dimensionnality of the data, queries return a vec4.
			/// @note If a channel is not defined (query to a grid containing only red
			using pixel_t = typename DataInterface<T>::pixel_t;
			/// @brief Allows to define bounds for an image's channels.
			/// @details Used to get the image intensity limits from a particular channel.
			using limit_t = typename DataInterface<T>::limit_t;
		public:
			DIMInterface(void);
			~DIMInterface(void);
		public:
			virtual void updateValues(void) override;
			virtual void loadInformation(void) override;
			virtual void writeInformation(void) override;
			virtual pixel_t getPixel(size3_t pos) override;
			virtual data_t getSubPixel(size4_t pos) override;
			virtual DIMInterface<T>& setPixel(size3_t pos, pixel_t _pix) override;
			virtual DIMInterface<T>& setSubPixel(size4_t pos, data_t p) override;
			virtual std::vector<pixel_t> getSubImage(size3_t begin, size3_t size) override;
		protected:
			std::fstream dimFile;
			std::fstream imaFile;
	};

}

#endif // IMAGE_INCLUDE_DIM_INTERFACE_HPP_
