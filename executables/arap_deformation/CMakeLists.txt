# Targets for the arap_deformation executable.

ADD_EXECUTABLE(arap_deformation
	../../qt/include/dialog_pick_grids_from_scene.hpp
	# Header files, for Qt MOCS compilation :
	../../viewer/include/scene.hpp
	../../viewer/include/planar_viewer.hpp
	../../viewer/include/neighbor_visu_viewer.hpp
	../../viewer/include/viewer_structs.hpp
	../../third_party/primitive/Sphere.h
	../../qt/include/GLmanipulator.hpp
	../../qt/include/manipulator.hpp
	qt/include/arap_controller.hpp
	qt/include/arap_main_widget.hpp
	qt/include/loader_widget.hpp

	# Main file :
	./arap_main.cpp

	# Source files :
	../../viewer/src/scene.cpp
	../../viewer/src/planar_viewer.cpp
	../../viewer/src/neighbor_visu_viewer.cpp
	../../viewer/src/viewer_structs.cpp
	../../third_party/primitive/Sphere.cpp
	../../qt/src/GLmanipulator.cpp
	../../qt/src/manipulator.cpp
	qt/src/arap_controller.cpp
	qt/src/arap_main_widget.cpp
	qt/src/loader_widget.cpp
)
TARGET_COMPILE_OPTIONS(arap_deformation
	# Set so CMake (more precisely clang/gcc) will not print 30-80 warnings about QGLViewer's
	# use of QString::null, a deprecated feature (causes long compilation times, and tons of
	# warnings which make it harder to filter through our own warnings/errors).
	PUBLIC "-Wno-deprecated-declarations"
)
TARGET_PRECOMPILE_HEADERS(arap_deformation
	# Made so that QtCreator will precompile this header and
	# shut up about the deprecated functions used within :
	PRIVATE ${QGLViewer_HEADER_DIR}/QGLViewer/qglviewer.h
	# Also, dramatically reduces compilation time.
)
TARGET_INCLUDE_DIRECTORIES(arap_deformation
	PRIVATE ${OPENGL_INCLUDE_DIRS}
	PRIVATE ${QGLViewer_HEADER_DIR}
	PRIVATE ${CMAKE_CURRENT_LIST_DIR}/../../third_party/glm/
	PRIVATE ${LOCAL_COMPILED_LIBS_PATH}/include
	# Nifti uses brace-include without the 'nifti/' prefix, leading
	# to manually have to include the lib's include dir in targets:
	PRIVATE ${LOCAL_COMPILED_LIBS_PATH}/include/nifti
	PRIVATE ${GSL_INCLUDE_DIRS}
)
TARGET_LINK_LIBRARIES(arap_deformation
	PUBLIC Qt5::Core
	PUBLIC Qt5::Gui
	PUBLIC Qt5::Xml
	PUBLIC Qt5::Widgets
	PUBLIC Qt5::OpenGL
	PUBLIC ${OPENGL_LIBRARIES}
	PUBLIC ${QGLViewer}
	PUBLIC ImageIO
	PUBLIC DiscreteGrid
	PUBLIC VisualisationWidgets
	PUBLIC writergrid
	PUBLIC writergrid_tiff
	PUBLIC ${CMAKE_PROJECT_NAME}_shaders
	PUBLIC ${CMAKE_PROJECT_NAME}_mesh
	PUBLIC ${CMAKE_PROJECT_NAME}_deformable_curve
	PUBLIC ${CMAKE_PROJECT_NAME}_meshop_arap
	PUBLIC ${CMAKE_PROJECT_NAME}_drawable_meshes
)
# }}}
