#ifndef VISUALIZATION_IMAGE_API_GRID_HPP_
#define VISUALIZATION_IMAGE_API_GRID_HPP_

#include "image_api_common.hpp"
#include "backend.hpp"

#include <memory>
#include <string>
#include <vector>

namespace Image {

	class Grid {
		public:
			/// @b Handy typedef for a pointer to a grid
			typedef std::shared_ptr<Grid> ptr;
		protected:
			/// @b Default grid ctor, made protected so it is not instanciated directly.
			Grid(ImageBackendImpl::ptr _backend);
		public:
			virtual ~Grid(void) = default;

		protected:
			/// @b The opaque pointer which will perform all the logic in regards to the reading of data.
			ImageBackendImpl::ptr pImageImpl;
	};

}

#endif // VISUALIZATION_IMAGE_API_GRID_HPP_
