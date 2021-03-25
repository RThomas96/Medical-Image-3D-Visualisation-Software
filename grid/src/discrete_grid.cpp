#include "../include/discrete_grid.hpp"

#include <glm/ext/matrix_transform.hpp>

glm::mat4 computeTransfoShear(double angleDeg, const std::shared_ptr<DiscreteGrid>& grid, glm::vec3 vxdims) {
	glm::mat4 transfoMat = glm::mat4(1.0);

	double angleRad = (angleDeg * M_PI) / 180.;

	transfoMat[0][0] = /* vxdims.x */ std::cos(angleRad);
	transfoMat[0][2] = /* vxdims.x */ std::sin(angleRad);
	transfoMat[1][1] = /* vxdims.y */ 1.f;
	transfoMat[2][2] = /* vxdims.z */ std::cos(angleRad);

	if (angleDeg < 0.) {
		auto dims = grid->getBoundingBox().getDiagonal();
		// compute translation along Z :
		float w = static_cast<float>(dims.x)/* vxdims.x */;
		float displacement = w * std::abs(std::sin(angleRad));
		transfoMat = glm::translate(transfoMat, glm::vec3(.0, .0, displacement));
	}

	return transfoMat;
}

DiscreteGrid::DiscreteGrid(bool _modifiable) {
	this->modifiable = _modifiable;
	this->gridName = "defaultGridName";
	this->data.clear();
	this->dataThreshold = DataType(0);
	this->gridDimensions = sizevec3(0);
	this->voxelDimensions = glm::vec3(1.f);
	this->boundingBox = bbox_t();
	this->transform_gridToWorld = glm::mat4(1.f);
	this->transform_worldToGrid = glm::mat4(1.f);
	this->dataBoundingBox = bbox_t();
	this->gridReader = nullptr;
	this->gridWriter = nullptr;
	this->isOffline = false;
}

DiscreteGrid::DiscreteGrid(std::shared_ptr<IO::GenericGridReader> reader) : DiscreteGrid() {
	this->gridReader = reader;
	this->fromGridReader();
}

DiscreteGrid::~DiscreteGrid(void) {
	this->data.clear();
}

DiscreteGrid& DiscreteGrid::setOffline(bool off) {
	this->isOffline = off;
	return *this;
}

DiscreteGrid& DiscreteGrid::fromGridReader() {
	if (this->gridReader == nullptr) { return *this; }
	// Updates this grid's data, bypassing the 'state'isModifiable' check.
	this->data.clear();

	this->setTransform_GridToWorld(this->gridReader->getTransform());
	this->dataThreshold = this->gridReader->getDataThreshold();
	this->gridDimensions = this->gridReader->getGridDimensions();
	this->voxelDimensions = this->gridReader->getVoxelDimensions();
	this->boundingBox = this->gridReader->getBoundingBox();
	this->dataBoundingBox = this->gridReader->getDataBoundingBox();
	this->setFilenames(this->gridReader->getFilenames());

	// get data from reader :
	this->gridReader->swapData(this->data);

	return *this;
}

DiscreteGrid& DiscreteGrid::setGridReader(std::shared_ptr<IO::GenericGridReader> reader) {
	this->gridReader = reader;
	return *this;
}

DiscreteGrid& DiscreteGrid::setGridWriter(std::shared_ptr<IO::GenericGridWriter> writer) {
	this->gridWriter = writer;
	this->gridWriter->setGrid(this->shared_from_this());
	return *this;
}

std::shared_ptr<IO::GenericGridReader> DiscreteGrid::getGridReader(void) const { return this->gridReader; }
std::shared_ptr<IO::GenericGridWriter> DiscreteGrid::getGridWriter(void) const { return this->gridWriter; }

glm::uvec3 DiscreteGrid::worldPositionToIndex(glm::vec4 p) const {
	// Get position in grid space first, then compute the index position using
	// voxel sizes.
	glm::vec4 gp = this->toGridSpace(p);
	// rgp = relative grid position
	glm::vec4 rgp = gp - glm::vec4(this->boundingBox.getMin(), 1.);
	glm::vec4 vx = glm::vec4(this->getVoxelDimensions(), 1.);
	unsigned int x = static_cast<unsigned int>(rgp.x/vx.x);
	unsigned int y = static_cast<unsigned int>(rgp.y/vx.y);
	unsigned int z = static_cast<unsigned int>(rgp.z/vx.z);
	return glm::uvec3(x,y,z);
}

glm::vec4 DiscreteGrid::toGridSpace(glm::vec4 pos_ws) const {
	return this->transform_worldToGrid * pos_ws;
}

glm::vec4 DiscreteGrid::toWorldSpace(glm::vec4 pos_gs) const {
	return this->transform_gridToWorld * pos_gs;
}

DiscreteGrid::DataType DiscreteGrid::fetchTexelWorldSpace(glm::vec4 pos_ws, bool verbose) const {
	if (verbose) { std::cerr << "texelWorldSpace() {" << pos_ws.x << ',' << pos_ws.y << ',' << pos_ws.z << ',' << pos_ws.a << "} ... "; }
	glm::vec4 pos_gs = this->toGridSpace(pos_ws);
	return this->fetchTexelGridSpace(pos_gs, verbose);
}

DiscreteGrid::DataType DiscreteGrid::fetchTexelGridSpace(glm::vec4 pos_gs, bool verbose) const {
	if (verbose) { std::cerr << "texelGridSpace() {" << pos_gs.x << ',' << pos_gs.y << ',' << pos_gs.z << ',' << pos_gs.a << "} ... "; }

	using val_t = bbox_t::vec::value_type;
	bbox_t::vec point_bb = bbox_t::vec(static_cast<val_t>(pos_gs.x), static_cast<val_t>(pos_gs.y), static_cast<val_t>(pos_gs.z));
	if (not this->boundingBox.contains(pos_gs)) { if (verbose) { std::cerr << "not contained in BB\n"; } return DataType(0); }

	DiscreteGrid::bbox_t::vec minBB = this->boundingBox.getMin();

	/// If the grid is offline, we don't want to divide the indices (can be divided in case the grid was downsampled)
	glm::vec3 vxDiv = this->getVoxelDimensions();

	// compute index of position :
	std::size_t x = static_cast<std::size_t>(std::floor((pos_gs.x - minBB.x) / vxDiv.x));
	std::size_t y = static_cast<std::size_t>(std::floor((pos_gs.y - minBB.y) / vxDiv.y));
	std::size_t z = static_cast<std::size_t>(std::floor((pos_gs.z - minBB.z) / vxDiv.z));
	if (verbose) { std::cerr << "index is [" << x << ',' << y << ',' << z << "] ... "; }
	// fetch from grid :
	return this->fetchTexelIndex(sizevec3(x,y,z), verbose);
}

DiscreteGrid::DataType DiscreteGrid::fetchTexelIndex(sizevec3 idx, bool verbose) const {
	DiscreteGrid::DataType d = this->getPixel(idx.x, idx.y, idx.z);
	if (verbose) { std::cerr << " value is " << +d << '\n'; }
	return d;
}

DiscreteGrid::DataType DiscreteGrid::getPixel(std::size_t x, std::size_t y, std::size_t z) const {
	std::size_t index = x + y * this->gridDimensions.x + z * this->gridDimensions.x * this->gridDimensions.y;
	if (x >= this->gridDimensions.x || y >= this->gridDimensions.y || z >= this->gridDimensions.z) {
		return DataType(0);
	}

	if (this->isOffline && this->gridReader->downsamplingLevel() != IO::DownsamplingLevel::Original) {
		#warning Fault is here !!! Need to get the real coordinates, not the downsampled ones !
		return this->gridReader->getPixel(x,y,z);
	} else {
		// sanity check, should be covered by the cases above :
		if (this->data.size() < index) { return DataType(0); }

		// return data at this index :
		else { return this->data[index]; }
	}
}

DiscreteGrid& DiscreteGrid::setPixel(std::size_t x, std::size_t y, std::size_t z, DataType value) {
	// early check :
	if (this->modifiable == false) { return *this; }

	// Check the dimensions are within spec :
	if (x >= this->gridDimensions.x || y >= this->gridDimensions.y || z >= this->gridDimensions.z) {
		return *this;
	}

	// change value :
	std::size_t idx = x + y * this->gridDimensions.x + z * this->gridDimensions.x * this->gridDimensions.y;
	this->data[idx] = value;

	return *this;
}

bool DiscreteGrid::hasData() const {
	// if offline, we MAY have a buffer but this doesn't count as data.
	if (this->isOffline) { return false; }
	// Otherwise, return if the data vector has been filled.
	return (this->data.size() != 0);
}

const DiscreteGrid::DataType* DiscreteGrid::getDataPtr() const {
	if (this->isOffline) { return nullptr; }
	return this->data.data();
}

const glm::mat4 DiscreteGrid::getTransform_WorldToGrid() const {
	return this->transform_worldToGrid;
}

const glm::mat4 DiscreteGrid::getTransform_GridToWorld() const {
	return this->transform_gridToWorld;
}

const DiscreteGrid::sizevec3& DiscreteGrid::getResolution() const {
	return this->gridDimensions;
}

const glm::vec3 DiscreteGrid::getVoxelDimensions() const {
	return this->voxelDimensions;
}

const DiscreteGrid::bbox_t& DiscreteGrid::getBoundingBox() const {
	return this->boundingBox;
}

DiscreteGrid& DiscreteGrid::setBoundingBox(bbox_t renderWindow) {
	if (not this->modifiable) { return *this; }
	// assumes the bbox given is in grid space :
	this->boundingBox = renderWindow;
	// update voxel dimensions :
	this->updateVoxelDimensions();
	return *this;
}

DiscreteGrid::bbox_t DiscreteGrid::getBoundingBoxWorldSpace() const {
	return this->boundingBox.transformTo(this->transform_gridToWorld);
}

DiscreteGrid::bbox_t DiscreteGrid::getDataBoundingBoxWorldSpace() const {
	return this->dataBoundingBox.transformTo(this->transform_gridToWorld);
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

DiscreteGrid& DiscreteGrid::recomputeBoundingBox(DataType threshold) {
	// Early return for empty grids :
	if (this->data.size() == 0) { return *this; };
	using val_t = DiscreteGrid::bbox_t::vec::value_type;

	DiscreteGrid::bbox_t newBB;
	std::cout << "Updating the grid \"" << this->gridName << "\" with a new threshold of " << +threshold << '\n';
	for (std::size_t z = 0; z < this->gridDimensions.z; ++z) {
		for (std::size_t y = 0; y < this->gridDimensions.y; ++y) {
			for (std::size_t x = 0; x < this->gridDimensions.x; ++x) {
				if (this->data[x+y*this->gridDimensions.x+z*this->gridDimensions.x*this->gridDimensions.y] >= threshold) {
					DiscreteGrid::bbox_t::vec v(static_cast<val_t>(x), static_cast<val_t>(y), static_cast<val_t>(z));
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

const DiscreteGrid::bbox_t& DiscreteGrid::getDataBoundingBox() const {
	return this->dataBoundingBox;
}

bool DiscreteGrid::isModifiable() const {
	return this->modifiable;
}

glm::vec4 DiscreteGrid::getVoxelPositionWorldSpace(sizevec3 idx){
	return this->toWorldSpace(this->getVoxelPositionGridSpace(idx));
}

glm::vec4 DiscreteGrid::getVoxelPositionGridSpace(sizevec3 idx, bool verbose) {
	// always get the 'real' voxel size (might need to be multiplied first)
	glm::vec3 vxdims = this->getVoxelDimensions();
	// displacement for half a voxel :
	glm::vec4 halfVoxel = glm::vec4(vxdims.x / 2.f, vxdims.y / 2.f, vxdims.z / 2.f, .0f);
	// voxel position, in grid space :
	glm::vec4 voxelPos = glm::vec4(
		static_cast<float>(idx.x) * vxdims.x,
		static_cast<float>(idx.y) * vxdims.y,
		static_cast<float>(idx.z) * vxdims.z,
		1.f
	);
	// origin of the grid (min BB position) :
	bbox_t::vec m = this->boundingBox.getMin();
	glm::vec4 minBBpos = glm::vec4(static_cast<float>(m.x), static_cast<float>(m.y), static_cast<float>(m.z), float(0.f));
	glm::vec4 finalPos = minBBpos + voxelPos + halfVoxel;

	if (verbose) {
		glm::vec4 worldPos = this->toWorldSpace(voxelPos);
		std::cerr << "[INFO] Output grid " << this->gridName << " : requesting voxel [" << idx.x << ", " << idx.y << ", " << idx.z << "].\n";
		bbox_t::vec mm = this->boundingBox.getMax();
		std::cerr << "[INFO]\tBounding box from [" << m.x << ", " << m.y << ", " << m.z << "] to [" << mm.x << ", " << mm.y << ", " << mm.z << "].\n";
		std::cerr << "[INFO]\tHalf-voxel dimension : {" << halfVoxel.x << ", " << halfVoxel.y << ", " << halfVoxel.z << "]\n";
		std::cerr << "[INFO]\tVoxel dimensions : \t{" << this->voxelDimensions.x << ", " << this->voxelDimensions.y << ", " << this->voxelDimensions.z << "]\n";
		std::cerr << "[INFO]\tVoxel position : \t[" << voxelPos.x << ", " << voxelPos.y << ", " << voxelPos.z << "]\n";
		std::cerr << "[INFO]\tIn World Space : \t[" << worldPos.x << ", " << worldPos.y << ", " << worldPos.z << "]\n";
		std::cerr << "[INFO]\tFinal position given : [" << finalPos.x << ", " << finalPos.y << ", " << finalPos.z << "]\n";
	}

	return finalPos;
}

DiscreteGrid& DiscreteGrid::setData(std::vector<DataType> &_data) {
	this->data.clear();
	this->data.resize(_data.size());
	#warning To change once an interface to data has been implemented
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

DiscreteGrid& DiscreteGrid::setVoxelDimensions(glm::vec3 _vxdims) {
	if (not this->modifiable) { return *this; }

	// set new voxel dimensions :
	this->voxelDimensions = _vxdims;

	// update the resolution to be the most possible with the current bounding box :
	bbox_t::vec diag = this->boundingBox.getDiagonal();
	sizevec3 newRes = glm::convert_to<size_t>(glm::round(diag / this->voxelDimensions));
	this->gridDimensions = newRes;

	// update BB : size will be recomputed, minimum stays the same
	bbox_t::vec newBBMax = glm::convert_to<float>(newRes) * this->voxelDimensions + this->boundingBox.getMin();
	bbox_t newBB(this->boundingBox.getMin(), newBBMax);
	this->boundingBox = newBB;

	// return reference to this
	return *this;
}

DiscreteGrid& DiscreteGrid::setTransform_WorldToGrid(glm::mat4 _w2g) {
	this->transform_worldToGrid = glm::mat4(_w2g);
	this->transform_gridToWorld = glm::inverse(_w2g);
	return *this;

}

DiscreteGrid& DiscreteGrid::setTransform_GridToWorld(glm::mat4 _g2w) {
	this->transform_gridToWorld = glm::mat4(_g2w);
	this->transform_worldToGrid = glm::inverse(_g2w);
	return *this;
}

DiscreteGrid& DiscreteGrid::setGridName(std::string name) {
	this->gridName = name;
	return *this;
}

const std::string& DiscreteGrid::getGridName(void) const {
	return this->gridName;
}

DiscreteGrid& DiscreteGrid::setFilenames(std::vector<std::string> fnames) {
	this->filenames = std::vector<std::string>(fnames);
	return *this;
}

const std::vector<std::string>& DiscreteGrid::getFilenames() const {
	return this->filenames;
}

bool DiscreteGrid::includesPointWorldSpace(glm::vec4 point, bool verbose) const {
	glm::vec4 point_gs = this->toGridSpace(point);
	if (verbose) { std::cerr << "[LOG]\t\tPoint submitted was {" << point.x << "," << point.y << ',' << point.z << "} ... "; }
	return this->includesPointGridSpace(point_gs, verbose);
}

bool DiscreteGrid::includesPointGridSpace(glm::vec4 point, bool verbose) const {
	using val_t = bbox_t::vec::value_type;
	bbox_t::vec point_bb = bbox_t::vec(static_cast<val_t>(point.x), static_cast<val_t>(point.y), static_cast<val_t>(point.z));
	bool b = this->boundingBox.contains(point_bb);
	if (verbose) {
		std::cerr << "grid space {" << point.x << "," << point.y << ',' << point.z << "} ... ";
		std::cerr << "bb space {" << point_bb.x << "," << point_bb.y << ',' << point_bb.z << "} ... ";
		std::cerr << "contained ? " << std::boolalpha << b << std::noboolalpha << '\n';
	}
	return b;
}

void DiscreteGrid::updateVoxelDimensions() {
	// if the resolution hasn't been set, return to prevent NaNs :
	if (this->gridDimensions.x == 0u || this->gridDimensions.y == 0u || this->gridDimensions.z == 0u) { return; }

	// voxel dimensions along each axis should be BBlength / resolution :
	glm::vec3 diag = this->boundingBox.getDiagonal();

	this->voxelDimensions.x = diag.x / static_cast<float>(this->gridDimensions.x);
	this->voxelDimensions.y = diag.y / static_cast<float>(this->gridDimensions.y);
	this->voxelDimensions.z = diag.z / static_cast<float>(this->gridDimensions.z);

	return;
}

void DiscreteGrid::printInfo(std::string message, std::string prefix) {
	if (message.length() > 0) {
		std::cerr << prefix << message << '\n';
	}
	std::cerr << prefix << "\tName : " << this->gridName << '\n';
	std::cerr << prefix << "\tData threshold is " << +this->dataThreshold << '\n';
	std::cerr << prefix << "\tGrid size is [" <<
			this->gridDimensions.x << ", " <<
			this->gridDimensions.y << ", " <<
			this->gridDimensions.z << "]\n";
	std::cerr << prefix << "\tVoxel dimensions are [" <<
			this->voxelDimensions.x << ", " <<
			this->voxelDimensions.y << ", " <<
			this->voxelDimensions.z << "]\n";
	std::cerr << prefix << "\tGrid BB is \n\t\t{" <<
			this->boundingBox.getMin().x << ", " <<
			this->boundingBox.getMin().y << ", " <<
			this->boundingBox.getMin().z << "} to\n\t\t{" <<
			this->boundingBox.getMax().x << ", " <<
			this->boundingBox.getMax().y << ", " <<
			this->boundingBox.getMax().z << "}\n";
	std::cerr << prefix << "\tData BB is \n\t\t{" <<
			this->dataBoundingBox.getMin().x << ", " <<
			this->dataBoundingBox.getMin().y << ", " <<
			this->dataBoundingBox.getMin().z << "} to\n\t\t{" <<
			this->dataBoundingBox.getMax().x << ", " <<
			this->dataBoundingBox.getMax().y << ", " <<
			this->dataBoundingBox.getMax().z << "}\n";
}
