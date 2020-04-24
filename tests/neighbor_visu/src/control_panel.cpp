#include "../include/control_panel.hpp"

#include <exception>

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>

ControlPanel::ControlPanel(QWidget* parent, DoubleViewerWidget* viewer) : QWidget(parent) {
	QVBoxLayout* xCoordSection = new QVBoxLayout();
	QVBoxLayout* yCoordSection = new QVBoxLayout();
	QVBoxLayout* zCoordSection = new QVBoxLayout();
	QHBoxLayout* spinBoxesLayout = new QHBoxLayout();

	this->xCoordSpinBox = new QSpinBox();
	this->yCoordSpinBox = new QSpinBox();
	this->zCoordSpinBox = new QSpinBox();
	this->updateCoordsButton = new QPushButton("Update coordinates");

	QLabel* xLabel = new QLabel("X Coordinate");
	QLabel* yLabel = new QLabel("Y Coordinate");
	QLabel* zLabel = new QLabel("Z Coordinate");

	xCoordSection->addWidget(xLabel);
	xCoordSection->addWidget(this->xCoordSpinBox);
	yCoordSection->addWidget(yLabel);
	yCoordSection->addWidget(this->yCoordSpinBox);
	zCoordSection->addWidget(zLabel);
	zCoordSection->addWidget(this->zCoordSpinBox);

	spinBoxesLayout->addLayout(xCoordSection);
	spinBoxesLayout->addLayout(yCoordSection);
	spinBoxesLayout->addLayout(zCoordSection);
	spinBoxesLayout->addWidget(this->updateCoordsButton);

	this->setLayout(spinBoxesLayout);

	this->initSignals(viewer);
}

void ControlPanel::initSignals(DoubleViewerWidget *viewer) {
	if (viewer == nullptr) {
#ifndef NDEBUG
		throw std::runtime_error("Could not assign a connection to a null value for viewer !");
		std::terminate();
#else
		return; // Silently ignore the error
		// TODO : Handle this more gracefully, without requiring an exception.
#endif
	}
	connect(this->xCoordSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), viewer, &DoubleViewerWidget::controlPanelXCoordChanged);
	connect(this->yCoordSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), viewer, &DoubleViewerWidget::controlPanelYCoordChanged);
	connect(this->zCoordSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), viewer, &DoubleViewerWidget::controlPanelZCoordChanged);
	connect(this->updateCoordsButton, &QPushButton::pressed, viewer, &DoubleViewerWidget::requestPositionUpdate);
}
