#include "../include/glwindow.hpp"

#include <fstream>

void GLProgram::compileShaders(void) {
	if (this->programHandle) { glDeleteProgram(this->programHandle); }
	if (this->vShaHandle) { glDeleteShader(this->vShaHandle); }
	if (this->fShaHandle) { glDeleteShader(this->fShaHandle); }

	// if a path was provided beforehand :
	if (this->vShaPath != "") {
		// create shader object
		this->vShaHandle = glCreateShader(GL_VERTEX_SHADER);
		// source contents :
		GLint vShaSize = 0;
		const GLchar* vShaContents = this->sourceShaderFile(this->vShaPath, vShaSize);
		// if contents exists :
		if (vShaContents != nullptr) {
			// send it to opengl
			glShaderSource(this->vShaHandle, 1, &vShaContents, &vShaSize);
			glCompileShader(this->vShaHandle);
			this->readShaderStatus(this->vShaHandle);
		} else {
			// delete shader
			glDeleteShader(this->vShaHandle);
		}
	}

	// if a path was provided beforehand :
	if (this->fShaPath != "") {
		// create shader object
		this->fShaHandle = glCreateShader(GL_FRAGMENT_SHADER);
		// source contents :
		GLint fShaSize = 0;
		const GLchar* fShaContents = this->sourceShaderFile(this->fShaPath, fShaSize);
		// if contents exists :
		if (fShaContents != nullptr) {
			// send it to opengl
			glShaderSource(this->fShaHandle, 1, &fShaContents, &fShaSize);
			glCompileShader(this->fShaHandle);
			this->readShaderStatus(this->fShaHandle);
		} else {
			// delete shader
			glDeleteShader(this->fShaHandle);
		}
	}
}

void GLProgram::compileShaders(const std::string vShaPathNew, const std::string fShaPathNew) {
	this->vShaPath = vShaPathNew;
	this->fShaPath = fShaPathNew;
	this->compileShaders();
}

GLint GLProgram::uniformLocation(const std::string uniformName) {
	if (this->programHandle) {
		this->bind();
		return glGetUniformLocation(this->programHandle, uniformName.c_str());
	}
}

GLchar* GLProgram::sourceShaderFile(const std::string pathname, GLint &size) {
	std::ifstream shaFile(pathname, std::ios::in);
	if (not shaFile.is_open()) {
		return nullptr;
	} else {
		shaFile.seekg(shaFile.end);
		std::size_t shaFileSize = static_cast<std::size_t>(shaFile.tellg());
		shaFile.seekg(shaFile.beg);
		GLchar* shaContents = new GLchar[shaFileSize];
		shaFile.read(shaContents, shaFileSize);
		shaFile.close();
		size = static_cast<GLint>(shaFileSize);
		return shaContents;
	}
}

void GLProgram::readShaderStatus(const GLuint handle) {
	GLint infoLogLength = 0;
	GLint charsWritten = 0;
	GLchar* infoLog;

	glGetShaderiv(handle, GL_INFO_LOG_LENGTH, &infoLogLength);
	if (infoLogLength > 0) {
		infoLog = new GLchar[infoLogLength+1];
		glGetShaderInfoLog(handle, infoLogLength, &charsWritten, infoLog);
		std::cerr << "Shader error : " << infoLog;
		delete[] infoLog;
	}
}
