#ifndef VISUALISATION_MESH_POINT_SELECTOR_HPP
#define VISUALISATION_MESH_POINT_SELECTOR_HPP

#include "../../viewer/include/neighbor_visu_viewer.hpp"
#include "../../viewer/include/planar_viewer.hpp"

#include <QGLViewer/qglviewer.h>

#include <QLabel>
#include <QPushButton>
#include <QSpinBox>
#include <QTableView>
#include <QWidget>

class MeshPointSelector {
	Q_OBJECT
public:
	MeshPointSelector();
	~MeshPointSelector();

public slots:
	void setMesh(std::shared_ptr<Mesh>);
	void setMaxARAPIterations(std::size_t);
	void selectClosestPoint(glm::vec4 position);

signals:
	void ready();
	void launchARAP();
	void selectPoint();

protected:
	std::shared_ptr<Mesh> mesh;
	std::vector<std::size_t> vertices_chosen;
	std::vector<glm::vec3> positions_chosen;

	QPushButton* button_launchARAP;
	QPushButton* button_selectPoint;
};

#endif	  // VISUALISATION_MESH_POINT_SELECTOR_HPP
