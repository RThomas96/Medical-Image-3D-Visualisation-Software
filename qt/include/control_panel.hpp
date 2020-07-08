#ifndef QT_INCLUDE_CONTROL_PANEL_HPP_
#define QT_INCLUDE_CONTROL_PANEL_HPP_

#include <QWidget>
#include <QSpinBox>
#include <QPushButton>
#include <QCheckBox>

class DoubleViewerWidget; // Forward declaration

// TODO : add slots for when the image loader changes source files, to get a bigger image, to change the spinboxes min and max values

/*!
 * \brief The ControlPanel class allows to control a DoubleViewer instance with three spin boxes and a button.
 * \details By using 3 spin boxes (one for each direction) and a button, we can control the neighbor search displayed
 * in the double viewer beside it.
 */
class ControlPanel : public QWidget {
	public:
		//! @brief Default constructor for the control panel, allowing to link the signals to the correct viewer.
		ControlPanel(QWidget* parent = nullptr, DoubleViewerWidget* viewer = nullptr);

	protected:
		//! @brief SpinBox controlling the X coordinate asked by the user
		QSpinBox* xCoordSpinBox;

		//! @brief SpinBox controlling the Y coordinate asked by the user
		QSpinBox* yCoordSpinBox;

		//! @brief SpinBox controlling the Z coordinate asked by the user
		QSpinBox* zCoordSpinBox;

		//! @brief Update button, to update the neighbor search in the viewer
		QPushButton* updateCoordsButton;

		//! @brief Checkbox to show the texture cube or not
		QCheckBox* showTexCubeCheckBox;

	private:
		//! @brief Initializes the signal connections for the control panel.
		void initSignals(DoubleViewerWidget* viewer);

	private slots:
		//! @brief Triggered when the spinbox controlling the X position is modified.
		void xCoordChangedPassthrough(int x);

		//! @brief Triggered when the spinbox controlling the Y position is modified.
		void yCoordChangedPassthrough(int y);

		//! @brief Triggered when the spinbox controlling the Z position is modified.
		void zCoordChangedPassthrough(int z);

		//! @brief Triggered when the 'Update' button is pressed
		void updateButtonPressedPassthrough();

	signals:
		//! @brief Emitted when the spinbox controlling the X position is modified.
		void xCoordChanged(int x);

		//! @brief Emitted when the spinbox controlling the Y position is modified.
		void yCoordChanged(int y);

		//! @brief Emitted when the spinbox controlling the Z position is modified.
		void zCoordChanged(int z);

		//! @brief Emitted once the 'Update' button is pressed
		void updateButtonPressed();
};

#endif // QT_INCLUDE_CONTROL_PANEL_HPP_
