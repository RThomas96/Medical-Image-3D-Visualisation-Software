#ifndef QT_INCLUDE_LOADER_WIDGET_HPP_
#define QT_INCLUDE_LOADER_WIDGET_HPP_

#include "../../macros.hpp"
#include "../../viewer/include/scene.hpp"
#include "../../image/include/reader.hpp"
#include "../../grid/include/discrete_grid.hpp"
#include "../../grid/include/input_discrete_grid.hpp"
#include "../../viewer/include/neighbor_visu_viewer.hpp"
#include "./scene_control.hpp"

#include <QDir>
#include <QLabel>
#include <QFrame>
#include <QWidget>
#include <QComboBox>
#include <QGroupBox>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QRadioButton>
#include <QDoubleSpinBox>
#include <QProgressBar>

class GridLoaderWidget : public QWidget {
		Q_OBJECT
	public:
		/// @brief Default constructor. Builds the widget's layout and sets up signals
		GridLoaderWidget(Scene* _scene, Viewer* _viewer, ControlPanel* _cp, QWidget* parent = nullptr);
		/// @brief Default destructor
		~GridLoaderWidget(void);
		/// @brief Sets up the different widgets
		void setupWidgets();
		/// @brief Sets up the layouts containing the widgets
		void setupLayouts();
		/// @brief Sets up the different connections between widgets
		void setupSignals();
		/// @brief Resets the grid information label
		void resetGridInfoLabel();
		/// @brief Computes the grid information based on the new data available
		void computeGridInfoLabel();
		void disableWidgets();
	public slots:
		void loadGridDIM1channel();
		void loadGridTIF1channel();
		void loadGridDIM2channel();
		void loadGridTIF2channel();
		void loadGrid();
	protected:
		QDir basePath;				///< Last path opened, or $HOME
		Scene* scene;				///< The scene to control/add grids to.
		Viewer* viewer;				///< The viewer to call when uploading the grid.
		ControlPanel* _cp;			///< The control panel to update
		std::shared_ptr<IO::GenericGridReader> readerR;	///< The pointer to a generic grid reader for R channel
		std::shared_ptr<IO::GenericGridReader> readerG;	///< The pointer to a generic grid reader for G channel
		std::shared_ptr<InputGrid> inputGridR;		///< The pointer to an input grid for R channel
		std::shared_ptr<InputGrid> inputGridG;		///< The pointer to an input grid for G channel
		IO::DownsamplingLevel dsLevel;		///< Currently selected downsampling method
		std::shared_ptr<Interpolators::genericInterpolator<DiscreteGrid::data_t>> interpolator; ///< interpolator

		QLabel* label_headerLoader;		///< Label header for the entire widget
		QLabel* label_load1channel;		///< Label before the 1-channel loading buttons
		QLabel* label_load2channel;		///< Label before the 2-channel loading buttons
		QLabel* label_headerTransformation;	///< Label header for the transformation frame
		QLabel* label_transformationAngle;	///< Label for the capture angle in the transformation
		QLabel* label_transformationDimensions;	///< Label for the voxel dimensions in the transformation
		QLabel* label_gridInfoR;		///< Grid reader information, red channel
		QLabel* label_gridInfoG;		///< Grid reader information, green channel

		QPushButton* button_loadDIM_1channel;	///< Button to load DIM/IMA files with one channel.
		QPushButton* button_loadDIM_2channel;	///< Button to load DIM/IMA files with two channels.
		QPushButton* button_loadTIF_1channel;	///< Button to load TIF[F] files with one channel.
		QPushButton* button_loadTIF_2channel;	///< Button to load TIF[F] files with two channels.
		QPushButton* button_loadGrids;		///< Button to launch the grid loader.

		QDoubleSpinBox* dsb_transformationA;	///< Double spinbox for the angle of the capture
		QDoubleSpinBox* dsb_transformationDX;	///< Double spinbox for the voxel dimensions on X
		QDoubleSpinBox* dsb_transformationDY;	///< Double spinbox for the voxel dimensions on Y
		QDoubleSpinBox* dsb_transformationDZ;	///< Double spinbox for the voxel dimensions on Z

		QVBoxLayout* layout_mainLayout;		///< Main layout, comprising the loader(s) and transfo layouts
		QHBoxLayout* layout_load1channel;	///< Layout for the 1-channel load procedure.
		QHBoxLayout* layout_load2channel;	///< Layout for the 2-channel load procedure.
		QGridLayout* layout_transfoDetails;	///< Layout for the transformation details
		QHBoxLayout* layout_downsampling;	///< Layout for the downsampling group box
		QGridLayout* layout_interpolator;	///< Layout for the interpolator group box
		QGridLayout* layout_roiSelection;	///< Layout for the ROI selection

		QGroupBox* groupbox_userLimits;		///< Does the user have min/max bounds of the ROI for the image ?
		QSpinBox* spinbox_userLimitMin;		///< Minimum value to define the ROI of the image
		QSpinBox* spinbox_userLimitMax;		///< Maximum value to define the ROI of the image
		QLabel* label_roiMin;				///< Label for the min ROI selector
		QLabel* label_roiMax;				///< Label for the max ROI selector

		QFrame* frame_load1channel;		///< Frame surrounding the load '1-channel' widgets
		QFrame* frame_load2channel;		///< Frame surrounding the load '2-channel' widgets
		QFrame* frame_transfoDetails;		///< Frame surrounding the load transformation widgets

		QGroupBox* groupBox_downsampling;	///< Group box for the downsample options when loading a grid
		QGroupBox* groupBox_interpolator;	///< Group box for the interpolator to choose when downsampling

		QRadioButton* radioButton_original;	///< Downsampling option : original resolution
		QRadioButton* radioButton_low;		///< Downsampling option : low resolution
		QRadioButton* radioButton_lower;	///< Downsampling option : lower resolution
		QRadioButton* radioButton_lowest;	///< Downsampling option : lowest resolution
		QRadioButton* radioButton_nn;		///< Interpolator option : nearest neighbor
		QRadioButton* radioButton_mean;		///< Interpolator option : mean value of labels within subpixel
		QRadioButton* radioButton_mp;		///< Interpolator option : most present label within subpixel
		QRadioButton* radioButton_min;		///< Interpolator option : minimum label within subpixel
		QRadioButton* radioButton_max;		///< Interpolator option : maximum label within subpixel

		QProgressBar* progress_load;
};

#endif // QT_INCLUDE_LOADER_WIDGET_HPP_
