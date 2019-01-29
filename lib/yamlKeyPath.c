/*
 * yamlKeyPath
 * Simplified YAML interface for C/Fortran
 *
 * Compile and use key path expressions to access YAML
 * document nodes.
 *
 */

#include "yamlKeyPath.h"
#include <ctype.h>

//

uint64_t
__yamlKeyPathCStringHash(
    const char      *cString,
    size_t          cStringLength
)
{
    uint64_t        hashVal = 5381;
    int             c;

    if ( cStringLength <= 0 ) {
        while ( (c = *cString++) ) hashVal = ((hashVal << 5) + hashVal) + c; /* hash * 33 + c */
    } else {
        while ( cStringLength-- && (c = *cString++) ) hashVal = ((hashVal << 5) + hashVal) + c; /* hash * 33 + c */
    }
    return hashVal;
}

//

typedef struct yamlKeyPathNodeMatchPrivate {
    yamlKeyPathNodeMatchType            matchData;
    struct yamlKeyPathNodeMatchPrivate  *link;
} yamlKeyPathNodeMatchPrivateType;

//

yamlKeyPathNodeMatchPrivateType*
yamlKeyPathNodeMatchPrivateAllocScalar()
{
    yamlKeyPathNodeMatchPrivateType*    newMatch = malloc(sizeof(yamlKeyPathNodeMatchPrivateType));
    
    if ( newMatch ) {
        newMatch->matchData.type = YAML_SCALAR_NODE;
        newMatch->link = NULL;
    }
    return newMatch;
}

//

yamlKeyPathNodeMatchPrivateType*
yamlKeyPathNodeMatchPrivateAllocSequenceIndex(
    int             index
)
{
    yamlKeyPathNodeMatchPrivateType*    newMatch = malloc(sizeof(yamlKeyPathNodeMatchPrivateType));
    
    if ( newMatch ) {
        newMatch->matchData.type = YAML_SEQUENCE_NODE;
        newMatch->matchData.parameter.index = index;
        newMatch->link = NULL;
    }
    return newMatch;
}

//

yamlKeyPathNodeMatchPrivateType*
yamlKeyPathNodeMatchPrivateAllocMappingKey(
    const char          *key,
    size_t              keyLength,
    yamlOptionsBitvec   options
)
{
    yamlKeyPathNodeMatchPrivateType*    newMatch;
    
    if ( ! key ) {
        keyLength = 0;
    }
    else if ( keyLength == yamlCStringFullLength ) {
        keyLength = strlen(key);
    }
    newMatch = malloc(sizeof(yamlKeyPathNodeMatchPrivateType) + keyLength + (key ? 1 : 0));
    
    if ( newMatch ) {
        newMatch->matchData.type = YAML_MAPPING_NODE;
        if ( key ) {
            newMatch->matchData.parameter.key = (void*)newMatch + sizeof(yamlKeyPathNodeMatchPrivateType);
            if ( options & yamlKeyPathCompileOptions_caseFoldKeys ) {
                size_t  i = 0;
                while ( i < keyLength ) {
                    ((char*)newMatch->matchData.parameter.key)[i] = tolower(key[i]);
                    i++;
                }
            } else {
                strncpy((char*)newMatch->matchData.parameter.key, key, keyLength);
            }
            ((char*)newMatch->matchData.parameter.key)[keyLength] = '\0';
        } else {
            newMatch->matchData.parameter.key = NULL;
        }
        newMatch->link = NULL;
    }
    return newMatch;
}

//

void
yamlKeyPathNodeMatchPrivateDeallocChain(
    yamlKeyPathNodeMatchPrivateType     *match
)
{
    while ( match ) {
        yamlKeyPathNodeMatchPrivateType *next = match->link;
        
        free((void*)match);
        match = next;
    }
}

//

typedef struct __yamlKeyPath {
    unsigned int                    refCount;
    uint64_t                        hash;
    yamlOptionsBitvec               options;
    yamlKeyPathNodeMatchPrivateType *matchList;
} yamlKeyPath;

//

yamlKeyPathRef
yamlKeyPathCreate(
    yamlOptionsBitvec   options,
    yamlErrorCode       *outError,
    yaml_node_type_t    nodeType,
    ...
)
{
    yamlKeyPath                         *newKeyPath = NULL;
    yamlKeyPathNodeMatchPrivateType     *listHead = NULL;
    yamlKeyPathNodeMatchPrivateType     *match;
    va_list                             vargs;
    
    va_start(vargs, nodeType);
    while ( nodeType != YAML_NO_NODE ) {
        yamlKeyPathNodeMatchPrivateType *newMatch = NULL;
        
        switch ( nodeType ) {
            case YAML_SCALAR_NODE: {
                newMatch = yamlKeyPathNodeMatchPrivateAllocScalar();
                break;
            }
            case YAML_SEQUENCE_NODE: {
                int     index = va_arg(vargs, int);
                
                if ( options & yamlKeyPathCompileOptions_oneBasedIndices ) index--;
                if ( index < 0 ) {
                    if ( outError ) *outError = yamlKeyPathError_invalidIndex;
                    yamlKeyPathNodeMatchPrivateDeallocChain(listHead);
                    return NULL;
                }
                newMatch = yamlKeyPathNodeMatchPrivateAllocSequenceIndex(index);
                break;
            }
            case YAML_MAPPING_NODE: {
                const char  *key = va_arg(vargs, const char*);
                
                newMatch = yamlKeyPathNodeMatchPrivateAllocMappingKey(key, 0, options);
                break;
            }
            default: {
                if ( outError ) *outError = yamlKeyPathError_invalidType;
                yamlKeyPathNodeMatchPrivateDeallocChain(listHead);
                return NULL;
            }
        }
        if ( newMatch == NULL ) {
            if ( outError ) *outError = yamlKeyPathError_outOfMemory;
            yamlKeyPathNodeMatchPrivateDeallocChain(listHead);
            return NULL;
        }
        if ( listHead ) {
            match->link = newMatch;
            match = newMatch;
        } else {
            listHead = match = newMatch;
        }
        nodeType = va_arg(vargs, yaml_node_type_t);
    }
    
    newKeyPath = malloc(sizeof(yamlKeyPath));
    if ( newKeyPath ) {
        size_t              stringFormLength;
        char                tmpStringForm[64];
        
        newKeyPath->refCount = 1;
        newKeyPath->options = options;
        newKeyPath->matchList = listHead;
        
        // The new object is complete enough that we can call the
        // yamlKeyPathSprintf() function to convert to a string:
        stringFormLength = yamlKeyPathSprintf(newKeyPath, tmpStringForm, sizeof(tmpStringForm));
        if ( stringFormLength < sizeof(tmpStringForm) ) {
            newKeyPath->hash = __yamlKeyPathCStringHash(tmpStringForm, stringFormLength);
        } else {
            char            *stringForm = malloc(1 + stringFormLength);
            
             if ( ! stringForm ) {
                if ( outError ) *outError = yamlKeyPathError_outOfMemory;
                yamlKeyPathNodeMatchPrivateDeallocChain(listHead);
                free((void*)newKeyPath);
                return NULL;
            }
            stringFormLength = yamlKeyPathSprintf(newKeyPath, stringForm, stringFormLength + 1);
            newKeyPath->hash = __yamlKeyPathCStringHash(stringForm, stringFormLength);
            free(stringForm);
        }
    }
    return newKeyPath;
}

//

yamlKeyPathRef
yamlKeyPathCreateWithString(
    const char          *keyPathString,
    size_t              keyPathStringLength,
    yamlOptionsBitvec   options,
    yamlErrorCode       *outError,
    const char*         *outErrorAtChar
)
{
    yamlKeyPath                         *newKeyPath = NULL;
    yamlKeyPathNodeMatchPrivateType     *listHead = NULL;
    yamlKeyPathNodeMatchPrivateType     *match;
    bool                                atRoot = true;
    const char                          *start = keyPathString, *end = keyPathString;
    
    if ( (keyPathStringLength == yamlCStringFullLength) && keyPathString ) keyPathStringLength = strlen(keyPathString);
    
    if ( outErrorAtChar ) *outErrorAtChar = NULL;
    
    if ( keyPathString && *keyPathString && keyPathStringLength ) {
        const char                      *endKeyPathString = keyPathString + keyPathStringLength;
        
        // Drop leading whitespace:
        while ( (keyPathString < endKeyPathString) && isspace(*keyPathString) ) keyPathString++;
        start = keyPathString;
        
        while ( *keyPathString && (keyPathString < endKeyPathString) && ! isspace(*keyPathString) ) {
            yamlKeyPathNodeMatchPrivateType *newMatch = NULL;
            
            if ( *keyPathString == '[' ) {
                const char              *savedPos = ++keyPathString;
                uint64_t                index = 0;
                
                while ( (keyPathString < endKeyPathString) && isdigit(*keyPathString) ) {
                    uint64_t            newIndex = index * 10 + (*keyPathString - '0');
                    
                    if ( newIndex < index ) {
                        if ( outError ) *outError = yamlKeyPathError_invalidIndex;
                        if ( outErrorAtChar ) *outErrorAtChar = savedPos;
                        yamlKeyPathNodeMatchPrivateDeallocChain(listHead);
                        return NULL;
                    }
                    index = newIndex;
                    keyPathString++;
                }
                if ( (keyPathString >= endKeyPathString) || (*keyPathString != ']') ) {
                    if ( outError ) *outError = yamlKeyPathError_invalidExpression;
                    if ( outErrorAtChar ) *outErrorAtChar = keyPathString;
                    yamlKeyPathNodeMatchPrivateDeallocChain(listHead);
                    return NULL;
                }
                keyPathString++;
                if ( options & yamlKeyPathCompileOptions_oneBasedIndices ) index--;
                if ( (index < 0) || (index > INT_MAX) ) {
                    if ( outError ) *outError = yamlKeyPathError_invalidIndex;
                    if ( outErrorAtChar ) *outErrorAtChar = savedPos;
                    yamlKeyPathNodeMatchPrivateDeallocChain(listHead);
                    return NULL;
                }
                newMatch = yamlKeyPathNodeMatchPrivateAllocSequenceIndex(index);
            }
            else if ( atRoot || (*keyPathString == '.') ) {
                const char      *startOfKey = keyPathString;
                const char      *endOfKey;
                
                if ( *startOfKey == '.' ) startOfKey++;
                endOfKey = startOfKey;
                
                // Isolate the key string:
                while ( (endOfKey < endKeyPathString) && *endOfKey && (*endOfKey != '.') && (*endOfKey != '[') ) endOfKey++;
                
                if ( endOfKey > startOfKey ) {
                    newMatch = yamlKeyPathNodeMatchPrivateAllocMappingKey(startOfKey, (endOfKey - startOfKey), options);
                } else {
                    if ( outError ) *outError = yamlKeyPathError_invalidExpression;
                    if ( outErrorAtChar ) *outErrorAtChar = startOfKey;
                    yamlKeyPathNodeMatchPrivateDeallocChain(listHead);
                    return NULL;
                }
                keyPathString = endOfKey;
            }
            else {
                if ( outError ) *outError = yamlKeyPathError_invalidExpression;
                if ( outErrorAtChar ) *outErrorAtChar = keyPathString;
                yamlKeyPathNodeMatchPrivateDeallocChain(listHead);
                return NULL;
            }
            if ( newMatch == NULL ) {
                if ( outError ) *outError = yamlKeyPathError_outOfMemory;
                if ( outErrorAtChar ) *outErrorAtChar = keyPathString;
                yamlKeyPathNodeMatchPrivateDeallocChain(listHead);
                return NULL;
            }
            if ( listHead ) {
                match->link = newMatch;
                match = newMatch;
            } else {
                listHead = match = newMatch;
            }
            atRoot = false;
        }
        end = keyPathString;
        
        // If there's trailing whitespace, ensure it's all whitespace:
        while ( (keyPathString < endKeyPathString) && isspace(*keyPathString) ) keyPathString++;
        if ( keyPathString < endKeyPathString ) {
            if ( outError ) *outError = yamlKeyPathError_invalidExpression;
            if ( outErrorAtChar ) *outErrorAtChar = keyPathString;
            yamlKeyPathNodeMatchPrivateDeallocChain(listHead);
            return NULL;
        }
    }

    newKeyPath = malloc(sizeof(yamlKeyPath));
    if ( newKeyPath ) {
        newKeyPath->refCount = 1;
        newKeyPath->options = options;
        newKeyPath->matchList = listHead;
        if ( start ) {
            newKeyPath->hash = __yamlKeyPathCStringHash(start, end - start);
        } else {
            newKeyPath->hash = __yamlKeyPathCStringHash("", 0);
        }
    }
    return newKeyPath;
}

//

unsigned int
yamlKeyPathGetRefCount(
    yamlKeyPathRef              aKeyPath
)
{
    return aKeyPath->refCount;
}

//

yamlKeyPathRef
yamlKeyPathRetain(
    yamlKeyPathRef              aKeyPath
)
{
    aKeyPath->refCount++;
    return aKeyPath;
}

//

void
yamlKeyPathRelease(
    yamlKeyPathRef              aKeyPath
)
{
    if ( --aKeyPath->refCount == 0 ) {
        if ( aKeyPath->matchList ) yamlKeyPathNodeMatchPrivateDeallocChain(aKeyPath->matchList);
        free((void*)aKeyPath);
    }
}

//

bool
yamlKeyPathIsEqual(
    yamlKeyPathRef              keyPath1,
    yamlKeyPathRef              keyPath2
)
{
    bool                        okay = true;
    
    if ( keyPath1 != keyPath2 ) {
        yamlKeyPathNodeMatchPrivateType *matchList1 = keyPath1->matchList;
        yamlKeyPathNodeMatchPrivateType *matchList2 = keyPath2->matchList;
        
        if ( ! matchList1 && ! matchList2 ) return true;
        
        if ( matchList1 && matchList2 ) {
            // Walk the two lists in parallel, comparing each entry:
            while ( okay && matchList1 && matchList2 ) {
                if ( matchList1->matchData.type != matchList2->matchData.type ) {
                    okay = false;
                } else {
                    switch ( matchList1->matchData.type ) {
                        case YAML_NO_NODE:
                        case YAML_SCALAR_NODE: {
                            break;
                        }
                        case YAML_MAPPING_NODE: {
                            if ( ! matchList1->matchData.parameter.key ) {
                                if ( matchList2->matchData.parameter.key ) okay = false;
                            } else if ( ! matchList2->matchData.parameter.key ) {
                                okay = false;
                            } else {
                                if ( strcmp(matchList1->matchData.parameter.key, matchList2->matchData.parameter.key) != 0 ) okay = false;
                            }
                            break;
                        }
                        case YAML_SEQUENCE_NODE:
                            if ( matchList1->matchData.parameter.index != matchList2->matchData.parameter.index ) okay = false;
                            break;
                    }
                }
                matchList1 = matchList1->link;
                matchList2 = matchList2->link;
            }
        }
    }
    return okay;
}

//

uint64_t
yamlKeyPathGetHash(
    yamlKeyPathRef              aKeyPath
)
{
    return aKeyPath->hash;
}

//


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
)
{
    if ( aKeyPath->matchList ) {
        return &aKeyPath->matchList->matchData;
    }
    return NULL;
}

//

yamlKeyPathNodeMatchType*
yamlKeyPathGetNextNodeMatch(
    yamlKeyPathNodeMatchType    *matchingElement
)
{
    if ( matchingElement ) {
        yamlKeyPathNodeMatchPrivateType *castToPrivate = (yamlKeyPathNodeMatchPrivateType*)matchingElement;
        if ( castToPrivate->link ) return &castToPrivate->link->matchData;
    }
    return NULL;
}

//

bool
yamlKeyPathNodeMatchEnumerate(
    yamlKeyPathRef                  aKeyPath,
    yamlKeyPathNodeMatchEnumerator  enumeratorFn,
    const void                      *context
)
{
    yamlKeyPathNodeMatchPrivateType *m = aKeyPath->matchList;
    bool                            shouldContinue = true;
    
    while ( m && shouldContinue ) {
        shouldContinue = enumeratorFn(&m->matchData, context);
        m = m->link;
    }
    return shouldContinue;
}

//

size_t
yamlKeyPathSprintf(
    yamlKeyPathRef                  aKeyPath,
    char                            *buffer,
    size_t                          bufferSize
)
{
    yamlKeyPathNodeMatchPrivateType *m = aKeyPath->matchList;
    size_t                          fullSize = 0;
    
    if ( buffer && bufferSize ) memset(buffer, 0, bufferSize);
    
    // Determine the full size of the resulting string and copy what
    // we can into buffer as we go:
    while ( m ) {
        switch ( m->matchData.type ) {
            case YAML_NO_NODE:
            case YAML_SCALAR_NODE:
                break;
            case YAML_SEQUENCE_NODE: {
                int             index = m->matchData.parameter.index;
                
                fullSize += 3;
                while ( index >= 10 ) {
                    fullSize++;
                    index /= 10;
                }
                if ( buffer && bufferSize ) {
                    int         n;
                    
                    *buffer++ = '[', bufferSize--;
                    n = snprintf(buffer, bufferSize, "%d", m->matchData.parameter.index + ( aKeyPath->options & yamlKeyPathCompileOptions_oneBasedIndices ) ? 1 : 0 );
                    buffer += n;
                    bufferSize -= ( n  > bufferSize ) ? bufferSize : n;
                    if ( bufferSize ) *buffer++ = ']', bufferSize--;
                }
                break;
            }
            case YAML_MAPPING_NODE: {
                if ( m->matchData.parameter.key ) {
                    
                    fullSize += strlen(m->matchData.parameter.key);
                    if ( m != aKeyPath->matchList ) {
                        if ( buffer && bufferSize ) *buffer++ = '.', bufferSize--;
                        fullSize++;
                    }
                    if ( buffer && bufferSize ) {
                        int     n = snprintf(buffer, bufferSize, "%s", m->matchData.parameter.key);
                        buffer += n;
                        bufferSize -= ( n  > bufferSize ) ? bufferSize : n;
                    }
                }
                break;
            }
        }
        m = m->link;
    }
    return fullSize;
}

//

int
__yamlKeyPathKeyCmp(
    yaml_char_t     *k1,
    size_t          lenk1,
    const char      *k2,
    bool            isCaseless
)
{
    int            cmp = 0;

    if ( isCaseless ) {
        while ( ! cmp && lenk1-- && *k1 && *k2 ) cmp = tolower(*k2++) - tolower(*k1++);
        if ( !cmp ) {
            if ( *k1 ) cmp = -*k1;
            else if ( *k2 ) cmp = *k2;
        }
    } else {
        while ( ! cmp && lenk1-- && *k1 && *k2 ) cmp = *k2++ - *k1++;
        if ( !cmp ) {
            if ( *k1 ) cmp = -*k1;
            else if ( *k2 ) cmp = *k2;
        }
    }
    return cmp;
}

yaml_node_t*
yamlKeyPathApplyToDocument(
    yamlKeyPathRef              aKeyPath,
    yamlOptionsBitvec           options,
    yaml_document_t             *yamlDocument,
    yaml_node_t                 *rootNode,
    yamlErrorCode               *failedError,
    yamlKeyPathNodeMatchType*   *failedAtElement
)
{
    if ( rootNode == NULL ) rootNode = yaml_document_get_root_node(yamlDocument);
    
    if ( failedError ) *failedError = 0;
    if ( failedAtElement ) *failedAtElement = NULL;

    if ( rootNode ) {
        yamlKeyPathNodeMatchPrivateType     *m = aKeyPath->matchList;
        
        while ( m && rootNode ) {
            // Correct node type?
            if ( rootNode->type == m->matchData.type ) {
                switch ( rootNode->type ) {
                    case YAML_MAPPING_NODE: {
                        yaml_node_pair_t    *startKey = rootNode->data.mapping.pairs.start;
                        yaml_node_pair_t    *endKey = rootNode->data.mapping.pairs.top;
                        
                        while ( startKey < endKey ) {
                            yaml_node_t     *key = yaml_document_get_node(yamlDocument, startKey->key);
                            
                            if ( key && (key->type == YAML_SCALAR_NODE) ) {
                                if ( __yamlKeyPathKeyCmp(key->data.scalar.value, key->data.scalar.length, m->matchData.parameter.key, (options & yamlKeyPathApplyOptions_keysAreCaseless) ? true : false) == 0 ) break;
                            } else {
                                if ( failedAtElement ) *failedAtElement = &m->matchData;
                                if ( failedError ) *failedError = yamlKeyPathError_internalError;
                                rootNode = NULL;
                                startKey = endKey;
                                break;
                            }
                            startKey++;
                        }
                        if ( startKey < endKey ) {
                            rootNode = yaml_document_get_node(yamlDocument, startKey->value);
                            if ( ! rootNode ) {
                                if ( failedAtElement ) *failedAtElement = &m->matchData;
                                if ( failedError ) *failedError = yamlKeyPathError_internalError;
                                break;
                            }
                        } else if ( rootNode ) {
                            if ( failedAtElement ) *failedAtElement = &m->matchData;
                            if ( failedError ) *failedError = yamlKeyPathError_invalidKey;
                            rootNode = NULL;
                            break;
                        }
                        break;
                    }
                    case YAML_SEQUENCE_NODE: {
                        yaml_node_item_t    *startItem = rootNode->data.sequence.items.start;
                        yaml_node_item_t    *endItem = rootNode->data.sequence.items.top;

                        if ( m->matchData.parameter.index >= (endItem - startItem) ) {
                            if ( failedAtElement ) *failedAtElement = &m->matchData;
                            if ( failedError ) *failedError = yamlKeyPathError_invalidIndex;
                            rootNode = NULL;
                        } else {
                            rootNode = yaml_document_get_node(yamlDocument, *(startItem + m->matchData.parameter.index));
                            if ( ! rootNode ) {
                                    if ( failedAtElement ) *failedAtElement = &m->matchData;
                                    if ( failedError ) *failedError = yamlKeyPathError_internalError;
                            }
                        }
                        break;
                    }
                    case YAML_SCALAR_NODE: {
                        // Type already matched, just keep going
                        break;
                    }
                    case YAML_NO_NODE: {
                        if ( failedAtElement ) *failedAtElement = &m->matchData;
                        if ( failedError ) *failedError = yamlKeyPathError_invalidType;
                        rootNode = NULL;
                        break;
                    }
                }
            } else {
                if ( failedAtElement ) *failedAtElement = &m->matchData;
                if ( failedError ) *failedError = yamlKeyPathError_invalidType;
                rootNode = NULL;
                break;
            }
            if ( rootNode ) m = m->link;
        }
    }
    return rootNode;
}

//
#if 0
#pragma mark -
#endif
//

const void*
__yamlKeyPathCacheValueCStringRetain(
    const void  *value
)
{
    return strdup((const char*)value);
}

void
__yamlKeyPathCacheValueCStringRelease(
    const void  *value
)
{
    free((void*)value);
}

const yamlKeyPathCacheValueCallbacks yamlKeyPathCacheValueCStringCallbacks = {
                                        .retain = __yamlKeyPathCacheValueCStringRetain,
                                        .release = __yamlKeyPathCacheValueCStringRelease
                                    };

//

typedef struct __yamlKeyPathCache {
    unsigned int                            refCount;
    
    yamlKeyPathCacheValueCallbacks          callbacks;
    
    unsigned int                            slotCount;
    yamlKeyPathRef                          *keys;
    const void*                             *values;
} yamlKeyPathCache;

//

yamlKeyPathCache*
__yamlKeyPathCacheAlloc(
    unsigned int        slotCount
)
{
    yamlKeyPathCache    *newCache;
    
    if ( slotCount == 0 ) slotCount = 17;
    newCache = (yamlKeyPathCache*)calloc(sizeof(yamlKeyPathCache) + slotCount * (sizeof(yamlKeyPathRef) + sizeof(const void*)), 1);
    if ( newCache ) {
        void            *base = (void*)newCache + sizeof(yamlKeyPathCache);
        
        newCache->refCount = 1;
        newCache->slotCount = slotCount;
        newCache->keys = base; base += slotCount * sizeof(yamlKeyPathRef);
        newCache->values = base;
    }
    return newCache;
}

//

yamlKeyPathCacheRef
yamlKeyPathCacheCreate(
    unsigned int                            slotCount,
    const yamlKeyPathCacheValueCallbacks    *callbacks
)
{
    yamlKeyPathCache        *newCache = __yamlKeyPathCacheAlloc(slotCount);
    
    if ( newCache ) {
        if ( callbacks ) newCache->callbacks = *callbacks;
        
    }
    return (yamlKeyPathCacheRef)newCache;
}

//

unsigned int
yamlKeyPathCacheGetRefCount(
    yamlKeyPathCacheRef aCache
)
{
    return aCache->refCount;
}

//

yamlKeyPathCacheRef
yamlKeyPathCacheRetain(
    yamlKeyPathCacheRef aCache
)
{
    aCache->refCount++;
    return aCache;
}

//

void
yamlKeyPathCacheRelease(
    yamlKeyPathCacheRef aCache
)
{
    if ( --aCache->refCount == 0 ) {
        yamlKeyPathCacheClear(aCache);
        free((void*)aCache);
    }
}

//

void
yamlKeyPathCacheAdd(
    yamlKeyPathCacheRef aCache,
    yamlKeyPathRef      aKeyPath,
    const void          *cachedValue
)
{
    uint64_t            keyHash = yamlKeyPathGetHash(aKeyPath);
    unsigned int        keyIndex = keyHash % aCache->slotCount;
    
    if ( aCache->keys[keyIndex] ) {
        yamlKeyPathRelease(aCache->keys[keyIndex]);
        if ( aCache->callbacks.release ) aCache->callbacks.release(aCache->values[keyIndex]);
        aCache->keys[keyIndex] = NULL;
    }
    aCache->keys[keyIndex] = yamlKeyPathRetain(aKeyPath);
    aCache->values[keyIndex] = ( aCache->callbacks.retain ? aCache->callbacks.retain(cachedValue) : cachedValue );
}

//

void
yamlKeyPathCacheDrop(
    yamlKeyPathCacheRef aCache,
    yamlKeyPathRef      aKeyPath
)
{
    uint64_t            keyHash = yamlKeyPathGetHash(aKeyPath);
    unsigned int        keyIndex = keyHash % aCache->slotCount;
    
    if ( aCache->keys[keyIndex] ) {
        if ( yamlKeyPathIsEqual(aCache->keys[keyIndex], aKeyPath) ) {
            yamlKeyPathRelease(aCache->keys[keyIndex]);
            if ( aCache->callbacks.release ) aCache->callbacks.release(aCache->values[keyIndex]);
            aCache->keys[keyIndex] = NULL;
        }
    }
}

//

void
yamlKeyPathCacheClear(
    yamlKeyPathCacheRef aCache
)
{
    unsigned int    i = 0;

    while ( i < aCache->slotCount ) {
        if ( aCache->keys[i] ) {
            yamlKeyPathRelease(aCache->keys[i]);
            if ( aCache->callbacks.release ) aCache->callbacks.release(aCache->values[i]);
        }
        i++;
    }
}

//

bool
yamlKeyPathCacheLookup(
    yamlKeyPathCacheRef aCache,
    yamlKeyPathRef      aKeyPath,
    const void*         *cachedValue
)
{
    uint64_t            keyHash = yamlKeyPathGetHash(aKeyPath);
    unsigned int        keyIndex = keyHash % aCache->slotCount;
    
    if ( aCache->keys[keyIndex] ) {
        if ( (aCache->keys[keyIndex] == aKeyPath) || yamlKeyPathIsEqual(aCache->keys[keyIndex], aKeyPath) ) {
            if ( cachedValue ) *cachedValue = aCache->values[keyIndex];
            return true;
        }
    }
    return false;
}

//
#if 0
#pragma mark -
#endif
//

#if 0

int
main()
{
    char                            keyPathStr[256];
    yamlErrorCode                   errCode;
    const char                      *errAtChar = NULL;
    
    
    yamlKeyPathRef                  p1 = yamlKeyPathCreate(
                                                0,
                                                &errCode,
                                                YAML_MAPPING_NODE, "a",
                                                YAML_MAPPING_NODE, "b",
                                                YAML_MAPPING_NODE, "c",
                                                YAML_SEQUENCE_NODE, 2,
                                                YAML_SEQUENCE_NODE, 1,
                                                YAML_NO_NODE
                                            );
    if ( errCode != yamlKeyPathError_none ) {
        printf("ERROR %d in yamlKeyPathCreate\n", errCode);
    } else {
        yamlKeyPathSprintf(p1, keyPathStr, sizeof(keyPathStr));
        printf("yamlKeyPath@%p[%u] { hash = 0x%016llX } \"%s\"\n", p1, p1->refCount, p1->hash, keyPathStr);
    }
    
    yamlKeyPathRef                  p2 = yamlKeyPathCreateWithString(
                                                "a.B.c[24][1]",
                                                0,
                                                yamlKeyPathCompileOptions_caseFoldKeys,
                                                &errCode,
                                                &errAtChar
                                            );
    if ( errCode != yamlKeyPathError_none ) {
        printf("ERROR %d in yamlKeyPathCreateWithString at \"%s\"\n", errCode, ( errAtChar ? errAtChar : ""));
    } else {
        yamlKeyPathSprintf(p2, keyPathStr, sizeof(keyPathStr));
        printf("yamlKeyPath@%p[%u] { hash = 0x%016llX } \"%s\"\n", p2, p2->refCount, p2->hash, keyPathStr);
    }
    
    printf("yamlKeyPathIsEqual(%p, %p) = %d\n", p1, p2, yamlKeyPathIsEqual(p1, p1));
    printf("yamlKeyPathIsEqual(%p, %p) = %d\n", p1, p2, yamlKeyPathIsEqual(p1, p2));
    
    yamlKeyPathCacheRef             cache = yamlKeyPathCacheCreate(0, &yamlKeyPathCacheValueCStringCallbacks);
    
    if ( cache ) {
        yamlKeyPathCacheAdd(cache, p1, "100");
        yamlKeyPathCacheAdd(cache, p2, "200");
        
        printf("yamlKeyPath@%p[%u] { hash = 0x%016llX }\n", p1, p1->refCount, p1->hash);
        printf("yamlKeyPath@%p[%u] { hash = 0x%016llX }\n", p2, p2->refCount, p2->hash);
    }
    
    return 0;
}

#endif
