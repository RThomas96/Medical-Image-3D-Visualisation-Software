#ifndef IMAGE_INCLUDE_MEMORY_INTERFACE_HPP_
#define IMAGE_INCLUDE_MEMORY_INTERFACE_HPP_

#include "./data_interface.hpp"

#include <variant>

namespace IO {
	template <typename T>
	class MemoryInterface : public DataInterface<T> {
		public:
			MemoryInterface(void);
			~MemoryInterface(void);
		public:
			///
		protected:
			std::vector<T> values;
	};
}

#endif // IMAGE_INCLUDE_MEMORY_INTERFACE_HPP_
