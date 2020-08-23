#include "../include/tetmesh.hpp"

TetMesh::TetMesh() {
	// Initialises all the values of the mesh to their default values.
	// Centers the mesh around the space's origin. Also, makes the mesh
	// by a call to TetMesh::makeTetrahedra().
	this->vertices.clear();
	this->tetrahedra.clear();
	this->origin = glm::vec4(.0, .0, .0, .0);
	this->inputGrids.clear();
	this->outputGrid = nullptr;

	this->makeTetrahedra();
}

TetMesh::~TetMesh() {
	// Free all storage allocated for the current mesh.
	this->vertices.clear();
	this->tetrahedra.clear();
}

TetMesh& TetMesh::addInputGrid(const std::shared_ptr<InputGrid>& toAdd) {
	this->inputGrids.push_back(toAdd);
	this->updateOutputGridData();
	return *this;
}

TetMesh& TetMesh::setOutputGrid(const std::shared_ptr<OutputGrid>& toSet) {
	this->outputGrid = toSet;
	this->updateVoxelSizes();
	this->updateOutputGridData();
	return *this;
}

TetMesh& TetMesh::resetPositions() {
	// Resets the positions so the mesh is centered around the origin.
	for (std::size_t i = 0; i < this->vertices.size(); ++i) {
		this->vertices[i] -= this->origin;
		this->vertices[i].w = 1.; // Sanity check.
	}
	return *this;
}

TetMesh& TetMesh::populateOutputGrid(InterpolationMethods method) {
	// early returns :
	if (this->outputGrid == nullptr) { return *this; }
	if (this->inputGrids.size() == 0) { return *this; }

	this->updateVoxelSizes();

	// Check the dimensions of the voxel grid (if it can host voxels) :
	DiscreteGrid::sizevec3 dims = this->outputGrid->getGridDimensions();

	auto bb = this->outputGrid->getBoundingBox();
	auto min = bb.getMin();
	auto max = bb.getMax();
	auto vd = this->outputGrid->getVoxelDimensions();
	std::cerr << "[LOG]\t Generating a voxel grid of dimensions : [" << dims.x << ',' << dims.y << ',' << dims.z << "]\n";
	std::cerr << "[LOG]\t Voxel dimensions are [" << vd.x << ',' << vd.y << ',' << vd.z << "]\n";
	std::cerr << "[LOG]\t Bounding box is from [" << min.x << ',' << min.y << ',' << min.z << "] to [" << max.x << ',' << max.y << ',' << max.z << "]\n";

	// If the grid to generate has "wrong" dimensions, warn and exit
	if (dims.x == 0 || dims.y == 0 || dims.z == 0) {
		std::cerr << "Grid dimensions contain a zero !" << '\n';
		return *this;
	}

	// reserve and allocate space :
	this->outputGrid->preallocateData();

	// iterate on the voxels of the output data grid :
	for (std::size_t k = 0; k < dims.z; ++k) {
		for (std::size_t j = 0; j < dims.y; ++j) {
			for (std::size_t i = 0; i < dims.x; ++i) {
				// generate 3D index :
				DiscreteGrid::sizevec3 idx = DiscreteGrid::sizevec3(i,j,k);
				// get grid-space origin :
				this->origin = this->outputGrid->getVoxelPositionGridSpace(idx);
				// set world-space origin from it :
				this->origin_WS = this->outputGrid->toWorldSpace(this->origin);

				// gather values from all input grids :
				std::vector<DiscreteGrid::DataType> values;
				for (const std::shared_ptr<InputGrid>& grid : this->inputGrids) {
					if (grid->includesPointWorldSpace(this->origin_WS)) {
						values.push_back(this->getInterpolatedValue(grid, method, idx));
					}
				}

				// do a basic mean of the values obtained from the different input grids :
				float globalVal = .0f;
				std::for_each(std::begin(values), std::end(values), [&](DiscreteGrid::DataType v) {
					globalVal += static_cast<float>(v) / static_cast<float>(values.size());
				});
				// set data :
				this->outputGrid->setVoxelData(idx, static_cast<DiscreteGrid::DataType>(globalVal));
			}
		}
	}

	// Data should now be done being generated ...
	return *this;
}

DiscreteGrid::DataType TetMesh::getInterpolatedValue(std::shared_ptr<InputGrid> grid, InterpolationMethods method, DiscreteGrid::sizevec3 idx) const {
	switch (method) {
		case InterpolationMethods::NearestNeighbor:
			return this->interpolate_NearestNeighbor(grid);
		case InterpolationMethods::TriLinear:
			return this->interpolate_TriLinear(grid);
		case InterpolationMethods::TriCubic:
		case InterpolationMethods::Barycentric:
			std::cerr << "Method not implemented in final version of interpolation structure." << '\n';
			return 0;
			break;
		default:
			std::cerr << "Method not recognized.\n";
			break;
	}
	return 0;
}

std::vector<glm::vec4> TetMesh::getVertices_WorldSpace() const {
	// Simple getter for the mesh's vertices.
	std::vector<glm::vec4> res(this->vertices);
	// convert all vertices to world space, instead of this stack's space :
	std::for_each(res.begin(), res.end(), [&](glm::vec4& v) {
		v = v * this->outputGrid->getTransform_GridToWorld();
	});
	return res;
}

TetMesh& TetMesh::printInfo() {
	// Prints info about the mesh :
	// input grid data (resolution, render window)

	if (this->inputGrids.size() == 0 && this->outputGrid == nullptr) {
		std::cerr << "[INFO] TetMesh has no relevant info" << '\n';
		return *this;
	}

	std::cerr << "[INFO]TetMesh has the following specs :" << '\n';
	if (this->inputGrids.size() == 0) {
		std::cerr << "[INFO](No input grids present)" << '\n';
	} else {
		std::cerr << "[INFO]Input grids :\n";
		// Print all of the grids' infos we can easily get :
		for (const std::shared_ptr<InputGrid>& grid : this->inputGrids) {
			std::cerr << "[INFO]\tInput grid named \"" << grid->getGridName() << "\" :\n";
			DiscreteGrid::sizevec3 dims = grid->getGridDimensions();
			// Bounding box dimensions :
			const DiscreteGrid::bbox_t& box = grid->getBoundingBox();
			const DiscreteGrid::bbox_t::vec& min = box.getMin();
			const DiscreteGrid::bbox_t::vec& max = box.getMax();
			std::cerr << "[INFO]\t\tResolution : " << dims.x << 'x' << dims.y << 'x' << dims.z << '\n';
			std::cerr << "[INFO]\t\tBounding box (initial space) : [" << min.x << 'x' << min.y << 'x' << min.z
					<< "] to [" << max.x << 'x' << max.y << 'x' << max.z << "]\n";
			std::cerr << "[INFO]\t\tThis grid is " << ((grid->isModifiable()) ? "not modifiable" : "modifiable") << '\n';
		}
	}
	if (this->outputGrid != nullptr) {
		std::cerr << "[INFO]Output grid :\n";
		std::cerr << "[INFO]\tOutput grid named \"" << this->outputGrid->getGridName() << "\" :\n";
		DiscreteGrid::sizevec3 dims = this->outputGrid->getGridDimensions();
		// Bounding box dimensions :
		const DiscreteGrid::bbox_t& box = this->outputGrid->getBoundingBox();
		const DiscreteGrid::bbox_t::vec& min = box.getMin();
		const DiscreteGrid::bbox_t::vec& max = box.getMax();
		std::cerr << "[INFO]\t\tResolution : " << dims.x << 'x' << dims.y << 'x' << dims.z << '\n';
		std::cerr << "[INFO]\t\tBounding box (initial space) : [" << min.x << 'x' << min.y << 'x' << min.z
				<< "] to [" << max.x << 'x' << max.y << 'x' << max.z << "]\n";
		std::cerr << "[INFO]\t\tThis grid is " << ((this->outputGrid->isModifiable()) ? "not modifiable" : "modifiable") << '\n';
	} else {
		std::cerr << "[INFO](No output grid yet)" << '\n';
	}

	return *this;
}

TetMesh::DataType TetMesh::interpolate_NearestNeighbor(const std::shared_ptr<InputGrid> grid) const {
	// DiscreteGrid::fetchTexelWorldSpace already applies NearestNeighbor on the position
	// given in argument, so we just fetch the value of the origin, giving us a NN interpolation :
	return grid->fetchTexelWorldSpace(this->outputGrid->toWorldSpace(this->origin));
}

TetMesh::DataType TetMesh::interpolate_TriLinear(const std::shared_ptr<InputGrid> grid) const {
	// For this trilinear interpolation, the point will always be at the center of the mesh created earlier.
	// However, we want this method to be as generic as possible, in the event of a catastrophic failure on our part.
	// We want the point in the center of the mesh to be a trilinear interpolation of the corners of the mesh.
	// The grid is constructed in arrays along X first, then along Y, then along Z.
	// As such :
	std::size_t pxyz =  0; // Those indices are arranged such
	std::size_t pxyZ = 18; // that if an axis letter is uppercased,
	std::size_t pxYz =  6; // it means we take the index of the
	std::size_t pxYZ = 24; // point which is at the end of this
	std::size_t pXyz =  2; // axis in the 'cube' the mesh is
	std::size_t pXyZ = 20; // representing.
	std::size_t pXYz =  8; // Can be confusing given their order,
	std::size_t pXYZ = 26; // but ensures a good interpolation.

	// We also already know the point to interpolate for is at the center of this cube. We can set the coefficients
	// for trilinear interpolation directly hardcoded in the computation, since they'll all be 0.5 :
	float coef_x = 0.5f;
	float coef_y = 0.5f;
	float coef_z = 0.5f;

#ifndef DIRECT_CASTING_FROM_FETCH
	// Get values :
	const TetMesh::DataType xyz = grid->fetchTexelWorldSpace(this->outputGrid->toWorldSpace(this->getVertexPosition(pxyz)));
	const TetMesh::DataType xyZ = grid->fetchTexelWorldSpace(this->outputGrid->toWorldSpace(this->getVertexPosition(pxyZ)));
	const TetMesh::DataType xYz = grid->fetchTexelWorldSpace(this->outputGrid->toWorldSpace(this->getVertexPosition(pxYz)));
	const TetMesh::DataType xYZ = grid->fetchTexelWorldSpace(this->outputGrid->toWorldSpace(this->getVertexPosition(pxYZ)));
	const TetMesh::DataType Xyz = grid->fetchTexelWorldSpace(this->outputGrid->toWorldSpace(this->getVertexPosition(pXyz)));
	const TetMesh::DataType XyZ = grid->fetchTexelWorldSpace(this->outputGrid->toWorldSpace(this->getVertexPosition(pXyZ)));
	const TetMesh::DataType XYz = grid->fetchTexelWorldSpace(this->outputGrid->toWorldSpace(this->getVertexPosition(pXYz)));
	const TetMesh::DataType XYZ = grid->fetchTexelWorldSpace(this->outputGrid->toWorldSpace(this->getVertexPosition(pXYZ)));

	float cyz = (1.f - coef_x) * static_cast<float>(xyz) + coef_x * static_cast<float>(Xyz);
	float cyZ = (1.f - coef_x) * static_cast<float>(xyZ) + coef_x * static_cast<float>(XyZ);
	float cYz = (1.f - coef_x) * static_cast<float>(xYz) + coef_x * static_cast<float>(XYz);
	float cYZ = (1.f - coef_x) * static_cast<float>(xYZ) + coef_x * static_cast<float>(XYZ);

	float cz = (1.f - coef_y) * cyz + coef_y * cYz;
	float cZ = (1.f - coef_y) * cyZ + coef_y * cYZ;
#else
	// Get values from the grid :
	float xyz = static_cast<float>(this->getVertexValue(grid, pxyz));
	float xyZ = static_cast<float>(this->getVertexValue(grid, pxyZ));
	float xYz = static_cast<float>(this->getVertexValue(grid, pxYz));
	float xYZ = static_cast<float>(this->getVertexValue(grid, pxYZ));
	float Xyz = static_cast<float>(this->getVertexValue(grid, pXyz));
	float XyZ = static_cast<float>(this->getVertexValue(grid, pXyZ));
	float XYz = static_cast<float>(this->getVertexValue(grid, pXYz));
	float XYZ = static_cast<float>(this->getVertexValue(grid, pXYZ));

	float cyz = (1.f - coef_x) * xyz + coef_x * Xyz;
	float cyZ = (1.f - coef_x) * xyZ + coef_x * XyZ;
	float cYz = (1.f - coef_x) * xYz + coef_x * XYz;
	float cYZ = (1.f - coef_x) * xYZ + coef_x * XYZ;

	float cz = (1.f - coef_y) * cyz + coef_y * cYz;
	float cZ = (1.f - coef_y) * cyZ + coef_y * cYZ;
#endif

	DataType result = static_cast<DataType>((1.f - coef_z) * cz + coef_z * cZ);

	return result;
}

TetMesh& TetMesh::updateVoxelSizes() {
	if (this->outputGrid == nullptr) { return *this; }
	glm::vec3 vxdims = this->outputGrid->getVoxelDimensions();
	glm::vec3 halfdims = vxdims/2.f;

	for (std::size_t k = 0; k < 3; ++k) {
		for (std::size_t j = 0; j < 3; ++j) {
			for (std::size_t i = 0; i < 3; ++i) {
				this->vertices[i+j*2u+k*2u*2u] = glm::vec4(static_cast<float>(i) * vxdims.x - halfdims.x, static_cast<float>(j) * vxdims.y - halfdims.y, static_cast<float>(k) * vxdims.z - halfdims.z, 1.);
			}
		}
	}

	return *this;
}

glm::vec4 TetMesh::getVertexPosition(std::size_t idx) const {
	return this->vertices[idx] + this->origin;
}

TetMesh::DataType TetMesh::getVertexValue(const std::shared_ptr<InputGrid> grid, std::size_t idx) const {
	return grid->fetchTexelWorldSpace(this->outputGrid->toWorldSpace(this->getVertexPosition(idx)));
}

TetMesh& TetMesh::updateOutputGridData() {
	// If there aren't any input grids nor any output grid, return early :
	if (this->inputGrids.size() == 0) { return *this; }
	if (this->outputGrid == nullptr) { return *this; }

	this->outputGrid->setBoundingBox(DiscreteGrid::bbox_t());

	for (const std::shared_ptr<InputGrid>& grid : this->inputGrids) {
		// Get bounding box of data in world space :
		DiscreteGrid::bbox_t newbb = grid->getDataBoundingBox().transformTo(grid->getTransform_GridToWorld());
		this->outputGrid->updateBoundingBox(newbb);
	}

	// get diagonal of bb :
	DiscreteGrid::bbox_t::vec diag = this->outputGrid->getBoundingBox().getDiagonal();
	// set resolution so each voxel's side length is a bit less than 1 :
	DiscreteGrid::sizevec3 dimensions = DiscreteGrid::sizevec3(
		static_cast<std::size_t>(std::ceil(diag.x)),
		static_cast<std::size_t>(std::ceil(diag.y)),
		static_cast<std::size_t>(std::ceil(diag.z))
	);
	this->outputGrid->setResolution(dimensions);

	return *this;
}

void TetMesh::makeTetrahedra() {
	// For now, a mesh of side 1, centered at the origin

	// Lambda returning the index of the vertex in the mesh :
	auto getIndex = [&](std::size_t i, std::size_t j, std::size_t k) {
		return k * 2u * 2u + j * 2u + i;
	};

	// At the start, the origin is at [.0, .0, .0]. But the center of
	// the first voxel is in fact at [.5, .5, .5], which means we
	// need to iterate in [-.5, 1.5] by steps of 1, on each axis.

	for (std::size_t k = 0; k < 3; ++k) {
		for (std::size_t j = 0; j < 3; ++j) {
			for (std::size_t i = 0; i < 3; ++i) {
				this->vertices.emplace_back(
					static_cast<float>(i) * 1.f - .5f,
					static_cast<float>(j) * 1.f - .5f,
					static_cast<float>(k) * 1.f - .5f,
					1.
				);
			}
		}
	}

	for (std::size_t k = 0; k < 2; ++k) {
		for (std::size_t j = 0; j < 2; ++j) {
			for (std::size_t i = 0; i < 2; ++i) {
				// Tetrahedra 1 :
				this->tetrahedra.push_back({
					getIndex(i+1, j+0, k+0),
					getIndex(i+1, j+1, k+0),
					getIndex(i+0, j+1, k+0),
					getIndex(i+1, j+1, k+1)
				});
				// Tetrahedra 2 :
				this->tetrahedra.push_back({
					getIndex(i+0, j+0, k+1),
					getIndex(i+0, j+0, k+0),
					getIndex(i+0, j+1, k+1),
					getIndex(i+1, j+0, k+1)
				});
				// Tetrahedra 3 :
				this->tetrahedra.push_back({
					getIndex(i+0, j+1, k+1),
					getIndex(i+1, j+0, k+0),
					getIndex(i+1, j+1, k+1),
					getIndex(i+1, j+0, k+1)
				});
				// Tetrahedra 4 :
				this->tetrahedra.push_back({
					getIndex(i+0, j+0, k+0),
					getIndex(i+1, j+0, k+0),
					getIndex(i+0, j+1, k+1),
					getIndex(i+1, j+0, k+1)
				});
				// Tetrahedra 5 :
				this->tetrahedra.push_back({
					getIndex(i+0, j+0, k+0),
					getIndex(i+1, j+0, k+0),
					getIndex(i+0, j+1, k+0),
					getIndex(i+0, j+1, k+1)
				});
				// Tetrahedra 6 :
				this->tetrahedra.push_back({
					getIndex(i+0, j+1, k+0),
					getIndex(i+1, j+0, k+0),
					getIndex(i+1, j+1, k+1),
					getIndex(i+0, j+1, k+1)
				});
			}
		}
	}

	// The mesh is now constructed. Or at least, it should be.
}






















































