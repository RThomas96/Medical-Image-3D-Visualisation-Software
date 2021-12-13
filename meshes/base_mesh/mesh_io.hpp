#ifndef VISUALISATION_MESH_IO_HPP
#define VISUALISATION_MESH_IO_HPP

#include <fstream>
#include <iostream>
#include <vector>

namespace FileIO {

	template <typename Point, typename Face>
	void openOFF(std::string const &filename, std::vector<Point> &vertices, std::vector<Face> &triangles) {
		std::cout << "Opening " << filename << std::endl;

		// open the file
		std::ifstream myfile;
		myfile.open(filename.c_str());
		if (! myfile.is_open())
		{
			std::cout << filename << " cannot be opened" << std::endl;
			return;
		}

		std::string magic_s;

		myfile >> magic_s;

		// check if it's OFF
		if (magic_s != "OFF")
		{
			std::cout << magic_s << " != OFF :   We handle ONLY *.off files." << std::endl;
			myfile.close();
			exit(1);
		}

		int n_vertices, n_faces, dummy_int;
		myfile >> n_vertices >> n_faces >> dummy_int;

		// Clear any verticies
		vertices.clear();

		// Read the verticies
		for (int v = 0; v < n_vertices; ++v)
		{
			float x, y, z;
			myfile >> x >> y >> z;
			vertices.push_back(Point(x, y, z));
		}

		// Clear any triangles
		triangles.clear();

		// Read the triangles
		for (int f = 0; f < n_faces; ++f)
		{
			int n_vertices_on_face;
			myfile >> n_vertices_on_face;
			if (n_vertices_on_face == 3)
			{
				unsigned int _v1, _v2, _v3;
				myfile >> _v1 >> _v2 >> _v3;
				triangles.push_back(Face(_v1, _v2, _v3));
			} else if (n_vertices_on_face == 4)
			{
				unsigned int _v1, _v2, _v3, _v4;

				myfile >> _v1 >> _v2 >> _v3 >> _v4;
				triangles.push_back(Face(_v1, _v2, _v3));
				triangles.push_back(Face(_v1, _v3, _v4));
			} else
			{
				std::cout << "We handle ONLY *.off files with 3 or 4 vertices per face" << std::endl;
				myfile.close();
				exit(1);
			}
		}
	}

}	 // namespace FileIO

#endif	  // VISUALISATION_MESH_IO_HPP
