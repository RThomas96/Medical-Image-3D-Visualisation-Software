#include "../include/discrete_grid.hpp"

glm::mat4 computeTransfoShear(double angleDeg, glm::vec3 origin) {
	glm::mat4 transfoMat = glm::mat4(1.);
	// Get angle radians :
	double angleRad = (angleDeg * M_PI) / 180.;

	transfoMat[0][0] = 0.39 * std::cos(angleRad);
	transfoMat[0][2] = 0.39 * std::sin(angleRad);
	transfoMat[1][1] = 0.39;
	transfoMat[2][2] = 1.927 * std::cos(angleRad);

	return transfoMat;
}

DiscreteGrid::DiscreteGrid(bool _modifiable) {
	this->modifiable = _modifiable;
	this->data.clear();
	this->gridDimensions = sizevec3(0);
	this->voxelDimensions = glm::vec3(1.f);
	this->transform_gridToWorld = glm::mat4(1.f);
	this->transform_worldToGrid = glm::mat4(1.f);
	this->gridName = "defaultGridName";
	this->boundingBox = bbox_t();
	this->dataBoundingBox = bbox_t();
	this->dataThreshold = DataType(0);
}

DiscreteGrid::DiscreteGrid(IO::GenericGridReader& reader) : DiscreteGrid() {
	this->fromGridReader(reader);
}

DiscreteGrid::~DiscreteGrid(void) {
	this->data.clear();
}

DiscreteGrid& DiscreteGrid::fromGridReader(IO::GenericGridReader& reader) {
	// Updates this grid's data, bypassing the 'state'isModifiable' check.
	this->data.clear();
	this->dataBoundingBox = reader.getDataBoundingBox();
	this->dataThreshold = reader.getDataThreshold();
	this->boundingBox = reader.getBoundingBox();
	this->gridDimensions = reader.getGridDimensions();
	this->voxelDimensions = reader.getVoxelDimensions();

	this->setTransform_GridToWorld(reader.getTransform());

	std::size_t gridsize = this->gridDimensions.x * this->gridDimensions.y * this->gridDimensions.z;
	this->data.resize(gridsize);
	// copy data :
	reader.swapData(this->data);

	return *this;
}

DiscreteGrid& DiscreteGrid::recomputeBoundingBox(DataType threshold) {
	// Early return for empty grids :
	if (this->data.size() == 0) { return *this; };
	DiscreteGrid::bbox_t newBB;
	std::cout << "Updating the grid \"" << this->gridName << "\" with a new threshold of " << +threshold << '\n';
	for (std::size_t z = 0; z < this->gridDimensions.z; ++z) {
		for (std::size_t y = 0; y < this->gridDimensions.y; ++y) {
			for (std::size_t x = 0; x < this->gridDimensions.x; ++x) {
				if (this->data[x+y*this->gridDimensions.x+z*this->gridDimensions.x*this->gridDimensions.y] >= threshold) {
					DiscreteGrid::bbox_t::vec v(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z));
					// add v to the data bounding box, updating itself :
					newBB.addPoint(v);
				}
			}
		}
	}
	this->dataBoundingBox = newBB;
	this->dataThreshold = threshold;
	std::cout << "Updated" << '\n';
	return *this;
}

glm::vec4 DiscreteGrid::toGridSpace(glm::vec4 pos_ws) {
	return pos_ws * this->transform_worldToGrid;
}

glm::vec4 DiscreteGrid::toWorldSpace(glm::vec4 pos_gs) {
	return pos_gs * this->transform_gridToWorld;
}

DiscreteGrid::DataType DiscreteGrid::fetchTexelGridSpace(glm::vec4 pos_gs) {
	if (pos_gs.x < .0f || pos_gs.y < .0f || pos_gs.z < .0f) { return DataType(0); }

	// compute index of position :
	std::size_t x = std::floor(pos_gs.x);
	std::size_t y = std::floor(pos_gs.y);
	std::size_t z = std::floor(pos_gs.z);
	return this->fetchTexelIndex(sizevec3(x,y,z));
}

DiscreteGrid::DataType DiscreteGrid::fetchTexelWorldSpace(glm::vec4 pos_ws) {
	glm::vec4 pos_gs = this->transform_worldToGrid * pos_ws;
	return this->fetchTexelGridSpace(pos_gs);
}

DiscreteGrid::DataType DiscreteGrid::fetchTexelIndex(sizevec3 idx) {
	// if bigger in any dimension, then return nothing :
	if (idx.x > this->gridDimensions.x || idx.y > this->gridDimensions.y || idx.z > this->gridDimensions.z) { return DataType(0); }

	std::size_t index = idx.x + idx.y * this->gridDimensions.x + idx.z * this->gridDimensions.x * this->gridDimensions.y;
	// sanity check, should be covered by the case above :
	if (this->data.size() < index) { return DataType(0); }
	// return data at this index :
	else { return this->data[index]; }
}

const std::vector<DiscreteGrid::DataType>& DiscreteGrid::getData() const {
	return this->data;
}

const glm::mat4& DiscreteGrid::getTransform_WorldToGrid() const {
	return this->transform_worldToGrid;
}

const glm::mat4& DiscreteGrid::getTransform_GridToWorld() const {
	return this->transform_gridToWorld;
}

const DiscreteGrid::sizevec3& DiscreteGrid::getGridDimensions() const {
	return this->gridDimensions;
}

const glm::vec3 DiscreteGrid::getVoxelDimensions() const {
	return this->voxelDimensions;
}

const DiscreteGrid::bbox_t& DiscreteGrid::getBoundingBox() const {
	return this->boundingBox;
}

const DiscreteGrid::bbox_t& DiscreteGrid::getDataBoundingBox() const {
	return this->dataBoundingBox;
}

DiscreteGrid::bbox_t DiscreteGrid::getBoundingBoxWorldSpace() const {
	return this->boundingBox.transformTo(this->transform_gridToWorld);
}

bool DiscreteGrid::isModifiable() const {
	return this->modifiable;
}

glm::vec4 DiscreteGrid::getVoxelPositionGridSpace(sizevec3 idx) {
	// origin of the grid (min BB position) :
	bbox_t::vec m = this->boundingBox.getMin();
	glm::vec4 minBBpos = glm::vec4(static_cast<float>(m.x), static_cast<float>(m.y), static_cast<float>(m.z), 0.f);
	// displacement for half a voxel :
	glm::vec4 halfVoxel = glm::vec4(this->voxelDimensions.x / 2.f, this->voxelDimensions.y / 2.f, this->voxelDimensions.z / 2.f, .0f);
	// voxel position, in grid space :
	glm::vec4 voxelPos = glm::vec4(
		static_cast<float>(idx.x) * this->voxelDimensions.x,
		static_cast<float>(idx.y) * this->voxelDimensions.y,
		static_cast<float>(idx.z) * this->voxelDimensions.z,
		1.f
	);
	return minBBpos + voxelPos + halfVoxel;
}

glm::vec4 DiscreteGrid::getVoxelPositionWorldSpace(sizevec3 idx){
	return this->toWorldSpace(this->getVoxelPositionGridSpace(idx));
}

DiscreteGrid& DiscreteGrid::setData(std::vector<DataType> &_data) {
	this->data.clear();
	this->data.resize(_data.size());
	// Copy data :
	std::copy(_data.begin(), _data.end(), this->data.begin());
	return *this;
}

DiscreteGrid& DiscreteGrid::setModifiable(bool _mod) {
	this->modifiable = _mod;
	return *this;
}

DiscreteGrid& DiscreteGrid::setResolution(sizevec3 dims) {
	if (not this->modifiable) { return *this; }
	this->gridDimensions = dims;
	this->updateVoxelDimensions();
	return *this;
}

DiscreteGrid& DiscreteGrid::setBoundingBox(bbox_t renderWindow) {
	if (not this->modifiable) { return *this; }
	// assumes the bbox given is in world space
	this->boundingBox = renderWindow;
	// update voxel dimensions :
	this->updateVoxelDimensions();
	// update the data bb :
	this->recomputeBoundingBox(this->dataThreshold);
	return *this;
}

DiscreteGrid& DiscreteGrid::updateBoundingBox(bbox_t renderWindow) {
	if (not this->modifiable) { return *this; }
	// assumes the bbox given is in world space. update :
	std::vector<bbox_t::vec> cornersBB = renderWindow.getAllCorners();
	this->boundingBox.addPoints(cornersBB);
	// update voxel dimensions :
	this->updateVoxelDimensions();
	// update the data bb :
	this->recomputeBoundingBox(this->dataThreshold);
	return *this;
}

DiscreteGrid& DiscreteGrid::setTransform_WorldToGrid(glm::mat4 _w2g) {
	this->transform_worldToGrid = glm::mat4(_w2g);
	this->transform_gridToWorld = glm::inverse(this->transform_worldToGrid);
	return *this;
}

DiscreteGrid& DiscreteGrid::setTransform_GridToWorld(glm::mat4 _g2w) {
	this->transform_gridToWorld = glm::mat4(_g2w);
	this->transform_worldToGrid = glm::inverse(this->transform_gridToWorld);
	return *this;
}

DiscreteGrid& DiscreteGrid::setGridName(std::string name) {
	this->gridName = name;
	return *this;
}

const std::string& DiscreteGrid::getGridName(void) const {
	return this->gridName;
}

void DiscreteGrid::updateVoxelDimensions() {
	// if the resolution hasn't been set, return to prevent NaNs :
	if (this->gridDimensions.x == 0u || this->gridDimensions.y == 0u || this->gridDimensions.z == 0u) { return; }
	// voxel dimensions along each axis should be BBlength / resolution :
	glm::vec3 diag = this->boundingBox.getDiagonal();

	//this->boundingBox.printInfo("Updating voxel dimensions ...");
	this->voxelDimensions.x = diag.x / static_cast<float>(this->gridDimensions.x);
	this->voxelDimensions.y = diag.y / static_cast<float>(this->gridDimensions.y);
	this->voxelDimensions.z = diag.z / static_cast<float>(this->gridDimensions.z);
	return;
}
