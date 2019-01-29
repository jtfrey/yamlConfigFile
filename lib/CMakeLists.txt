
# Optional features:
OPTION(FORTRAN_INTEGER8 "Fortran INTEGER type defaults to 8-byte (64-bit)" OFF)
OPTION(FORTRAN_LOGICAL8 "Fortran LOGICAL type defaults to 8-byte (64-bit)" OFF)
OPTION(FORTRAN_REAL8 "Fortran REAL type defaults to 8-byte (64-bit, double-precision)" OFF)
OPTION(FORTRAN_NO_UNDERSCORING "Do not add an underscore (_) to C-implemented Fortran functions" OFF)

# We absolutely need libyaml:
FIND_PACKAGE(libyaml REQUIRED)

# The source make use of strncasecmp and strncpy, so let's be sure they're present:
INCLUDE(CheckFunctionExists)
CHECK_FUNCTION_EXISTS(strncasecmp HAVE_STRNCASECMP)
CHECK_FUNCTION_EXISTS(strncpy HAVE_STRNCPY)

# Make all variable substitutions and generate config.h:
CONFIGURE_FILE(yamlBaseTypes.h.in yamlBaseTypes.h)

# libyamlConfigFile.so
ADD_LIBRARY(libyamlConfigFile SHARED yamlBaseTypes.c yamlKeyPath.c yamlConfigFile.c yamlFortranInterface.c)
SET_TARGET_PROPERTIES(libyamlConfigFile PROPERTIES OUTPUT_NAME yamlConfigFile)
SET_TARGET_PROPERTIES(libyamlConfigFile PROPERTIES PUBLIC_HEADER "${CMAKE_CURRENT_BINARY_DIR}/yamlBaseTypes.h;${COMMON_HEADERS_DIR}/yamlKeyPath.h;${COMMON_HEADERS_DIR}/yamlConfigFile.h;${COMMON_HEADERS_DIR}/yamlFortranInterface-f.h")

# Should C functions usable from Fortran have a trailing underscore?
IF (FORTRAN_NO_UNDERSCORING)
	TARGET_COMPILE_DEFINITIONS(libyamlConfigFile PUBLIC FORTRAN_NO_UNDERSCORING=1)
ENDIF (FORTRAN_NO_UNDERSCORING)

TARGET_INCLUDE_DIRECTORIES(libyamlConfigFile PUBLIC ${LIBYAML_INCLUDE_DIRS} ${CMAKE_CURRENT_BINARY_DIR})
TARGET_LINK_LIBRARIES(libyamlConfigFile ${LIBYAML_LIBRARIES})

# What does "make install" do?
INSTALL(TARGETS libyamlConfigFile
		LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
		PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
