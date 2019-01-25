# - Try to find libyaml
# Once done this will define
#  LIBYAML_FOUND - System has libyaml
#  LIBYAML_INCLUDE_DIRS - The libyaml include directories
#  LIBYAML_LIBRARIES - The libraries needed to use libyaml
#  LIBYAML_DEFINITIONS - Compiler switches required for using libyaml

find_path(LIBYAML_INCLUDE_DIR yaml.h
          HINTS ${LIBYAML_INCLUDEDIR}
          PATH_SUFFIXES libyaml )

find_library(LIBYAML_LIBRARY NAMES yaml libyaml
             HINTS ${LIBYAML_LIBDIR})

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set LIBYAML_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(libyaml  DEFAULT_MSG
                                  LIBYAML_LIBRARY LIBYAML_INCLUDE_DIR)

mark_as_advanced(LIBYAML_INCLUDE_DIR LIBYAML_LIBRARY )

set(LIBYAML_LIBRARIES ${LIBYAML_LIBRARY} )
set(LIBYAML_INCLUDE_DIRS ${LIBYAML_INCLUDE_DIR} )
