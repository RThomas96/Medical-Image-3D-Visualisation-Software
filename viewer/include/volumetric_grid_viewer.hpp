#ifndef VISUALIZATION_VIEWER_INCLUDE_VOLUMETRIC_GRID_VIEWER_HPP_
#define VISUALIZATION_VIEWER_INCLUDE_VOLUMETRIC_GRID_VIEWER_HPP_

#include "../../image/api/include/backend.hpp"
#include "../../image/api/include/grid.hpp"

#include <QOpenGLFunctions_3_2_Core>
#include <memory>
#include <vector>

/// @b This class is a proxy object, enabling to display a grid in real-time.
class VolumetricGridViewer {
		Q_PROPERTY(bool hidden READ hidden WRITE hide NOTIFY visibilityChanged)
	public:
		VolumetricGridViewer(void) = delete;
		~VolumetricGridViewer(void);

		/// @b Initializes the GL data that must be used to show the grid.
		void initializeGLData();

		/// @b Draw the grid.
		/// @warning Assumes a GL context is current, valid, and bound.
		void draw();

	public:

	protected:
		/// @b The grid we are displaying
		Image::Grid::Ptr grid;
		/// @b Is this grid supposed to be shown or not ?
		bool hidden;
		/// @b Texture handle for the grid image
};

#endif // VISUALIZATION_VIEWER_INCLUDE_VOLUMETRIC_GRID_VIEWER_HPP_
