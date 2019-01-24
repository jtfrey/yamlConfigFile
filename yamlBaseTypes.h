/*
 * yamlBaseTypes
 * Simplified YAML interface for C/Fortran
 *
 * Basic types used by all component APIs.
 *
 */

#ifndef __YAMLBASETYPES_H__
#define __YAMLBASETYPES_H__

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <limits.h>
#include <stdbool.h>
#include <stdarg.h>
#include <errno.h>
#include <yaml.h>

/*!
    @typedef yamlErrorCode
    Type of error codes used in this library.
*/
typedef unsigned int yamlErrorCode;

/*!
    @enum yamlError
    Basic error codes for all components of the API.
*/
enum {
    yamlError_none = 0
};


/*!
    @typedef yamlOptions
    Type of bitwise-enumerated options used in this library.
*/
typedef unsigned int yamlOptionsBitvec;

/*!
    @enum yamlOptions
    Basic options for all components of the API.
*/
enum {
    yamlOptions_none = 0
};

#endif /* __YAMLBASETYPES_H__ */
