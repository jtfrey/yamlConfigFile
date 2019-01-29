/*
 * yamlKeyPath
 * Simplified YAML interface for C/Fortran
 *
 * Compile and use key path expressions to access YAML
 * document nodes.
 *
 */

#ifndef __YAMLKEYPATH_H__
#define __YAMLKEYPATH_H__

#include "yamlBaseTypes.h"

/*!
    @enum yamlKeyPathError
    Error codes returned by the key path API.
*/
enum {
    yamlKeyPathError_min = 100,
    yamlKeyPathError_outOfMemory = yamlKeyPathError_min,
    yamlKeyPathError_invalidType,
    yamlKeyPathError_invalidIndex,
    yamlKeyPathError_invalidKey,
    yamlKeyPathError_internalError,
    yamlKeyPathError_invalidExpression,
    yamlKeyPathError_max
};


/*!
    @define IS_YAMLKEYPATH_ERROR
    Preprocessor macro that evaluates whether an error code
    belongs to the yamlKeyPath API.
*/
#define IS_YAMLKEYPATH_ERROR(X)  (((X) >= yamlKeyPathError_min) && ((X) < yamlKeyPathError_max))


/*!
    @enum yamlKeyPathCompileOptions
    Bitwise enumeration of optional behaviors in key path expression
    compilation.
*/
enum {
    yamlKeyPathCompileOptions_oneBasedIndices = 1 << 0,
    yamlKeyPathCompileOptions_caseFoldKeys = 1 << 1
};


/*!
    @typedef yamlKeyPathNodeMatchType
    Type of a YAML path match component.
 
    @field type
        The expected YAML node type (YAML_SCALAR_NODE, YAML_SEQUENCE_NODE,
        YAML_MAPPING_NODE)
    @field parameter.key
        C string key for YAML_MAPPING_NODE type
    @field parameter.index
        Zero-based index for YAML_SEQUENCE_NODE type
*/
typedef struct {
    yaml_node_type_t        type;
    union {
        const char          *key;
        int                 index;
    } parameter;
} yamlKeyPathNodeMatchType;


/*!
    @typedef yamlKeyPathRef
    Type of a reference to a compiled YAML key path.
*/
typedef struct __yamlKeyPath * yamlKeyPathRef;


/*!
    @function yamlKeyPathCreate
    Create a new compiled key path using a sequence of YAML node
    types and parameters.  For example, the path
 
        a.b.c[2][1]
 
    could be constructed as
 
        yamlKeyPathCreate(
                YAML_MAPPING_NODE, "a",
                YAML_MAPPING_NODE, "b",
                YAML_MAPPING_NODE, "c",
                YAML_SEQUENCE_NODE, 2,
                YAML_SEQUENCE_NODE, 1,
                YAML_NO_NODE
            );
 
    Mapping keys should be C strings and indices should be of C int
    type.
 
    It is possible to use the YAML_SCALAR_NODE type with no argument
    in order to enforce the matching of a terminal scalar node.  For
    example, if the root node must be scalar:
 
        yamlKeyPathCreate(
                YAML_SCALAR_NODE,
                YAML_NO_NODE
            );
 
    or if the result of a key or index lookup must be scalar:
 
        yamlKeyPathCreate(
                YAML_MAPPING_NODE, "a",
                YAML_MAPPING_NODE, "b",
                YAML_MAPPING_NODE, "c",
                YAML_SEQUENCE_NODE, 2,
                YAML_SEQUENCE_NODE, 1,
                YAML_SCALAR_NODE,
                YAML_NO_NODE
            );
 
    @param options
        Optional behaviors for the compilation
    @param outError
        If not NULL, set to the error that occurred during compilation
    @param nodeType
        First type of YAML node to match against
 
    @return A compiled key path, or NULL on error.
*/
yamlKeyPathRef
yamlKeyPathCreate(
    yamlOptionsBitvec       options,
    yamlErrorCode           *outError,
    yaml_node_type_t        nodeType,
    ...
);


/*!
    @function yamlKeyPathCreateWithString
    Create a new compiled key path using a key path represented as a
    string.  For example:
 
        a.b.c[2][1]
 
    @param keyPathString
        The key path string to compile
    @param keyPathStringLength
        The length of the key path string
    @param options
        Optional behaviors for the compilation
    @param outError
        If not NULL, set to the error that occurred during compilation
    @param outErrorAtChar
        If not NULL, set to the pointer to the character at which the
        error occurred
 
    @return A compiled key path, or NULL on error.
*/
yamlKeyPathRef
yamlKeyPathCreateWithString(
    const char              *keyPathString,
    size_t                  keyPathStringLength,
    yamlOptionsBitvec       options,
    yamlErrorCode           *outError,
    const char*             *outErrorAtChar
);


/*!
    @function yamlKeyPathGetRefCount
    Returns the reference count of the key path object.
*/
unsigned int
yamlKeyPathGetRefCount(
    yamlKeyPathRef              aKeyPath
);


/*!
    @function yamlKeyPathRetain
    Returns a reference to an extant key path object.
*/
yamlKeyPathRef
yamlKeyPathRetain(
    yamlKeyPathRef              aKeyPath
);


/*!
    @function yamlKeyPathRelease
    Release a reference to a key path object.  When the reference count
    reaches zero, the object is deallocated.
*/
void
yamlKeyPathRelease(
    yamlKeyPathRef              aKeyPath
);


/*!
    @function yamlKeyPathGetHash
    Get the hash value of the key path object.
*/
uint64_t
yamlKeyPathGetHash(
    yamlKeyPathRef              aKeyPath
);


/*!
    @function yamlKeyPathIsEqual
    Compare two key path objects for equality.
 
    @return Boolean true if the two objects are the same
*/
bool
yamlKeyPathIsEqual(
    yamlKeyPathRef              keyPath1,
    yamlKeyPathRef              keyPath2
);


/*!
    @function yamlKeyPathGetFirstNodeMatch
    Return the first (root) node-matching element in the compiled expression.
 
    @param aKeyPath
        A compiled key path
 
    @return The first node-matching element for the expression, or NULL.
*/
yamlKeyPathNodeMatchType*
yamlKeyPathGetFirstNodeMatch(
    yamlKeyPathRef              aKeyPath
);


/*!
    @function yamlKeyPathGetNextNodeMatch
    Return the next node-matching element after this one.
 
    @param matchingElement
        The node-matching element whose child should be returned.
 
    @return The next node-matching element, or NULL if matchingElement was the
        last element.
*/
yamlKeyPathNodeMatchType*
yamlKeyPathGetNextNodeMatch(
    yamlKeyPathNodeMatchType    *matchingElement
);


/*!
    @define YAMLKEYPATH_NODEMATCH_FPRINTF
    Macro that expands to a loop that walks node match element M and
    all its children, printing the key path they represent to file
    stream S.  No leading or trailing whitespace (including terminating
    <NL> characters) is printed to the file stream.
*/
#define YAMLKEYPATH_NODEMATCH_FPRINTF(S, M) { \
        yamlKeyPathNodeMatchType    *__m__ = (M); \
        FILE                        *__s__ = (S); \
        while (__m__) { \
            switch (__m__->type) { \
                case YAML_SEQUENCE_NODE: fprintf(__s__, "[%d]", __m__->parameter.index); break; \
                case YAML_MAPPING_NODE: fprintf(__s__, ".%s", __m__->parameter.key); break; \
            } \
            __m__ = yamlKeyPathGetNextNodeMatch(__m__); \
        } \
    } while (0)
            

/*!
    @typedef yamlKeyPathNodeMatchEnumerator
    Type signature of a callback function that enumerates the node-matching
    elements in sequence.
 
    @param nodeMatchElement
        Pointer to a node-match element
    @param context
        Opaque user data pointer
 
    @return The callback should return false to halt enumeration, true to continue
*/
typedef bool (*yamlKeyPathNodeMatchEnumerator)(yamlKeyPathNodeMatchType *nodeMatchElement, const void *context);


/*!
    @function yamlKeyPathNodeMatchEnumerate
    Walk the sequence of node-matching elements associated with the
    compiled key path expression in sequence, calling enumeratorFn
    on each.
 
    @param aKeyPath
        A compiled key path
    @param enumeratorFn
        Function to call for each node-matching element
    @param context
        Opaque user data pointer to pass to the enumerator function
 
    @return True if the list of elements was fully enumerated, false
        otherwise.
*/
bool
yamlKeyPathNodeMatchEnumerate(
    yamlKeyPathRef                  aKeyPath,
    yamlKeyPathNodeMatchEnumerator  enumeratorFn,
    const void                      *context
);


/*!
    @function yamlKeyPathSprintf
    Convert a compiled key path expression to a string form and write
    to buffer.  Up to bufferSize - 1 characters will be written; the
    buffer is always guaranteed to be NUL-terminated.
 
    If the returned length is greater-than or equal-to the bufferSize
    then the buffer was not large enough.
 
    @param aKeyPath
        A compiled key path
    @param buffer
        Character buffer into which the string form should be
        written, or NULL if the length alone of is desired
    @param bufferSize
        Byte size of the buffer
 
    @return The full length of the string form of the key path
*/
size_t
yamlKeyPathSprintf(
    yamlKeyPathRef                  aKeyPath,
    char                            *buffer,
    size_t                          bufferSize
);


/*!
    @enum yamlKeyPathApplyOptions
    Enumeration of optional behaviors of the path application function.
 
    @constant yamlKeyPathApplyOptions_keysAreCaseless
        When matching mapping keys use caseless string comparison
*/
enum {
    yamlKeyPathApplyOptions_keysAreCaseless = 1 << 0
};


/*!
    @function yamlKeyPathApplyToDocument
    Starting at the given "root" node within the yamlDocument, attempt to
    descend through the nodes indicated by aKeyPath and return the final
    node matched.
 
    If any mismatch occurs, the nature of the error will be stored at
    failedError.  The node-matching element where the failure occurred
    is stored at failedAtElement (when applicable).
 
    @param aKeyPath
        A compiled key path
    @param options
        Optional behaviors to the function
    @param yamlDocument
        The YAML document in which the node search will occur
    @param rootNode
        The YAML document node at which the search begins; NULL implies
        the root node of the document
    @param failedError
        If not NULL and the search fails, an error code will be stored here
    @param failedAtElement
        If not NULL and the search fails, the node-matching element where
        the failure occured will be stored here
 
    @return The resulting YAML document node or NULL on error
*/
yaml_node_t*
yamlKeyPathApplyToDocument(
    yamlKeyPathRef              aKeyPath,
    yamlOptionsBitvec           options,
    yaml_document_t             *yamlDocument,
    yaml_node_t                 *rootNode,
    yamlErrorCode               *failedError,
    yamlKeyPathNodeMatchType*   *failedAtElement
);


/*!
    @typedef yamlKeyPathCacheRef
    Type of a reference to a yamlKeyPathCache
*/
typedef struct __yamlKeyPathCache * yamlKeyPathCacheRef;


/*!
    @typedef yamlKeyPathCacheValueRetainCallback
    Type signature of a callback function that copies a value to be
    stored in a yamlKeyPathCache
 
    @param value
        Pointer-sized value to be copied
 
    @return The copied value to store in the cache
*/
typedef const void* (*yamlKeyPathCacheValueRetainCallback)(const void *value);


/*!
    @typedef yamlKeyPathCacheValueReleaseCallback
    Type signature of a callback function that destroys a value
    stored in a yamlKeyPathCache
 
    @param value
        Pointer-sized value to be destroyed
*/
typedef void (*yamlKeyPathCacheValueReleaseCallback)(const void *value);

/*!
    @typedef yamlKeyPathCacheValueCallbacks
    Structure containing the callbacks used to move values into and out
    of a yamlKeyPathCache.
 
    @field retain
        Callback that copies a value, or NULL if the value is used
        verbatim
    @field release
        Callback that destroys a value, or NULL if no action is
        necessary
*/
typedef struct {
    yamlKeyPathCacheValueRetainCallback     retain;
    yamlKeyPathCacheValueReleaseCallback    release;
} yamlKeyPathCacheValueCallbacks;


/*!
    @constant yamlKeyPathCacheValueCStringCallbacks
    Predefined yamlKeyPathCacheValueCallbacks structure containing callbacks
    that handle C strings.
*/
const yamlKeyPathCacheValueCallbacks yamlKeyPathCacheValueCStringCallbacks;


/*!
    @function yamlKeyPathCacheCreate
    Create a new yamlKeyPath cache.
 
    @param slotCount
        Number of cache slots to create or 0 for the default
    @param callbacks
        Pointer to a structure containing callbacks used to copy/destroy
        values
 
    @return A newly-initialized yamlKeyPath cache, or NULL on error
*/
yamlKeyPathCacheRef
yamlKeyPathCacheCreate(
    unsigned int                            slotCount,
    const yamlKeyPathCacheValueCallbacks    *callbacks
);


/*!
    @function yamlKeyPathCacheGetRefCount
    Returns the reference count of the yamlKeyPath cache object.
*/
unsigned int
yamlKeyPathCacheGetRefCount(
    yamlKeyPathCacheRef aCache
);


/*!
    @function yamlKeyPathCacheRetain
    Returns a reference to an extant yamlKeyPath cache object.
*/
yamlKeyPathCacheRef
yamlKeyPathCacheRetain(
    yamlKeyPathCacheRef aCache
);


/*!
    @function yamlKeyPathCacheRelease
    Release a reference to a yamlKeyPath cache object.  When the reference count
    reaches zero, the object is deallocated.
*/
void
yamlKeyPathCacheRelease(
    yamlKeyPathCacheRef aCache
);


/*!
    @function yamlKeyPathCacheAdd
    Attempts to add the given cachedValue to aCache, associating it with aKeyPath.
    The cachedValue is retained according to the callbacks that were passed when
    aCache was created.
 
    Any extant value that gets evicted is destroyed according to the callbacks that
    were passed when aCache was created.
 
    @param aCache
        The cache to affect
    @param aKeyPath
        The yamlKeyPath to associate with cachedValue
    @param cachedValue
        The value to add to the cache
*/
void
yamlKeyPathCacheAdd(
    yamlKeyPathCacheRef aCache,
    yamlKeyPathRef      aKeyPath,
    const void          *cachedValue
);


/*!
    @function yamlKeyPathCacheDrop
    If a value is cached associated with a key equal to aKeyPath, the value is
    evicted and destroyed according to the callbacks that were passed when aCache
    was created.
 
    @param aCache
        The cache to affect
    @param aKeyPath
        The yamlKeyPath whose value should be evicted
*/
void
yamlKeyPathCacheDrop(
    yamlKeyPathCacheRef aCache,
    yamlKeyPathRef      aKeyPath
);


/*!
    @function yamlKeyPathCacheClear
    Remove all key-value pairs from the cache.
 
    @param aCache
        The cache to affect
*/
void
yamlKeyPathCacheClear(
    yamlKeyPathCacheRef aCache
);


/*!
    @function yamlKeyPathCacheLookup
    If a yamlKeyPath equal to aKeyPath is present in aCache, return the cached
    value.
 
    @param aCache
        The cache to affect
    @param aKeyPath
        The yamlKeyPath whose value should be returned
    @param cachedValue
        Pointer to an opaque pointer that should be set to the cached value, or
        NULL if the value is not required (just the presence of aKeyPath via
        the return value)
 
    @return Boolean true if a cached value was found, false if aKeyPath is not
        present in the cache
*/
bool
yamlKeyPathCacheLookup(
    yamlKeyPathCacheRef aCache,
    yamlKeyPathRef      aKeyPath,
    const void*         *cachedValue
);



#endif /* __YAMLKEYPATH_H__ */
