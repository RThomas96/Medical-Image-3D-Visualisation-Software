#include "../include/opengl_debug_log.hpp"

#include <QHBoxLayout>

OpenGLDebugLog::OpenGLDebugLog(void) {
	this->groupDepth = 0;
	this->setupWidgets();
}

OpenGLDebugLog::~OpenGLDebugLog(void) {
}

void OpenGLDebugLog::setupWidgets() {
	this->messageOutput = new QPlainTextEdit;
	this->layout		= new QHBoxLayout;

	this->messageOutput->setReadOnly(true);

	this->layout->addWidget(this->messageOutput);
	this->setLayout(this->layout);
}

void OpenGLDebugLog::addErrorMessage(QString _m) {
	this->messageOutput->appendPlainText("========= Error Message from Application =========");
	this->messageOutput->appendHtml(_m);
	this->messageOutput->appendPlainText("==================================================");
	return;
}

void OpenGLDebugLog::setupSignals() {
	// nothing for now ...
}

void OpenGLDebugLog::addOpenGLMessage(QOpenGLDebugMessage message) {
	QString src		= "";
	QString sev		= "";
	QString typ		= "";
	QFlags severity = message.severity();
	QFlags type		= message.type();
	QFlags source	= message.source();

	if (severity & QOpenGLDebugMessage::Severity::NotificationSeverity) {
		sev += "{Notif}";
	}
	if (severity & QOpenGLDebugMessage::Severity::LowSeverity) {
		sev += "{Low  }";
	}
	if (severity & QOpenGLDebugMessage::Severity::MediumSeverity) {
		sev += "{Med  }";
	}
	if (severity & QOpenGLDebugMessage::Severity::HighSeverity) {
		sev += "{High }";
	}

	if (type & QOpenGLDebugMessage::Type::ErrorType) {
		typ += "[	ERROR   ]";
	}
	if (type & QOpenGLDebugMessage::Type::DeprecatedBehaviorType) {
		typ += "[ DEPREC. ]";
	}
	if (type & QOpenGLDebugMessage::Type::UndefinedBehaviorType) {
		typ += "[ UNDEF.  ]";
	}
	if (type & QOpenGLDebugMessage::Type::PortabilityType) {
		typ += "[ PORTAB. ]";
	}
	if (type & QOpenGLDebugMessage::Type::PerformanceType) {
		typ += "[  PERF.  ]";
	}
	if (type & QOpenGLDebugMessage::Type::OtherType) {
		typ += "[  OTHER. ]";
	}
	if (type & QOpenGLDebugMessage::Type::MarkerType) {
		typ += "[ MARKER  ]";
	}
	if (type & QOpenGLDebugMessage::Type::GroupPushType) {
		typ += "[  PUSH   ]";
		this->groupDepth++;
	}
	if (type & QOpenGLDebugMessage::Type::GroupPopType) {
		typ += "[   POP   ]";
		this->groupDepth--;
	}

	if (source & QOpenGLDebugMessage::Source::APISource) {
		src += "[OpenGL  ]";
	}
	if (source & QOpenGLDebugMessage::Source::WindowSystemSource) {
		src += "[WinSys  ]";
	}
	if (source & QOpenGLDebugMessage::Source::ShaderCompilerSource) {
		src += "[ShaComp ]";
	}
	if (source & QOpenGLDebugMessage::Source::ThirdPartySource) {
		src += "[3rdParty]";
	}
	if (source & QOpenGLDebugMessage::Source::OtherSource) {
		src += "[Other   ]";
	}

	QString glMessage = QString(this->groupDepth, '\t') + sev + " " + typ + " " + src + " " + QString::number(message.id()) + " : " + message.message();

	// Currently outputs any message on the GL stack, regardless of severity, type, or source :
	this->messageOutput->appendPlainText(glMessage);
}
