/*
 * yamlFortranInterface
 * Simplified YAML interface for C/Fortran
 *
 * Fortran API.
 *
 */

#include "yamlKeyPath.h"
#include "yamlConfigFile.h"

//

#ifdef FORTRAN_INTEGER8
#define YAMLFORTRANINTERFACE_INT_MAX	LONG_MAX
#define YAMLFORTRANINTERFACE_INT_MIN	LONG_MIN
typedef long int	yamlFortranInterfaceInteger;
#else
#define YAMLFORTRANINTERFACE_INT_MAX	INT_MAX
#define YAMLFORTRANINTERFACE_INT_MIN	INT_MIN
typedef int         yamlFortranInterfaceInteger;
#endif

#ifdef FORTRAN_LOGICAL8
typedef long int    yamlFortranInterfaceLogical;
#else
typedef int         yamlFortranInterfaceLogical;
#endif

#ifdef FORTRAN_REAL8
typedef double		yamlFortranInterfaceReal;
#else
typedef float		yamlFortranInterfaceReal;
#endif

//

enum {
    yamlFortranInterfaceError_min = 300,
    yamlFortranInterfaceError_invalidUnit = yamlFortranInterfaceError_min,
    yamlFortranInterfaceError_integerRange,
    yamlFortranInterfaceError_invalidYAMLNodeTypeString,
    yamlFortranInterfaceError_max
};

//

const char*
__yamlFortranInterfaceQuickKeyPathString(
    yamlKeyPathRef          aKeyPath
)
{
    static char             *sharedBuffer = NULL;
    static size_t           sharedBufferLen = 0;
    size_t                  actualLen = yamlKeyPathSprintf(aKeyPath, sharedBuffer, sharedBufferLen);
    
    if ( actualLen >= sharedBufferLen ) {
        size_t              blockCount = (actualLen / 64) + (actualLen % 64) ? 1 : 0;
        char                *newBuffer = realloc(sharedBuffer, blockCount * 64);
        
        if ( ! newBuffer ) {
            fprintf(stderr, "[YAML::Fortran] fatal error -- out of memory\n");
            exit(ENOMEM);
        }
        sharedBuffer = newBuffer;
        sharedBufferLen = blockCount * 64;
        
        yamlKeyPathSprintf(aKeyPath, sharedBuffer, sharedBufferLen);
    }
    return (const char*)sharedBuffer;
}

//

void
yamlconfigfile_open_(
	const char                  *filename,
	yamlFortranInterfaceInteger	*yamlUnit,
	yamlFortranInterfaceInteger	filenameLength
)
{
	*yamlUnit = 0;
	if ( filenameLength > 0 ) {
		char		*C_filename = malloc(filenameLength + 1);
	
		if ( C_filename ) {
			yamlConfigFileRef   configFile;
			
			memcpy(C_filename, filename, filenameLength);
			C_filename[filenameLength] = '\0';
			
			configFile = yamlConfigFileCreateWithFileAtPath(C_filename, yamlConfigFileOptions_oneBasedIndices | yamlConfigFileOptions_caseFoldKeys);
			if ( configFile ) {
                yamlConfigFileId    fileId = yamlConfigFileGetId(configFile);
                
                if ( fileId != YAMLCONFIGFILE_UNCACHED_ID ) *yamlUnit = fileId;
                yamlConfigFileRelease(configFile);
            }
			free(C_filename);
		}
	}	
}

//

void
yamlconfigfile_close_(
	yamlFortranInterfaceInteger *yamlUnit
)
{
	yamlConfigFileRef       theConfigFile = yamlConfigFileCacheGetId(*yamlUnit);
    
    if ( theConfigFile ) yamlConfigFileCacheEvict(theConfigFile);
}

//

void
yamlconfigfile_closeall_()
{
	yamlConfigFileCacheFlush();
}

//

const char*     __yamlConfigFileTypeNamesSequence[] = {
                        "sequence",
                        "list",
                        "array",
                        NULL
                    };
const char*     __yamlConfigFileTypeNamesMapping[] = {
                        "mapping",
                        "map",
                        "dict"
                        "dictionary",
                        "hash",
                        NULL
                    };

bool
__yamlFortranInterfaceStringIsEqual(
    const char      *fortranString,
    size_t          fortranStringLen,
    const char      *cString
)
{
    if ( strncasecmp(fortranString, cString, fortranStringLen) == 0 ) {
        // We know that cString starts with fortranString; does cString
        // have any more characters tha fortranString?
        if ( cString[fortranStringLen] == '\0' ) return true;
    }
    return false;
}

yaml_node_type_t
__yamlFortranInterfaceParseTypeString(
    const char      *typeString,
    size_t          typeStringLen
)
{
    const char*     *stringList;
    
    if ( __yamlFortranInterfaceStringIsEqual(typeString, typeStringLen, "scalar") ) return YAML_SCALAR_NODE;
    
    // Test sequence types:
    stringList = __yamlConfigFileTypeNamesSequence;
    while ( *stringList ) if ( __yamlFortranInterfaceStringIsEqual(typeString, typeStringLen, *stringList++) ) return YAML_SEQUENCE_NODE;
    
    // Test mapping types:
    stringList = __yamlConfigFileTypeNamesMapping;
    while ( *stringList ) if ( __yamlFortranInterfaceStringIsEqual(typeString, typeStringLen, *stringList++) ) return YAML_MAPPING_NODE;
    
    return YAML_NO_NODE;
}

//

void
__yamlFortranInterfacePrintKeyPathCompileError(
    yamlErrorCode               errorCode,
    const char                  *keyPath,
    size_t                      keyPathLen,
    const char                  *errorAtChar
)
{
    fprintf(stderr, "[YAML::Fortran] failed to compile key path ");
    if ( errorAtChar ) {
        fprintf(stderr, "at \"%.*s\" ", (keyPathLen - (errorAtChar - keyPath)), keyPath);
    }
    fprintf(stderr, "(err = %d)\n", errorCode);
}

//

void
__yamlFortranInterfacePrintKeyPathUsageError(
    const char                  *errorDescString,
    yamlErrorCode               errorCode,
    yamlKeyPathRef              compiledKeyPath,
    yamlKeyPathNodeMatchType    *failedAtMatchElement
)
{
    fprintf(stderr, "[YAML::Fortran] %s %s (err = %d)\n", errorDescString, __yamlFortranInterfaceQuickKeyPathString(compiledKeyPath), errorCode);
    if ( failedAtMatchElement ) {
        fprintf(stderr, "[YAML::Fortran] -> at ");
        while ( failedAtMatchElement ) {
            switch ( failedAtMatchElement->type ) {
                case YAML_SEQUENCE_NODE:
                    fprintf(stderr, "[%d]", failedAtMatchElement->parameter.index + 1);
                    break;
                case YAML_MAPPING_NODE:
                    fprintf(stderr, ".%s", failedAtMatchElement->parameter.key);
                    break;
            }
            failedAtMatchElement = yamlKeyPathGetNextNodeMatch(failedAtMatchElement);
        }
        fputc('\n', stderr);
    }
}

//

yamlFortranInterfaceLogical
yamlConfigFile_getnodeistype_(
    yamlFortranInterfaceInteger *yamlUnit,
    const char                  *keyPath,
    const char                  *type,
    yamlFortranInterfaceInteger *ierr,
    yamlFortranInterfaceInteger keyPathLen,
    yamlFortranInterfaceInteger typeLen
)
{
    yamlConfigFileRef           theConfigFile = yamlConfigFileCacheGetId(*yamlUnit);
    bool                        okay = false;
    
    if ( theConfigFile ) {
        yamlErrorCode               errorCode = 0;
        const char                  *errorAtChar = NULL;
        yamlKeyPathRef              compiledKeyPath = yamlKeyPathCreateWithString(
                                                            keyPath,
                                                            keyPathLen,
                                                            yamlKeyPathCompileOptions_oneBasedIndices | yamlKeyPathCompileOptions_caseFoldKeys,
                                                            &errorCode,
                                                            &errorAtChar
                                                        );
        if ( compiledKeyPath ) {
            yamlKeyPathNodeMatchType*   failedAtMatchElement = NULL;
            yaml_node_type_t            theType = __yamlFortranInterfaceParseTypeString(type, typeLen);
                
            if ( theType != YAML_NO_NODE ) {
                okay = yamlConfigFileGetNodeAtPathIsType(
                                theConfigFile,
                                compiledKeyPath,
                                NULL,
                                &errorCode,
                                &failedAtMatchElement,
                                theType
                            );
                if ( errorCode != yamlError_none ) {
                    __yamlFortranInterfacePrintKeyPathUsageError("failed to traverse key path", errorCode, compiledKeyPath, failedAtMatchElement);
                }
            } else {
                errorCode = yamlFortranInterfaceError_invalidYAMLNodeTypeString;
                fprintf(stderr, "[YAML::Fortran] not a valid YAML node type: %.*s (err = %d)\n", typeLen, type, errorCode);
            }
            yamlKeyPathRelease(compiledKeyPath);
        } else {
            __yamlFortranInterfacePrintKeyPathCompileError(errorCode, keyPath, keyPathLen, errorAtChar);
        }
    } else {
        *ierr = yamlFortranInterfaceError_invalidUnit;
    }
    return (yamlFortranInterfaceLogical)(okay ? 1 : 0);
}

//

yamlFortranInterfaceInteger
yamlconfigfile_getnodecollectionsize_(
    yamlFortranInterfaceInteger *yamlUnit,
    const char                  *keyPath,
    yamlFortranInterfaceInteger *ierr,
    yamlFortranInterfaceInteger keyPathLen
)
{
    yamlConfigFileRef           theConfigFile = yamlConfigFileCacheGetId(*yamlUnit);
    unsigned int                count = 0;
    
    if ( theConfigFile ) {
        yamlErrorCode               errorCode = 0;
        const char                  *errorAtChar = NULL;
        yamlKeyPathRef              compiledKeyPath = yamlKeyPathCreateWithString(
                                                            keyPath,
                                                            keyPathLen,
                                                            yamlKeyPathCompileOptions_oneBasedIndices | yamlKeyPathCompileOptions_caseFoldKeys,
                                                            &errorCode,
                                                            &errorAtChar
                                                        );
        if ( compiledKeyPath ) {
            yamlKeyPathNodeMatchType*   failedAtMatchElement = NULL;
            unsigned int                elemCount = 0;
            bool                        okay;
            
            okay = yamlConfigFileGetNodeAtPathMappingOrSequenceLength(
                            theConfigFile,
                            compiledKeyPath,
                            NULL,
                            &errorCode,
                            &failedAtMatchElement,
                            &count
                        );
            if ( ! okay && (errorCode != yamlError_none) ) {
                __yamlFortranInterfacePrintKeyPathUsageError("failed to traverse key path", errorCode, compiledKeyPath, failedAtMatchElement);
            }
        } else {
            __yamlFortranInterfacePrintKeyPathCompileError(errorCode, keyPath, keyPathLen, errorAtChar);
        }
    } else {
        *ierr = yamlFortranInterfaceError_invalidUnit;
    }
    return (yamlFortranInterfaceInteger)count;
}

//

struct yamlFortranInterfaceSummaryPreprocess {
	int     id_digits;
	int     filename_len;
};

bool
__yamlFortranInterfaceSummaryPreprocessEnumerator(
	yamlConfigFileRef   aConfigFile,
	const void          *context
)
{
	struct yamlFortranInterfaceSummaryPreprocess    *preproc = (struct yamlFortranInterfaceSummaryPreprocess*)context;
	const char          *filePath = yamlConfigFileGetSourceFilePath(aConfigFile);
    unsigned int		i;
	size_t              width = 1;
	
	i = yamlConfigFileGetId(aConfigFile);
	while ( i >= 10 ) {
		width++;
		i /= 10;
	}
	if ( width > preproc->id_digits ) preproc->id_digits = width;
	
    if ( filePath ) {
        width = strlen(filePath);
        if ( width > preproc->filename_len ) preproc->filename_len = width;
	}
    
	return true;
}

bool
__yamlFortranInterfaceSummaryPrintEnumerator(
	yamlConfigFileRef   aConfigFile,
	const void          *context
)
{
    const char          *filePath = yamlConfigFileGetSourceFilePath(aConfigFile);
    
	printf((const char*)context, yamlConfigFileGetId(aConfigFile), filePath ? filePath : "");
	return true;
}

void
yamlconfigfile_summary_()
{
	struct yamlFortranInterfaceSummaryPreprocess    preproc = { 0, 0 };
    
    yamlConfigFileCacheEnumerate(__yamlFortranInterfaceSummaryPreprocessEnumerator, (const void*)&preproc);
    if ( preproc.id_digits > 0 ) {
		char			format[64];
		int				i;
		
		if ( preproc.id_digits < 4 ) preproc.id_digits = 4;
		if ( preproc.filename_len < 8 ) preproc.filename_len = 8;
		
		printf("+-");
		i = preproc.id_digits; while ( i-- ) fputc('-', stdout);
		printf("-+-");
		i = preproc.filename_len; while ( i-- ) fputc('-', stdout);
		printf("-+\n");
		snprintf(format, sizeof(format), "| %%-%ds | %%-%ds |\n", preproc.id_digits, preproc.filename_len);
		printf(format, "Unit", "Filename");
		printf("+-");
		i = preproc.id_digits; while ( i-- ) fputc('-', stdout);
		printf("-+-");
		i = preproc.filename_len; while ( i-- ) fputc('-', stdout);
		printf("-+\n");
		
		snprintf(format, sizeof(format), "| %%-%dd | %%-%ds |\n", preproc.id_digits, preproc.filename_len);
		yamlConfigFileCacheEnumerate(__yamlFortranInterfaceSummaryPrintEnumerator, format);
		
		printf("+-");
		i = preproc.id_digits; while ( i-- ) fputc('-', stdout);
		printf("-+-");
		i = preproc.filename_len; while ( i-- ) fputc('-', stdout);
		printf("-+\n");
	}
}

//

yamlFortranInterfaceLogical
yamlconfigfile_getstring_(
    yamlFortranInterfaceInteger     *yamlUnit,
    const char                      *keyPath,
    char                            *value,
    yamlFortranInterfaceInteger     *ierr,
    yamlFortranInterfaceInteger     keyPathLen,
    yamlFortranInterfaceInteger     valueLen
)
{
    yamlConfigFileRef               theConfigFile = yamlConfigFileCacheGetId(*yamlUnit);
    bool                            okay = false;
    
    if ( theConfigFile ) {
        yamlErrorCode               errorCode = 0;
        const char                  *errorAtChar = NULL;
        yamlKeyPathRef              compiledKeyPath = yamlKeyPathCreateWithString(
                                                            keyPath,
                                                            keyPathLen,
                                                            yamlKeyPathCompileOptions_oneBasedIndices | yamlKeyPathCompileOptions_caseFoldKeys,
                                                            &errorCode,
                                                            &errorAtChar
                                                        );
        if ( compiledKeyPath ) {
            yamlKeyPathNodeMatchType*   failedAtMatchElement = NULL;
            size_t                      tmpValueLen = (size_t)valueLen;
            
            okay = yamlConfigFileCoerceScalarAtPath(
                            theConfigFile,
                            compiledKeyPath,
                            NULL,
                            &errorCode,
                            &failedAtMatchElement,
                            yamlConfigFileCoerceToType_charArray,
                            value,
                            &tmpValueLen
                        );
            if ( okay ) {
                // Make sure any unused characters are blank:
                while ( tmpValueLen < valueLen ) value[tmpValueLen++] = ' ';
            } else {
                __yamlFortranInterfacePrintKeyPathUsageError("failed to coerce value at path", errorCode, compiledKeyPath, failedAtMatchElement);
            }
            yamlKeyPathRelease(compiledKeyPath);
        } else {
            __yamlFortranInterfacePrintKeyPathCompileError(errorCode, keyPath, keyPathLen, errorAtChar);
        }
    } else {
        *ierr = yamlFortranInterfaceError_invalidUnit;
    }
    return (yamlFortranInterfaceLogical)(okay ? 1 : 0);
}
    
//

#define YAMLFORTRANINTERFACE_GET_BOILERPLATE(COERCE_TYPE, C_TYPE) { \
    yamlConfigFileRef               theConfigFile = yamlConfigFileCacheGetId(*yamlUnit); \
    bool                            okay = false; \
    \
    if ( theConfigFile ) { \
        yamlErrorCode               errorCode = 0; \
        const char                  *errorAtChar = NULL; \
        yamlKeyPathRef              compiledKeyPath = yamlKeyPathCreateWithString( \
                                                            keyPath, \
                                                            keyPathLen, \
                                                            yamlKeyPathCompileOptions_oneBasedIndices | yamlKeyPathCompileOptions_caseFoldKeys, \
                                                            &errorCode, \
                                                            &errorAtChar \
                                                        ); \
        if ( compiledKeyPath ) { \
            yamlKeyPathNodeMatchType*   failedAtMatchElement = NULL; \
             \
            okay = yamlConfigFileCoerceScalarAtPath( \
                            theConfigFile, \
                            compiledKeyPath, \
                            NULL, \
                            &errorCode, \
                            &failedAtMatchElement, \
                            yamlConfigFileCoerceToType_ ## COERCE_TYPE, \
                            (C_TYPE*)value \
                        ); \
            if ( ! okay ) { \
                __yamlFortranInterfacePrintKeyPathUsageError("failed to coerce value at path", errorCode, compiledKeyPath, failedAtMatchElement); \
            } \
            yamlKeyPathRelease(compiledKeyPath); \
        } else { \
            __yamlFortranInterfacePrintKeyPathCompileError(errorCode, keyPath, keyPathLen, errorAtChar); \
        } \
    } else { \
        *ierr = yamlFortranInterfaceError_invalidUnit; \
    } \
    return (yamlFortranInterfaceLogical)(okay ? 1 : 0); \
}

//

yamlFortranInterfaceLogical
yamlconfigfile_getinteger_(
    yamlFortranInterfaceInteger     *yamlUnit,
    const char                      *keyPath,
    yamlFortranInterfaceInteger     *value,
    yamlFortranInterfaceInteger     *ierr,
    yamlFortranInterfaceInteger     keyPathLen
)
#ifdef FORTRAN_INTEGER8
YAMLFORTRANINTERFACE_GET_BOILERPLATE(int64, int64_t)
#else
YAMLFORTRANINTERFACE_GET_BOILERPLATE(int32, int32_t)
#endif

//

yamlFortranInterfaceLogical
yamlconfigfile_getinteger4_(
    yamlFortranInterfaceInteger     *yamlUnit,
    const char                      *keyPath,
    int32_t                         *value,
    yamlFortranInterfaceInteger     *ierr,
    yamlFortranInterfaceInteger     keyPathLen
)
YAMLFORTRANINTERFACE_GET_BOILERPLATE(int32, int32_t)

//

yamlFortranInterfaceLogical
yamlconfigfile_getinteger8_(
    yamlFortranInterfaceInteger     *yamlUnit,
    const char                      *keyPath,
    int64_t                         *value,
    yamlFortranInterfaceInteger     *ierr,
    yamlFortranInterfaceInteger     keyPathLen
)
YAMLFORTRANINTERFACE_GET_BOILERPLATE(int64, int64_t)

//

yamlFortranInterfaceLogical
yamlconfigfile_getreal_(
    yamlFortranInterfaceInteger     *yamlUnit,
    const char                      *keyPath,
    yamlFortranInterfaceReal        *value,
    yamlFortranInterfaceInteger     *ierr,
    yamlFortranInterfaceInteger     keyPathLen
)
#ifdef FORTRAN_REAL8
YAMLFORTRANINTERFACE_GET_BOILERPLATE(double, double)
#else
YAMLFORTRANINTERFACE_GET_BOILERPLATE(float, float)
#endif

//

yamlFortranInterfaceLogical
yamlconfigfile_getreal4_(
    yamlFortranInterfaceInteger     *yamlUnit,
    const char                      *keyPath,
    float                           *value,
    yamlFortranInterfaceInteger     *ierr,
    yamlFortranInterfaceInteger     keyPathLen
)
YAMLFORTRANINTERFACE_GET_BOILERPLATE(float, float)

//

yamlFortranInterfaceLogical
yamlconfigfile_getreal8_(
    yamlFortranInterfaceInteger     *yamlUnit,
    const char                      *keyPath,
    double                          *value,
    yamlFortranInterfaceInteger     *ierr,
    yamlFortranInterfaceInteger     keyPathLen
)
YAMLFORTRANINTERFACE_GET_BOILERPLATE(double, double)

//

yamlFortranInterfaceLogical
yamlconfigfile_getdoubleprecision_(
    yamlFortranInterfaceInteger     *yamlUnit,
    const char                      *keyPath,
    double                          *value,
    yamlFortranInterfaceInteger     *ierr,
    yamlFortranInterfaceInteger     keyPathLen
)
YAMLFORTRANINTERFACE_GET_BOILERPLATE(double, double)

//

yamlFortranInterfaceLogical
yamlconfigfile_getlogical_(
    yamlFortranInterfaceInteger     *yamlUnit,
    const char                      *keyPath,
    yamlFortranInterfaceLogical     *value,
    yamlFortranInterfaceInteger     *ierr,
    yamlFortranInterfaceInteger     keyPathLen
)
{
    yamlConfigFileRef               theConfigFile = yamlConfigFileCacheGetId(*yamlUnit);
    bool                            okay = false;
    
    if ( theConfigFile ) {
        yamlErrorCode               errorCode = 0;
        const char                  *errorAtChar = NULL;
        yamlKeyPathRef              compiledKeyPath = yamlKeyPathCreateWithString(
                                                            keyPath,
                                                            keyPathLen,
                                                            yamlKeyPathCompileOptions_oneBasedIndices | yamlKeyPathCompileOptions_caseFoldKeys,
                                                            &errorCode,
                                                            &errorAtChar
                                                        );
        if ( compiledKeyPath ) {
            yamlKeyPathNodeMatchType*   failedAtMatchElement = NULL;
            bool                        bval;
            
            okay = yamlConfigFileCoerceScalarAtPath(
                            theConfigFile,
                            compiledKeyPath,
                            NULL,
                            &errorCode,
                            &failedAtMatchElement,
                            yamlConfigFileCoerceToType_bool,
                            &bval
                        );
            if ( okay ) {
                *value = bval ? 1 : 0;
            } else {
                __yamlFortranInterfacePrintKeyPathUsageError("failed to coerce value at path", errorCode, compiledKeyPath, failedAtMatchElement);
            }
            yamlKeyPathRelease(compiledKeyPath);
        } else {
            __yamlFortranInterfacePrintKeyPathCompileError(errorCode, keyPath, keyPathLen, errorAtChar);
        }
    } else {
        *ierr = yamlFortranInterfaceError_invalidUnit;
    }
    return (yamlFortranInterfaceLogical)(okay ? 1 : 0);
}

//

#define YAMLFORTRANINTERFACE_GETARRAY_BOILERPLATE(COERCE_TYPE, C_TYPE) { \
    yamlConfigFileRef               theConfigFile = yamlConfigFileCacheGetId(*yamlUnit); \
    bool                            okay = false; \
    \
    if ( theConfigFile ) { \
        yamlErrorCode               errorCode = 0; \
        const char                  *errorAtChar = NULL; \
        yamlKeyPathRef              compiledKeyPath = yamlKeyPathCreateWithString( \
                                                            keyPath, \
                                                            keyPathLen, \
                                                            yamlKeyPathCompileOptions_oneBasedIndices | yamlKeyPathCompileOptions_caseFoldKeys, \
                                                            &errorCode, \
                                                            &errorAtChar \
                                                        ); \
        if ( compiledKeyPath ) { \
            yamlKeyPathNodeMatchType*   failedAtMatchElement = NULL; \
             \
            okay = yamlConfigFileCoerceSequenceAtPath( \
                            theConfigFile, \
                            compiledKeyPath, \
                            NULL, \
                            &errorCode, \
                            &failedAtMatchElement, \
                            (unsigned int)(*startIndex - 1), \
                            (unsigned int)(*endIndex - 1), \
                            yamlConfigFileCoerceToType_ ## COERCE_TYPE, \
                            (C_TYPE*)arrayPtr \
                        ); \
            if ( ! okay ) { \
                __yamlFortranInterfacePrintKeyPathUsageError("failed to coerce value at path", errorCode, compiledKeyPath, failedAtMatchElement); \
            } \
            yamlKeyPathRelease(compiledKeyPath); \
        } else { \
            __yamlFortranInterfacePrintKeyPathCompileError(errorCode, keyPath, keyPathLen, errorAtChar); \
        } \
    } else { \
        *ierr = yamlFortranInterfaceError_invalidUnit; \
    } \
    return (yamlFortranInterfaceLogical)(okay ? 1 : 0); \
}

//

yamlFortranInterfaceLogical
yamlconfigfile_getintegerarray_(
    yamlFortranInterfaceInteger     *yamlUnit,
    const char                      *keyPath,
    yamlFortranInterfaceInteger     *startIndex,
    yamlFortranInterfaceInteger     *endIndex,
    yamlFortranInterfaceInteger     *arrayPtr,
    yamlFortranInterfaceInteger     *ierr,
    yamlFortranInterfaceInteger     keyPathLen
)
#ifdef FORTRAN_INTEGER8
YAMLFORTRANINTERFACE_GETARRAY_BOILERPLATE(int64, int64_t)
#else
YAMLFORTRANINTERFACE_GETARRAY_BOILERPLATE(int32, int32_t)
#endif

//

yamlFortranInterfaceLogical
yamlconfigfile_getinteger4array_(
    yamlFortranInterfaceInteger     *yamlUnit,
    const char                      *keyPath,
    yamlFortranInterfaceInteger     *startIndex,
    yamlFortranInterfaceInteger     *endIndex,
    int32_t                         *arrayPtr,
    yamlFortranInterfaceInteger     *ierr,
    yamlFortranInterfaceInteger     keyPathLen
)
YAMLFORTRANINTERFACE_GETARRAY_BOILERPLATE(int32, int32_t)

//

yamlFortranInterfaceLogical
yamlconfigfile_getinteger8array_(
    yamlFortranInterfaceInteger     *yamlUnit,
    const char                      *keyPath,
    yamlFortranInterfaceInteger     *startIndex,
    yamlFortranInterfaceInteger     *endIndex,
    int64_t                         *arrayPtr,
    yamlFortranInterfaceInteger     *ierr,
    yamlFortranInterfaceInteger     keyPathLen
)
YAMLFORTRANINTERFACE_GETARRAY_BOILERPLATE(int64, int64_t)

//

yamlFortranInterfaceLogical
yamlconfigfile_getrealarray_(
    yamlFortranInterfaceInteger     *yamlUnit,
    const char                      *keyPath,
    yamlFortranInterfaceInteger     *startIndex,
    yamlFortranInterfaceInteger     *endIndex,
    yamlFortranInterfaceReal        *arrayPtr,
    yamlFortranInterfaceInteger     *ierr,
    yamlFortranInterfaceInteger     keyPathLen
)
#ifdef FORTRAN_REAL8
YAMLFORTRANINTERFACE_GETARRAY_BOILERPLATE(double, double)
#else
YAMLFORTRANINTERFACE_GETARRAY_BOILERPLATE(float, float)
#endif

//

yamlFortranInterfaceLogical
yamlconfigfile_getreal4array_(
    yamlFortranInterfaceInteger     *yamlUnit,
    const char                      *keyPath,
    yamlFortranInterfaceInteger     *startIndex,
    yamlFortranInterfaceInteger     *endIndex,
    float                           *arrayPtr,
    yamlFortranInterfaceInteger     *ierr,
    yamlFortranInterfaceInteger     keyPathLen
)
YAMLFORTRANINTERFACE_GETARRAY_BOILERPLATE(float, float)

//

yamlFortranInterfaceLogical
yamlconfigfile_getreal8array_(
    yamlFortranInterfaceInteger     *yamlUnit,
    const char                      *keyPath,
    yamlFortranInterfaceInteger     *startIndex,
    yamlFortranInterfaceInteger     *endIndex,
    double                          *arrayPtr,
    yamlFortranInterfaceInteger     *ierr,
    yamlFortranInterfaceInteger     keyPathLen
)
YAMLFORTRANINTERFACE_GETARRAY_BOILERPLATE(double, double)

//

yamlFortranInterfaceLogical
yamlconfigfile_getdoubleprecisionarray_(
    yamlFortranInterfaceInteger     *yamlUnit,
    const char                      *keyPath,
    yamlFortranInterfaceInteger     *startIndex,
    yamlFortranInterfaceInteger     *endIndex,
    double                          *arrayPtr,
    yamlFortranInterfaceInteger     *ierr,
    yamlFortranInterfaceInteger     keyPathLen
)
YAMLFORTRANINTERFACE_GETARRAY_BOILERPLATE(double, double)

//

yamlFortranInterfaceLogical
yamlconfigfile_getlogicalarray_(
    yamlFortranInterfaceInteger     *yamlUnit,
    const char                      *keyPath,
    yamlFortranInterfaceInteger     *startIndex,
    yamlFortranInterfaceInteger     *endIndex,
    yamlFortranInterfaceLogical     *arrayPtr,
    yamlFortranInterfaceInteger     *ierr,
    yamlFortranInterfaceInteger     keyPathLen
)
{
    yamlConfigFileRef               theConfigFile = yamlConfigFileCacheGetId(*yamlUnit);
    bool                            okay = false;
   
    if ( theConfigFile ) {
        yamlErrorCode               errorCode = 0;
        const char                  *errorAtChar = NULL;
        yamlKeyPathRef              compiledKeyPath = yamlKeyPathCreateWithString(
                                                            keyPath,
                                                            keyPathLen,
                                                            yamlKeyPathCompileOptions_oneBasedIndices | yamlKeyPathCompileOptions_caseFoldKeys,
                                                            &errorCode,
                                                            &errorAtChar
                                                        );
        if ( compiledKeyPath ) {
            yamlKeyPathNodeMatchType*   failedAtMatchElement = NULL;
            
            okay = yamlConfigFileCoerceSequenceAtPath(
                            theConfigFile,
                            compiledKeyPath,
                            NULL,
                            &errorCode,
                            &failedAtMatchElement,
                            (unsigned int)(*startIndex - 1),
                            (unsigned int)(*endIndex - 1),
                            yamlConfigFileCoerceToType_bool,
                            (bool*)arrayPtr
                        );
            if ( okay ) {
                // If the C boolean type is different in size, we have to expand into
                // the buffer:
                if ( sizeof(bool) < sizeof(yamlFortranInterfaceLogical) ) {
                    unsigned int                    i = endIndex - startIndex + 1;
                    yamlFortranInterfaceLogical     *to = (yamlFortranInterfaceLogical*)arrayPtr + i;
                    bool                            *from = (bool*)arrayPtr + i;
                    
                    while ( i-- ) {
                        bool    tmp = *from--;
                        *to-- = (yamlFortranInterfaceLogical)(tmp ? 1 : 0);
                    }
                }
            } else {
                fprintf(stderr, "[YAML::Fortran] failed to coerce value at path %s (err = %d)\n", __yamlFortranInterfaceQuickKeyPathString(compiledKeyPath), errorCode);
                if ( failedAtMatchElement ) {
                    fprintf(stderr, "[YAML::Fortran] -> at ");
                    while ( failedAtMatchElement ) {
                        switch ( failedAtMatchElement->type ) {
                            case YAML_SEQUENCE_NODE:
                                fprintf(stderr, "[%d]", failedAtMatchElement->parameter.index + 1);
                                break;
                            case YAML_MAPPING_NODE:
                                fprintf(stderr, ".%s", failedAtMatchElement->parameter.key);
                                break;
                        }
                        failedAtMatchElement = yamlKeyPathGetNextNodeMatch(failedAtMatchElement);
                    }
                    fputc('\n', stderr);
                }
            }
            yamlKeyPathRelease(compiledKeyPath);
        } else {
            fprintf(stderr, "[YAML::Fortran] failed to compile key path ");
            if ( errorAtChar ) {
                fprintf(stderr, "at \"%.*s\" ", (keyPathLen - (errorAtChar - keyPath)), keyPath);
            }
            fprintf(stderr, "(err = %d)\n", errorCode);
        }
    } else {
        *ierr = yamlFortranInterfaceError_invalidUnit;
    }
    return (yamlFortranInterfaceLogical)(okay ? 1 : 0);
}
