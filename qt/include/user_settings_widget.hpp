#ifndef QT_INCLUDE_USER_SETTINGS_WIDGET_HPP_
#define QT_INCLUDE_USER_SETTINGS_WIDGET_HPP_

#include "../../macros.hpp"
#include <iostream>

class UserSettings {
	protected:
		UserSettings(void);
	public:
		~UserSettings(void);
	public:
		static UserSettings getInstance(void);
		bool canLoadImageSize(std::size_t sizeBits);
		void loadImageSize(std::size_t sizeBits);
		void setUserAllowedBitSize(std::size_t uabs);
	public:
		std::size_t getUserAllowedBitSize() const { return this->userAllowedBitSize; }
		std::size_t getUserRemainingBitSize() const { return this->userRemainingBitSize; }
		std::size_t getUserLoadedSize() const { return this->userLoadedSize; }
	protected:
		void init();
	protected:
		bool isInit;
		std::size_t userAllowedBitSize;
		std::size_t userRemainingBitSize;
		std::size_t userLoadedSize;
};

#include <QLabel>
#include <QWidget>
#include <QSpinBox>
#include <QComboBox>
#include <QHBoxLayout>

class UserSettingsWidget : public QWidget {
	Q_OBJECT;
	public:
		UserSettingsWidget(QWidget* parent = nullptr);
		~UserSettingsWidget(void);
	public slots:
		void updateMemValue(int val);
		void updateMemUnit(int val);
	protected:
		void initWidgets();
		void initSignals();
	protected:
		int lastMemUnit;	///< keeps track of the last mem unit, to perform conversions
		UserSettings settings;
		QLabel* label_mem;
		QSpinBox* spinbox_allowedMemory;
		QHBoxLayout* layout;
		QComboBox* comboBox_memUnit;
};

#endif // QT_INCLUDE_USER_SETTINGS_WIDGET_HPP_