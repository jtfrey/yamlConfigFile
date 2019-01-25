#
# The YAML C/Fortran config file API
#
CMAKE_MINIMUM_REQUIRED (VERSION 3.0)
PROJECT (YAMLConfigFile VERSION 0.1 LANGUAGES C)

# Must use GNUInstallDirs to install libraries into correct
# locations on all platforms.
INCLUDE(GNUInstallDirs)

INCLUDE(CheckFunctionExists)

# Add our local modlues to the module path
SET(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules/")

FIND_PACKAGE(libyaml REQUIRED)

# The source make use of strncasecmp and strncpy, so let's be sure they're present:
CHECK_FUNCTION_EXISTS(strncasecmp HAVE_STRNCASECMP)
CHECK_FUNCTION_EXISTS(strncpy HAVE_STRNCPY)

# Make all variable substitutions and generate config.h:
CONFIGURE_FILE(yamlBaseTypes.h.in yamlBaseTypes.h)

# libyamlConfigFile.so
ADD_LIBRARY(libyamlConfigFile SHARED yamlBaseTypes.c yamlKeyPath.c yamlConfigFile.c yamlFortranInterface.c)
SET_TARGET_PROPERTIES(libyamlConfigFile PROPERTIES OUTPUT_NAME yamlConfigFile)
SET_TARGET_PROPERTIES(libyamlConfigFile PROPERTIES PUBLIC_HEADER "${CMAKE_CURRENT_BINARY_DIR}/yamlBaseTypes.h;yamlKeyPath.h;yamlConfigFile.h;yamlFortranInterface-f.h")
TARGET_INCLUDE_DIRECTORIES(libyamlConfigFile PUBLIC ${LIBYAML_INCLUDE_DIRS} ${CMAKE_CURRENT_BINARY_DIR})
TARGET_LINK_LIBRARIES(libyamlConfigFile ${LIBYAML_LIBRARIES})

# What does "make install" do?
INSTALL(TARGETS libyamlConfigFile
		LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
		PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
