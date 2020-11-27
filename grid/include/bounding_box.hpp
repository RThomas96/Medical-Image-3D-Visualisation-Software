#ifndef GRID_INCLUDE_BOUNDING_BOX_HPP_
#define GRID_INCLUDE_BOUNDING_BOX_HPP_

#include "../../macros.hpp"
#include "../../features.hpp"

#include <glm/glm.hpp>
#include <iostream>
#include <vector>
#include <algorithm>
#include <type_traits>

/// @brief Simple representation of an Axis Aligned Bounding Box.
/// @details This class has two template parameters : the first one, DataType is the type of the element composing the
/// points of the bounding box. This is the parameter that will define the type of glm::vec3 used in this class, and
/// thus provide the precision to which a point is defined. The second template parameter is a compile-time check which
/// ensures the DataType passed to the template to be a floating point type only (float, double, long double, and some
/// variants of float such as fp16, fp32 if those are enabled by the compiler).
template <typename DataType, restrict_floating_point_check<DataType> = nullptr>
class BoundingBox_General {

	public:
		/// @brief Defines a vector of the type defined by the instanced template.
		typedef glm::vec<3, DataType, glm::defaultp> vec;
		using data_t = DataType;

	public:
		/// @brief Initializes a bounding box at the origin, of size 0.
		BoundingBox_General(void) {
			DataType min = std::numeric_limits<DataType>::lowest();
			DataType max = std::numeric_limits<DataType>::max();
			this->min = vec(max, max, max);
			this->max = vec(min, min, min);
		}

		/// @brief Creates a bounding box around the coordinates given in argument
		BoundingBox_General(vec _min, vec _max) : BoundingBox_General() {
			this->setMin(_min);
			this->setMax(_max);
		}

		/// @brief Destroys the bounding box.
		~BoundingBox_General(void) {}

		BoundingBox_General(const BoundingBox_General& bb) {
			this->min = bb.min;
			this->max = bb.max;
		}

		BoundingBox_General(BoundingBox_General&& bb) {
			this->min = bb.min;
			this->max = bb.max;
		}

		/// @brief Assignment operator, copying values from the bounding box given
		BoundingBox_General& operator= (const BoundingBox_General& bb) {
			this->min = bb.min;
			this->max = bb.max;
			return *this;
		}

		/// @brief Assignment operator, copying values from the bounding box given
		BoundingBox_General& operator= (BoundingBox_General&& bb) {
			this->min = bb.min;
			this->max = bb.max;
			return *this;
		}

		/// @brief Sets the new minimum point of the bounding box.
		/// @details Does not do __any__ checks of the validity of the
		/// bounding box after this operation. If you provide a min point
		/// bigger than the max point, it is up to you to fix it.
		__attribute__((flatten)) BoundingBox_General& setMin(vec point) {
			this->min = point;
			return *this;
		}

		/// @brief Sets the new maximum point of the bounding box.
		/// @details Does not do __any__ checks of the validity of the
		/// bounding box after this operation. If you provide a max point
		/// smaller than the min point, it is up to you to fix it.
		BoundingBox_General& setMax(vec point) {
			this->max = point;
			return *this;
		}

		/// @brief Returns the positions of the 8 corners of this bounding box.
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
#ifdef GLM_MAT_BEFORE_VEC
				pos = transform * pos;
#else
				pos = pos * transform;
#endif
				v.x = static_cast<DataType>(pos.x);
				v.y = static_cast<DataType>(pos.y);
				v.z = static_cast<DataType>(pos.z);
				newbb.addPoint(v);
			});
			return newbb;
		}

		__attribute__((always_inline)) bool contains(vec point) const {
			return point.x > this->min.x && point.x < this->max.x &&
				point.y > this->min.y && point.y < this->max.y &&
				point.z > this->min.z && point.z < this->max.z;
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
			return glm::abs(this->max - this->min);
		}

		/// @brief Sets the X coordinate of the min point to the value given.
		__attribute__((always_inline)) BoundingBox_General& setMinX(double _d) {
			DataType d = static_cast<DataType>(_d);
			this->min.x = std::min(this->min.x, d);
			return *this;
		}

		/// @brief Sets the Y coordinate of the min point to the value given.
		__attribute__((always_inline)) BoundingBox_General& setMinY(double _d) {
			DataType d = static_cast<DataType>(_d);
			this->min.y = std::min(this->min.y, d);
			return *this;
		}

		/// @brief Sets the Z coordinate of the min point to the value given.
		__attribute__((always_inline)) BoundingBox_General& setMinZ(double _d) {
			DataType d = static_cast<DataType>(_d);
			this->min.z = std::min(this->min.z, d);
			return *this;
		}

		/// @brief Sets the X coordinate of the max point to the value given.
		__attribute__((always_inline)) BoundingBox_General& setMaxX(double _d) {
			DataType d = static_cast<DataType>(_d);
			this->max.x = std::max(this->max.x, d);
			return *this;
		}

		/// @brief Sets the Y coordinate of the max point to the value given.
		__attribute__((always_inline)) BoundingBox_General& setMaxY(double _d) {
			DataType d = static_cast<DataType>(_d);
			this->max.y = std::max(this->max.y, d);
			return *this;
		}

		/// @brief Sets the Z coordinate of the max point to the value given.
		__attribute__((always_inline)) BoundingBox_General& setMaxZ(double _d) {
			DataType d = static_cast<DataType>(_d);
			this->max.z = std::max(this->max.z, d);
			return *this;
		}

		/// @brief Sets the X coordinate of the min point to the value given.
		__attribute__((always_inline)) BoundingBox_General& setMinX_forced(double _d) {
			DataType d = static_cast<DataType>(_d);
			this->min.x = d;
			return *this;
		}

		/// @brief Sets the Y coordinate of the min point to the value given.
		__attribute__((always_inline)) BoundingBox_General& setMinY_forced(double _d) {
			DataType d = static_cast<DataType>(_d);
			this->min.y = d;
			return *this;
		}

		/// @brief Sets the Z coordinate of the min point to the value given.
		__attribute__((always_inline)) BoundingBox_General& setMinZ_forced(double _d) {
			DataType d = static_cast<DataType>(_d);
			this->min.z = d;
			return *this;
		}

		/// @brief Sets the X coordinate of the max point to the value given.
		__attribute__((always_inline)) BoundingBox_General& setMaxX_forced(double _d) {
			DataType d = static_cast<DataType>(_d);
			this->max.x = d;
			return *this;
		}

		/// @brief Sets the Y coordinate of the max point to the value given.
		__attribute__((always_inline)) BoundingBox_General& setMaxY_forced(double _d) {
			DataType d = static_cast<DataType>(_d);
			this->max.y = d;
			return *this;
		}

		/// @brief Sets the Z coordinate of the max point to the value given.
		__attribute__((always_inline)) BoundingBox_General& setMaxZ_forced(double _d) {
			DataType d = static_cast<DataType>(_d);
			this->max.z = d;
			return *this;
		}

		__attribute__((flatten)) void printInfo(std::string message, std::string prefix = "") const {
			if (message.length() != 0) {
				std::cerr << prefix << " | " << message << '\n';
			}
			std::cerr << prefix << '\t' << '[' << this->min.x << ',' << this->min.y << ',' << this->min.z << ']' << '\n';
			std::cerr << prefix << '\t' << '[' << this->max.x << ',' << this->max.y << ',' << this->max.z << ']' << '\n';
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
