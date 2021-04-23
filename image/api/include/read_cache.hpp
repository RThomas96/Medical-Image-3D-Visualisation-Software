#ifndef VISUALIZATION_IMAGE_API_INCLUDE_READ_CACHE_HPP_
#define VISUALIZATION_IMAGE_API_INCLUDE_READ_CACHE_HPP_

#include <memory>
#include <vector>

namespace Image {

	/// @brief Very simple read cache which supports arbitrary indexes and data arrays.
	/// @details Allows to very simple cache read information of type 'cache_data' from a source with unique indexes of
	/// type 'cache_idx', in order to retrieve them later.
	template <typename cache_idx, typename cache_data>
	struct ReadCache {
		public:
			/// @brief Type alias to the internal index representation
			using index_t = cache_idx;

			/// @brief Type alias to the internal data representation
			using data_t_ptr = std::shared_ptr<cache_data>;
		protected:
			/// @brief The internal structuring of data in the cache vector
			using cached_data_t = std::pair<index_t, data_t_ptr>;

		public:
			/// @brief Default ctor. Allocates just enough memory for the empty struct.
			ReadCache(void) : m_data(0), lastInsertedElement(0) {}

			/// @brief Default dtor. Deallocates any elements
			~ReadCache(void) { this->clearCache(); }

			/// @brief Returns true if the cache has the data named referenced by Index 'x'
			bool hasData(const index_t searched) const {
				// For this, we don't need to conform to the lastInsertedElement index.
				// Just check we have the data requested :
				for (std::size_t i = 0; i < this->m_data.size(); ++i) {
					if (this->m_data[i].first == searched) { return true; }
				}
				return false;
			}

			/// @brief Returns a reference to the data at Index 'i'
			data_t_ptr getData(const index_t searched) const {
				// Check if we have the data, and if we do return it immediately :
				for (std::size_t i = 0; i < this->m_data.size(); ++i) {
					if (this->m_data[i].first == searched) { return this->m_data[i].second; }
				}
				// Otherwise, return a nullptr :
				return nullptr;
			}

			std::size_t findIndex(const index_t searched) {
				// Check if we have the data, and if we do return it immediately :
				for (std::size_t i = 0; i < this->m_data.size(); ++i) {
					if (this->m_data[i].first == searched) { return i; }
				}
				// Otherwise, return a nullptr :
				return this->maxCachedElements;
			}

			data_t_ptr getDataIndexed(const std::size_t idx) {
				// Check if we have the data, and if we do return it immediately :
				if (idx >= this->maxCachedElements || idx >= this->m_data.size()) { return nullptr; }
				// Otherwise, return a nullptr :
				return this->m_data[idx].second;
			}

			/// @brief Loads the data into the cache, cleearing up a space if necessary.
			std::size_t loadData(const index_t index, data_t_ptr& data) {
				// If we already have filled the vector, wrap around with the help of lastInsertedElement :
				if (this->m_data.size() == this->maxCachedElements) {
					// Might need to wrap around :
					if (this->lastInsertedElement == this->maxCachedElements-1) { this->lastInsertedElement = 0; }
					else { this->lastInsertedElement++; }
					// Remove the element in the place of lastInsertedElement, and replace it with the new data :
					this->m_data[this->lastInsertedElement].first = index;
					this->m_data[this->lastInsertedElement].second.swap(data);
				} else {
					// Otherwise, just call emplace_back() to add to the vector :
					this->lastInsertedElement = this->m_data.size();
					this->m_data.emplace_back(index, data);
				}
				return this->lastInsertedElement;
			}

			/// @brief Clears the cache manually.
			void clearCache(void) {
				// Reset the shared_ptrs so they can be deleted later (once they're all freed) :
				for (std::pair<index_t, data_t_ptr>& cached : this->m_data) { cached.second.reset(); }
				// Clear the vector :
				this->m_data.clear();
			}

		protected:
			///  @brief The maximum number of elements we can have stored at any time during the cache's lifetime
			constexpr static std::size_t maxCachedElements = 16;

			/// @brief The actual cached data.
			std::vector<cached_data_t> m_data;

			/// @brief The position of the last inserted element in the vector of data.
			std::size_t lastInsertedElement;
	};
}

#endif // VISUALIZATION_IMAGE_API_INCLUDE_READ_CACHE_HPP_
