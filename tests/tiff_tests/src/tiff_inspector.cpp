#include "../include/tiff_inspector.hpp"

#include <iostream>
#include <chrono>
#include <sstream>

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollBar>
#include <QFileDialog>

#include "TinyTIFF/tinytiffreader.h"

tiff_inspector::tiff_inspector(QWidget* parent) {
	this->setParent(parent);
	this->file_name = "";
	QVBoxLayout* vlayout = new QVBoxLayout();
	QHBoxLayout* hlayout = new QHBoxLayout();
	this->open_image_button = new QPushButton("Open TIFF Image");
	this->image_name_label = new QLabel("Title : No image loaded.");
	this->image_dims_label = new QLabel("Dimensions : Hard to get efficiently");
	this->image_size_label = new QLabel("Size : N/A");
	this->image_time_label = new QLabel("Time to open : N/A");
	this->image_info_label = new QPlainTextEdit("No image selected");
	this->image_info_label->setReadOnly(true);
	this->tiff_displayer = new tiff_display(nullptr);

	vlayout->addWidget(this->open_image_button);
	vlayout->addWidget(this->image_name_label);
	vlayout->addWidget(this->image_dims_label);
	vlayout->addWidget(this->image_size_label);
	vlayout->addWidget(this->image_time_label);
	vlayout->addWidget(this->image_info_label);

	hlayout->addLayout(vlayout);
	hlayout->addWidget(this->tiff_displayer);

	connect(this->open_image_button, &QPushButton::pressed, this, &tiff_inspector::on_set_image_clicked);

	this->setLayout(hlayout);
}

void tiff_inspector::update_labels_for_image_data() {
	std::string img_name_txt = "Title : No image loaded.";
	std::string img_dims_txt = "Dimensions : Hard to get efficiently";
	std::string img_size_txt = "Size : N/A";
	std::string img_time_txt = "Time to open : N/A";
	std::string img_info_separator = "============\n============";
	std::string img_info_txt = "No image loaded.";
	std::chrono::high_resolution_clock::time_point start_point, end_point;
	std::stringstream strbuf;

	if (this->file_name != "") {
		TinyTIFFReaderFile* tiff_file = nullptr;
		QString last_name = this->file_name.split("/").last();
		QImage original_image(this->file_name);
		original_image.save("input_image.tif");
		start_point = std::chrono::high_resolution_clock::now();
		tiff_file = TinyTIFFReader_open(this->file_name.toStdString().c_str());
		end_point = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double, std::milli> duration = end_point - start_point;
		strbuf << "Time to open : " << duration.count() << "ms";
		img_time_txt = strbuf.str();
		strbuf.str("");
		img_name_txt = "Image name : " + last_name.toStdString();
		if (tiff_file) {
			if (TinyTIFFReader_wasError(tiff_file)) {
				std::cerr << "TinyTIFF error : " << TinyTIFFReader_getLastError(tiff_file) << std::endl;
			}
			img_info_txt = TinyTIFFReader_getImageDescription(tiff_file);
			if (img_info_txt.length() == 0) {
				img_info_txt = "No image description was embedded in the file.\n";
				img_info_txt += "Image frame count : " + std::to_string(TinyTIFFReader_countFrames(tiff_file)) + '\n';
				img_info_txt += "File name : " + this->file_name.toStdString() + '\n';
				img_info_txt += "Projected time for 4k images : " + std::to_string(4.0 * (duration.count()));
			}
			strbuf << "Image size : " << TinyTIFFReader_getWidth(tiff_file) << "x" << TinyTIFFReader_getHeight(tiff_file);
			img_size_txt = strbuf.str();
			strbuf.str("");
			this->tiff_displayer->load(tiff_file);
			TinyTIFFReader_close(tiff_file);
		} else {
			std::cerr << "Error : no image loaded (file name : \"" << this->file_name.toStdString() << "\"" << std::endl;
		}
	}
	this->image_name_label->setText(img_name_txt.c_str());
	this->image_dims_label->setText(img_dims_txt.c_str());
	this->image_size_label->setText(img_size_txt.c_str());
	this->image_time_label->setText(img_time_txt.c_str());
	this->image_info_label->appendPlainText(img_info_separator.c_str());
	this->image_info_label->appendPlainText(img_info_txt.c_str());
	this->image_info_label->verticalScrollBar()->setSliderPosition(this->image_info_label->verticalScrollBar()->maximum());
	return;
}

void tiff_inspector::on_set_image_clicked() {
	QString fn = QFileDialog::getOpenFileName(this, tr("Open TIFF image"), "../", tr("TIFF Image files (*.tif *.tiff)"));
	std::cerr << "Got filename " << fn.toStdString() << std::endl;
	this->file_name = fn;
	this->update_labels_for_image_data();
}
