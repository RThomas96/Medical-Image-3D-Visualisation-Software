#ifndef VISUALIZATION_IMAGE_INCLUDE_INTERPOLATOR_HPP_
#define VISUALIZATION_IMAGE_INCLUDE_INTERPOLATOR_HPP_

#include "../../macros.hpp"

#include <vector>
#include <map>

namespace Interpolators {

	template <typename DataType>
	struct genericInterpolator {
		public:
			using data_t = DataType;
		protected:
			genericInterpolator(void) = default;
			~genericInterpolator(void) = default;	///< Default destructor
		public:
			virtual data_t operator()(std::size_t neighborHoodSize, std::vector<DataType>& truth) const;
	};

	template <typename DataType>
	struct nearestNeighboor : public genericInterpolator<DataType> {
		public:
			using data_t = DataType;
		public:
			nearestNeighboor(void) = default;	///< Default constructor
			~nearestNeighboor(void) = default;	///< Default destructor
		public:
			virtual data_t operator()(std::size_t nbsize, std::vector<data_t>& data) const override {
				// take value at center, or as close as possible to the center
				std::size_t half = nbsize/std::size_t(2);
				std::size_t idx = half + half * nbsize + half * nbsize*nbsize;
				return data[idx];
			}
	};

	template <typename DataType>
	struct meanValue : public genericInterpolator<DataType> {
		public:
			using data_t = DataType;
		public:
			meanValue(void) = default;	///< Default constructor
			~meanValue(void) = default;	///< Default destructor
		public:
			virtual data_t operator()(std::size_t nbsize, std::vector<data_t>& data) const override {
				// take value at center, or as close as possible to the center
				double factor = 1. / static_cast<double>(nbsize * nbsize * nbsize);
				double mean = .0f;
				for (const data_t& d : data) {
					mean += static_cast<double>(d) * factor;
				}
				return mean;
			}
	};

	template <typename DataType, restrict_integer_check<DataType> = nullptr>
	struct mostPresent : public genericInterpolator<DataType> {
		public:
			using data_t = DataType;
		public:
			mostPresent(void) = default;	///< Default constructor
			~mostPresent(void) = default;	///< Default destructor
		public:
			virtual data_t operator()(std::size_t nbsize, std::vector<data_t>& data) const override {
				// map keys (values) to occurences in data :
				std::map<data_t, std::size_t> map;
				for (data_t& d : data) {
					// have we inserted the data beforehand ?
					auto pos = map.find(d);
					// if not, add it with count = 1
					if (pos == map.end()) { map.insert(std::make_pair<data_t, std::size_t>(std::move(d), 1)); }
					// otherwise, increase occurences
					else { map[d] = map[d]+1; }
				}
				std::size_t max = 0;		// in case the symbols all appear the same, it matters not
				data_t val_max = data[0];	// which we take for val_max, so best to take data[0]
				for (const auto& [key, val] : map) {
					// find max occurences and max val, to return it
					if (val > max) { max = val; val_max = key; }
				}
				return val_max;
			}
	};

	template <typename DataType>
	struct min : public genericInterpolator<DataType> {
		public:
			using data_t = DataType;
		public:
			min(void) = default;	///< Default constructor
			~min(void) = default;	///< Default destructor
		public:
			virtual data_t operator()(std::size_t nbsize, std::vector<data_t>& data) const override {
				data_t min = std::numeric_limits<data_t>::max();
				std::for_each(data.begin(), data.end(), [&min](data_t d) { min = std::min(min, d); });
				return min;
			}
	};

	template <typename DataType>
	struct max : public genericInterpolator<DataType> {
		public:
			using data_t = DataType;
		public:
			max(void) = default;	///< Default constructor
			~max(void) = default;	///< Default destructor
		public:
			virtual data_t operator()(std::size_t nbsize, std::vector<data_t>& data) const override {
				data_t max = std::numeric_limits<data_t>::lowest();
				std::for_each(data.begin(), data.end(), [&max](data_t d) { max = std::max(max, d); });
				return max;
			}
	};

}

#endif // VISUALIZATION_IMAGE_INCLUDE_INTERPOLATOR_HPP_
