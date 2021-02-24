#include "./main_widget.hpp"

#include <QDateTime>
#include <iostream>

MainWidget::MainWidget(QWidget* parent) : QMainWindow(parent) {
	this->_container = nullptr;
	this->mainLayout = nullptr;
	this->widget_OpenGL = nullptr;
	this->label_textureStatus = nullptr;
	this->log_GLErrors = nullptr;
	this->button_createTextureOnce = nullptr;
	this->button_createTextureMultiple = nullptr;

	this->setupWidgets();
	this->setupLayouts();
	this->setupSignals();
	return;
}

MainWidget::~MainWidget() {
	this->button_createTextureOnce->disconnect();
	this->button_createTextureMultiple->disconnect();

	delete this->button_createTextureMultiple;
	delete this->button_createTextureOnce;
	delete this->label_textureStatus;
	delete this->widget_OpenGL;
	delete this->log_GLErrors;
	delete this->mainLayout;
	delete this->_container;
}

void MainWidget::addUserMessage(const QString _message) {
	QString user_header= "[" + QDateTime::currentDateTime().toString(Qt::ISODate)
			     + "] <i>User message</i> : ";
	QString formatted_message = user_header + _message;
	std::cerr << _message.toStdString() << '\n';
	this->log_GLErrors->appendHtml(formatted_message);
	return;
}

void MainWidget::addOpenGLMessage(const QOpenGLDebugMessage message) {
	QString src = "";
	QString sev = "";
	QString typ = "";
	QFlags severity = message.severity();
	QFlags type = message.type();
	QFlags source = message.source();

	QString err = "<span style=\"color:red;\">";
	QString warn = "<span style=\"color:red;\">";
	QString end = "</span>";

	if (severity & QOpenGLDebugMessage::Severity::NotificationSeverity)	{ sev += "{Notif}"; }
	if (severity & QOpenGLDebugMessage::Severity::LowSeverity)		{ sev += warn + "{Low  }" + end; }
	if (severity & QOpenGLDebugMessage::Severity::MediumSeverity)		{ sev += warn + "{Med  }" + end; }
	if (severity & QOpenGLDebugMessage::Severity::HighSeverity)		{ sev += err + "{High }" + end; }

	if (type & QOpenGLDebugMessage::Type::ErrorType)			{ typ += "[	ERROR     ]"; }
	if (type & QOpenGLDebugMessage::Type::DeprecatedBehaviorType)		{ typ += "[ DEPRE. BEHAV. ]"; }
	if (type & QOpenGLDebugMessage::Type::UndefinedBehaviorType)		{ typ += "[ UNDEF. BEHAV. ]"; }
	if (type & QOpenGLDebugMessage::Type::PortabilityType)			{ typ += "[  PORTABILITY  ]"; }
	if (type & QOpenGLDebugMessage::Type::PerformanceType)			{ typ += "[      PERF     ]"; }
	if (type & QOpenGLDebugMessage::Type::OtherType)			{ typ += "[     OTHER	  ]"; }
	if (type & QOpenGLDebugMessage::Type::MarkerType)			{ typ += "[     MARKER    ]"; }
	if (type & QOpenGLDebugMessage::Type::GroupPushType)			{ typ += "[  GROUP PUSH   ]"; }
	if (type & QOpenGLDebugMessage::Type::GroupPopType)			{ typ += "[   GROUP POP   ]"; }

	if (source & QOpenGLDebugMessage::Source::APISource)			{ src += "[OpenGL  ]"; }
	if (source & QOpenGLDebugMessage::Source::WindowSystemSource)		{ src += "[WinSys  ]"; }
	if (source & QOpenGLDebugMessage::Source::ShaderCompilerSource)		{ src += "[ShaComp ]"; }
	if (source & QOpenGLDebugMessage::Source::ThirdPartySource)		{ src += "[3rdParty]"; }
	if (source & QOpenGLDebugMessage::Source::OtherSource)			{ src += "[Other   ]"; }

	QString glMessage = sev + " " + typ + " " + src + " " + QString::number(message.id()) + " : " + message.message();

	// Currently outputs any message on the GL stack, regardless of severity, type, or source :
	this->log_GLErrors->appendHtml(glMessage);
}

void MainWidget::setupWidgets() {
	this->_container = new QWidget;
	this->mainLayout = new QGridLayout;
	this->widget_OpenGL = new MyGLWidget(this);
	this->label_textureStatus = new QLabel("Texture not yet created.");
	this->log_GLErrors = new QPlainTextEdit;
	this->button_createTextureOnce = new QPushButton("Texture creation with one glTexImage3D call");
	this->button_createTextureMultiple = new QPushButton("Texture creation with multiple glTexSubImage3D calls");

	this->log_GLErrors->setReadOnly(true);
	return;
}

void MainWidget::setupLayouts() {
	this->mainLayout->addWidget(this->widget_OpenGL, 0, 0, 1, 2);
	this->mainLayout->addWidget(this->button_createTextureOnce, 1, 0);
	this->mainLayout->addWidget(this->button_createTextureMultiple, 1, 1);
	this->mainLayout->addWidget(this->log_GLErrors, 2, 0, 1, 2);

	this->_container->setLayout(this->mainLayout);
	this->setCentralWidget(this->_container);
	return;
}

void MainWidget::setupSignals() {
	QObject::connect(this->button_createTextureOnce, &QPushButton::clicked,
			 this->widget_OpenGL, &MyGLWidget::createTexture3DOnce);
	QObject::connect(this->button_createTextureMultiple, &QPushButton::clicked,
			 this->widget_OpenGL, &MyGLWidget::createTexture3DMultiple);
	// Done.
}


