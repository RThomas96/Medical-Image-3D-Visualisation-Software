#ifndef NVIDIA_BUG_REPORT_MAIN_WIDGET_HPP_
#define NVIDIA_BUG_REPORT_MAIN_WIDGET_HPP_

#include "./glwidget.hpp"

#include <QLabel>
#include <QPushButton>
#include <QMainWindow>
#include <QGridLayout>
#include <QPlainTextEdit>
// For debug :
#include <QOpenGLDebugMessage>

class MainWidget : public QMainWindow {
	Q_OBJECT
	public:
		MainWidget(QWidget* parent = nullptr);
		~MainWidget();
	public slots:
		// Add user message
		void addUserMessage(const QString _message);
		// Log OpenGL message
		void addOpenGLMessage(const QOpenGLDebugMessage _message);
	protected:
		void setupWidgets(); // Allocates and inits widgets
		void setupLayouts(); // Creates layouts and fills them
		void setupSignals(); // Links objects and signals together
	protected:
		QWidget* _container;
		QGridLayout* mainLayout;			///< Widget layout in the window
		MyGLWidget* widget_OpenGL;			///< The GL widget, used to create/upload textures
		QLabel* label_textureStatus;			///< Displays the texture status (created/error ?)
		QPlainTextEdit* log_GLErrors;			///< Output of the GL_KHR_debug extension
		QPushButton* button_createTextureOnce;		///< Create the texture with one call to TexImage
		QPushButton* button_createTextureMultiple;	///< Same above, but in multiple TexSubImage after alloc.
};

#endif // NVIDIA_BUG_REPORT_MAIN_WIDGET_HPP_
