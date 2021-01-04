#include "../include/qt_proxies.hpp"

Proxies::BoundingBox::~BoundingBox() {}

void Proxies::BoundingBox::setMinX(double newVal) {
	std::visit([newVal](auto& v) {v->setMinX_forced(newVal);}, this->boundingBox);
	this->grid->updateVoxelDimensions();
}

void Proxies::BoundingBox::setMinY(double newVal) {
	std::visit([newVal](auto& v) {v->setMinY_forced(newVal);}, this->boundingBox);
	this->grid->updateVoxelDimensions();
}

void Proxies::BoundingBox::setMinZ(double newVal) {
	std::visit([newVal](auto& v) {v->setMinZ_forced(newVal);}, this->boundingBox);
	this->grid->updateVoxelDimensions();
}

void Proxies::BoundingBox::setMaxX(double newVal) {
	std::visit([newVal](auto& v) {v->setMaxX_forced(newVal);}, this->boundingBox);
	this->grid->updateVoxelDimensions();
}

void Proxies::BoundingBox::setMaxY(double newVal) {
	std::visit([newVal](auto& v) {v->setMaxY_forced(newVal);}, this->boundingBox);
	this->grid->updateVoxelDimensions();
}

void Proxies::BoundingBox::setMaxZ(double newVal) {
	std::visit([newVal](auto& v) {v->setMaxZ_forced(newVal);}, this->boundingBox);
	this->grid->updateVoxelDimensions();
}

Proxies::Resolution::~Resolution(void) {}

void Proxies::Resolution::setResolutionX(int newVal) {
	this->vec->x = static_cast<data_t>(newVal);
	this->grid->updateVoxelDimensions();
}

void Proxies::Resolution::setResolutionY(int newVal) {
	this->vec->y = static_cast<data_t>(newVal);
	this->grid->updateVoxelDimensions();
}

void Proxies::Resolution::setResolutionZ(int newVal) {
	this->vec->z = static_cast<data_t>(newVal);
	this->grid->updateVoxelDimensions();
}
