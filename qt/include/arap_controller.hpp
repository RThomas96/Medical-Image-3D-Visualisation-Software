#ifndef VISUALISATION_QT_INCLUDE_ARAP_CONTROLLER_HPP_
#define VISUALISATION_QT_INCLUDE_ARAP_CONTROLLER_HPP_

#include <glm/glm.hpp>

#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QListView>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>

class Viewer;
class Scene;

class ARAPController : public QWidget {
public:
	ARAPController(Viewer* main_viewer, Scene* _scene);
	virtual ~ARAPController() = default;

	void init();
	void initSignals();

public slots:
	void addImageConstraint(glm::vec3 position);
	void addMeshConstraint(std::size_t vtx_idx);
	void launchARAPInScene();

protected:
	QList<glm::vec4> image_constraints;
	QList<std::size_t> mesh_constraints;
	Viewer* viewer;
	Scene* scene;

	// Qt layout/widgets stuff :
	QListView* listview_mesh_constraints;
	QListView* listview_image_constraints;
	QPushButton* button_launchARAP;

	QGridLayout* widget_layout;
	QLabel* label_grid_name;
	QLabel* label_mesh_info;
};

#endif	  // VISUALISATION_QT_INCLUDE_ARAP_CONTROLLER_HPP_
