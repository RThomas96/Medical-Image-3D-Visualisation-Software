# ResourceDependency.cmake :
# 	Like the ShaderDependency.cmake file, but for the icons
# 	used in the program.

FILE(COPY ${CMAKE_SOURCE_DIR}/resources DESTINATION ${CMAKE_BINARY_DIR})
FILE(COPY ${CMAKE_SOURCE_DIR}/shaders DESTINATION ${CMAKE_BINARY_DIR})
