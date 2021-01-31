#ifndef IMAGE_INCLUDE_DISK_INTERFACE_HPP_
#define IMAGE_INCLUDE_DISK_INTERFACE_HPP_

#include "./data_interface.hpp"

namespace IO {

	// Fwd declaration of the memory interface type :
	template <typename T> class MemoryInterface;

	template <typename T>
	class DiskInterface : public DataInterface<T> {
		protected:
			DiskInterface(void);
		public:
			~DiskInterface(void);
		public:
			const DataInterface_ptr_t toMemoryInterface();
		protected:
	};

}

#endif // IMAGE_INCLUDE_DISK_INTERFACE_HPP_
