#include "drawable_grid.hpp"

#include <glm/gtc/type_ptr.hpp>

#include <memory>

DrawableGrid::DrawableGrid(GridGLView::Ptr grid): gl(nullptr) {
    this->displayTetmesh = false;

    this->program = 0;
    this->vaoVolumetricBuffers = 0;
    this->grid = grid;
    this->blendFirstPass = 1.;
    this->drawOnlyBoundaries = true;
    this->multiGridRendering = false;

    std::cout << "Create drawable grid" << std::endl;
};

void DrawableGrid::recompileShaders() {
    std::cout << "Compile shaders of drawable grid" << std::endl;
    GLuint newVolumetricProgram	 = this->compileShaders("../shaders/transfer_mesh.vert", "../shaders/transfer_mesh.geom", "../shaders/transfer_mesh.frag");
    if (newVolumetricProgram) {
        gl->glDeleteProgram(this->program);
        this->program = newVolumetricProgram;
    }
}

void DrawableGrid::initializeGL(ShaderCompiler::GLFunctions *functions) {
    std::cout << "Initialize drawable grid" << std::endl;
    this->gl = functions;
    this->shaderCompiler = std::make_unique<ShaderCompiler>(functions);
    this->recompileShaders();
    this->createBuffers();
    this->generateColorScales();
    this->tex3D_buildBuffers();
}

void DrawableGrid::generateColorScales() {
    TextureUpload colorScaleUploadParameters;

    int maximumTextureSize = 0;
    glGetIntegerv(GL_MAX_3D_TEXTURE_SIZE, &maximumTextureSize);
    std::size_t textureSize = maximumTextureSize / 2u;
    float textureSize_f		= static_cast<float>(maximumTextureSize / 2u);

    std::vector<glm::vec3> colorScaleData_greyscale(textureSize);
    std::vector<glm::vec3> colorScaleData_hsv2rgb(textureSize);

    // Generate the greyscale :
    for (std::size_t i = 0; i < textureSize; ++i) {
        float intensity = static_cast<float>(i) / textureSize_f;
        glm::vec3 currentGreyscale(intensity, intensity, intensity);
        colorScaleData_greyscale[i] = currentGreyscale;
    }

    std::cerr << "Generated data for the greyscale color scale\n";

    // Generate the HSV2RGB :
    for (std::size_t i = 0; i < textureSize; ++i) {
        glm::vec3 hsv						= glm::vec3(float(i) / textureSize_f, 1., 1.);
        hsv.x								= glm::mod(100.0 + hsv.x, 1.0);	   // Ensure [0,1[
        float HueSlice						= 6.0 * hsv.x;	  // In [0,6[
        float HueSliceInteger				= floor(HueSlice);
        float HueSliceInterpolant			= HueSlice - HueSliceInteger;	 // In [0,1[ for each hue slice
        glm::vec3 TempRGB					= glm::vec3(hsv.z * (1.0 - hsv.y), hsv.z * (1.0 - hsv.y * HueSliceInterpolant), hsv.z * (1.0 - hsv.y * (1.0 - HueSliceInterpolant)));
        float IsOddSlice					= glm::mod(HueSliceInteger, 2.0f);	  // 0 if even (slices 0, 2, 4), 1 if odd (slices 1, 3, 5)
        float ThreeSliceSelector			= 0.5 * (HueSliceInteger - IsOddSlice);	   // (0, 1, 2) corresponding to slices (0, 2, 4) and (1, 3, 5)
        glm::vec3 ScrollingRGBForEvenSlices = glm::vec3(hsv.z, TempRGB.z, TempRGB.x);	 // (V, Temp Blue, Temp Red) for even slices (0, 2, 4)
        glm::vec3 ScrollingRGBForOddSlices	= glm::vec3(TempRGB.y, hsv.z, TempRGB.x);	 // (Temp Green, V, Temp Red) for odd slices (1, 3, 5)
        glm::vec3 ScrollingRGB				= mix(ScrollingRGBForEvenSlices, ScrollingRGBForOddSlices, IsOddSlice);
        float IsNotFirstSlice				= glm::clamp(ThreeSliceSelector, 0.0f, 1.0f);	 // 1 if NOT the first slice (true for slices 1 and 2)
        float IsNotSecondSlice				= glm::clamp(ThreeSliceSelector - 1.0f, 0.0f, 1.f);	   // 1 if NOT the first or second slice (true only for slice 2)
        colorScaleData_hsv2rgb[i]			= glm::vec4(glm::mix(glm::vec3(ScrollingRGB), glm::mix(glm::vec3(ScrollingRGB.z, ScrollingRGB.x, ScrollingRGB.y), glm::vec3(ScrollingRGB.y, ScrollingRGB.z, ScrollingRGB.x), IsNotSecondSlice), IsNotFirstSlice), 1.f);	   // Make the RGB rotate right depending on final slice index
    }

    std::cerr << "Generated data for the HSV2RGB color scale" << '\n';

    colorScaleUploadParameters.minmag.x	 = GL_LINEAR;
    colorScaleUploadParameters.minmag.y	 = GL_LINEAR;
    colorScaleUploadParameters.lod.y	 = -1000.f;
    colorScaleUploadParameters.wrap.x	 = GL_CLAMP_TO_EDGE;
    colorScaleUploadParameters.wrap.y	 = GL_CLAMP_TO_EDGE;
    colorScaleUploadParameters.wrap.z	 = GL_CLAMP_TO_EDGE;
    colorScaleUploadParameters.swizzle.r = GL_RED;
    colorScaleUploadParameters.swizzle.g = GL_GREEN;
    colorScaleUploadParameters.swizzle.b = GL_BLUE;
    colorScaleUploadParameters.swizzle.a = GL_ONE;

    colorScaleUploadParameters.level		  = 0;
    colorScaleUploadParameters.internalFormat = GL_RGB;
    colorScaleUploadParameters.size.x		  = textureSize;
    colorScaleUploadParameters.size.y		  = 1;
    colorScaleUploadParameters.size.z		  = 1;
    colorScaleUploadParameters.format		  = GL_RGB;
    colorScaleUploadParameters.type			  = GL_FLOAT;
    colorScaleUploadParameters.data			  = colorScaleData_greyscale.data();

    glDeleteTextures(1, &this->colorScaleGreyscale);
    this->colorScaleGreyscale = this->uploadTexture1D(colorScaleUploadParameters);

    colorScaleUploadParameters.data	   = colorScaleData_hsv2rgb.data();
    glDeleteTextures(1, &this->colorScaleHsv2rgb);
    this->colorScaleHsv2rgb = this->uploadTexture1D(colorScaleUploadParameters);
}

void DrawableGrid::createBuffers() {
    auto createVAO = [&, this](std::string name) -> GLuint {
        GLuint buf = 0;
        this->gl->glGenVertexArrays(1, &buf);
        this->gl->glBindVertexArray(buf);
        if (this->gl->glIsVertexArray(buf) == GL_FALSE) {
            std::cerr << "[ERROR][" << __FILE__ << ":" << __LINE__ << "] : Could not create VAO object " << name << '\n';
        }
        return buf;
    };

    /// @brief Create a buffer, bind it and see if it has been succesfully created server-side.
    auto createVBO = [&, this](GLenum bufType, std::string name) -> GLuint {
        GLuint buf = 0;
        this->gl->glGenBuffers(1, &buf);
        this->gl->glBindBuffer(bufType, buf);
        if (this->gl->glIsBuffer(buf) == GL_FALSE) {
            std::cerr << "[ERROR][" << __FILE__ << ":" << __LINE__ << "] : Could not create buffer object " << name << '\n';
        }
        return buf;
    };

    this->vaoVolumetricBuffers  = createVAO("vaoHandle_VolumetricBuffers");
    this->vboTexture3DVertPos  = createVBO(GL_ARRAY_BUFFER, "vboHandle_Texture3D_VertPos");
    this->vboTexture3DVertNorm = createVBO(GL_ARRAY_BUFFER, "vboHandle_Texture3D_VertNorm");
    this->vboTexture3DVertTex  = createVBO(GL_ARRAY_BUFFER, "vboHandle_Texture3D_VertTex");
    this->vboTexture3DVertIdx  = createVBO(GL_ELEMENT_ARRAY_BUFFER, "vboHandle_Texture3D_VertIdx");

    glGenTextures(1, &this->dualRenderingTexture);

    glBindTexture(GL_TEXTURE_2D, this->dualRenderingTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 2024, 1468, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    glGenTextures(1, &this->frameDepthBuffer);
    glBindTexture(GL_TEXTURE_2D, this->frameDepthBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0,GL_DEPTH_COMPONENT32, 2024, 1468, 0,GL_DEPTH_COMPONENT, GL_FLOAT, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    gl->glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, this->frameDepthBuffer, 0);

    // Set "renderedTexture" as our colour attachement #0
    gl->glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, this->dualRenderingTexture, 0);

    // Set the list of draw buffers.
    GLenum DrawBuffers[1] = {GL_COLOR_ATTACHMENT0};
    gl->glDrawBuffers(1, DrawBuffers); // "1" is the size of DrawBuffers

    // Always check that our framebuffer is ok
    if(gl->glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "WARNING: framebuffer doesn't work !!" << std::endl;
    else
        std::cout << "Framebuffer works perfectly :) !!" << std::endl;

}

GLuint DrawableGrid::compileShaders(std::string _vPath, std::string _gPath, std::string _fPath) {
    gl->glUseProgram(0);
    this->shaderCompiler->reset();
    this->shaderCompiler->pragmaReplacement_file("include_color_shader", "../shaders/colorize_new_flow.glsl");
    this->shaderCompiler->vertexShader_file(_vPath).geometryShader_file(_gPath).fragmentShader_file(_fPath);
    if (this->shaderCompiler->compileShaders()) {
        return this->shaderCompiler->programName();
    }
    std::cerr << this->shaderCompiler->errorString() << '\n';
    return 0;
}


void DrawableGrid::prepareUniforms(GLfloat* mvMat, GLfloat* pMat, glm::vec3 camPos, glm::vec3 planePosition, glm::vec3 planeDirection, bool drawFront) {
    /// @brief Shortcut for glGetUniform, since this can result in long lines.
    auto getUniform = [&](const char* name) -> GLint {
        GLint g = gl->glGetUniformLocation(program, name);
        return g;
    };

    gl->glUseProgram(program);

    std::size_t tex = 0;
    glActiveTexture(GL_TEXTURE0 + tex);
    glBindTexture(GL_TEXTURE_2D, vertexPositions);
    gl->glUniform1i(getUniform("vertices_translations"), tex);
    tex++;

    glActiveTexture(GL_TEXTURE0 + tex);
    glBindTexture(GL_TEXTURE_2D, faceNormals);
    gl->glUniform1i(getUniform("normals_translations"), tex);
    tex++;

    glActiveTexture(GL_TEXTURE0 + tex);
    //glBindTexture(GL_TEXTURE_2D, grid->volumetricMesh.visibilityMap);
    //gl->glUniform1i(getUniform("visibility_texture"), tex);
    tex++;

    glActiveTexture(GL_TEXTURE0 + tex);
    glBindTexture(GL_TEXTURE_2D, textureCoordinates);
    gl->glUniform1i(getUniform("texture_coordinates"), tex);
    tex++;

    glActiveTexture(GL_TEXTURE0 + tex);
    glBindTexture(GL_TEXTURE_2D, neighborhood);
    gl->glUniform1i(getUniform("neighbors"), tex);
    tex++;

    glActiveTexture(GL_TEXTURE0 + tex);
    glBindTexture(GL_TEXTURE_3D, gridTexture);
    gl->glUniform1i(getUniform("texData"), tex);
    tex++;

    glActiveTexture(GL_TEXTURE0 + tex);
    glBindTexture(GL_TEXTURE_2D, this->dualRenderingTexture);
    gl->glUniform1i(getUniform("firstPass_texture"), tex);
    tex++;

    glActiveTexture(GL_TEXTURE0 + tex);
    glBindTexture(GL_TEXTURE_2D, this->frameDepthBuffer);
    gl->glUniform1i(getUniform("firstPass_depthTexture"), tex);
    tex++;

    // For the segmented data visualisation
    // 1D texture that contain the value ranges to display
    glActiveTexture(GL_TEXTURE0 + tex);
    glBindTexture(GL_TEXTURE_1D, valuesRangeToDisplay);
    gl->glUniform1i(getUniform("valuesRangeToDisplay"), tex);
    tex++;

    // For the segmented data visualisation
    // 1D texture that contain the color associated for each range
    glActiveTexture(GL_TEXTURE0 + tex);
    glBindTexture(GL_TEXTURE_1D, valuesRangeColorToDisplay);
    gl->glUniform1i(getUniform("colorRangeToDisplay"), tex);
    tex++;

    float val = 1.;
    if(drawFront)
        val = 0.;
    gl->glUniform1fv(getUniform("isFirstPass"), 1, &val);

    glm::vec3 floatres = glm::convert_to<float>(grid->grid->sampler.getSamplerDimension());
    gl->glUniform3fv(getUniform("voxelSize"), 1, glm::value_ptr(grid->grid->getVoxelSize()));
    gl->glUniform3fv(getUniform("gridSize"), 1, glm::value_ptr(floatres));
    gl->glUniform3fv(getUniform("visuBBMin"), 1, glm::value_ptr(grid->grid->bbMin));
    gl->glUniform3fv(getUniform("visuBBMax"), 1, glm::value_ptr(grid->grid->bbMax));
    gl->glUniform1ui(getUniform("shouldUseBB"), 0);
    gl->glUniform1f(getUniform("maxValue"), grid->grid->maxValue);
    gl->glUniform3fv(getUniform("volumeEpsilon"), 1, glm::value_ptr(glm::vec3(1.5, 1.5, 1.5)));

    gl->glUniform3fv(getUniform("cam"), 1, glm::value_ptr(camPos));
    gl->glUniform3fv(getUniform("cut"), 1, glm::value_ptr(planePosition));
    gl->glUniform3fv(getUniform("cutDirection"), 1, glm::value_ptr(planeDirection));
    // Clip distance
    gl->glUniform1f(getUniform("clipDistanceFromCamera"), 5.f);

    gl->glUniform1ui(getUniform("displayWireframe"), this->displayTetmesh);
    gl->glUniform1f(getUniform("blendFirstPass"), this->blendFirstPass);

    int drawOnly = 1;
    if(!this->drawOnlyBoundaries)
        drawOnly = 0;
    gl->glUniform1i(getUniform("drawOnlyBoundaries"), drawOnly);

    gl->glUniformMatrix4fv(getUniform("mMat"), 1, GL_FALSE, glm::value_ptr(glm::mat4(1.f)));
    gl->glUniformMatrix4fv(getUniform("vMat"), 1, GL_FALSE, mvMat);
    gl->glUniformMatrix4fv(getUniform("pMat"), 1, GL_FALSE, pMat);

    gl->glUniform1f(getUniform("specRef"), .8f);
    gl->glUniform1f(getUniform("shininess"), .8f);
    gl->glUniform1f(getUniform("diffuseRef"), .8f);

    gl->glUniform3fv(getUniform("color0"), 1, glm::value_ptr(grid->color_0));
    gl->glUniform3fv(getUniform("color1"), 1, glm::value_ptr(grid->color_1));

    glActiveTexture(GL_TEXTURE0 + tex);
    glBindTexture(GL_TEXTURE_1D, this->colorScaleGreyscale);
    gl->glUniform1i(getUniform("colorScales[0]"), tex);
    tex++;

    glActiveTexture(GL_TEXTURE0 + tex);
    glBindTexture(GL_TEXTURE_1D, this->colorScaleHsv2rgb);
    gl->glUniform1i(getUniform("colorScales[1]"), tex);
    tex++;

    glActiveTexture(GL_TEXTURE0 + tex);
    glBindTexture(GL_TEXTURE_1D, this->colorScaleUser);
    gl->glUniform1i(getUniform("colorScales[2]"), tex);
    tex++;

    glActiveTexture(GL_TEXTURE0 + tex);
    glBindTexture(GL_TEXTURE_1D, this->colorScaleUser);
    gl->glUniform1i(getUniform("colorScales[3]"), tex);
    tex++;

    // Directly copy the uboHandle_color class into the GPU
    gl->glBindBufferBase(GL_UNIFORM_BUFFER, 0, uboHandle_colorAttributes);
}

GLuint DrawableGrid::uploadTexture1D(const TextureUpload& tex) {
    glEnable(GL_TEXTURE_1D);

    GLuint texHandle = 0;
    glGenTextures(1, &texHandle);
    glBindTexture(GL_TEXTURE_1D, texHandle);

    // Min and mag filters :
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, tex.minmag.x);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, tex.minmag.y);

    // Set the min and max LOD values :
    glTexParameterf(GL_TEXTURE_1D, GL_TEXTURE_MIN_LOD, tex.lod.x);
    glTexParameterf(GL_TEXTURE_1D, GL_TEXTURE_MAX_LOD, tex.lod.y);

    // Set the wrap parameters :
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, tex.wrap.x);

    // Set the swizzle the user wants :
    glTexParameteriv(GL_TEXTURE_1D, GL_TEXTURE_SWIZZLE_RGBA, glm::value_ptr(tex.swizzle));

    // Set the pixel alignment :
    glPixelStorei(GL_PACK_ALIGNMENT, tex.alignment.x);
    glPixelStorei(GL_UNPACK_ALIGNMENT, tex.alignment.y);

    glTexImage1D(GL_TEXTURE_1D,	   // GLenum : Target
      static_cast<GLint>(tex.level),	// GLint  : Level of detail of the current texture (0 = original)
      tex.internalFormat,	 // GLint  : Number of color components in the picture.
      tex.size.x,	 // GLsizei: Image width
      static_cast<GLint>(0),	// GLint  : Border. This value MUST be 0.
      tex.format,	 // GLenum : Format of the pixel data
      tex.type,	   // GLenum : Type (the data type as in uchar, uint, float ...)
      tex.data	  // void*  : Data to load into the buffer
    );

    return texHandle;
}

void DrawableGrid::tex3D_buildBuffers() {
    // Tetra ///////////////////////////////////////////////////////////////////////
    //    v0----- v
    //   /       /|
    //  v ------v3|
    //  | |     | |
    //  | |v ---|-|v2
    //  |/      |/
    //  v1------v

    float v0[3] = {-1, -1, 1};
    float v1[3] = {-1, 1, -1};
    float v2[3] = {1, -1, -1};
    float v3[3] = {1, 1, 1};

    // vertex coords array
    GLfloat vertices[] = {v3[0], v3[1], v3[2], v1[0], v1[1], v1[2], v2[0], v2[1], v2[2],	// v3-v1-v2
      v3[0], v3[1], v3[2], v2[0], v2[1], v2[2], v1[0], v1[1], v1[2],	// v3-v2-v1
      v3[0], v3[1], v3[2], v0[0], v0[1], v0[2], v1[0], v1[1], v1[2],	// v3-v0-v1
      v2[0], v2[1], v2[2], v1[0], v1[1], v1[2], v0[0], v0[1], v0[2]};	 // v2-v1-v0
    // normal array
    GLfloat normals[] = {v3[0], v3[1], v3[2], v1[0], v1[1], v1[2], v2[0], v2[1], v2[2],	   // v3-v1-v2
      v3[0], v3[1], v3[2], v2[0], v2[1], v2[2], v1[0], v1[1], v1[2],	// v3-v2-v1
      v3[0], v3[1], v3[2], v0[0], v0[1], v0[2], v1[0], v1[1], v1[2],	// v3-v0-v1
      v2[0], v2[1], v2[2], v1[0], v1[1], v1[2], v0[0], v0[1], v0[2]};	 // v2-v1-v0
    // index array of vertex array for glDrawElements()
    // Notice the indices are listed straight from beginning to end as exactly
    // same order of vertex array without hopping, because of different normals at
    // a shared vertex. For this case, glDrawArrays() and glDrawElements() have no
    // difference.
    GLushort indices[] = {0, 1, 2,
      3, 4, 5,
      6, 7, 8,
      9, 10, 11};
    // texture coords :
    GLfloat textureCoords[] = {0., 0., 1., 1., 2., 2.,
      3., 3., 4., 4., 5., 5.,
      6., 6., 7., 7., 8., 8.,
      9., 9., 10., 10., 11., 11.};

    gl->glBindBuffer(GL_ARRAY_BUFFER, this->vboTexture3DVertPos);
    gl->glBufferData(GL_ARRAY_BUFFER, 12 * 3 * sizeof(GLfloat), vertices, GL_STATIC_DRAW);

    gl->glBindBuffer(GL_ARRAY_BUFFER, this->vboTexture3DVertNorm);
    gl->glBufferData(GL_ARRAY_BUFFER, 12 * 3 * sizeof(GLfloat), normals, GL_STATIC_DRAW);

    gl->glBindBuffer(GL_ARRAY_BUFFER, this->vboTexture3DVertTex);
    gl->glBufferData(GL_ARRAY_BUFFER, 12 * 2 * sizeof(GLfloat), textureCoords, GL_STATIC_DRAW);

    gl->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->vboTexture3DVertIdx);
    gl->glBufferData(GL_ELEMENT_ARRAY_BUFFER, 12 * sizeof(GLushort), indices, GL_STATIC_DRAW);
}

void DrawableGrid::drawGrid(GLfloat *mvMat, GLfloat *pMat, glm::vec3 camPos, glm::vec3 planePosition, glm::vec3 planeDirection, bool inFrame) {
    //this->updateMinMaxDisplayValues();
    this->prepareUniforms(mvMat, pMat, camPos, planePosition, planeDirection, !inFrame);
    gl->glBindVertexArray(this->vaoVolumetricBuffers);

    gl->glEnableVertexAttribArray(0);
    gl->glBindBuffer(GL_ARRAY_BUFFER, this->vboTexture3DVertPos);
    gl->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*) 0);

    gl->glEnableVertexAttribArray(1);
    gl->glBindBuffer(GL_ARRAY_BUFFER, this->vboTexture3DVertNorm);
    gl->glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, (void*) 0);

    gl->glEnableVertexAttribArray(2);
    gl->glBindBuffer(GL_ARRAY_BUFFER, this->vboTexture3DVertTex);
    gl->glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, (void*) 0);

    gl->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->vboTexture3DVertIdx);

    if(inFrame && this->multiGridRendering) {
        GLint defaultFBO;
        gl->glGetIntegerv(GL_FRAMEBUFFER_BINDING, &defaultFBO);

        gl->glDeleteFramebuffers(1, &this->frameBuffer);
        gl->glGenFramebuffers(1, &this->frameBuffer);
        gl->glBindFramebuffer(GL_FRAMEBUFFER, this->frameBuffer);

        glDeleteTextures(1, &dualRenderingTexture);
        glGenTextures(1, &dualRenderingTexture);

        glBindTexture(GL_TEXTURE_2D, dualRenderingTexture);
        glTexImage2D(GL_TEXTURE_2D, 0,GL_RGB, 2024, 1468, 0,GL_RGB, GL_UNSIGNED_BYTE, 0);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

        gl->glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, dualRenderingTexture, 0);

        glDeleteTextures(1, &frameDepthBuffer);
        glGenTextures(1, &frameDepthBuffer);
        glBindTexture(GL_TEXTURE_2D, frameDepthBuffer);
        glTexImage2D(GL_TEXTURE_2D, 0,GL_DEPTH_COMPONENT16, 2024, 1468, 0,GL_DEPTH_COMPONENT, GL_FLOAT, 0);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        gl->glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, frameDepthBuffer, 0);

        // Set the list of draw buffers.
        GLenum DrawBuffers[1] = {GL_COLOR_ATTACHMENT0};
        gl->glDrawBuffers(1, DrawBuffers); // "1" is the size of DrawBuffers
        /***/
        gl->glDrawElementsInstanced(GL_TRIANGLES, 12, GL_UNSIGNED_SHORT, (void*) 0, tetrahedraCount);
        gl->glBindFramebuffer(GL_FRAMEBUFFER, defaultFBO);
        //glDrawBuffer(0);
    } else {
        gl->glDrawElementsInstanced(GL_TRIANGLES, 12, GL_UNSIGNED_SHORT, (void*) 0, tetrahedraCount);
    }

    // Unbind program, buffers and VAO :
    gl->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    gl->glBindVertexArray(0);
    gl->glUseProgram(0);
}

void DrawableGrid::setMultiGridRendering(bool value) {
    if(!value && this->multiGridRendering) {
        gl->glDeleteFramebuffers(1, &this->frameBuffer);
        glDeleteTextures(1, &this->dualRenderingTexture);
        glDeleteTextures(1, &this->frameDepthBuffer);
    }
    this->multiGridRendering = value;
    //if(this->isCage(activeMesh)) {
    //    int gridIdx = this->getGridIdxLinkToCage(activeMesh);
    //    if(gridIdx != -1) {
    //        this->gridToDraw = gridIdx;
    //        this->sendTetmeshToGPU(gridIdx, InfoToSend(InfoToSend::VERTICES | InfoToSend::NORMALS | InfoToSend::TEXCOORD | InfoToSend::NEIGHBORS));
    //    }
    //}
}

void DrawableGrid::setUniformBufferData(GLuint uniform_buffer, std::size_t begin_bytes, std::size_t size_bytes, GLvoid* data) {
    gl->glBindBuffer(GL_UNIFORM_BUFFER, uniform_buffer);
    gl->glBufferSubData(GL_UNIFORM_BUFFER, begin_bytes, size_bytes, data);
}

void DrawableGrid::updateMinMaxDisplayValues() {
    this->setUniformBufferData(uboHandle_colorAttributes, 0, 32, &grid->mainColorChannelAttributes());
    this->setUniformBufferData(uboHandle_colorAttributes, 32, 32, &grid->colorChannelAttributes[0]);
    this->setUniformBufferData(uboHandle_colorAttributes, 64, 32, &grid->colorChannelAttributes[1]);
    this->setUniformBufferData(uboHandle_colorAttributes, 96, 32, &grid->colorChannelAttributes[2]);

    float maxValue = grid->grid->maxValue;
    glDeleteTextures(1, &valuesRangeToDisplay);

    TextureUpload texParams;

    texParams.minmag.x         = GL_NEAREST;
    texParams.minmag.y         = GL_NEAREST;
    texParams.lod.y	           = -1000.f;
    texParams.wrap.s	       = GL_CLAMP_TO_EDGE;
    texParams.wrap.t	       = GL_CLAMP_TO_EDGE;

    texParams.internalFormat   = GL_RGB32F;
    texParams.size.y		   = 1;
    texParams.size.z		   = 1;
    texParams.format		   = GL_RGB;
    texParams.type		       = GL_FLOAT;

    std::vector<glm::vec3> data;
    std::vector<glm::vec3> data_color;
    for(int i = 0; i <= maxValue; ++i) {
        data.push_back(glm::vec3(0., 0., 0.));
        data_color.push_back(glm::vec3(0., 0., 0.));
    }
    for(int i = 0; i < displayRangeSegmentedData.size(); ++i) {
        if(displaySegmentedData[i]) {
            for(int j = displayRangeSegmentedData[i].first; j <= displayRangeSegmentedData[i].second; ++j) {
                if(j < data.size()) {
                    data[j] = glm::vec3(1., 1., 1.);
                    data_color[j] = displayColorSegmentedData[i];
                }
            }
        }
    }

    texParams.size.x		   = data.size();
    texParams.data			   = data.data();

    valuesRangeToDisplay = this->uploadTexture1D(texParams);

    texParams.size.x		   = data_color.size();
    texParams.data			   = data_color.data();

    valuesRangeColorToDisplay = this->uploadTexture1D(texParams);

    grid->visu_map = data;
    grid->color_map = data_color;
}
