#ifndef VIEWER_INCLUDE_NEIGHBOR_VISU_VIEWER_HPP_
#define VIEWER_INCLUDE_NEIGHBOR_VISU_VIEWER_HPP_

#include "../../features.hpp"

#include "./scene.hpp"

#include <QGLViewer/qglviewer.h>
#include <QTimer>

// #include <renderdoc_app.h>

#include <memory>

/// @ingroup graphpipe
/// @brief A viewer that displays a scene, either in real space or in initial space
class Viewer : public QGLViewer {
	Q_OBJECT
public:
	/// @brief Default constructor for the viewer.
	Viewer(Scene* const scene, QStatusBar* program_bar, QWidget* parent = nullptr);
	~Viewer();
	/// @brief Multiplier to apply to scene radii for the scene's view.
	static float sceneRadiusMultiplier;

	/// @brief Updates info from the scene, binding its context for rendering.
	void updateInfoFromScene();

protected:
	/// @brief Initializes the scene, and the viewer's variables.
	virtual void init() override;
	/// @brief Draws the scene, in the space the viewer is supposed to show.
	virtual void draw() override;
	/// @brief Handles key events from the user.
	virtual void keyPressEvent(QKeyEvent* e) override;
	/// @brief Mouse press event generated by the widget.
	virtual void mousePressEvent(QMouseEvent* e) override;
	/// @brief Mouse press event generated by the widget.
	virtual void mouseMoveEvent(QMouseEvent* e) override;
	/// @brief Mouse press event generated by the widget.
	virtual void mouseReleaseEvent(QMouseEvent* e) override;
	/// @brief Wheel event for the mouse.
	virtual void wheelEvent(QWheelEvent* _w) override;
	/// @brief Defines the 'Help'/'About' string defined for this viewer.
	virtual QString helpString(void) const override;
	/// @brief Defines the 'Help'/'About' string for the keyboard for this viewer.
	virtual QString keyboardString(void) const override;
	/// @brief Defines the 'Help'/'About' string for the mouse for this viewer.
	virtual QString mouseString(void) const override;
	/// @brief Overrides the function to resize the widget.
	virtual void resizeGL(int w, int h) override;
	/// @brief Resets and removes the local point query
	void resetLocalPointQuery();

private:
	/// @brief The scene to control.
	Scene* const scene;
	/// @brief Should we draw it in volumetric mode ?
	bool drawVolumetric;
	/// @brief A refresh timer for the viewer, to update in real time.
	QTimer* refreshTimer;
	/// @brief Should we capture a frame ?
	bool shouldCapture;
	/// @brief Is the Ctrl key pressed down ?
	bool keyboard_CtrlDown;
	/// @brief The texture attached to the secondary framebuffer output.
	GLuint renderTarget;
	/// @brief Are we in "select mode" ? (for selecting coordinates in rasterized/volumetric view)
	bool selectMode;
	/// @brief Framebuffer dimensions, updated once per resize event
	glm::ivec2 fbSize;
	/// @brief Current cursor position relative to the widget's geometry origin
	glm::ivec2 cursorPos_current;
	/// @brief Last known cursor position relative to the widget's geometry origin
	glm::ivec2 cursorPos_last;
	/// @brief The number of frames a modifier (RMB,LMB,Ctrl ...) has been held for.
	std::size_t framesHeld;
	/// @brief Request to read a fragment"s position
	glm::ivec2 posRequest;
	/// @brief Program's status bar
	QStatusBar* statusBar;
	bool drawAxisOnTop;

	//
	// Stubs for ARAP integration
	//
	std::vector<glm::vec3> spheres;	///< Spheres to draw
	float sphere_size;				///< Sphere size in the renderer
	glm::vec3 temp_sphere_position;	///< Last-requested sphere position
	std::size_t temp_mesh_idx;		///< The mesh index selected
	std::size_t temp_mesh_vtx_idx;	///< The mesh vertex idx of the selected vertex (temp_sphere_position)
	std::size_t temp_img_idx;		///< The image index if found. WARNING : WE ASSUME IT IS ALWAYS 0, EVEN IF NO IMAGES ARE LOADED
	glm::vec3 temp_img_pos;			///< The position of that image index
public slots:
	/// @brief Update the view, as a slot without any arguments (currently only used by QTimer)
	void updateView() { this->update(); }
	/// @brief Updates the camera position once one or two grids are loaded in the scene.
	void updateCameraPosition(void);
	/// @brief Asks the scene to load a grid into itself.
	// void loadGrid(const std::shared_ptr<InputGrid>& g);
	/// @brief Asks the scene to load two grids into itself.
	// void loadTwoGrids(const std::shared_ptr<InputGrid>& g1, const std::shared_ptr<InputGrid>& g2);
	void newAPI_loadGrid(Image::Grid::Ptr ptr);
	/// @brief Re-centers the camera around the scene-defined center point
	void centerScene(void);
	void guessMousePosition(void);
};

#endif	  // VIEWER_INCLUDE_NEIGHBOR_VISU_VIEWER_HPP_
