# The mesh library :

ADD_SUBDIRECTORY(base_mesh)
ADD_SUBDIRECTORY(drawable)
IF (WIN32)
ADD_SUBDIRECTORY(operations/arap) # later ...
ENDIF()
