
ADD_EXECUTABLE(findnorm findnorm_cmake.c)

SET(ELMERGRID_BIN "${CMAKE_BINARY_DIR}/elmergrid/src/ElmerGrid")
SET(ELMERSOLVER_BIN "${CMAKE_BINARY_DIR}/fem/src/ElmerSolver_mpi")
SET(FINDNORM_BIN "${CMAKE_CURRENT_BINARY_DIR}/findnorm")

SET(ENV{ELMER_LIB} "{CMAKE_SOURCE_DIR}/fem/src")
SET(ENV{ELMER_HOME} "{CMAKE_SOURCE_DIR}/fem/src")

ADD_SUBDIRECTORY(1dtests)
ADD_SUBDIRECTORY(1sttime)
ADD_SUBDIRECTORY(2ndtime)
ADD_SUBDIRECTORY(adaptivity2)
