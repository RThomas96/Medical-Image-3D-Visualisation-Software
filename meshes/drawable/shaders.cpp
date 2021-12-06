#include "./shaders.hpp"

#include <QGLViewer/qglviewer.h>
#include <QOpenGLWidget>

#include <fstream>
#include <sstream>

ShaderCompiler::ShaderCompiler(ShaderCompiler::GLFunctions* functions) :
	vshader_contents(), gshader_contents(), fshader_contents(),
	vshader_handle(0), gshader_handle(0), fshader_handle(0),
	program_handle(0), error_text(), pragma_replacements()
{
	this->gl = functions;
}

ShaderCompiler::~ShaderCompiler() = default;

const std::vector<ShaderCompiler::PragmaReplacement>& ShaderCompiler::pragmaReplacements() const {
	return this->pragma_replacements;
}

ShaderCompiler& ShaderCompiler::pragmaReplacement_file(std::string pragma, std::string filename) {
	if (pragma.empty()) { return *this; }
	std::ifstream file_in(filename, std::ios_base::in | std::ios_base::binary);
	if (not file_in.is_open()) { return *this; }
	std::size_t file_size = 0;
	std::string file_contents = "";

	file_in.seekg(0, file_in.end);
	file_size = static_cast<std::size_t>(file_in.tellg());
	file_in.seekg(0, file_in.beg);
	file_contents.resize(file_size);
	file_in.read(file_contents.data(), file_size);

	this->pragma_replacements.emplace_back(std::make_pair(pragma, file_contents));
	return *this;
}

ShaderCompiler& ShaderCompiler::pragmaReplacement_text(std::string pragma, std::string replacement) {
	this->pragma_replacements.emplace_back(std::make_pair(pragma, replacement));
	return *this;
}

const GLuint ShaderCompiler::vertexShader() const {
	return this->vshader_handle;
}

ShaderCompiler& ShaderCompiler::vertexShader_file(const std::string& _vshader_file) {
	if (_vshader_file.empty()) { return *this; }
	std::string shader_contents = this->parse_file(_vshader_file);
	if (not shader_contents.empty()) {
		this->vshader_contents = shader_contents;
	}
	return *this;
}

const GLuint ShaderCompiler::geometryShader() const {
	return this->gshader_handle;
}

ShaderCompiler& ShaderCompiler::geometryShader_file(const std::string& _gshader_file) {
	if (_gshader_file.empty()) { return *this; }
	std::string shader_contents = this->parse_file(_gshader_file);
	if (not shader_contents.empty()) {
		this->gshader_contents = shader_contents;
	}
	return *this;
}

const GLuint ShaderCompiler::fragmentShader() const {
	return this->fshader_handle;
}

ShaderCompiler& ShaderCompiler::fragmentShader_file(const std::string& _fshader_file) {
	if (_fshader_file.empty()) { return *this; }
	std::string shader_contents = this->parse_file(_fshader_file);
	if (not shader_contents.empty()) {
		this->fshader_contents = shader_contents;
	}
	return *this;
}

bool ShaderCompiler::compileShaders() {
	GLboolean compilerAvailable = GL_FALSE;
	glGetBooleanv(GL_SHADER_COMPILER, &compilerAvailable);
	if (compilerAvailable == GL_FALSE) {
		std::stringstream error_feed;
		error_feed << "[" << __FILE__ << ":" << __LINE__ << "] : No shader compiler was available.\nExiting the program.\n";
		this->error_text += error_feed.str();
		exit(EXIT_FAILURE);
	}

	if (not this->vshader_contents.empty()) {
		GLuint new_vshader = this->compile_shader(this->vshader_contents, GL_VERTEX_SHADER);
		if (new_vshader != 0) {
			// If gotten here, shader is validated :
			this->vshader_handle = new_vshader;
		}
	} else { return false; }
	if (not this->gshader_contents.empty()){
		GLuint new_gshader = this->compile_shader(this->gshader_contents, GL_GEOMETRY_SHADER);
		if (new_gshader != 0) {
			// If gotten here, shader is validated :
			this->gshader_handle = new_gshader;
		}
	} else { return false; }
	if (not this->fshader_contents.empty()) {
		GLuint new_fshader = this->compile_shader(this->fshader_contents, GL_FRAGMENT_SHADER);
		if (new_fshader != 0) {
			// If gotten here, shader is validated :
			this->fshader_handle = new_fshader;
		}
	} else { return false; }

	GLuint new_program = this->gl->glCreateProgram();
	if (this->vshader_handle != 0) { this->gl->glAttachShader(new_program, this->vshader_handle); } else { return false; }
	if (this->gshader_handle != 0) { this->gl->glAttachShader(new_program, this->gshader_handle); }
	if (this->fshader_handle != 0) { this->gl->glAttachShader(new_program, this->fshader_handle); } else { return false; }

	this->gl->glLinkProgram(new_program);

	int InfoLogLength = 0;
	std::stringstream error_feed;
	this->gl->glGetProgramiv(new_program, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0) {
		std::vector<char> ProgramErrorMessage(InfoLogLength + 1);
		this->gl->glGetProgramInfoLog(new_program, InfoLogLength, NULL, &ProgramErrorMessage[0]);
		error_feed << __FILE__ << ":" << __LINE__ << " : Warning : errors while linking program :" << '\n';
		error_feed << "------------------------------------------------------------------" << '\n';
		error_feed << "------------------------------------------------------------------" << '\n';
		error_feed << ProgramErrorMessage.data() << '\n';
		error_feed << "------------------------------------------------------------------" << '\n';
		error_feed << "------------------------------------------------------------------" << '\n';
	}

	GLint Result = GL_FALSE;
	this->gl->glGetProgramiv(new_program, GL_LINK_STATUS, &Result);
	if (Result == GL_FALSE) {
		// Return 0 (no program created) :
		error_feed << "[" << __FILE__ << ":" << __LINE__ << "] : Could not link shader.\n";
		this->error_text += error_feed.str();
		return false;
	}
	// Change the shader associated !
	this->program_handle = new_program;
	this->error_text += error_feed.str();
	return true;
}

GLuint ShaderCompiler::programName() const {
	return this->program_handle;
}

bool ShaderCompiler::hasError() const {
	return (this->program_handle == 0) && not this->error_text.empty();
}

std::string ShaderCompiler::errorString() const {
	return this->error_text;
}

std::string ShaderCompiler::parse_file(std::string filename) {
	// Read the file and replace the strings that are known :
	std::ifstream shader_file = std::ifstream(filename.c_str(), std::ios_base::in | std::ios_base::binary);
	if (not shader_file.is_open()) {
		this->error_text += "\nError : could not open file \"" + filename + "\".\n";
		return "";
	}

	std::string read_buffer, shader_contents;
	while (getline(shader_file, read_buffer)) {
		if (read_buffer.find("#pragma") != std::string::npos) {
			std::string replacement = this->token_to_replacement(read_buffer);
			if (not replacement.empty()) {
				read_buffer = replacement;
			}
		}
		shader_contents += read_buffer + '\n';
	}
	shader_contents += '\0';

	return shader_contents;
}

std::string ShaderCompiler::token_to_replacement(std::string pragma_token) {
	for(const auto& pragma_replacement : this->pragma_replacements) {
		if (pragma_token.find(pragma_replacement.first) != std::string::npos) {
			return pragma_replacement.second;
		}
	}
	return std::string();
}

GLuint ShaderCompiler::compile_shader(std::string contents, GLuint type) {
	// Create handle, and source the file :
	GLuint shader_id = this->gl->glCreateShader(type);
	std::size_t shader_size = contents.size();
	const char* shader_contents = contents.c_str();
	this->gl->glShaderSource(shader_id, 1, const_cast<const char**>(&shader_contents), NULL);
	this->gl->glCompileShader(shader_id);

	GLint shaderInfoLength = 0;
	GLint charsWritten	   = 0;
	char* shaderInfoLog	   = nullptr;
	std::stringstream error_feed;
	std::string shader_type_str = "";
	switch (type) {
		case GL_VERTEX_SHADER :			shader_type_str = "vertex"; break;
		case GL_GEOMETRY_SHADER :		shader_type_str = "geometry"; break;
		case GL_FRAGMENT_SHADER :		shader_type_str = "fragment"; break;
		case GL_COMPUTE_SHADER :		shader_type_str = "compute"; break;
		case GL_TESS_EVALUATION_SHADER:	shader_type_str = "tesselation evaluation"; break;
		case GL_TESS_CONTROL_SHADER:	shader_type_str = "tesselation control"; break;
		default:						shader_type_str = "<unknown>"; break;
	}

	// Get shader information after compilation :
	this->gl->glGetShaderiv(shader_id, GL_INFO_LOG_LENGTH, &shaderInfoLength);
	if (shaderInfoLength > 1) {
		error_feed << __FILE__ << ":" << __LINE__ << " : start Log ***********************************************" << '\n';

		error_feed << "Information about " << shader_type_str << " shader : " << '\n';
		shaderInfoLog = new char[shaderInfoLength];
		this->gl->glGetShaderInfoLog(shader_id, shaderInfoLength, &charsWritten, shaderInfoLog);
		error_feed << shaderInfoLog << '\n';
		delete[] shaderInfoLog;

		error_feed << __FILE__ << ":" << __LINE__ << " : end Log ***********************************************" << '\n';

		/*error_feed << "Shader contents ... ===============================\n";
		error_feed << "Shader contents ... ===============================\n";
		error_feed << contents;
		error_feed << "Shader contents ... ===============================\n";
		error_feed << "Shader contents ... ===============================\n";*/
	}
	this->error_text += error_feed.str();
	error_feed.str(""); // this resets the contents of the stringstream

	// Check compilation status :
	GLint result = GL_FALSE;
	this->gl->glGetShaderiv(shader_id, GL_COMPILE_STATUS, &result);
	if (result == GL_FALSE) {
		error_feed << "[" << __FILE__ << ":" << __LINE__ << "] : Could not compile shader.\n";
		return 0;
	}

	// If gotten here, shader is validated :
	return shader_id;
}

void ShaderCompiler::reset() {
	this->vshader_contents = "";
	this->gshader_contents = "";
	this->fshader_contents = "";
	this->vshader_handle = 0;
	this->gshader_handle = 0;
	this->fshader_handle = 0;
	this->program_handle = 0;
	this->error_text = "";
	this->pragma_replacements.clear();
}
