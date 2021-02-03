#include "../include/user_settings_widget.hpp"

#include <fstream>

UserSettings::UserSettings() {
	this->isInit = false;
	this->userAllowedBitSize = 0;
	this->userLoadedSize = 0;
	this->userRemainingBitSize = 0;
}

UserSettings::~UserSettings() = default;

UserSettings UserSettings::getInstance() {
	static UserSettings settings{};
	std::cerr << "querying an instance" << '\n';
	if (settings.isInit == false) { settings.init(); }
	return settings;
}

bool UserSettings::canLoadImageSize(std::size_t sizeBits) {
	return this->userLoadedSize + sizeBits < this->userAllowedBitSize;
}

void UserSettings::loadImageSize(std::size_t sizeBits) {
	this->userLoadedSize += sizeBits;
	this->userRemainingBitSize = this->userAllowedBitSize - this->userLoadedSize;
}

void UserSettings::setUserAllowedBitSize(std::size_t uabs) {
	this->userAllowedBitSize = uabs;
}

void UserSettings::init() {
	if (this->isInit == true) { return; }
	#if defined(WIN32) || defined(WIN64) || defined(_MSC_VER) || defined(_WIN32)
	MEMORYSTATUSEX memstats{};
	if (GlobalMemoryStatusEx(memstats) != 0) {
		std::cerr << "error : cannot get memory stats !" << '\n';
		this->userAllowedBitSize = 2 * 1024 * 1024 * 1024; // 2GB by default
	} else {
		// size is in bytes here (convert to bits) :
		this->userAllowedBitSize = static_cast<std::size_t>(memstats.ullAvailPhys*8/2);
		std::cerr << "set user allowed size to " << memstats.ullAvailPhys/1024/1024/2 << " MB by default\n";
	}
	this->isInit = true;
	return;
	#endif

	#if defined(__linux__) || defined(__gnu_linux__) || defined(__GNUC__) || defined(__clang__)
	std::ifstream meminfo("/proc/meminfo", std::ios_base::in);
	std::string data = "";
	std::string matching = "MemAvailable";
	unsigned long long avail = 0;
	while (meminfo.good() && not meminfo.eof()) {
		meminfo >> data;
		if (data.find(matching) != std::string::npos) {
			meminfo >> avail;
		}
	}
	// size is in bytes here (convert to bits) :
	this->userAllowedBitSize = static_cast<std::size_t>(avail*1024*8/2);
	this->userRemainingBitSize = this->userAllowedBitSize;
	this->isInit = true;
	return;
	#endif

	#warning Implement the logic for macOS here ?
}

UserSettingsWidget::UserSettingsWidget(QWidget* parent) : QWidget(parent), settings(UserSettings::getInstance()) {
	this->setAttribute(Qt::WA_DeleteOnClose);
	this->initWidgets();
	this->initSignals();
}

UserSettingsWidget::~UserSettingsWidget() {
	this->spinbox_allowedMemory->disconnect();
	this->comboBox_memUnit->disconnect();
	delete this->spinbox_allowedMemory;
	delete this->comboBox_memUnit;
	delete this->label_mem;
	delete this->layout;
}

void UserSettingsWidget::initWidgets() {
	this->spinbox_allowedMemory = new QSpinBox;
	this->label_mem = new QLabel("Maximum amount of memory used (bits) : ");
	this->comboBox_memUnit = new QComboBox;

	this->comboBox_memUnit->addItem("B");
	this->comboBox_memUnit->addItem("kB");
	this->comboBox_memUnit->addItem("MB");
	this->comboBox_memUnit->addItem("GB");
	this->comboBox_memUnit->setMaxCount(4);

	// trick to make the spinbox as large as it can :
	this->spinbox_allowedMemory->setValue(std::numeric_limits<int>::max());
	// cap it to -1 as a minimum value (used to represent no limit) :
	this->spinbox_allowedMemory->setRange(-1, std::numeric_limits<int>::max());
	this->spinbox_allowedMemory->setMinimum(-1);

	this->layout = new QHBoxLayout;
	this->layout->addWidget(this->label_mem);
	this->layout->addWidget(this->spinbox_allowedMemory);
	this->layout->addWidget(this->comboBox_memUnit);

	this->lastMemUnit = 2;
	// originally set in mbytes when showing to user, so divide by 1024^2
	// (and then by 8 since we're getting bits but showing bytes) :
	int value = static_cast<int>(this->settings.getUserAllowedBitSize() / 1024 / 1024 / 8);
	this->spinbox_allowedMemory->setValue(value);
	this->comboBox_memUnit->setCurrentIndex(this->lastMemUnit);

	this->setLayout(this->layout);

	return;
}

void UserSettingsWidget::initSignals() {
	QObject::connect(this->spinbox_allowedMemory, QOverload<int>::of(&QSpinBox::valueChanged), this, &UserSettingsWidget::updateMemValue);
	QObject::connect(this->comboBox_memUnit, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &UserSettingsWidget::updateMemUnit);
}

void UserSettingsWidget::updateMemUnit(int comboBoxValue) {
	// value would have been updated in user settings, get allowed size
	// directly from it rather than from the spinbox :
	std::size_t rawvalue = this->settings.getUserAllowedBitSize();
	std::cerr << "Current bit size : " << rawvalue << '\n';
	if (comboBoxValue >= 0) { rawvalue /=    8; }	// chose bytes
	if (comboBoxValue >= 1) { rawvalue /= 1024; }	// chose bytes, or kilobytes
	if (comboBoxValue >= 2) { rawvalue /= 1024; }	// chose bytes, or kilobytes, or megabytes
	if (comboBoxValue >= 3) { rawvalue /= 1024; }	// chose bytes, or kilobytes, or megabytes, or gigabytes
	std::cerr << "Output bit size : " << rawvalue << '\n';
	// update value :
	this->spinbox_allowedMemory->blockSignals(true);
	this->spinbox_allowedMemory->setValue(static_cast<int>(rawvalue));
	this->spinbox_allowedMemory->blockSignals(false);
}

void UserSettingsWidget::updateMemValue(int value) {
	// if value if -1, then no limit is applied ! disable unit picker and return
	if (value == -1) {
		this->comboBox_memUnit->setDisabled(true);
		this->settings.setUserAllowedBitSize(std::numeric_limits<std::size_t>::max());
		return;
	} else {
		// enable combo box
		this->comboBox_memUnit->setEnabled(true);
	}
	// get index of chosen unit :
	int comboBoxValue = this->comboBox_memUnit->currentIndex();
	std::size_t rawvalue = static_cast<std::size_t>(value);
	std::cerr << "V Current bit size : " << rawvalue << '\n';
	// scale new value by the unit chosen :
	if (comboBoxValue >= 0) { rawvalue *=    8; }	// chose bytes
	if (comboBoxValue >= 1) { rawvalue *= 1024; }	// chose bytes, or kilobytes
	if (comboBoxValue >= 2) { rawvalue *= 1024; }	// chose bytes, or kilobytes, or megabytes
	if (comboBoxValue >= 3) { rawvalue *= 1024; }	// chose bytes, or kilobytes, or megabytes, or gigabytes
	std::cerr << "V Output bit size : " << rawvalue << '\n';
	this->settings.setUserAllowedBitSize(rawvalue);
}
