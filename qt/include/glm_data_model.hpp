#ifndef VISUALISATION_QT_INCLUDE_GLM_DATA_MODEL_HPP_
#define VISUALISATION_QT_INCLUDE_GLM_DATA_MODEL_HPP_

#include <glm/glm.hpp>

#include <QAbstractItemModel>
#include <QAbstractListModel>
#include <QVector>

class GLMDataModel : public QAbstractListModel {
	Q_OBJECT

public:
	GLMDataModel(QObject* parent = nullptr);
	~GLMDataModel() = default;

public:
	/// @brief Returns the number of rows to display
	int rowCount(const QModelIndex& model_index = QModelIndex()) const override;
	/// @brief Returns the data at the given model index.
	QVariant data(const QModelIndex& model_index = QModelIndex(), int role = Qt::ItemDataRole::DisplayRole) const override;
	/// @brief Sets the data at the given model index.
	bool setData(const QModelIndex& model_index, const QVariant& value, int role = Qt::ItemDataRole::EditRole) override;
	/// @brief The function which specifies which data to return for the headers, both horizontal and vertical.
	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
	/// @brief The function which returns the item flags at a given index.
	Qt::ItemFlags flags(const QModelIndex& model_index) const override;
	/// @brief Called when inserting rows.
	bool insertRows(int position, int rows, const QModelIndex& parent = QModelIndex()) override;
	/// @brief Called when removing rows.
	bool removeRows(int position, int rows, const QModelIndex& parent = QModelIndex()) override;

private:
	QVector<glm::vec3> positions;
};

#endif // VISUALISATION_QT_INCLUDE_GLM_DATA_MODEL_HPP_
