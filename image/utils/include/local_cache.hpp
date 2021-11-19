#ifndef VISUALISATION_IMAGE_UTILS_INCLUDE_LOCAL_CACHE_HPP_
#define VISUALISATION_IMAGE_UTILS_INCLUDE_LOCAL_CACHE_HPP_

#include <algorithm>
#include <memory>
#include <vector>

namespace Image {

	/// @brief The LocalCache class is responsible for keeping locally-modified slices
	template <typename cache_idx, typename cache_data>
	struct LocalCache
	{
	public:
		/// @brief The index type of the slice (usually std::size_t)
		using index_t = cache_idx;
		using data_t = cache_data;
		using data_t_ptr = std::shared_ptr<data_t>;

	protected:
		using cached_data_t = std::pair<index_t, data_t_ptr>;
	public:
		LocalCache() = default;

		~LocalCache() { this->reset(); }

		/// @brief Checks if the local cache has the slice 'slice_idx' cached or not.
		/// @returns True if the slice has been cached, and false if not.
		bool hasSlice(index_t slice_idx) const {
			for (std::size_t i = 0; i < this->m_data.size(); ++i) {
				if (this->m_data[i].first == slice_idx) {
					return true;
				}
			}
			return false;
		}

		/// @brief Returns the locally-cached slice, or nullptr.
		data_t_ptr getSlice(const index_t searched) const {
			for (std::size_t i = 0; i < this->m_data.size(); ++i) {
				if (this->m_data[i].first == searched) {
					return this->m_data[i].second;
				}
			}
			return nullptr;
		}

		/// @brief Loads the given slice into cache.
		void loadSlice(const index_t index, data_t_ptr data_ptr) const {
			this->m_data.push_back(std::make_pair(index, data_ptr));
		}

		/// @brief Find the index 'index' in the loaded slices, and return the position in the data vector.
		std::size_t findIndex(const index_t index) const {
			for (std::size_t i = 0; i < this->m_data.size(); ++i) {
				if (this->m_data[i].first == index) {
					return i;
				}
			}
			return this->m_data.size();
		}

		/// @brief Returns the data at index 'index'
		data_t_ptr getDataIndexed(const std::size_t index) const {
			if (index != this->m_data.size()) { return this->m_data[index]; }
			return nullptr;
		}

		/// @brief Deletes all slices kept in cache.
		void reset() {
			this->m_data.clear();
		}

	protected:
		std::vector<cached_data_t> m_data;
	};

} // namespace Image

#endif // VISUALISATION_IMAGE_UTILS_INCLUDE_LOCAL_CACHE_HPP_
