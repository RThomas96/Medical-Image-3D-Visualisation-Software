#ifndef GRID_INCLUDE_BOUNDING_BOX_HPP_
#define GRID_INCLUDE_BOUNDING_BOX_HPP_

#include <glm/glm.hpp>
#include <iostream>
#include <vector>
#include <algorithm>

/// @brief Simple representation of an Axis Aligned Bounding Box
template <typename DataType> class BoundingBox_General {

	public:
		/// @brief Defines a vector of the type defined by the instanced template.
		typedef glm::vec<3, DataType, glm::defaultp> vec;

	public:
		/// @brief Initializes a bounding box at the origin, of size 0.
		BoundingBox_General(void) {
			DataType min = std::numeric_limits<DataType>::min();
			DataType max = std::numeric_limits<DataType>::max();
			this->min = vec(max, max, max);
			this->max = vec(min, min, min);
		}

		/// @brief Creates a bounding box around the coordinates given in argument
		BoundingBox_General(vec _min, vec _max) : BoundingBox_General() {
			this->setMinX(_min.x).setMinY(_min.y).setMinZ(_min.z);
			this->setMaxX(_max.x).setMaxY(_max.y).setMaxZ(_max.z);
		}

		/// @brief Destroys the bounding box.
		~BoundingBox_General(void) {}

		/// @brief Sets the new minimum point of the bounding box.
		/// @details If the vector is larger than the max point, this
		/// function call reverses the order of boundaries.
		__attribute__((flatten)) BoundingBox_General& setMin(vec point) {
			this->setMinX(point.x).setMinY(point.y).setMinZ(point.z);
			return *this;
		}

		/// @brief Sets the new maximum point of the bounding box.
		/// @details If the vector is small than the min point, this
		/// function call reverses the order of boundaries.
		BoundingBox_General& setMax(vec point) {
			this->setMaxX(point.x).setMaxY(point.y).setMaxZ(point.z);
			return *this;
		}

		std::vector<vec> getAllCorners(void) const {
			std::vector<vec> corners;
			corners.push_back(vec(this->min.x, this->min.y, this->min.z));
			corners.push_back(vec(this->min.x, this->min.y, this->max.z));
			corners.push_back(vec(this->min.x, this->max.y, this->min.z));
			corners.push_back(vec(this->min.x, this->max.y, this->max.z));
			corners.push_back(vec(this->max.x, this->min.y, this->min.z));
			corners.push_back(vec(this->max.x, this->min.y, this->max.z));
			corners.push_back(vec(this->max.x, this->max.y, this->min.z));
			corners.push_back(vec(this->max.x, this->max.y, this->max.z));
			return corners;
		}

		__attribute__((flatten)) BoundingBox_General& addPoint(vec point) {
			this->setMinX(point.x).setMinY(point.y).setMinZ(point.z);
			this->setMaxX(point.x).setMaxY(point.y).setMaxZ(point.z);
			return *this;
		}

		BoundingBox_General& addPoints(const std::vector<vec>& points) {
			for (const vec& p : points) {
				this->addPoint(p);
			}
			return *this;
		}

		BoundingBox_General transformTo(const glm::mat4 transform) const {
			// Transform the bounding box to another space
			std::vector<vec> corners = this->getAllCorners();
			// Create new BB englobing this space :
			BoundingBox_General<DataType> newbb;
			// For each element, convert to another space :
			std::for_each(corners.begin(), corners.end(), [&](vec &v){
				glm::vec4 pos = glm::vec4(static_cast<float>(v.x), static_cast<float>(v.y), static_cast<float>(v.z), 1.);
				pos = transform * pos;
				v.x = static_cast<DataType>(pos.x);
				v.y = static_cast<DataType>(pos.y);
				v.z = static_cast<DataType>(pos.z);
				newbb.addPoint(v);
			});
			return newbb;
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

		/// @brief Sets the X coordinate of the min point to the value given.
		BoundingBox_General& setMinX(double _d) {
			DataType d = static_cast<DataType>(_d);
			this->min.x = std::min(this->min.x, d);
			return *this;
		}

		/// @brief Sets the Y coordinate of the min point to the value given.
		BoundingBox_General& setMinY(double _d) {
			DataType d = static_cast<DataType>(_d);
			this->min.y = std::min(this->min.y, d);
			return *this;
		}

		/// @brief Sets the Z coordinate of the min point to the value given.
		BoundingBox_General& setMinZ(double _d) {
			DataType d = static_cast<DataType>(_d);
			this->min.z = std::min(this->min.z, d);
			return *this;
		}

		/// @brief Sets the X coordinate of the max point to the value given.
		BoundingBox_General& setMaxX(double _d) {
			DataType d = static_cast<DataType>(_d);
			this->max.x = std::max(this->max.x, d);
			return *this;
		}

		/// @brief Sets the Y coordinate of the max point to the value given.
		BoundingBox_General& setMaxY(double _d) {
			DataType d = static_cast<DataType>(_d);
			this->max.y = std::max(this->max.y, d);
			return *this;
		}

		/// @brief Sets the Z coordinate of the max point to the value given.
		BoundingBox_General& setMaxZ(double _d) {
			DataType d = static_cast<DataType>(_d);
			this->max.z = std::max(this->max.z, d);
			return *this;
		}

		void printInfo(std::string message) const {
			if (message.length() != 0) {
				std::cerr << message << '\n';
			}
			std::cerr << '\t' << '[' << this->min.x << ',' << this->min.y << ',' << this->min.z << ']' << '\n';
			std::cerr << '\t' << '[' << this->max.x << ',' << this->max.y << ',' << this->max.z << ']' << '\n';
		}

	protected:
		/// @brief Set the boundaries of the bounding box
		__attribute__((flatten)) void setBoundaries(vec _min, vec _max) {
			this->setMinX(_min.x).setMinY(_min.y).setMinZ(_min.z);
			this->setMaxX(_max.x).setMaxY(_max.y).setMaxZ(_max.z);
		}

	private:

		/// @brief the lower point of the BB
		vec min;

		/// @brief higher point of the BB
		vec max;
};

#endif // GRID_INCLUDE_BOUNDING_BOX_HPP_
