#ifndef LOGOUTPUT_HPP
#define LOGOUTPUT_HPP

#include <QPlainTextEdit>
#include <QScrollBar>

#include <iostream>

class PlainLogOutput : public QPlainTextEdit {
		Q_OBJECT
	public:
		void appendMessage(const std::string message) {
			this->appendPlainText(message.c_str());
			this->appendPlainText("\n");
			this->verticalScrollBar()->setValue(this->verticalScrollBar()->maximum());
		}
		void cout(const std::string message) { this->appendMessage(message); }
		void cerr(const std::string message) {
			QString errBegin = "<div style=\"color:red;\">";
			QString errEnd = "</div>";
			this->appendHtml(errBegin);
			this->appendMessage(message.c_str());
			this->appendHtml(errEnd);
			this->verticalScrollBar()->setValue(this->verticalScrollBar()->maximum());
		}
};

#endif // LOGOUTPUT_HPP
