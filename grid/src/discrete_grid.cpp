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
	this->gridName = "defaultGridName";
	this->data.clear();
	this->dataBoundingBox = bbox_t();
	this->dataThreshold = DataType(0);
	this->gridDimensions = sizevec3(0);
	this->voxelDimensions = glm::vec3(1.f);
#ifdef ENABLE_TRANSFORMATIONS
	this->transform_gridToWorld = glm::mat4(1.f);
	this->transform_worldToGrid = glm::mat4(1.f);
#endif
#ifdef ENABLE_BASIC_BB
	this->boundingBox = bbox_t();
#endif
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

	this->setTransform_GridToWorld(reader.getTransform());
	this->dataBoundingBox = reader.getDataBoundingBox();
	this->dataThreshold = reader.getDataThreshold();
	this->gridDimensions = reader.getGridDimensions();
	this->voxelDimensions = reader.getVoxelDimensions();
#ifdef ENABLE_BASIC_BB
	this->boundingBox = reader.getBoundingBox();
#endif

	std::size_t gridsize = this->gridDimensions.x * this->gridDimensions.y * this->gridDimensions.z;
	this->data.resize(gridsize);
	// copy data :
	reader.swapData(this->data);

	//this->printInfo("[DiscreteGrid::fromReader()]", "[INFO]");

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

glm::vec4 DiscreteGrid::toGridSpace(glm::vec4 pos_ws) const {
#ifdef ENABLE_TRANSFORMATIONS
	return this->transform_worldToGrid * pos_ws;
#else
	return pos_ws;
#endif
}

glm::vec4 DiscreteGrid::toWorldSpace(glm::vec4 pos_gs) const {
#ifdef ENABLE_TRANSFORMATIONS
	return this->transform_gridToWorld * pos_gs;
#else
	return pos_gs;
#endif
}

DiscreteGrid::DataType DiscreteGrid::fetchTexelGridSpace(glm::vec4 pos_gs) const {
#ifdef ENABLE_BB_TRANSFORM
	if (not this->boundingBox.contains(pos_gs)) { return DataType(0); }
#endif

#ifdef ADJUST_TO_BB
	DiscreteGrid::bbox_t::vec minBB = this->boundingBox.getMin();
#else
	DiscreteGrid::bbox_t::vec minBB = DiscreteGrid::bbox_t::vec(DiscreteGrid::bbox_t::vec::value_type(0));
#endif
	// compute index of position :
	std::size_t x = static_cast<std::size_t>(std::floor((pos_gs.x - minBB.x) / this->voxelDimensions.x));
	std::size_t y = static_cast<std::size_t>(std::floor((pos_gs.y - minBB.y) / this->voxelDimensions.y));
	std::size_t z = static_cast<std::size_t>(std::floor((pos_gs.z - minBB.z) / this->voxelDimensions.z));
	std::cerr << "[TRACE] PosFT : [" << x << ", " << y << ", " << z << "]\n";
	return this->fetchTexelIndex(sizevec3(x,y,z));
}

DiscreteGrid::DataType DiscreteGrid::fetchTexelWorldSpace(glm::vec4 pos_ws) const {
#ifdef ENABLE_TRANSFORMATIONS
	glm::vec4 pos_gs = this->transform_worldToGrid * pos_ws;
#else
	glm::vec4 pos_gs = pos_ws;
#endif
	return this->fetchTexelGridSpace(pos_gs);
}

DiscreteGrid::DataType DiscreteGrid::fetchTexelIndex(sizevec3 idx) const {
	std::size_t index = idx.x + idx.y * this->gridDimensions.x + idx.z * this->gridDimensions.x * this->gridDimensions.y;
	// sanity check, should be covered by the case above :
	if (this->data.size() < index) {
		std::cerr << "[TRACE] Targt : [idx:" << index << ", val:" << +0 << ']' << '\n';
		return DataType(0); }
	// return data at this index :
	else {
		std::cerr << "[TRACE] Targt : [idx:" << index << ", val:" << +this->data[index] << "]\n";
		std::cerr << "[TRACE] Targt : [" << this->gridDimensions.x << ", " << this->gridDimensions.y << ", " << this->gridDimensions.z << "]\n\n";
		return this->data[index]; }
}

const std::vector<DiscreteGrid::DataType>& DiscreteGrid::getData() const {
	return this->data;
}

const glm::mat4 DiscreteGrid::getTransform_WorldToGrid() const {
#ifdef ENABLE_TRANSFORMATIONS
	return this->transform_worldToGrid;
#else
	return glm::mat4(1.f);
#endif
}

const glm::mat4 DiscreteGrid::getTransform_GridToWorld() const {
#ifdef ENABLE_TRANSFORMATIONS
	return this->transform_gridToWorld;
#else
	return glm::mat4(1.f);
#endif
}

const DiscreteGrid::sizevec3& DiscreteGrid::getGridDimensions() const {
	return this->gridDimensions;
}

const glm::vec3 DiscreteGrid::getVoxelDimensions() const {
	return this->voxelDimensions;
}

#ifdef ENABLE_BASIC_BB
const DiscreteGrid::bbox_t& DiscreteGrid::getBoundingBox() const {
	return this->boundingBox;
}

DiscreteGrid& DiscreteGrid::setBoundingBox(bbox_t renderWindow) {
	if (not this->modifiable) { return *this; }
	// assumes the bbox given is in world space
	this->boundingBox = renderWindow;
	// update voxel dimensions :
	this->updateVoxelDimensions();
	return *this;
}

#ifdef ENABLE_BB_TRANSFORM
DiscreteGrid::bbox_t DiscreteGrid::getBoundingBoxWorldSpace() const {
	return this->boundingBox.transformTo(this->transform_gridToWorld);
}

DiscreteGrid& DiscreteGrid::updateBoundingBox(bbox_t renderWindow) {
	if (not this->modifiable) { return *this; }
	// assumes the bbox given is in world space. update :
	std::vector<bbox_t::vec> cornersBB = renderWindow.getAllCorners();
	this->boundingBox.addPoints(cornersBB);
	// update voxel dimensions :
	this->updateVoxelDimensions();
	return *this;
}
#endif
#endif

const DiscreteGrid::bbox_t& DiscreteGrid::getDataBoundingBox() const {
	return this->dataBoundingBox;
}

bool DiscreteGrid::isModifiable() const {
	return this->modifiable;
}

glm::vec4 DiscreteGrid::getVoxelPositionGridSpace(sizevec3 idx, bool verbose) {
#ifdef ADJUST_TO_BB
	// origin of the grid (min BB position) :
	bbox_t::vec m = this->boundingBox.getMin();
	glm::vec4 minBBpos = glm::vec4(static_cast<float>(m.x), static_cast<float>(m.y), static_cast<float>(m.z), float(0.f));
#endif
	// displacement for half a voxel :
	glm::vec4 halfVoxel = glm::vec4(this->voxelDimensions.x / 2.f, this->voxelDimensions.y / 2.f, this->voxelDimensions.z / 2.f, .0f);
	// voxel position, in grid space :
	glm::vec4 voxelPos = glm::vec4(
		static_cast<float>(idx.x) * this->voxelDimensions.x,
		static_cast<float>(idx.y) * this->voxelDimensions.y,
		static_cast<float>(idx.z) * this->voxelDimensions.z,
		1.f
	);
#ifdef ADJUST_TO_BB
	glm::vec4 finalPos = minBBpos + voxelPos + halfVoxel;
#else
	glm::vec4 finalPos = voxelPos + halfVoxel;
#endif

	if (verbose) {
		glm::vec4 worldPos = this->toWorldSpace(voxelPos);
		std::cerr << "[INFO] Output grid " << this->gridName << " : requesting voxel [" <<
			     idx.x << ", " << idx.y << ", " << idx.z << "].\n";
#ifdef ADJUST_TO_BB
		bbox_t::vec mm = this->boundingBox.getMax();
		std::cerr << "[INFO]\tBounding box from [" << m.x << ", " << m.y << ", " << m.z << "] to [" <<
			     mm.x << ", " << mm.y << ", " << mm.z << "].\n";
#endif
		std::cerr << "[INFO]\tHalf-voxel dimension : {" << halfVoxel.x << ", " << halfVoxel.y <<
			     ", " << halfVoxel.z << "]\n";
		std::cerr << "[INFO]\tVoxel dimensions : \t{" << this->voxelDimensions.x << ", " <<
			     this->voxelDimensions.y << ", " << this->voxelDimensions.z << "]\n";
		std::cerr << "[INFO]\tVoxel position : \t[" << voxelPos.x << ", " << voxelPos.y <<
			     ", " << voxelPos.z << "]\n";
		std::cerr << "[INFO]\tIn World Space : \t[" << worldPos.x << ", " << worldPos.y <<
			     ", " << worldPos.z << "]\n";
		std::cerr << "[INFO]\tFinal position given : [" << finalPos.x << ", " << finalPos.y <<
			     ", " << finalPos.z << "]\n";
	}

	return finalPos;
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

DiscreteGrid& DiscreteGrid::setTransform_WorldToGrid(glm::mat4 _w2g) {
#ifdef ENABLE_TRANSFORMATIONS
#ifndef REVERSE_MATRIX_ORDER
	this->transform_worldToGrid = glm::mat4(_w2g);
	this->transform_gridToWorld = glm::inverse(this->transform_worldToGrid);
#else
	this->transform_gridToWorld = glm::mat4(_w2g);
	this->transform_worldToGrid = glm::inverse(this->transform_gridToWorld);
#endif
#endif
	return *this;

}

DiscreteGrid& DiscreteGrid::setTransform_GridToWorld(glm::mat4 _g2w) {
#ifdef ENABLE_TRANSFORMATIONS
#ifndef REVERSE_MATRIX_ORDER
	this->transform_gridToWorld = glm::mat4(_g2w);
	this->transform_worldToGrid = glm::inverse(_g2w);
#else
	this->transform_worldToGrid = glm::mat4(_g2w);
	this->transform_gridToWorld = glm::inverse(_g2w);
#endif
#endif
	return *this;
}

DiscreteGrid& DiscreteGrid::setGridName(std::string name) {
	this->gridName = name;
	return *this;
}

const std::string& DiscreteGrid::getGridName(void) const {
	return this->gridName;
}

bool DiscreteGrid::includesPointWorldSpace(glm::vec4 point) const {
	glm::vec4 point_gs = this->toGridSpace(point);
	return this->includesPointGridSpace(point_gs);
}

bool DiscreteGrid::includesPointGridSpace(glm::vec4 point) const {
#ifdef ADJUST_TO_BB
	using val_t = bbox_t::vec::value_type;
	bbox_t::vec point_bb = bbox_t::vec(static_cast<val_t>(point.x), static_cast<val_t>(point.y), static_cast<val_t>(point.z));
	return this->boundingBox.contains(point_bb);
#else
	return true;
#endif
}

void DiscreteGrid::updateVoxelDimensions() {
	// if the resolution hasn't been set, return to prevent NaNs :
	if (this->gridDimensions.x == 0u || this->gridDimensions.y == 0u || this->gridDimensions.z == 0u) { return; }
	// voxel dimensions along each axis should be BBlength / resolution :
#ifdef ENABLE_BASIC_BB
	glm::vec3 diag = this->boundingBox.getDiagonal();

	//this->boundingBox.printInfo("Updating voxel dimensions ...");
	this->voxelDimensions.x = diag.x / static_cast<float>(this->gridDimensions.x);
	this->voxelDimensions.y = diag.y / static_cast<float>(this->gridDimensions.y);
	this->voxelDimensions.z = diag.z / static_cast<float>(this->gridDimensions.z);

	std::cerr << "[LOG] Resolution after update : [" << this->gridDimensions.x << ',' << this->gridDimensions.y << ',' << this->gridDimensions.z << "]\n";
	std::cerr << "[LOG] Voxel dims after update : [" << this->voxelDimensions.x << ',' << this->voxelDimensions.y << ',' << this->voxelDimensions.z << "]\n";
#else
	this->voxelDimensions = glm::vec3(1.f, 1.f, 1.f);
#endif
	return;
}

void DiscreteGrid::printInfo(std::string message, std::string prefix) {
	if (message.length() > 0) {
		std::cerr << prefix << message << '\n';
	}
	std::cerr << prefix << '\t' << "Name : " << this->gridName << '\n';
	std::cerr << prefix << "\tData threshold is " << +this->dataThreshold << '\n';
	std::cerr << prefix << "\tGrid size is [" <<
			this->gridDimensions.x << ", " <<
			this->gridDimensions.y << ", " <<
			this->gridDimensions.z << "]\n";
	std::cerr << prefix << "\tVoxel dimensions are [" <<
			this->voxelDimensions.x << ", " <<
			this->voxelDimensions.y << ", " <<
			this->voxelDimensions.z << "]\n";
	std::cerr << prefix << "\tData BB is [" <<
			this->dataBoundingBox.getMin().x << ", " <<
			this->dataBoundingBox.getMin().y << ", " <<
			this->dataBoundingBox.getMin().z << "] to [" <<
			this->dataBoundingBox.getMax().x << ", " <<
			this->dataBoundingBox.getMax().y << ", " <<
			this->dataBoundingBox.getMax().z << "]\n";
#ifdef ENABLE_BASIC_BB
	std::cerr << prefix << "\tGrid BB is [" <<
			this->boundingBox.getMin().x << ", " <<
			this->boundingBox.getMin().y << ", " <<
			this->boundingBox.getMin().z << "] to [" <<
			this->boundingBox.getMax().x << ", " <<
			this->boundingBox.getMax().y << ", " <<
			this->boundingBox.getMax().z << "]\n";
#endif
}
