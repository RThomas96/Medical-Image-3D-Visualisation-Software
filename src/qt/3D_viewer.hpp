#ifndef VIEWER_INCLUDE_NEIGHBOR_VISU_VIEWER_HPP_
#define VIEWER_INCLUDE_NEIGHBOR_VISU_VIEWER_HPP_

#include "../core/interaction/manipulator.hpp"

#include "./scene.hpp"

#include <QTimer>
#include <QCursor>

#include <memory>

class Viewer : public QGLViewer {
	Q_OBJECT
public:
	Viewer(Scene* scene, QStatusBar* program_bar, QWidget* parent = nullptr);
	~Viewer();
	static float sceneRadiusMultiplier;

	void addStatusBar(QStatusBar* _sb);

    void enterEvent(QEvent * event)
    {
        // Allow to automatically set the Viewer as active when mouse over, a click isn't needed anymore
        //this->activateWindow();
        //this->grabKeyboard();
        //QWidget::enterEvent(event);
    }

    void leaveEvent(QEvent * event) {
        QWidget::leaveEvent(event);
    }


protected:
	virtual void init() override;
	virtual void draw() override;
    virtual void keyReleaseEvent(QKeyEvent* e) override;
	virtual void keyPressEvent(QKeyEvent* e) override;
	virtual void mousePressEvent(QMouseEvent* e) override;
	virtual void mouseMoveEvent(QMouseEvent* e) override;
	virtual void mouseReleaseEvent(QMouseEvent* e) override;
	virtual void wheelEvent(QWheelEvent* _w) override;
	virtual QString helpString(void) const override;
	virtual QString keyboardString(void) const override;
	virtual QString mouseString(void) const override;
	virtual void resizeGL(int w, int h) override;

private:
	Scene* scene;
	bool drawVolumetric;
	QTimer* refreshTimer;
	bool shouldCapture;
	bool keyboard_CtrlDown;
	GLuint renderTarget;
	bool selectMode;
	glm::ivec2 fbSize;
	glm::ivec2 cursorPos_current;
	glm::ivec2 cursorPos_last;
	std::size_t framesHeld;
	glm::ivec2 posRequest;
	QStatusBar* statusBar;
	bool drawAxisOnTop;

    QPoint mousePos;
    QCursor* cursor;

    void castRayFromMouse(glm::vec3& origin, glm::vec3& direction);

public slots:
	void updateView() { this->update(); }
    void setCenter(const glm::vec3& center);
    void setRadius(const float radius);
    void castRay(void);
    void setCursorType(CursorType cursorType);
    void setCameraType(qglviewer::Camera::Type cameraType);

signals:
    void keyPressed(QKeyEvent* e);
    void keyReleased(QKeyEvent* e);
    void mousePressed(QMouseEvent* e);
    void mouseReleased(QMouseEvent* e);
    void sceneRadiusChanged(float sceneRadius);
};

#endif	  // VIEWER_INCLUDE_NEIGHBOR_VISU_VIEWER_HPP_
