#ifndef VISUALISATION_OBJ_LOADER_HPP
#define VISUALISATION_OBJ_LOADER_HPP

#include <cfloat>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <vector>

namespace FileIO {
	template <typename Point, typename Face>
	bool read(std::istream& _in, std::vector<Point>& vertices, std::vector<Face>& triangles) {
		std::cout << "[OBJReader] : read file\n";

		std::string line;
		std::string keyWrd;

		float x, y, z;

		std::vector<int> vhandles;

		while (_in && ! _in.eof())
		{
			std::getline(_in, line);
			if (_in.bad()) {
				std::cout << "  Warning! Could not read file properly!\n";
				return false;
			}

			size_t start = line.find_first_not_of(" \t\r\n");
			size_t end	 = line.find_last_not_of(" \t\r\n");

			if ((std::string::npos == start) || (std::string::npos == end))
				line = "";
			else
				line = line.substr(start, end - start + 1);

			// comment
			if (line.size() == 0 || line[0] == '#' || isspace(line[0])) {
				continue;
			}

			std::stringstream stream(line);

			stream >> keyWrd;

			// material file
			if (keyWrd == "mtllib" || keyWrd == "usemtl")
			{
			}
			// vertex
			else if (keyWrd == "v")
			{
				stream >> x;
				stream >> y;
				stream >> z;

				if (! stream.fail())
					vertices.push_back(Point(x, y, z));
			}

			// texture coord
			else if (keyWrd == "vt" || keyWrd == "vn")
			{
			}
			// face
			else if (keyWrd == "f")
			{
				int component(0), nV(0);
				int value;

				vhandles.clear();

				// read full line after detecting a face
				std::string faceLine;
				std::getline(stream, faceLine);
				std::stringstream lineData(faceLine);

				// work on the line until nothing left to read
				while (! lineData.eof())
				{
					// read one block from the line ( vertex/texCoord/normal )
					std::string vertex;
					lineData >> vertex;

					do {
						// get the component (vertex/texCoord/normal)
						size_t found = vertex.find("/");

						// parts are seperated by '/' So if no '/' found its the last component
						if (found != std::string::npos) {
							// read the index value
							std::stringstream tmp(vertex.substr(0, found));

							// If we get an empty string this property is undefined in the file
							if (vertex.substr(0, found).empty()) {
								// Switch to next field
								vertex = vertex.substr(found + 1);

								// Now we are at the next component
								++component;

								// Skip further processing of this component
								continue;
							}

							// Read current value
							tmp >> value;

							// remove the read part from the string
							vertex = vertex.substr(found + 1);

						} else {
							// last component of the vertex, read it.
							std::stringstream tmp(vertex);
							tmp >> value;

							// Clear vertex after finished reading the line
							vertex = "";

							// Nothing to read here ( garbage at end of line )
							if (tmp.fail()) {
								continue;
							}
						}

						// store the component ( each component is referenced by the index here! )
						switch (component)
						{
							case 0:	   // vertex
								if (value < 0) {
									// Calculation of index :
									// -1 is the last vertex in the list
									// As obj counts from 1 and not zero add +1
									value = vertices.size() + value + 1;
								}
								// Obj counts from 1 and not zero .. array counts from zero therefore -1
								vhandles.push_back(value - 1);
								break;

							case 1:	   // texture coord
								break;

							case 2:	   // normal
								break;
						}

						// Prepare for reading next component
						++component;

						// Read until line does not contain any other info
					} while (! vertex.empty());

					component = 0;
					nV++;
				}

				if (vhandles.size() > 3)
				{
					// model is not triangulated, so let us do this on the fly...
					// to have a more uniform mesh, we add randomization
					unsigned int k = (false) ? (rand() % vhandles.size()) : 0;
					for (unsigned int i = 0; i < vhandles.size() - 2; ++i)
					{
						triangles.push_back(Face(vhandles[(k + 0) % vhandles.size()], vhandles[(k + i + 1) % vhandles.size()], vhandles[(k + i + 2) % vhandles.size()]));
					}
				} else if (vhandles.size() == 3)
				{
					triangles.push_back(Face(vhandles[0], vhandles[1], vhandles[2]));
				} else
				{
					printf("TriMesh::LOAD: Unexpected number of face vertices (<3). Ignoring face");
				}
			}
		}

		return true;
	}

	template <typename Point, typename Face>
	bool objLoader(const std::string& _filename, std::vector<Point>& vertices, std::vector<Face>& triangles) {
		std::fstream in(_filename.c_str(), std::ios_base::in);

		if (! in.is_open() || ! in.good())
		{
			std::cout << "[OBJReader] : cannot not open file "
					  << _filename
					  << std::endl;
			return false;
		}

		{
#if defined(WIN32)
			std::string::size_type dot = _filename.find_last_of("\\/");
#else
			std::string::size_type dot = _filename.rfind("/");
#endif
			std::string path_ = (dot == std::string::npos) ? "./" : std::string(_filename.substr(0, dot + 1));
		}

		bool result = FileIO::read(in, vertices, triangles);

		in.close();
		return result;
	}

	template <typename Point, typename Face>
	void openHomemadeOBJ(std::string const& filename, std::vector<Point>& vertices, std::vector<Face>& triangles,
	  std::vector<int>& movedVertexIndices, std::vector<Point>& leftCondyle, std::vector<Point>& rightCondyle,
	  std::vector<Point>& chin) {
		std::ifstream myfile;
		myfile.open(filename.c_str());

		if (! myfile.is_open())
		{
			std::cout << filename << " cannot be opened" << std::endl;
			return;
		}

		while (! myfile.eof())
		{
			char c;

			myfile >> c;

			if (c == 'v')
			{
				std::cout << "VERTEX" << std::endl;
				int id;
				float x, y, z;

				myfile >> x >> y >> z >> id;

				vertices.push_back(Point(x, y, z));
				movedVertexIndices.push_back(id);

				if (id == 1) {
					leftCondyle.push_back(Point(x, y, z));
				} else if (id == 2) {
					rightCondyle.push_back(Point(x, y, z));
				} else if (id == 3) {
					chin.push_back(Point(x, y, z));
				}
			} else if (c == 'f')
			{
				std::cout << "FACE" << std::endl;
				int v1, v2, v3;
				myfile >> v1 >> v2 >> v3;
				triangles.push_back(Face(v1 - 1, v2 - 1, v3 - 1));
				std::cout << v1 << " " << v2 << " " << v3 << std::endl;
			}
		}
	}

}	 // namespace FileIO

#endif	  // VISUALISATION_OBJ_LOADER_HPP
