#ifndef VISUALISATION_DIALOG_PICK_GRIDS_FROM_SCENE_H
#define VISUALISATION_DIALOG_PICK_GRIDS_FROM_SCENE_H

#include "../../meshes/base_mesh/Mesh.hpp"
#include "../../new_grid/include/grid.hpp"
#include "../../viewer/include/viewer_structs.hpp"

#include <QComboBox>
#include <QDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QLayout>
#include <QPushButton>
#include <QSpacerItem>
#include <QVBoxLayout>

/// @brief Allows to pick a grid from the scene, in order to 'pair' a mesh with it.
/// @warning Those names are horribly chosen. Pick better ones, or drop this class altogether.
class GridPickerFromScene : public QDialog {
public:
	GridPickerFromScene() { init(); }
	virtual ~GridPickerFromScene() = default;

protected:
	void init() {
		this->user_choice	= new QComboBox();
		this->button_cancel = new QPushButton("Cancel");
		this->button_accept = new QPushButton("OK");

		QHBoxLayout* hori  = new QHBoxLayout();
		QVBoxLayout* vert  = new QVBoxLayout();
		QLabel* user_label = new QLabel("What grid do you want to pair the mesh with ?");

		hori->addWidget(this->button_cancel);
		hori->addWidget(this->button_accept);
		vert->addWidget(user_label);
		vert->addWidget(this->user_choice);
		vert->addLayout(hori);

		this->setLayout(vert);
		this->setWindowTitle("Pair the mesh with a grid ?");
	}

public:
	void chooseGrids(std::vector<GridGLView::Ptr>& grids) {
		for (const auto& grid : grids) {
			this->user_choice->addItem(QString(grid->grid->getImageName().c_str()));
		}
		this->selected_grid = 0;
		this->pressed_ok	= false;
		QObject::connect(this->user_choice, QOverload<int>::of(&QComboBox::currentIndexChanged), [&](int i) {
			this->selected_grid = i;
		});
		QObject::connect(this->button_cancel, &QPushButton::pressed, this, [&]() {
			this->pressed_ok = false;
			this->close();
		});
		QObject::connect(this->button_accept, &QPushButton::pressed, this, [&]() {
			this->pressed_ok = true;
			this->close();
		});
		this->exec();
	}
	bool choice_Cancelled() { return pressed_ok == false; }
	bool choice_Accepted() { return pressed_ok == true; }
	int choice_getGrid() { return selected_grid; }

protected:
	QComboBox* user_choice;
	QPushButton* button_cancel;
	QPushButton* button_accept;
	int selected_grid;
	bool pressed_ok;
};

class MeshPickerFromScene : public QDialog {
public:
	MeshPickerFromScene() { init(); }
	virtual ~MeshPickerFromScene() = default;

protected:
	void init() {
		this->user_choice	= new QComboBox();
		this->button_cancel = new QPushButton("Cancel");
		this->button_accept = new QPushButton("OK");

		QHBoxLayout* hori  = new QHBoxLayout();
		QVBoxLayout* vert  = new QVBoxLayout();
		QLabel* user_label = new QLabel("What mesh do you want to pair the curve with a mesh ?");

		hori->addWidget(this->button_cancel);
		hori->addWidget(this->button_accept);
		vert->addWidget(user_label);
		vert->addWidget(this->user_choice);
		vert->addLayout(hori);

		this->setLayout(vert);
		this->setWindowTitle("Pair the curve with a mesh ?");
	}

public:
	void chooseMeshes(std::vector<Mesh::Ptr>& meshes) {
		for (std::size_t i = 0; i < meshes.size(); ++i) {
			this->user_choice->addItem(QString::number(i));
		}
		this->selected_grid = 0;
		this->pressed_ok	= false;
		QObject::connect(this->user_choice, QOverload<int>::of(&QComboBox::currentIndexChanged), [&](int i) {
			this->selected_grid = i;
		});
		QObject::connect(this->button_cancel, &QPushButton::pressed, this, [&]() {
			this->pressed_ok = false;
			this->close();
		});
		QObject::connect(this->button_accept, &QPushButton::pressed, this, [&]() {
			this->pressed_ok = true;
			this->close();
		});
		this->exec();
	}
	bool choice_Cancelled() { return pressed_ok == false; }
	bool choice_Accepted() { return pressed_ok == true; }
	int choice_getMesh() { return selected_grid; }

protected:
	QComboBox* user_choice;
	QPushButton* button_cancel;
	QPushButton* button_accept;
	int selected_grid;
	bool pressed_ok;
};

#endif	  // VISUALISATION_DIALOG_PICK_GRIDS_FROM_SCENE_H
