#include "../include/tetmesh.hpp"

#include <random>
#include <iomanip>

TetMesh::TetMesh() {
	// Initialises all the values of the mesh to their default values.
	// Centers the mesh around the space's origin. Also, makes the mesh
	// by a call to TetMesh::makeTetrahedra().
	this->vertices.clear();
	this->tetrahedra.clear();
	this->origin = glm::vec4(.0, .0, .0, .0);
	this->inputGrids.clear();
	this->outputGrid = nullptr;

	this->generationRate = -std::numeric_limits<double>::infinity();

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

std::vector<std::shared_ptr<InputGrid>> TetMesh::getInputGrids() const {
	return this->inputGrids;
}

TetMesh& TetMesh::setOutputGrid(const std::shared_ptr<OutputGrid>& toSet) {
	this->outputGrid = toSet;
	this->updateOutputGridData();
	this->updateVoxelSizes();
	return *this;
}

TetMesh& TetMesh::setOutputGrid_raw(const std::shared_ptr<OutputGrid>& toSet) {
	this->outputGrid = toSet;
	this->updateVoxelSizes();
	return *this;
}

TetMesh& TetMesh::populateOutputGrid(InterpolationMethods method) {
	// early returns :
	if (this->outputGrid == nullptr) { return *this; }
	if (this->inputGrids.size() == 0) { return *this; }

	this->updateVoxelSizes();

	// Check the dimensions of the voxel grid (if it can host voxels) :
	DiscreteGrid::sizevec3 dims = this->outputGrid->getResolution();

	// If the grid to generate has "wrong" dimensions, warn and exit
	if (dims.x == 0 || dims.y == 0 || dims.z == 0) {
		std::cerr << "Grid dimensions contain a zero !" << '\n';
		return *this;
	}

	std::cerr << "[DEBUG] =====================================================================================\n";
	std::cerr << "[DEBUG] TetMesh generating over [" << dims.x << ", " << dims.y << ", " << dims.z << "]\n";
	std::cerr << "[DEBUG] Listing input grids : " << '\n';
	for (const auto& g : this->inputGrids) {
		g->printInfo("After update, before output preallocation :", "[DEBUG]\t");
	}
	this->outputGrid->printInfo("After update, before preallocation :", "[DEBUG]\t");
	std::cerr << "[DEBUG] =====================================================================================\n";

	// reserve and allocate space :
	this->outputGrid->preallocateData();

	this->outputGrid->getGridWriter()->preAllocateData();

	bool verbose = false;

	using clock_t = std::chrono::high_resolution_clock;
	using duration_t = clock_t::duration;
	using timepoint_t = clock_t::time_point;

	timepoint_t start, end;
	duration_t elapsed = duration_t::zero();

	std::cerr << "[LOG] Starting to iterate on the image to generate ... (" << dims.z << " levels)\n";
	// iterate on the voxels of the output data grid :
	for (std::size_t k = 0; k < dims.z; ++k) {
		this->outputGrid->setCurrentSlice(k);
		start = clock_t::now();
		for (std::size_t j = 0; j < dims.y; ++j) {
			for (std::size_t i = 0; i < dims.x; ++i) {
				// generate 3D index :
				DiscreteGrid::sizevec3 idx = DiscreteGrid::sizevec3(i,j,k);

				// get grid-space origin :
				this->origin = this->outputGrid->getVoxelPositionGridSpace(idx, verbose);
				// set world-space origin from it :
				this->origin_WS = this->outputGrid->toWorldSpace(this->origin);

				// gather values from all input grids :
				std::vector<data_t> values;
				for (const std::shared_ptr<InputGrid>& grid : this->inputGrids) {
					// glm::vec4 now = grid->toGridSpace(this->origin_WS);
					if (grid->includesPointWorldSpace(this->origin_WS, verbose)) {
						values.push_back(this->getInterpolatedValue(grid, method, idx, verbose));
					}
				}

				// do a basic mean of the values obtained from the different input grids :
				data_t globalVal = .0f;
				std::for_each(std::begin(values), std::end(values), [&](data_t v) {
					globalVal += static_cast<data_t>(v) / static_cast<data_t>(values.size());
				});

				// set data :
				this->outputGrid->setPixel(i, j, k, globalVal);
			}
		}
		end = clock_t::now();
		elapsed += (end - start);

		// #warning Writes data directly to disk here.
		this->outputGrid->writeSlice();
		std::cerr << '\n';
	}

	// Get the number of generated voxels :
	double gigavxcount = static_cast<double>(dims.x * dims.y * dims.z) / (1.e9);

	// cast duration to double floating point and divide by the size of voxels, to get a GVx/H rate :
	this->generationRate = gigavxcount / std::chrono::duration_cast<std::chrono::duration<double, std::ratio<3600>>>(elapsed).count();

	std::cerr << "====================================" << '\n';
	std::cerr << "\tTime to generate   : " << std::chrono::duration_cast<std::chrono::seconds>(elapsed).count() << " seconds\n";
	std::cerr << "\tRate of generation : " << this->getGenerationRate() << " GV/h" << '\n';
	std::cerr << "====================================" << '\n';

	// Data should now be done being generated ...
	return *this;
}

TetMesh& TetMesh::populateOutputGrid_threaded(InterpolationMethods method, IO::ThreadedTask::Ptr& task) {
	// early returns :
	if (this->outputGrid == nullptr) { return *this; }
	if (this->inputGrids.size() == 0) { return *this; }

	this->updateVoxelSizes();

	// Check the dimensions of the voxel grid (if it can host voxels) :
	DiscreteGrid::sizevec3 dims = this->outputGrid->getResolution();

	task->setSteps(dims.z);

	// If the grid to generate has "wrong" dimensions, warn and exit
	if (dims.x == 0 || dims.y == 0 || dims.z == 0) {
		std::cerr << "Grid dimensions contain a zero !" << '\n';
		task->end();
		return *this;
	}

	// reserve and allocate space :
	this->outputGrid->preallocateData();

	using clock_t = std::chrono::high_resolution_clock;
	using duration_t = clock_t::duration;
	using timepoint_t = clock_t::time_point;

	timepoint_t start, end;
	duration_t elapsed = duration_t::zero();

	std::cerr << "[LOG] Starting to iterate on the image to generate ... (" << dims.z << " levels)\n";
	// iterate on the voxels of the output data grid :
	for (std::size_t k = 0; k < dims.z; ++k) {
		this->outputGrid->setCurrentSlice(k);
		start = clock_t::now();
		for (std::size_t j = 0; j < dims.y; ++j) {
			for (std::size_t i = 0; i < dims.x; ++i) {
				// generate 3D index :
				DiscreteGrid::sizevec3 idx = DiscreteGrid::sizevec3(i,j,k);

				// get grid-space origin :
				this->origin = this->outputGrid->getVoxelPositionGridSpace(idx);
				// set world-space origin from it :
				this->origin_WS = this->outputGrid->toWorldSpace(this->origin);

				// gather values from all input grids :
				std::vector<data_t> values;
				for (const std::shared_ptr<InputGrid>& grid : this->inputGrids) {
					// glm::vec4 now = grid->toGridSpace(this->origin_WS);
					if (grid->includesPointWorldSpace(this->origin_WS)) {
						values.push_back(this->getInterpolatedValue(grid, method, idx));
					}
				}

				// do a basic mean of the values obtained from the different input grids :
				data_t globalVal = .0f;
				std::for_each(std::begin(values), std::end(values), [&](data_t v) {
					globalVal += static_cast<data_t>(v) / static_cast<data_t>(values.size());
				});

				// set data :
				this->outputGrid->setPixel(i, j, k, globalVal);
			}
		}
		end = clock_t::now();
		elapsed += (end - start);

		// #warning Writes data directly to disk here.
		task->advance();
	}

	task->end();

	// Data should now be done being generated ...
	return *this;
}

TetMesh& TetMesh::populateOutputGrid_RGB(InterpolationMethods method) {
	// early returns :
	if (this->outputGrid == nullptr) { return *this; }
	if (this->inputGrids.size() == 0) { return *this; }

	this->updateVoxelSizes();

	// Check the dimensions of the voxel grid (if it can host voxels) :
	DiscreteGrid::sizevec3 dims = this->outputGrid->getResolution();

	// If the grid to generate has "wrong" dimensions, warn and exit
	if (dims.x == 0 || dims.y == 0 || dims.z == 0) {
		std::cerr << "Grid dimensions contain a zero !" << '\n';
		return *this;
	}

	std::cerr << "[DEBUG] =====================================================================================\n";
	std::cerr << "[DEBUG] TetMesh g	enerating over [" << dims.x << ", " << dims.y << ", " << dims.z << "]\n";
	std::cerr << "[DEBUG] Listing input grids : " << '\n';
	for (const auto& g : this->inputGrids) {
		g->printInfo("After update, before output preallocation :", "[DEBUG]\t");
	}
	this->outputGrid->printInfo("After update, before preallocation :", "[DEBUG]\t");
	std::cerr << "[DEBUG] =====================================================================================\n";

	// reserve and allocate space :
	this->outputGrid->preallocateData();

	this->outputGrid->getGridWriter()->preAllocateData();

	bool verbose = false;

	using clock_t = std::chrono::high_resolution_clock;
	using duration_t = clock_t::duration;
	using timepoint_t = clock_t::time_point;

	timepoint_t start, end;
	duration_t elapsed = duration_t::zero();

	std::vector<data_t> imgdata;

	std::shared_ptr<IO::GenericGridWriter> writer = this->outputGrid->getGridWriter();
	if (writer == nullptr) {
		std::cerr << "ERROR : no writer can be accessed\n";
		return *this;
	}

	std::cerr << "[LOG] Starting to iterate on the image to generate ... (" << dims.z << " levels)\n";
	// iterate on the voxels of the output data grid :
	for (std::size_t k = 0; k < dims.z; ++k) {
		this->outputGrid->setCurrentSlice(k);
		imgdata.clear();
		imgdata.resize(dims.x * dims.y * 3);
		start = clock_t::now();
		for (std::size_t j = 0; j < dims.y; ++j) {
			for (std::size_t i = 0; i < dims.x; ++i) {
				// generate 3D index :
				DiscreteGrid::sizevec3 idx = DiscreteGrid::sizevec3(i,j,k);

				// get grid-space origin :
				this->origin = this->outputGrid->getVoxelPositionGridSpace(idx, verbose);
				// set world-space origin from it :
				this->origin_WS = this->outputGrid->toWorldSpace(this->origin);

				// gather values from all input grids :
				std::vector<data_t> values;
				for (const std::shared_ptr<InputGrid>& grid : this->inputGrids) {
					// glm::vec4 now = grid->toGridSpace(this->origin_WS);
					if (grid->includesPointWorldSpace(this->origin_WS, verbose)) {
						values.push_back(this->getInterpolatedValue(grid, method, idx, verbose));
					}
				}

				if (values.size() != 2) {
					std::cerr << "[ERROR] For iteration {" << i << ',' << j << ',' << k << "}, no ";
					std::cerr << "RGB output was possible. [COLOR_OUT]\n";
				}
				glm::vec<3, data_t, glm::defaultp> color ;
				if (values.size() > 0) {
					color = this->h_and_e_colouring(values[0], this->inputGrids[0], values[1], this->inputGrids[1]);
				} else {
					color = glm::vec<3, data_t, glm::defaultp>(0, 0, 0);
				}

				// Copy into the img buffer :
				for (std::size_t c = 0; c < 3; ++c) {
					imgdata[j*dims.x*3 + i*3 + c] = color[c];
				}
			}
		}
		end = clock_t::now();
		elapsed += (end - start);

		// #warning Writes data directly to disk here.
		std::cerr << "[LOG] Writing slice " << k << "/" << dims.z << " ...\n";
		writer->writeSlice_RGB(imgdata, k);
	}

	// Get the number of generated voxels :
	double gigavxcount = static_cast<double>(dims.x * dims.y * dims.z) / (1.e9);

	// cast duration to double floating point and divide by the size of voxels, to get a GVx/H rate :
	this->generationRate = gigavxcount / std::chrono::duration_cast<std::chrono::duration<double, std::ratio<3600>>>(elapsed).count();

	std::cerr << "====================================" << '\n';
	std::cerr << "\tTime to generate   : " << std::chrono::duration_cast<std::chrono::seconds>(elapsed).count() << " seconds\n";
	std::cerr << "\tRate of generation : " << this->getGenerationRate() << " GV/h" << '\n';
	std::cerr << "====================================" << '\n';

	// Data should now be done being generated ...
	return *this;
}

glm::vec<3, TetMesh::data_t, glm::defaultp> TetMesh::h_and_e_colouring(data_t _r, std::shared_ptr<InputGrid> &_rg, data_t _b, std::shared_ptr<InputGrid> &_bg) {
	glm::vec2 bounds_r = glm::convert_to<float>(_rg->getGridReader()->getTextureLimits());
	glm::vec2 bounds_b = glm::convert_to<float>(_bg->getGridReader()->getTextureLimits());
	glm::vec2 color_bounds = glm::vec2(
		std::min(bounds_r.x, bounds_b.x),
		std::max(bounds_r.y, bounds_b.y)
	);

	float color_r = static_cast<float>(_r);
	float color_b = static_cast<float>(_b);
	/* No need to discard or clamp for texels outside the texture range, we aren't doing alpha nor discards */
	float color_k = 2.5;
	float sc = color_bounds.y - color_bounds.x;
	float eosin = (color_r - color_bounds.x) / sc;
	float dna = (color_b - color_bounds.x) / sc;

	float eosin_r_coef = 0.050;
	float eosin_g_coef = 1.000;
	float eosin_b_coef = 0.544;

	float hematoxylin_r_coef = 0.860;
	float hematoxylin_g_coef = 1.000;
	float hematoxylin_b_coef = 0.300;

	glm::vec3 real_color = glm::vec3(
		std::exp(-hematoxylin_r_coef * dna * color_k) * std::exp(-eosin_r_coef * eosin * color_k),
		std::exp(-hematoxylin_g_coef * dna * color_k) * std::exp(-eosin_g_coef * eosin * color_k),
		std::exp(-hematoxylin_b_coef * dna * color_k) * std::exp(-eosin_b_coef * eosin * color_k)
	);

	glm::vec<3, data_t, glm::defaultp> color = glm::vec<3, data_t, glm::defaultp>(
		static_cast<data_t>(color_bounds.x + real_color.x * sc),
		static_cast<data_t>(color_bounds.x + real_color.y * sc),
		static_cast<data_t>(color_bounds.x + real_color.z * sc)
	);

	return color;
}

TetMesh::data_t TetMesh::getInterpolatedValue(std::shared_ptr<InputGrid> grid, InterpolationMethods method, DiscreteGrid::sizevec3 idx, bool verbose) const {
	switch (method) {
		case InterpolationMethods::NearestNeighbor:
			return this->interpolate_NearestNeighbor(grid, verbose);
		case InterpolationMethods::TriLinear:
			return this->interpolate_TriLinear(grid, verbose);
		default: // should never be encountered as long as all enum values are above, just to check
			std::cerr << "[ERROR] Interpolation method not recognized.\n";
			break;
	}
	return 0;
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
			DiscreteGrid::sizevec3 dims = grid->getResolution();
			std::cerr << "[INFO]\t\tResolution : " << dims.x << 'x' << dims.y << 'x' << dims.z << '\n';
			// Bounding box dimensions :
			const DiscreteGrid::bbox_t& box = grid->getBoundingBox();
			const DiscreteGrid::bbox_t::vec& min = box.getMin();
			const DiscreteGrid::bbox_t::vec& max = box.getMax();
			std::cerr << "[INFO]\t\tBounding box (initial space) : [" << min.x << 'x' << min.y << 'x' << min.z
					<< "] to [" << max.x << 'x' << max.y << 'x' << max.z << "]\n";
			std::cerr << "[INFO]\t\tThis grid is " << ((grid->isModifiable()) ? "not modifiable" : "modifiable") << '\n';
		}
	}
	if (this->outputGrid != nullptr) {
		std::cerr << "[INFO]Output grid :\n";
		std::cerr << "[INFO]\tOutput grid named \"" << this->outputGrid->getGridName() << "\" :\n";
		DiscreteGrid::sizevec3 dims = this->outputGrid->getResolution();
		std::cerr << "[INFO]\t\tResolution : " << dims.x << 'x' << dims.y << 'x' << dims.z << '\n';

		// Bounding box dimensions :
		const DiscreteGrid::bbox_t& box = this->outputGrid->getBoundingBox();
		const DiscreteGrid::bbox_t::vec& min = box.getMin();
		const DiscreteGrid::bbox_t::vec& max = box.getMax();
		std::cerr << "[INFO]\t\tBounding box (initial space) : [" << min.x << 'x' << min.y << 'x' << min.z
				<< "] to [" << max.x << 'x' << max.y << 'x' << max.z << "]\n";

		std::cerr << "[INFO]\t\tThis grid is " << ((this->outputGrid->isModifiable()) ? "modifiable" : "not modifiable") << '\n';
	} else {
		std::cerr << "[INFO](No output grid yet)" << '\n';
	}

	return *this;
}

double TetMesh::getGenerationRate() const {
	return this->generationRate;
}

TetMesh::data_t TetMesh::interpolate_NearestNeighbor(const std::shared_ptr<InputGrid> grid, bool verbose) const {
	// DiscreteGrid::fetchTexelWorldSpace already applies NearestNeighbor on the position
	// given in argument, so we just fetch the value of the origin, giving us a NN interpolation :
	if (verbose) { std::cerr << "[LOG]\t\t"; }
	return grid->fetchTexelWorldSpace(this->origin_WS, verbose);
}

TetMesh::data_t TetMesh::interpolate_TriLinear(const std::shared_ptr<InputGrid> grid, bool verbose) const {
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

	// Get values from the grid :
	float xyz = static_cast<float>(this->getVertexValue(grid, pxyz, verbose));
	float xyZ = static_cast<float>(this->getVertexValue(grid, pxyZ, verbose));
	float xYz = static_cast<float>(this->getVertexValue(grid, pxYz, verbose));
	float xYZ = static_cast<float>(this->getVertexValue(grid, pxYZ, verbose));

	if (verbose) {
		auto out = this->outputGrid->toWorldSpace(this->origin);
		std::cerr << "[LOG]\t\tOrigin position : {" << this->origin.x << ',' << this->origin.y << ',' << this->origin.z << "}\n[LOG]\t\t\t";
		auto g = grid->fetchTexelWorldSpace(out, verbose);
		std::cerr << "[LOG]\t\tValue returned at origin : " << +g << '\n';
	}

	float Xyz = static_cast<float>(this->getVertexValue(grid, pXyz, verbose));
	float XyZ = static_cast<float>(this->getVertexValue(grid, pXyZ, verbose));
	float XYz = static_cast<float>(this->getVertexValue(grid, pXYz, verbose));
	float XYZ = static_cast<float>(this->getVertexValue(grid, pXYZ, verbose));

	float cyz = (1.f - coef_x) * xyz + coef_x * Xyz;
	float cyZ = (1.f - coef_x) * xyZ + coef_x * XyZ;
	float cYz = (1.f - coef_x) * xYz + coef_x * XYz;
	float cYZ = (1.f - coef_x) * xYZ + coef_x * XYZ;

	float cz = (1.f - coef_y) * cyz + coef_y * cYz;
	float cZ = (1.f - coef_y) * cyZ + coef_y * cYZ;

	data_t result = static_cast<data_t>((1.f - coef_z) * cz + coef_z * cZ);

	return result;
}

TetMesh& TetMesh::updateVoxelSizes() {
	/* TODO : we should probably soft-fail here instead of ignoring the potential problem. To debate. */
	if (this->outputGrid == nullptr) { return *this; }

	this->makeTetrahedra(this->outputGrid->getVoxelDimensions(), 1);

	return *this;
}

glm::vec4 TetMesh::getVertexPosition(std::size_t idx) const {
	return this->vertices[idx] + this->origin;
}

TetMesh::data_t TetMesh::getVertexValue(const std::shared_ptr<InputGrid> grid, std::size_t idx, bool verbose) const {
	auto out = this->outputGrid->toWorldSpace(this->getVertexPosition(idx));
	if (verbose) { std::cerr << "[LOG]\t\t\t"; }
	return grid->fetchTexelWorldSpace(out, verbose);
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

void TetMesh::makeTetrahedra(glm::vec3 vxDims, std::size_t size) {
	// For now, a mesh of side 1, centered at the origin
	this->vertices.clear();
	this->tetrahedra.clear();

	std::size_t neighborWidth = 2u * size;

	// Lambda returning the index of the vertex in the mesh :
	auto getIndex = [&](std::size_t i, std::size_t j, std::size_t k) {
		return k * neighborWidth * neighborWidth + j * neighborWidth + i;
	};

	// We can have the vertices directly on 'whole' coordinates since those
	// are basically just an offset to fetch values by. The origin is at the
	// center of the created vertices.

	int isize = static_cast<int>(size);
	for (int k = -isize; k <= isize; ++k) {
		for (int j = -isize; j <= isize; ++j) {
			for (int i = -isize; i <= isize; ++i) {
				this->vertices.emplace_back(
					static_cast<float>(i) * vxDims.x,
					static_cast<float>(j) * vxDims.y,
					static_cast<float>(k) * vxDims.z,
					.0	// 0 here since those positions will serve
					// as offsets applied to a world space position
				);
			}
		}
	}

	for (std::size_t k = 0; k < neighborWidth; ++k) {
		for (std::size_t j = 0; j < neighborWidth; ++j) {
			for (std::size_t i = 0; i < neighborWidth; ++i) {
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






















































