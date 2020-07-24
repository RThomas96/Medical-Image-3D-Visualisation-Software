#include "../include/output_discrete_grid.hpp"

#include <iomanip>

OutputGrid::OutputGrid(void) : DiscreteGrid() {
	this->setModifiable(true);
	this->data.clear();
	this->boundingBox = bbox_t();
	this->gridName = "defaultOutputGrid";
	this->transform_gridToWorld = glm::mat4(1.f);
	this->transform_worldToGrid = glm::mat4(1.f);
	std::cerr << "===========================================================" << '\n';
	std::cerr << "==================<OUTPUTGRID CREATOR>=====================" << '\n';
	std::cerr << "===========================================================" << '\n';
	std::cerr << '[' << this->boundingBox.getMin().x << ' ' << this->boundingBox.getMin().y << ' ' << this->boundingBox.getMin().z << ']' << '\n';
	std::cerr << '[' << this->boundingBox.getMax().x << ' ' << this->boundingBox.getMax().y << ' ' << this->boundingBox.getMax().z << ']' << '\n';
	std::cerr << "===========================================================" << '\n';
	std::cerr << "==================<OUTPUTGRID CREATOR>=====================" << '\n';
	std::cerr << "===========================================================" << '\n';
}

OutputGrid::~OutputGrid() {}

void OutputGrid::setVoxelData(sizevec3 idx, DataType val) {
	// early check :
	if (idx.x >= this->gridDimensions.x || idx.y >= this->gridDimensions.y || idx.z >= this->gridDimensions.z) { return; }
	// set value :
	this->data[idx.x+idx.y*this->gridDimensions.x+idx.z*this->gridDimensions.x*this->gridDimensions.y] = val;
	return;
}

OutputGrid& OutputGrid::preallocateData() {
	return this->preallocateData(this->gridDimensions);
}

OutputGrid& OutputGrid::preallocateData(sizevec3 dims) {
	this->data.clear();
	this->data.resize(dims.x*dims.y*dims.z);
	return *this;
}

OutputGrid& OutputGrid::updateRenderBox(const std::shared_ptr<InputGrid> input) {
	// Get input grid render box :
	std::vector<bbox_t::vec> corners = input->getBoundingBox()
							.transformTo(input->getTransform_GridToWorld())
							.transformTo(this->transform_worldToGrid)
							.getAllCorners();
	// Add all points to this render bounding box :
	this->boundingBox.addPoints(corners);

	return *this;
}

OutputGrid& OutputGrid::setBoundingBox(bbox_t renderWindow) {
	// Warning : assumes the bounding box given is in world space
	renderWindow.printInfo("RenderWindow given :");

	this->boundingBox.printInfo("Before adding points !!!! :");

	//std::cerr << "RenderWindow corners :\n";
	// get bb in this stack's space :
	std::vector<bbox_t::vec> corners = renderWindow.getAllCorners();
	std::for_each(corners.begin(), corners.end(), [&](bbox_t::vec& v) {
		glm::vec4 p = glm::vec4(static_cast<float>(v.x),static_cast<float>(v.y),static_cast<float>(v.z),1.);
		p = p*this->transform_worldToGrid;
		//std::cerr << std::setprecision(4) << '\t' << '[' << v.x << ',' << v.y << ',' << v.z << "] <==> [" << p.x << ',' << p.y << ',' << p.z << ']' << '\n';
		return p;
	});

	std::for_each(corners.begin(), corners.end(), [&](bbox_t::vec& v) {
		this->boundingBox.addPoint(v);
		//std::cerr << '\t' << "Adding point " << std::setprecision(4) << '[' << v.x << ',' << v.y << ',' << v.z << "]\n";
		//this->boundingBox.printInfo("");
	});

	this->boundingBox.addPoints(corners);

	std::cerr << "OutputGrid \"" << this->gridName << "\" has this bounding box :\n";
	this->boundingBox.printInfo("");

	return *this;
}
