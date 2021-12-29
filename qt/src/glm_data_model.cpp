#include "../include/glm_data_model.hpp"

#include <glm/gtx/io.hpp>

#include <iostream>
#include <sstream>

GLMDataModel::GLMDataModel(QObject *parent) :
	QAbstractListModel(parent) {
	this->positions.clear();
}

int GLMDataModel::rowCount(const QModelIndex &model_index) const {
	return this->positions.count();
}

QVariant GLMDataModel::data(const QModelIndex &model_index, int role) const {
	if (role == Qt::ItemDataRole::DisplayRole || role == Qt::ItemDataRole::EditRole) {
		if (model_index.isValid()) {
			if (model_index.row() < this->positions.count()) {
				std::stringstream glm_data;
				glm_data << this->positions.at(model_index.row());
				std::string glm_string = glm_data.str();
				return QVariant(QString::fromStdString(glm_string));
			}
		}
		return {};
	}
	return {};
}

bool GLMDataModel::setData(const QModelIndex &model_index, const QVariant &value, int role) {
	if (role != Qt::ItemDataRole::EditRole) {
		std::cerr << "Tried to set data but role wasn't edit.\n";
		return false;
	}

	// If we want to edit, and index is valid :
	if (model_index.isValid() && role == Qt::ItemDataRole::EditRole) {
		// Get data, if we can convert it to glm::vec3 insert it otherwise error and return false :
		auto model_data = model_index.data(Qt::ItemDataRole::EditRole);
		if (model_data.isValid() && model_data.canConvert<glm::vec3>()) {
			this->positions[model_index.row()] = model_index.data().value<glm::vec3>();
			emit dataChanged(model_index, model_index, {role});
			return true;
		} else if (model_data.isValid()) {
			std::cerr << "Error : should edit but cannot cast data from " << model_index.data().typeName() << " to glm::vec3\n";
		} else {
			std::cerr << "Error : should edit but model data not valid for edit role.\n";
		}
		return false;
	} else if (not model_index.isValid() && model_index.row() == this->positions.count()) {
		std::cerr << "Model index was not valid, but is equal to the length of thr vector\n";
		std::cerr << "APPEND OPERATION TO GLM MODEL\n";
		auto model_data = model_index.data(Qt::ItemDataRole::EditRole);
		if (model_data.isValid() && model_data.canConvert<glm::vec3>()) {
			this->positions.push_back(model_data.value<glm::vec3>());
			emit dataChanged(model_index, model_index, {role});
			return true;
		} else if (model_data.isValid()) {
			std::cerr << "Error : should append but cannot cast data from " << model_data.typeName() << " to glm::vec3.\n";
		} else {
			std::cerr << "Error : should edit but model data not valid for edit role.\n";
		}
	} else {
		std::cerr << "Tried to edit for invalid index.\n";
	}
	return false;
}

QVariant GLMDataModel::headerData(int section, Qt::Orientation orientation, int role) const {
	//
}

Qt::ItemFlags GLMDataModel::flags(const QModelIndex &model_index) const {
	//
}

bool GLMDataModel::insertRows(int position, int rows, const QModelIndex &parent) {
	beginInsertRows(QModelIndex(), position, position+rows-1);
	endInsertRows();
	return true;
}

bool GLMDataModel::removeRows(int position, int rows, const QModelIndex &parent) {
	beginRemoveRows(QModelIndex(), position, position+rows-1);
}