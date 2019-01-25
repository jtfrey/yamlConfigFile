/*
 * yamlConfigFile
 * Simplified YAML interface for C/Fortran
 *
 * Wrapper for a YAML document with an API to retrieve key
 * path values and automatically coerce to standard types.
 *
 */

#ifndef __YAMLCONFIGFILE_H__
#define __YAMLCONFIGFILE_H__

#include "yamlBaseTypes.h"
#include "yamlKeyPath.h"

/*!
    @enum yamlConfigFileError
    Error codes associated with the yamlConfigFile API.
*/
enum {
    yamlConfigFileError_min = 200,
    yamlConfigFileError_outOfMemory = yamlConfigFileError_min,
    yamlConfigFileError_failedToCoerce,
    yamlConfigFileError_invalidType,
    yamlConfigFileError_internalError,
    yamlConfigFileError_numberOutOfRange,
    yamlConfigFileError_invalidSequenceIndex,
    yamlConfigFileError_max
} yamlConfigFileError;

/*!
    @define IS_YAMLCONFIGFILE_ERROR
    Preprocessor macro that evaluates whether an error code
    belongs to the yamlConfigFile API.
*/
#define IS_YAMLCONFIGFILE_ERROR(X)  (((X) >= yamlConfigFileError_min) && ((X) < yamlConfigFileError_max))

/*!
    @typedef yamlConfigFileId
    Type of a unique integer id assigned to a cached yamlConfigFile
    object.  The Fortran API uses this id as the YAML "unit" for
    all operations.
*/
typedef unsigned int yamlConfigFileId;

/*!
    @define YAMLCONFIGFILE_UNCACHED_ID
    File id assigned to yamlConfigFile objects that are not cached.
*/
#define YAMLCONFIGFILE_UNCACHED_ID      0

/*!
    @typedef yamlConfigFileRef
    Type of a reference to a yamlConfigFile object.
*/
typedef struct __yamlConfigFile * yamlConfigFileRef;

/*!
    @enum yamlConfigFileOptions
    Bitwise enumeration of optional behaviors for yamlConfigFile objects.
 
    @constant yamlConfigFileOptions_ignoreCachedCopy
        When creating a new object by filename, ignore an extant file in
        the cache
    @constant yamlConfigFileOptions_caseFoldFilename
        When copying the source filename into the yamlConfigFile object,
        convert to lowercase
    @constant yamlConfigFileOptions_doNotCache
        Do not add a reference to the yamlConfigFile to the cache
    @constant yamlConfigFileOptions_noKeyPathCache
        Do not use a yamlKeyPath cache to accelerate path-to-node
        resolution
*/
enum {
    yamlConfigFileOptions_none = 0,
    yamlConfigFileOptions_ignoreCachedCopy = 1 << 0,
    yamlConfigFileOptions_caseFoldFilename = 1 << 1,
    yamlConfigFileOptions_doNotCache = 1 << 2,
    yamlConfigFileOptions_noKeyPathCache = 1 << 3,
    //
    yamlConfigFileOptions_keyPathOptionShift = 16,
    yamlConfigFileOptions_oneBasedIndices = 1 << yamlConfigFileOptions_keyPathOptionShift,
    yamlConfigFileOptions_caseFoldKeys = 2 << yamlConfigFileOptions_keyPathOptionShift,
    yamlConfigFileOptions_keyPathOptionMask = yamlConfigFileOptions_oneBasedIndices | yamlConfigFileOptions_caseFoldKeys
};

/*!
    @define YAMLCONFIGFILEOPTIONS_KEYPATHOPTIONS
    Map any key-path options contained in a yamlConfigFileOptions options
    value to their yamlKeyPathOptions variants.
*/
#define YAMLCONFIGFILEOPTIONS_KEYPATHOPTIONS(X)    (((X) & yamlConfigFileOptions_keyPathOptionMask) >> yamlConfigFileOptions_keyPathOptionShift)

/*!
    @function yamlConfigFileCreateWithFilePointer
    Create a new YAML file wrapper using an open file stream.  Since no
    filename is present in this form, the resulting yamlConfigFile has an
    implied yamlConfigFileOptions_doNotCache option.
 
    @param sourceFilePtr
        File stream containing the YAML document to be read
    @param options
        Optional behaviors from the yamlConfigFileOptions enumeration
 
    @return A newly-initialized yamlConfigFile, or NULL on error
*/
yamlConfigFileRef
yamlConfigFileCreateWithFilePointer(
    FILE                *sourceFilePtr,
    yamlOptionsBitvec   options
);

/*!
    @function yamlConfigFileCreateWithFilePointer
    Create a new YAML file wrapper using an open file stream.  Since no
    filename is present in this form, the resulting yamlConfigFile has an
    implied yamlConfigFileOptions_doNotCache option.
 
    @param sourceFilePtr
        File stream containing the YAML document to be read
    @param options
        Optional behaviors from the yamlConfigFileOptions enumeration
 
    @return A newly-initialized yamlConfigFile, or NULL on error
*/
yamlConfigFileRef
yamlConfigFileCreateWithFileAtPath(
    const char          *sourceFilePath,
    yamlOptionsBitvec   options
);

/*!
    @function yamlConfigFileGetRefCount
    Returns the reference count of aConfigFile.
*/
unsigned int
yamlConfigFileGetRefCount(
    yamlConfigFileRef   aConfigFile
);

/*!
    @function yamlConfigFileRetain
    Returns a reference to an extant yamlConfigFile object.
*/
yamlConfigFileRef
yamlConfigFileRetain(
    yamlConfigFileRef   aConfigFile
);

/*!
    @function yamlConfigFileRelease
    Release a reference to a yamlConfigFile object.  When the reference
    count reaches zero, the object is deallocated.
*/
void
yamlConfigFileRelease(
    yamlConfigFileRef   aConfigFile
);

/*!
    @function yamlConfigFileGetId
    Returns the file id assigned to aConfigFile when it was cached.
*/
yamlConfigFileId
yamlConfigFileGetId(
    yamlConfigFileRef   aConfigFile
);

/*!
    @function yamlConfigFileGetSourceFilePath
    Returns a C string pointer to the source filename from which aConfigFile
    was created.
 
    @return A pointer to a C string, or NULL if aConfigFile was not created
        using a filename
*/
const char*
yamlConfigFileGetSourceFilePath(
    yamlConfigFileRef   aConfigFile
);

/*!
    @function yamlConfigFileGetRootNode
    Returns the root node of the YAML document wrapped by aConfigFile.
*/
yaml_node_t*
yamlConfigFileGetRootNode(
    yamlConfigFileRef   aConfigFile
);

/*!
    @function yamlConfigFileGetNodeAtPath
    Starting from an arbitrary node in the YAML document wrapped by aConfigFile,
    traverse the key path in theKeyPath and return the resulting node.
 
    @param aConfigFile
        The yamlConfigFile containing the document to traverse
    @param theKeyPath
        The YAML key path to traverse
    @param relativeToNode
        The YAML document node at which to begin the traversal; NULL implies the
        root node of the document
    @param outError
        If not NULL and an error occurs, set to the applicable error code
    @param failedAtMatchElement
        If not NULL and an error occurs at a particular component of theKeyPath, set
        to that component (remaining path components can be traversed with
        yamlKeyPathGetNextNodeMatch())
 
    @return NULL on error, otherwise the YAML document node associated with
        theKeyPath
*/
yaml_node_t*
yamlConfigFileGetNodeAtPath(
    yamlConfigFileRef           aConfigFile,
    yamlKeyPathRef              theKeyPath,
    yaml_node_t                 *relativeToNode,
    yamlErrorCode               *outError,
    yamlKeyPathNodeMatchType*   *failedAtMatchElement
);

/*!
    @function yamlConfigFileGetNodeAtPathIsType
    Starting from an arbitrary node in the YAML document wrapped by aConfigFile,
    traverse the key path in theKeyPath.  If a node is found, test whether its type
    matches yamlNodeType.
 
    @param aConfigFile
        The yamlConfigFile containing the document to traverse
    @param theKeyPath
        The YAML key path to traverse
    @param relativeToNode
        The YAML document node at which to begin the traversal; NULL implies the
        root node of the document
    @param outError
        If not NULL and an error occurs, set to the applicable error code
    @param failedAtMatchElement
        If not NULL and an error occurs at a particular component of theKeyPath, set
        to that component (remaining path components can be traversed with
        yamlKeyPathGetNextNodeMatch())
    @param yamlNodeType
        Is the node at theKeyPath of this type?
 
    @return Boolean true if the node was found and was the correct type, otherwise
        false
*/
bool
yamlConfigFileGetNodeAtPathIsType(
    yamlConfigFileRef           aConfigFile,
    yamlKeyPathRef              theKeyPath,
    yaml_node_t                 *relativeToNode,
    yamlErrorCode               *outError,
    yamlKeyPathNodeMatchType*   *failedAtMatchElement,
    yaml_node_type_t            yamlNodeType
);

/*!
    @function yamlConfigFileGetNodeAtPathIsType
    Starting from an arbitrary node in the YAML document wrapped by aConfigFile,
    traverse the key path in theKeyPath.  If a node is found, copy its value to
    the provided buffer.  The buffer is always NUL-terminated.
 
    The actual length of the node value is always returned in
    contentStringLength, so on return if contentStringLength is >= the size
    of the buffer, the consumer knows the full contant has not been copied
    into the buffer.
 
    @param aConfigFile
        The yamlConfigFile containing the document to traverse
    @param theKeyPath
        The YAML key path to traverse
    @param relativeToNode
        The YAML document node at which to begin the traversal; NULL implies the
        root node of the document
    @param outError
        If not NULL and an error occurs, set to the applicable error code
    @param failedAtMatchElement
        If not NULL and an error occurs at a particular component of theKeyPath, set
        to that component (remaining path components can be traversed with
        yamlKeyPathGetNextNodeMatch())
    @param contentString
        Buffer to which the node value should be copied
    @param contentStringLength
        On input, the maximum length of the contentString buffer; on output, the
        full length of the node value
 
    @return Boolean true if the node was found and was scalar, otherwise false
*/
bool
yamlConfigFileGetNodeAtPathContentString(
    yamlConfigFileRef           aConfigFile,
    yamlKeyPathRef              theKeyPath,
    yaml_node_t                 *relativeToNode,
    yamlErrorCode               *outError,
    yamlKeyPathNodeMatchType*   *failedAtMatchElement,
    const char*                 *contentString,
    size_t                      *contentStringLength
);

/*!
    @function yamlConfigFileGetNodeAtPathSequenceLength
    Starting from an arbitrary node in the YAML document wrapped by aConfigFile,
    traverse the key path in theKeyPath.  If a node is found and is a YAML
    sequence node, returns the number of nodes in the sequence.
 
    @param aConfigFile
        The yamlConfigFile containing the document to traverse
    @param theKeyPath
        The YAML key path to traverse
    @param relativeToNode
        The YAML document node at which to begin the traversal; NULL implies the
        root node of the document
    @param outError
        If not NULL and an error occurs, set to the applicable error code
    @param failedAtMatchElement
        If not NULL and an error occurs at a particular component of theKeyPath, set
        to that component (remaining path components can be traversed with
        yamlKeyPathGetNextNodeMatch())
    @param sequenceLength
        Set to the number of nodes in the sequence
 
    @return Boolean true if the node was found and was sequence, otherwise false
*/
bool
yamlConfigFileGetNodeAtPathSequenceLength(
    yamlConfigFileRef           aConfigFile,
    yamlKeyPathRef              theKeyPath,
    yaml_node_t                 *relativeToNode,
    yamlErrorCode               *outError,
    yamlKeyPathNodeMatchType*   *failedAtMatchElement,
    unsigned int                *sequenceLength
);

/*!
    @function yamlConfigFileGetNodeAtPathMappingKeyCount
    Starting from an arbitrary node in the YAML document wrapped by aConfigFile,
    traverse the key path in theKeyPath.  If a node is found and is a YAML
    mapping node, returns the number of key-value pairs in the mapping.
 
    @param aConfigFile
        The yamlConfigFile containing the document to traverse
    @param theKeyPath
        The YAML key path to traverse
    @param relativeToNode
        The YAML document node at which to begin the traversal; NULL implies the
        root node of the document
    @param outError
        If not NULL and an error occurs, set to the applicable error code
    @param failedAtMatchElement
        If not NULL and an error occurs at a particular component of theKeyPath, set
        to that component (remaining path components can be traversed with
        yamlKeyPathGetNextNodeMatch())
    @param mappingKeyCount
        Set to the number of key-value pairs in the mapping
 
    @return Boolean true if the node was found and was mapping, otherwise false
*/
bool
yamlConfigFileGetNodeAtPathMappingKeyCount(
    yamlConfigFileRef           aConfigFile,
    yamlKeyPathRef              theKeyPath,
    yaml_node_t                 *relativeToNode,
    yamlErrorCode               *outError,
    yamlKeyPathNodeMatchType*   *failedAtMatchElement,
    unsigned int                *mappingKeyCount
);

/*!
    @function yamlConfigFileGetNodeAtPathMappingOrSequenceLength
    Starting from an arbitrary node in the YAML document wrapped by aConfigFile,
    traverse the key path in theKeyPath.  If a node is found and is a YAML
    sequence or mapping node, returns the number of nodes in the sequence or
    mapping.
 
    @param aConfigFile
        The yamlConfigFile containing the document to traverse
    @param theKeyPath
        The YAML key path to traverse
    @param relativeToNode
        The YAML document node at which to begin the traversal; NULL implies the
        root node of the document
    @param outError
        If not NULL and an error occurs, set to the applicable error code
    @param failedAtMatchElement
        If not NULL and an error occurs at a particular component of theKeyPath, set
        to that component (remaining path components can be traversed with
        yamlKeyPathGetNextNodeMatch())
    @param sequenceOrMappingLength
        Set to the number of nodes in the sequence or mapping
 
    @return Boolean true if the node was found and was sequence or mapping, otherwise
        false
*/
bool
yamlConfigFileGetNodeAtPathMappingOrSequenceLength(
    yamlConfigFileRef           aConfigFile,
    yamlKeyPathRef              theKeyPath,
    yaml_node_t                 *relativeToNode,
    yamlErrorCode               *outError,
    yamlKeyPathNodeMatchType*   *failedAtMatchElement,
    unsigned int                *sequenceOrMappingLength
);

/*!
    @typedef yamlConfigFileCoerceToType
    Type of an enumeration of the C data types to which a scalar YAML document
    node's value can be coerced.
*/
typedef unsigned int yamlConfigFileCoerceToType;

/*!
    @enum yamlConfigFileCoerceToType
    The types available for coercion.
 
    @constant yamlConfigFileCoerceToType_cstring
        Additional arguments:  const char* buffer, size_t *bufferLen
 
        Copy the value to a buffer, ensuring NUL-termination.  On return,
        bufferLen will be set to the full length of the node value, allowing
        for tests of overflow
 
    @constant yamlConfigFileCoerceToType_charArray
        Additional arguments:  const char* buffer, size_t *bufferLen
 
        Copy the value to a buffer.  The buffer may or may not be NUL-terminated.
        On return, bufferLen will be set to the full length of the node value,
        allowing for tests of overflow
 
    @constant yamlConfigFileCoerceToType_bool
        Additional arguments:  bool *value
 
        t/true/y/yes/on/1 maps to boolean true
        f/false/n/no/off/0 maps to boolean false
        Also, any non-zero integer value map to boolean true
 
    @constant yamlConfigFileCoerceToType_int
        Additional arguments:  int *value
 
        Convert value to integer and set if in [INT_MIN,INT_MAX] range
 
    @constant yamlConfigFileCoerceToType_unsignedInt
        Additional arguments:  unsigned int *value
 
        Convert value to integer and set if in [0,UINT_MAX] range
 
    @constant yamlConfigFileCoerceToType_longInt
        Additional arguments:  long int *value
 
        Convert value to integer and set if in [LONG_MIN,LONG_MAX] range
 
    @constant yamlConfigFileCoerceToType_unsignedLongInt
        Additional arguments:  unsigned long int *value
 
        Convert value to integer and set if in [0,ULONG_MAX] range
 
    @constant yamlConfigFileCoerceToType_longLongInt
        Additional arguments:  long long int *value
 
        Convert value to integer and set
 
    @constant yamlConfigFileCoerceToType_unsignedLongLongInt
        Additional arguments:  unsigned long long int *value
 
        Convert value to integer and set
 
    @constant yamlConfigFileCoerceToType_int8
        Additional arguments:  int8_t *value
 
        Convert value to integer and set if in signed 8-bit range
 
    @constant yamlConfigFileCoerceToType_uint8
        Additional arguments:  uint8_t *value
 
        Convert value to integer and set if in unsigned 8-bit range
 
    @constant yamlConfigFileCoerceToType_int16
        Additional arguments:  int16_t *value
 
        Convert value to integer and set if in signed 16-bit range
 
    @constant yamlConfigFileCoerceToType_uint16
        Additional arguments:  uint16_t *value
 
        Convert value to integer and set if in unsigned 16-bit range
 
    @constant yamlConfigFileCoerceToType_int32
        Additional arguments:  int32_t *value
 
        Convert value to integer and set if in signed 32-bit range
 
    @constant yamlConfigFileCoerceToType_uint32
        Additional arguments:  uint32_t *value
 
        Convert value to integer and set if in unsigned 32-bit range
 
    @constant yamlConfigFileCoerceToType_int64
        Additional arguments:  int64_t *value
 
        Convert value to integer and set if in signed 64-bit range
 
    @constant yamlConfigFileCoerceToType_uint64
        Additional arguments:  uint64_t *value
 
        Convert value to integer and set if in unsigned 64-bit range
 
    @constant yamlConfigFileCoerceToType_float
        Additional arguments:  float *value
 
        Convert value to single-precision floating point and set
 
    @constant yamlConfigFileCoerceToType_double
        Additional arguments:  double *value
 
        Convert value to double-precision floating point and set
*/
enum {
    yamlConfigFileCoerceToType_cstring = 0,
    yamlConfigFileCoerceToType_charArray,
    //
    yamlConfigFileCoerceToType_bool,
    yamlConfigFileCoerceToType_int,
    yamlConfigFileCoerceToType_unsignedInt,
    yamlConfigFileCoerceToType_longInt,
    yamlConfigFileCoerceToType_unsignedLongInt,
    yamlConfigFileCoerceToType_longLongInt,
    yamlConfigFileCoerceToType_unsignedLongLongInt,
    //
    yamlConfigFileCoerceToType_int8,
    yamlConfigFileCoerceToType_uint8,
    yamlConfigFileCoerceToType_int16,
    yamlConfigFileCoerceToType_uint16,
    yamlConfigFileCoerceToType_int32,
    yamlConfigFileCoerceToType_uint32,
    yamlConfigFileCoerceToType_int64,
    yamlConfigFileCoerceToType_uint64,
    //
    yamlConfigFileCoerceToType_float,
    yamlConfigFileCoerceToType_double
};

/*!
    @function yamlConfigFileCoerceScalarAtPath
    Starting from an arbitrary node in the YAML document wrapped by aConfigFile,
    traverse the key path in theKeyPath.  If a node is found and is a YAML
    scalar node, attempt to convert its value to the specified type.
 
    @param aConfigFile
        The yamlConfigFile containing the document to traverse
    @param theKeyPath
        The YAML key path to traverse
    @param relativeToNode
        The YAML document node at which to begin the traversal; NULL implies the
        root node of the document
    @param outError
        If not NULL and an error occurs, set to the applicable error code
    @param failedAtMatchElement
        If not NULL and an error occurs at a particular component of theKeyPath, set
        to that component (remaining path components can be traversed with
        yamlKeyPathGetNextNodeMatch())
    @param coerceToType
        C data type to which the value should be converted
    @param ...
        Each yamlConfigFileCoerceToType specifies what additional arguments should
        be passed; at a minimum, a pointer at which the converted value should be
        stored is required
 
    @return Boolean true if the node was found, was scalar, and its value was properly
        converted/copied; false otherwise
*/
bool
yamlConfigFileCoerceScalarAtPath(
    yamlConfigFileRef           aConfigFile,
    yamlKeyPathRef              theKeyPath,
    yaml_node_t                 *relativeToNode,
    yamlErrorCode               *outError,
    yamlKeyPathNodeMatchType*   *failedAtMatchElement,
    yamlConfigFileCoerceToType  coerceToType,
    ...
);

/*!
    @function yamlConfigFileCoerceScalarAtPathString
    A convenience function that behaves like yamlConfigFileCoerceScalarAtPath
    but handles compilation of the key path string itself.
    
    The key is compiled using the key path options that were passed to
    aConfigFile when it was created.
    
    @param outErrorAtChar
        If not NULL and a key path compilation error occurs, points to the
        bad character in pathString
*/
bool
yamlConfigFileCoerceScalarAtPathString(
    yamlConfigFileRef           aConfigFile,
    const char                  *pathString,
    size_t                      pathStringLen,
    yaml_node_t                 *relativeToNode,
    yamlErrorCode               *outError,
    const char*                 *outErrorAtChar,
    yamlKeyPathNodeMatchType*   *failedAtMatchElement,
    yamlConfigFileCoerceToType  coerceToType,
    ...
);

/*!
    @function yamlConfigFileCoerceSequenceAtPath
    Starting from an arbitrary node in the YAML document wrapped by aConfigFile,
    traverse the key path in theKeyPath.  If a node is found and is a YAML
    sequence node, attempt to convert one or more values in the sequence to the
    specified type.
 
    The number of nodes to be converted is (1 + endSequenceIndex - startSequenceIndex).
 
    @param aConfigFile
        The yamlConfigFile containing the document to traverse
    @param theKeyPath
        The YAML key path to traverse
    @param relativeToNode
        The YAML document node at which to begin the traversal; NULL implies the
        root node of the document
    @param outError
        If not NULL and an error occurs, set to the applicable error code
    @param failedAtMatchElement
        If not NULL and an error occurs at a particular component of theKeyPath, set
        to that component (remaining path components can be traversed with
        yamlKeyPathGetNextNodeMatch())
    @param startSequenceIndex
        Begin converting the node at this offset in the sequence
    @param endSequenceIndex
        Node at this offset in the sequence is the final node to be converted
    @param coerceToType
        C data type to which the value should be converted; the string types are NOT
        handled by this function
    @param ...
        Each yamlConfigFileCoerceToType specifies what additional arguments should
        be passed; at a minimum, a pointer at which the converted value(s) should be
        stored is required
 
    @return Boolean true if the node was found, was scalar, and its value was properly
        converted/copied; false otherwise
*/
bool
yamlConfigFileCoerceSequenceAtPath(
    yamlConfigFileRef           aConfigFile,
    yamlKeyPathRef              theKeyPath,
    yaml_node_t                 *relativeToNode,
    yamlErrorCode               *outError,
    yamlKeyPathNodeMatchType*   *failedAtMatchElement,
    unsigned int                startSequenceIndex,
    unsigned int                endSequenceIndex,
    yamlConfigFileCoerceToType  coerceToType,
    ...
);

/*!
    @function yamlConfigFileCoerceSequenceAtPathString
    A convenience function that behaves like yamlConfigFileCoerceSequenceAtPath
    but handles compilation of the key path string itself.
    
    The key is compiled using the key path options that were passed to
    aConfigFile when it was created.
    
    @param outErrorAtChar
        If not NULL and a key path compilation error occurs, points to the
        bad character in pathString
*/
bool
yamlConfigFileCoerceSequenceAtPathString(
    yamlConfigFileRef           aConfigFile,
    const char                  *pathString,
    size_t                      pathStringLen,
    yaml_node_t                 *relativeToNode,
    yamlErrorCode               *outError,
    const char*                 *outErrorAtChar,
    yamlKeyPathNodeMatchType*   *failedAtMatchElement,
    unsigned int                startSequenceIndex,
    unsigned int                endSequenceIndex,
    yamlConfigFileCoerceToType  coerceToType,
    ...
);


/*!
    @function yamlConfigFileCacheGetId
    Check the yamlConfigFile cache for a file with the given id.
 
    @return The cached yamlConfigFile object with the id matching aConfigFileId, or
        NULL if no such file id is present
*/
yamlConfigFileRef
yamlConfigFileCacheGetId(
    yamlConfigFileId    aConfigFileId
);

/*!
    @typedef yamlConfigFileCacheEnumerator
    Type signature of a function called by yamlConfigFileCacheEnumerate on each file
    in the cache.  The context is a user-provided pointer-sized value.
 
    The function should return boolean false if the enumeration should be halted, true
    otherwise.
*/
typedef bool (*yamlConfigFileCacheEnumerator)(yamlConfigFileRef aConfigFile, const void *context);

/*!
    @function yamlConfigFileCacheEnumerate
    Call enumeratorFn on each yamlConfigFile in the cache.
 
    @param enumeratorFn
        Pointer to an enumerator function
    @param context
        User-provided pointer-sized value passed to each invocation of the enumeratorFn
 
    @return Boolean true if all cached files were enumerated, false if the enumeration
        was halted early
*/
bool
yamlConfigFileCacheEnumerate(
    yamlConfigFileCacheEnumerator   enumeratorFn,
    const void                      *context
);

/*!
    @function yamlConfigFileCacheEvict
    If aConfigFile is present in the cache, remove it.
*/
void
yamlConfigFileCacheEvict(
    yamlConfigFileRef   aConfigFile
);

/*!
    @function yamlConfigFileCacheFlush
    Remove all yamlConfigFile objects from the cache.
*/
void
yamlConfigFileCacheFlush();

#endif /* __YAMLCONFIGFILE_H__ */
