#
# The YAML C/Fortran config file API
#
CMAKE_MINIMUM_REQUIRED (VERSION 3.0)
PROJECT (YAMLConfigFile VERSION 0.1 LANGUAGES C)

# Must use GNUInstallDirs to install libraries into correct
# locations on all platforms.
INCLUDE(GNUInstallDirs)

# Add our local modlues to the module path
SET(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules/")

# Make the demo executables an optional build component:
OPTION(ENABLE_DEMO_PROGRAMS "Build demonstration programs" OFF)

# Common headers are found here:
SET(COMMON_HEADERS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")
INCLUDE_DIRECTORIES(${COMMON_HEADERS_DIR})

# The library itself:
ADD_SUBDIRECTORY(lib)

IF (ENABLE_DEMO_PROGRAMS)
	ADD_SUBDIRECTORY(demos)
ENDIF (ENABLE_DEMO_PROGRAMS)
