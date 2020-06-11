#ifndef TESTS_NEIGHBOR_BISU_INCLUDE_BOUNDING_BOX_HPP_
#define TESTS_NEIGHBOR_BISU_INCLUDE_BOUNDING_BOX_HPP_

#include <glm/glm.hpp>
#include <iostream>

/// @brief Simple representation of an Axis Aligned Bounding Box
template <typename DataType> class BoundingBox_General {

	public:
		/// @brief Defines a vector of the type defined by the instanced template.
		typedef glm::vec<3, DataType, glm::defaultp> vec;

	public:
		/// @brief Initializes a bounding box at the origin, of size 0.
		BoundingBox_General(void) {
			this->min = vec(static_cast<DataType>(.0), static_cast<DataType>(.0), static_cast<DataType>(.0));
			this->max = vec(static_cast<DataType>(.0), static_cast<DataType>(.0), static_cast<DataType>(.0));
		}

		/// @brief Creates a bounding box around the coordinates given in argument
		BoundingBox_General(vec _min, vec _max) : BoundingBox_General() {
			this->setBoundaries(_min, _max);
		}

		/// @brief Destroys the bounding box.
		~BoundingBox_General(void) {}

		/// @brief Sets the new minimum point of the bounding box.
		/// @details If the vector is larger than the max point, this
		/// function call reverses the order of boundaries.
		BoundingBox_General& setMin(vec _min) {
			this->setBoundaries(_min, this->max);
			return *this;
		}

		/// @brief Sets the new maximum point of the bounding box.
		/// @details If the vector is small than the min point, this
		/// function call reverses the order of boundaries.
		BoundingBox_General& setMax(vec _max) {
			this->setBoundaries(this->min, _max);
			return *this;
		}

		/// @brief Get a read-only reference to the minimum point.
		const vec& getMin(void) const {
			return this->min;
		}

		/// @brief Get a read-only reference to the maximum point.
		const vec& getMax(void) const {
			return this->max;
		}

		/// @brief Get the coordinates of the diagonal of the BB, defined as (max-min).
		vec getDiagonal(void) const {
			return this->max - this->min;
		}

	protected:
		/// @brief Set the boundaries of the bounding box
		void setBoundaries(vec _min, vec _max) {
			bool switchPlaces = (glm::length(_min) > glm::length(_max));
			this->min = switchPlaces ? _max : _min;
			this->max = switchPlaces ? _min : _max;
		}

	private:

		/// @brief the lower point of the BB
		vec min;

		/// @brief higher point of the BB
		vec max;
};

#endif // TESTS_NEIGHBOR_BISU_INCLUDE_BOUNDING_BOX_HPP_
