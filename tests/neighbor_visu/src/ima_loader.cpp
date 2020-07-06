#include "../include/ima_loader.hpp"

#include <fstream>
#include <cstdlib>
#include <cstring>
#include <iostream>

#include <QFileDialog>
#include <QStringList>

bool FileExists(const char* filename) {
	// We could do a more time-optimized version of it
	// by returning fileTester == nullptr, but I still
	// prefer to properly close the inode associated
	// with it, to sanitize the system.

	// Try to open the file :
	FILE* fileTester = fopen(filename, "r");
	// Is it open ? If not, return false :
	if (fileTester == nullptr) return false;
	// Otherwise, close the file and return true :
	fclose(fileTester);
	return true;
}

char* FileBaseName(const char* filename) {
	// Note : this function does not check if the file exists.
	// It only extracts the basename from the string provided.

	// Locate last occurence of the character dot '.'
	const char* separator = strrchr(filename, '.');
	const char* path_slash = strrchr(filename, '/');
	// If none was found, return nulltpr :
	if (separator == nullptr) { return nullptr; }
	// Otherwise, allocate a new string and copy contents :
	std::size_t length = static_cast<std::size_t>(separator - &filename[0]);
	std::size_t length_to_path = static_cast<std::size_t>(separator - path_slash);
	// If the dot is a for relative folder access, discard the file (since it has no extension)
	if (length_to_path < 0) { return nullptr; }
	// Allocate memory for file's basename
	char* basename = static_cast<char*>(calloc(length + 1, sizeof(char)));
	memcpy(basename, filename, length * sizeof(char));
	// null-terminate the string
	basename[length] = '\0';
	// return it !
	return basename;
}

char* AppendExtension(char* basename, const char* extension) {
	// Early exits
	if (basename == nullptr) return nullptr;
	if (extension == nullptr) return nullptr;

	std::size_t baselength = strlen(basename); //strlen ignores terminating \0 !
	std::size_t extlength = strlen(extension); //strlen ignores terminating \0 !

	// If basename is empty string, return nothing
	if (baselength == 0) return nullptr;
	// If extension is empty string, return basename
	if (extlength == 0) return basename;

	// Realloc basename (add one for the dot, and one for the terminating char) :
	char* new_basename = static_cast<char*>(calloc(baselength+extlength+2, sizeof(char)));
	if (basename == nullptr) {
		return nullptr;
	}
	memcpy(new_basename, basename, baselength);
	new_basename[baselength] = '.';
	memcpy(new_basename+baselength+1, extension, extlength);
	new_basename[baselength+extlength+1] = '\0';
	return new_basename;
}


IMALoader::IMALoader() {
	this->data.clear();
}

IMALoader::~IMALoader() {
	this->data.clear();
}

const std::vector<unsigned char>& IMALoader::loadData() {
	QString file_name = QFileDialog::getOpenFileName(nullptr, "Open 3D images", "../../", "DIM/IMA files (*.dim *.ima)");
	std::string fname = file_name.toStdString();
	char* basename = FileBaseName(fname.c_str());

	char* ima_input_file_path = nullptr; ima_input_file_path = AppendExtension(basename, "ima");
	char* dim_input_file_path = nullptr; dim_input_file_path = AppendExtension(basename, "dim");

	// check the filenames are properly created
	if (dim_input_file_path == nullptr) { std::cerr << "base and ext concat could not be performed for dim" << '\n'; return this->data; }
	if (ima_input_file_path == nullptr) { std::cerr << "base and ext concat could not be performed for ima" << '\n'; return this->data; }

	// Open the files, and check they are open
	std::ifstream dim_input_file(dim_input_file_path);
	if (not dim_input_file.is_open()) {
		std::cerr << dim_input_file_path << " cannot be opened." << '\n';
		return this->data;
	}

	// Read info from DIM file :
	std::size_t nx, ny, nz;

	// read number of voxels :
	dim_input_file >> nx;
	dim_input_file >> ny;
	dim_input_file >> nz;

	// read info :
	std::string token, type;
	double input_dx, input_dy, input_dz;

	do {
		dim_input_file >> token;
		if (token.find("-type") != std::string::npos) { dim_input_file >> type; }
		else if (token.find("-dx") != std::string::npos) { dim_input_file >> input_dx; }
		else if (token.find("-dy") != std::string::npos) { dim_input_file >> input_dy; }
		else if (token.find("-dz") != std::string::npos) { dim_input_file >> input_dz; }
		else {
			std::cerr << "token " << token << " did not represent anything" << '\n';
		}
	} while (not dim_input_file.eof());

	std::cout << "IMA file at " << ima_input_file_path << " should contain a voxel grid with the following properties :" << '\n';
	std::cout << '\t' << "Voxels : " << nx << "x" << ny << "x" << nz << '\n';
	std::cout << '\t' << "Voxel dimensions : " << input_dx << "x" << input_dy << "x" << input_dz << '\n';
	std::cout << '\t' << "Type : " << type << '\n';
	std::cout << '\t' << "=== For the moment, ignoring dX, dY, and dZ values ===" << '\n';

	dim_input_file.close();
	// Open the IMA file :
	std::ifstream ima_input_file(ima_input_file_path);
	if (not ima_input_file.is_open()) {
		std::cerr << ima_input_file_path << " cannot be opened." << '\n';
		return this->data;
	}

	this->data.resize(nx*ny*nz);

	// TODO============================================
	// TODO============================================
	// This cast underneath could cause nasty problems. If some arise, check here first.
	// FIXME===========================================
	// FIXME===========================================
	ima_input_file.read((char*)this->data.data(), static_cast<std::size_t>(nx*ny*nz));

	return this->data;
}
