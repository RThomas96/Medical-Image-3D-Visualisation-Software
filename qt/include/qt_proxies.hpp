#ifndef QT_INCLUDE_QT_PROXIES_HPP_
#define QT_INCLUDE_QT_PROXIES_HPP_

#include "../../features.hpp"
#include "../../macros.hpp"

//#include "../../grid/include/bounding_box.hpp"
#include "../../grid/include/discrete_grid.hpp"

#include <QWidget>

#include <variant>

/// @brief Namespace used to define some Qt-based proxies for regular classes.
namespace Proxies {

	/// @ingroup qtwidgets
	/// @brief Creates a proxy to a bounding box, in order to allow control through Qt's signal/slot mechanism.
	/// @note Each instance pf a BoundingBox_Proxy will only control one bounding box.
	/// @warning Legacy code.
	class BoundingBox : public QWidget {
		Q_OBJECT
	private:
		/// @brief Alias for the quite verbose 'BoundingBox_General' name.
		template <typename precision_t>
		using bbox_t = BoundingBox_General<precision_t>;

	public:
		/// @brief Basic constructor, creates a proxy for a specific bounding box.
		template <typename T>
		BoundingBox(const std::shared_ptr<DiscreteGrid>& _grid, bbox_t<T>* box) :
			boundingBox(box), grid(_grid) {}
		/// @brief Copy operator from another bounding box proxy
		BoundingBox(const BoundingBox& other) = delete;
		BoundingBox(BoundingBox&& other)	  = delete;
		BoundingBox& operator=(const BoundingBox& other) = delete;
		BoundingBox& operator=(BoundingBox&& other) = delete;
		/// @brief Destroys the proxy.
		~BoundingBox(void);
	public slots:
		/// @brief Allows to set the minimum value of the bounding box on X
		void setMinX(double newVal);
		/// @brief Allows to set the minimum value of the bounding box on Y
		void setMinY(double newVal);
		/// @brief Allows to set the minimum value of the bounding box on Z
		void setMinZ(double newVal);
		/// @brief Allows to set the maximum value of the bounding box on X
		void setMaxX(double newVal);
		/// @brief Allows to set the maximum value of the bounding box on Y
		void setMaxY(double newVal);
		/// @brief Allows to set the maximum value of the bounding box on Z
		void setMaxZ(double newVal);

	protected:
		/// @brief A variant for all types of bounding box allowed.
		std::variant<bbox_t<float>*, bbox_t<double>*, bbox_t<long double>*> boundingBox;
		/// @brief Grid associated with this
		std::shared_ptr<DiscreteGrid> grid;
	};

	/// @ingroup qtwidgets
	/// @brief Serves as a proxy for setting and modifying a glm::vec3 with Qt's signal/slot mechanism.
	/// @warning Legacy code.
	class Resolution : public QWidget {
		Q_OBJECT
	private:
		using sizevec3 = typename DiscreteGrid::sizevec3;
		using data_t   = typename DiscreteGrid::sizevec3::value_type;

	public:
		Resolution(const std::shared_ptr<DiscreteGrid>& _grid, sizevec3* _res) :
			vec(_res), grid(_grid) {}
		Resolution(const Resolution& other) = delete;
		Resolution(Resolution&& other)		= delete;
		Resolution& operator=(const Resolution& other) = delete;
		Resolution& operator=(Resolution&& other) = delete;
		~Resolution(void);
	public slots:
		/// @brief Sets the grid's resolution on X.
		void setResolutionX(int newVal);
		/// @brief Sets the grid's resolution on Y.
		void setResolutionY(int newVal);
		/// @brief Sets the grid's resolution on Z.
		void setResolutionZ(int newVal);

	protected:
		/// @brief Pointer to the vector to modify.
		sizevec3* vec;
		/// @brief The grid to control
		const std::shared_ptr<DiscreteGrid> grid;
	};

}	 // namespace Proxies

#endif	  // QT_INCLUDE_QT_PROXIES_HPP_
