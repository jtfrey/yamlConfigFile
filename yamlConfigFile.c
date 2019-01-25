/*
 * yamlConfigFile
 * Simplified YAML interface for C/Fortran
 *
 * Wrapper for a YAML document with an API to retrieve key
 * path values and automatically coerce to standard types.
 *
 */

#include "yamlConfigFile.h"

//

#ifndef YAMLCONFIGFILE_NODE_COERCE_STACKBUFFERSIZE
#define YAMLCONFIGFILE_NODE_COERCE_STACKBUFFERSIZE  48
#endif

#ifdef YAMLCONFIGFILE_DEBUG
#define DEBUG_PRINTF(F, ...) fprintf(stderr, "[DEBUG] " F "\n", __VA_ARGS__)
#else
#define DEBUG_PRINTF(F, ...)
#endif

//

bool
__yamlScalarNodeToBool(
    yaml_node_t     *node,
    bool            *outValue
)
{
	if ( node->data.scalar.length > 0 ) {
		char		*s = (char*)node->data.scalar.value;
		size_t		l = node->data.scalar.length;
		
		if ( l == 1 ) {
			switch (*s ) {
			
				case 't':
				case 'T':
				case 'y':
				case 'Y':
				case '1':
					*outValue = true;
					return true;
			
				case 'f':
				case 'F':
				case 'n':
				case 'N':
				case '0':
					*outValue = false;
					return true;
			
			}
		}
		if ( ((l == 4) && (strncasecmp(s, "true", l) == 0)) ||
			((l == 3) && (strncasecmp(s, "yes", l) == 0)) ||
			((l == 2) && (strncasecmp(s, "on", l) == 0)) )
		{
			*outValue = true;
			return true;
		}
		if ( ((l == 5) && (strncasecmp(s, "false", l) == 0)) ||
			((l == 2) && (strncasecmp(s, "no", l) == 0)) ||
			((l == 3) && (strncasecmp(s, "off", l) == 0)) )
		{
			*outValue = false;
			return true;
		}
		if ( isdigit(*s) || (*s == '-') || (*s == '+') ) {
			if ( (*s == '-') || (*s == '+') ) s++; l--;
			if ( l > 0 ) {
				bool	zero = (*s == '0') ? true : false;
			
				s++, l--;
				while ( l ) {
					if ( ! isdigit(*s) ) break;
					if ( zero ) zero = (*s == '0') ? true : false;
					s++, l--;
				}
				if ( l == 0 ) {
					*outValue = ! zero;
					return true;
				}
			}
		}
	}
	return false;
}

//

bool
__yamlScalarNodeToSignedInt(
	yaml_node_t		*node,
	long long int   *outValue
)
{
    bool            okay = false;
    
	if ( node->data.scalar.length > 0 ) {
        char        nodeValueOnStack[YAMLCONFIGFILE_NODE_COERCE_STACKBUFFERSIZE];
        char        *nodeValue;
        
        if ( node->data.scalar.length < YAMLCONFIGFILE_NODE_COERCE_STACKBUFFERSIZE ) {
            nodeValue = nodeValueOnStack;
        } else {
            nodeValue = malloc(1 + node->data.scalar.length);
        }
		if ( nodeValue ) {
			long long int	ival;
			char		*endptr = NULL;
			
			memcpy(nodeValue, node->data.scalar.value, node->data.scalar.length);
			nodeValue[node->data.scalar.length] = '\0';
			ival = strtoll(nodeValue, &endptr, 0);
            
            if ( endptr > nodeValue ) {
                while ( *endptr && isspace(*endptr) ) endptr++;
                if ( ! *endptr ) {
                    *outValue = ival;
                    okay = true;
                }
            }
		}
        if ( nodeValue != nodeValueOnStack ) free(nodeValue);
	}
	return okay;
}

//

bool
__yamlScalarNodeToUnsignedInt(
	yaml_node_t             *node,
	unsigned long long int  *outValue
)
{
    bool            okay = false;
    
	if ( node->data.scalar.length > 0 ) {
        char        nodeValueOnStack[YAMLCONFIGFILE_NODE_COERCE_STACKBUFFERSIZE];
        char        *nodeValue;
        
        if ( node->data.scalar.length < YAMLCONFIGFILE_NODE_COERCE_STACKBUFFERSIZE ) {
            nodeValue = nodeValueOnStack;
        } else {
            nodeValue = malloc(1 + node->data.scalar.length);
        }
		if ( nodeValue ) {
			unsigned long long int  ival;
			char                    *endptr = NULL;
			
			memcpy(nodeValue, node->data.scalar.value, node->data.scalar.length);
			nodeValue[node->data.scalar.length] = '\0';
			ival = strtoull(nodeValue, &endptr, 0);
            
            if ( endptr > nodeValue ) {
                while ( *endptr && isspace(*endptr) ) endptr++;
                if ( ! *endptr ) {
                    *outValue = ival;
                    okay = true;
                }
            }
		}
        if ( nodeValue != nodeValueOnStack ) free(nodeValue);
	}
	return okay;
}

//

bool
__yamlScalarNodeToFloat(
	yaml_node_t		*node,
	float           *outValue
)
{
    bool            okay = false;
    
	if ( node->data.scalar.length > 0 ) {
        char        nodeValueOnStack[YAMLCONFIGFILE_NODE_COERCE_STACKBUFFERSIZE];
        char        *nodeValue;
        
        if ( node->data.scalar.length < YAMLCONFIGFILE_NODE_COERCE_STACKBUFFERSIZE ) {
            nodeValue = nodeValueOnStack;
        } else {
            nodeValue = malloc(1 + node->data.scalar.length);
        }
		if ( nodeValue ) {
			float           fval;
			char            *endptr = NULL;
			
			memcpy(nodeValue, node->data.scalar.value, node->data.scalar.length);
			nodeValue[node->data.scalar.length] = '\0';
			fval = strtof(nodeValue, &endptr);
            
            if ( endptr > nodeValue ) {
                while ( *endptr && isspace(*endptr) ) endptr++;
                if ( ! *endptr ) {
                    *outValue = fval;
                    okay = true;
                }
            }
		}
        if ( nodeValue != nodeValueOnStack ) free(nodeValue);
	}
	return okay;
}

//

bool
__yamlScalarNodeToDouble(
	yaml_node_t		*node,
	double          *outValue
)
{
    bool            okay = false;
    
	if ( node->data.scalar.length > 0 ) {
        char        nodeValueOnStack[YAMLCONFIGFILE_NODE_COERCE_STACKBUFFERSIZE];
        char        *nodeValue;
        
        if ( node->data.scalar.length < YAMLCONFIGFILE_NODE_COERCE_STACKBUFFERSIZE ) {
            nodeValue = nodeValueOnStack;
        } else {
            nodeValue = malloc(1 + node->data.scalar.length);
        }
		if ( nodeValue ) {
			double          dval;
			char            *endptr = NULL;
			
			memcpy(nodeValue, node->data.scalar.value, node->data.scalar.length);
			nodeValue[node->data.scalar.length] = '\0';
			dval = strtod(nodeValue, &endptr);
            
            if ( endptr > nodeValue ) {
                while ( *endptr && isspace(*endptr) ) endptr++;
                if ( ! *endptr ) {
                    *outValue = dval;
                    okay = true;
                }
            }
		}
        if ( nodeValue != nodeValueOnStack ) free(nodeValue);
	}
	return okay;
}

//
#if 0
#pragma mark -
#endif
//

typedef struct __yamlConfigFileCacheLine {
    struct __yamlConfigFileCacheLine    *link;
    yamlConfigFileRef                   theConfigFile;
} yamlConfigFileCacheLine;

static yamlConfigFileCacheLine          *__yamlConfigFileCacheHead = NULL;

//

yamlConfigFileId
__yamlConfigFileCacheAdd(
    yamlConfigFileRef       aConfigFile
)
{
    yamlConfigFileCacheLine *cacheLine = __yamlConfigFileCacheHead, *prevCacheLine = NULL, *newCacheLine;
    yamlConfigFileId        nextFreeId = 1;
    
    while ( cacheLine ) {
        yamlConfigFileId    cacheLineFileId = yamlConfigFileGetId(cacheLine->theConfigFile);
        
        if ( cacheLineFileId == nextFreeId ) {
            // If the id wraps to zero then the cache is 100% full:
            if ( ++nextFreeId == 0 ) {
                DEBUG_PRINTF("yamlConfigFileCache is full!!", NULL);
                return YAMLCONFIGFILE_UNCACHED_ID;
            }
        } else if ( cacheLineFileId > nextFreeId ) {
            break;
        }
        prevCacheLine = cacheLine;
        cacheLine = cacheLine->link;
    }
    
    // Allocate a cache line:
    newCacheLine = malloc(sizeof(yamlConfigFileCacheLine));
    if ( newCacheLine ) {
        DEBUG_PRINTF("yamlConfigFile@%p added to yamlConfigFileCache", aConfigFile);
        newCacheLine->theConfigFile = yamlConfigFileRetain(aConfigFile);
        newCacheLine->link = cacheLine;
        if ( prevCacheLine ) {
            prevCacheLine->link = newCacheLine;
        } else {
            __yamlConfigFileCacheHead = newCacheLine;
        }
    } else {
        nextFreeId = YAMLCONFIGFILE_UNCACHED_ID;
    }
    return nextFreeId;
}

//

yamlConfigFileRef
__yamlConfigFileCacheLookup(
    const char          *sourceFilePath,
    yamlOptionsBitvec   options
)
{
    yamlConfigFileCacheLine *cacheLine = __yamlConfigFileCacheHead;
    
    if ( options & yamlConfigFileOptions_caseFoldFilename ) {
        while ( cacheLine ) {
            const char  *s1 = sourceFilePath;
            const char  *s2 = yamlConfigFileGetSourceFilePath(cacheLine->theConfigFile);
            
            while ( *s1 && *s2 && (tolower(*s1) == *s2) ) s1++, s2++;
            if ( ! *s1 && ! *s2 ) return cacheLine->theConfigFile;
            cacheLine = cacheLine->link;
        }
    } else {
        while ( cacheLine ) {
            if ( strcmp(sourceFilePath, yamlConfigFileGetSourceFilePath(cacheLine->theConfigFile)) == 0 ) return cacheLine->theConfigFile;
            cacheLine = cacheLine->link;
        }
    }
    return NULL;
}

//

yamlConfigFileRef
yamlConfigFileCacheGetId(
    yamlConfigFileId    aConfigFileId
)
{
    yamlConfigFileRef       foundFile = NULL;
    yamlConfigFileCacheLine *cacheLine = __yamlConfigFileCacheHead;
    
    while ( cacheLine ) {
        yamlConfigFileId    cacheLineFileId = yamlConfigFileGetId(cacheLine->theConfigFile);
        
        if ( cacheLineFileId == aConfigFileId ) {
            foundFile = cacheLine->theConfigFile;
            break;
        }
        else if ( cacheLineFileId > aConfigFileId ) {
            break;
        }
        cacheLine = cacheLine->link;
    }
    return foundFile;
}

//

bool
yamlConfigFileCacheEnumerate(
    yamlConfigFileCacheEnumerator   enumeratorFn,
    const void                      *context
)
{
    bool                            okay = true;
    yamlConfigFileCacheLine         *cacheLine = __yamlConfigFileCacheHead;
    
    while ( okay && cacheLine ) {
        okay = enumeratorFn(cacheLine->theConfigFile, context);
        cacheLine = cacheLine->link;
    }
    return okay;
}

//

void
yamlConfigFileCacheEvict(
    yamlConfigFileRef   aConfigFile
)
{
    yamlConfigFileCacheLine         *cacheLine = __yamlConfigFileCacheHead, *prevCacheLine = NULL;
    
    while ( cacheLine ) {
        if ( cacheLine->theConfigFile == aConfigFile ) {
            DEBUG_PRINTF("evicted yamlConfigFile@%p from yamlConfigFileCache", aConfigFile);
            yamlConfigFileRelease(aConfigFile);
            if ( prevCacheLine ) {
                prevCacheLine->link = cacheLine->link;
            } else {
                __yamlConfigFileCacheHead = cacheLine->link;
            }
            free((void*)cacheLine);
            break;
        }
        prevCacheLine = cacheLine;
        cacheLine = cacheLine->link;
    }
}

//

void
yamlConfigFileCacheFlush()
{
    yamlConfigFileCacheLine         *cacheLine = __yamlConfigFileCacheHead;
    
    DEBUG_PRINTF("flushing yamlConfigFileCache", NULL);
    while ( cacheLine ) {
        yamlConfigFileCacheLine     *nextCacheLine = cacheLine->link;
        
        DEBUG_PRINTF("  evicted yamlConfigFile@%p from yamlConfigFileCache", cacheLine->theConfigFile);
        yamlConfigFileRelease(cacheLine->theConfigFile);
        free((void*)cacheLine);
        cacheLine = nextCacheLine;
    }
    __yamlConfigFileCacheHead = NULL;
}

//
#if 0
#pragma mark -
#endif
//

enum {
    yamlConfigFileState_isDocumentParsed = 1 << 0
};

//

typedef struct __yamlConfigFile {
    unsigned int        refCount;
    yamlConfigFileId    fileId;
    const char          *sourceFilePath;
    yamlOptionsBitvec   options;
    yamlOptionsBitvec   state;
    yaml_document_t		document;
    yamlKeyPathCacheRef keyPathCache;
} yamlConfigFile;

//

yamlConfigFile*
__yamlConfigFileAlloc(
    const char          *sourceFilePath,
    yamlOptionsBitvec   options
)
{
    size_t              sourceFilePathLen = (sourceFilePath ? (strlen(sourceFilePath) + 1) : 0);
    yamlConfigFile      *newConfigFile = calloc(1, sizeof(yamlConfigFile) + sourceFilePathLen);
    
    if ( newConfigFile ) {
        newConfigFile->refCount = 1;
        newConfigFile->fileId = YAMLCONFIGFILE_UNCACHED_ID;
        newConfigFile->options = options;
        if ( sourceFilePath ) {
            newConfigFile->sourceFilePath = (void*)newConfigFile + sizeof(yamlConfigFile);
            if ( options & yamlConfigFileOptions_caseFoldFilename ) {
                size_t  i = 0;
                
                while ( sourceFilePath[i] ) {
                    ((char*)newConfigFile->sourceFilePath)[i] = tolower(sourceFilePath[i]);
                    i++;
                }
            } else {
                strncpy((char*)newConfigFile->sourceFilePath, sourceFilePath, sourceFilePathLen);
            }
        }
        if ( ! (options & yamlConfigFileOptions_noKeyPathCache) ) newConfigFile->keyPathCache = yamlKeyPathCacheCreate(0, NULL);

#ifdef YAMLCONFIGFILE_DEBUG
        DEBUG_PRINTF("yamlConfigFile allocated:", NULL);
        if ( sourceFilePath ) DEBUG_PRINTF("  sourceFilePath : \"%s\"", newConfigFile->sourceFilePath);
        DEBUG_PRINTF("  options : %08x", newConfigFile->options);
        if ( newConfigFile->keyPathCache ) DEBUG_PRINTF("  keyPathCache : %p", newConfigFile->keyPathCache);
#endif
    }
    return newConfigFile;
}

//

yamlConfigFileRef
__yamlConfigFileCreate(
    FILE                *sourceFilePtr,
    const char          *sourceFilePath,
    yamlOptionsBitvec   options
)
{
    yamlConfigFileRef   newConfigFile = NULL;
    yaml_parser_t       parser;

    // Initialize parser:
    if ( yaml_parser_initialize(&parser) ) {
        // Attach our input stream:
        yaml_parser_set_input_file(&parser, sourceFilePtr);
        
        newConfigFile = __yamlConfigFileAlloc(sourceFilePath, options);
        if ( newConfigFile ) {
            // Attempt to parse the document:
            if ( yaml_parser_load(&parser, &newConfigFile->document) ) {
                newConfigFile->state = yamlConfigFileState_isDocumentParsed;
                
                // Do we need to cache it?
                if ( sourceFilePath && ! (options & yamlConfigFileOptions_doNotCache) ) {
                    newConfigFile->fileId = __yamlConfigFileCacheAdd(newConfigFile);
                    DEBUG_PRINTF("file \"%s\" cached with id %u", sourceFilePath, newConfigFile->fileId);
                }
            } else {
                yamlConfigFileRelease(newConfigFile);
                newConfigFile = NULL;
            }
        }
        yaml_parser_delete(&parser);
    }
    return newConfigFile;
}

//

yamlConfigFileRef
yamlConfigFileCreateWithFilePointer(
    FILE                *sourceFilePtr,
    yamlOptionsBitvec   options
)
{
    return __yamlConfigFileCreate(sourceFilePtr, NULL, options | yamlConfigFileOptions_doNotCache);
}

//

yamlConfigFileRef
yamlConfigFileCreateWithFileAtPath(
    const char          *sourceFilePath,
    yamlOptionsBitvec   options
)
{
    yamlConfigFileRef   newConfigFile = NULL;
    
    if ( ! (options & yamlConfigFileOptions_ignoreCachedCopy) ) {
        newConfigFile = __yamlConfigFileCacheLookup(sourceFilePath, options);
        if ( newConfigFile ) {
            newConfigFile = yamlConfigFileRetain(newConfigFile);
            DEBUG_PRINTF("cached file found for \"%s\" (%p)", sourceFilePath, newConfigFile);
        }
    }
    if ( ! newConfigFile ) {
        FILE            *sourceFilePtr = fopen(sourceFilePath, "r");
        
        if ( sourceFilePtr ) {
            newConfigFile = __yamlConfigFileCreate(sourceFilePtr, sourceFilePath, options);
            fclose(sourceFilePtr);
        }
    }
    return newConfigFile;
}

//

unsigned int
yamlConfigFileGetRefCount(
    yamlConfigFileRef   aConfigFile
)
{
    return aConfigFile->refCount;
}

//

yamlConfigFileRef
yamlConfigFileRetain(
    yamlConfigFileRef   aConfigFile
)
{
    aConfigFile->refCount++;
    return aConfigFile;
}

//

void
yamlConfigFileRelease(
    yamlConfigFileRef   aConfigFile
)
{
    if ( --aConfigFile->refCount == 0 ) {
        DEBUG_PRINTF("closing yamlConfigFile@%p (path \"%s\")", aConfigFile, aConfigFile->sourceFilePath ? aConfigFile->sourceFilePath : "<n/a>", aConfigFile);
        if ( aConfigFile->keyPathCache ) yamlKeyPathCacheRelease(aConfigFile->keyPathCache);
        if ( aConfigFile->state & yamlConfigFileState_isDocumentParsed ) yaml_document_delete(&aConfigFile->document);
        free((void*)aConfigFile);
    }
}

//

yamlConfigFileId
yamlConfigFileGetId(
    yamlConfigFileRef   aConfigFile
)
{
    return aConfigFile->fileId;
}

//

const char*
yamlConfigFileGetSourceFilePath(
    yamlConfigFileRef   aConfigFile
)
{
    return aConfigFile->sourceFilePath;
}

//

yaml_node_t*
yamlConfigFileGetRootNode(
    yamlConfigFileRef   aConfigFile
)
{
    return ( (aConfigFile->state & yamlConfigFileState_isDocumentParsed) ? yaml_document_get_root_node(&aConfigFile->document) : NULL );
}

//

yaml_node_t*
yamlConfigFileGetNodeAtPath(
    yamlConfigFileRef           aConfigFile,
    yamlKeyPathRef              theKeyPath,
    yaml_node_t                 *relativeToNode,
    yamlErrorCode               *outError,
    yamlKeyPathNodeMatchType*   *failedAtMatchElement
)
{
    yaml_node_t                 *outNode = NULL;
    
    if ( outError ) *outError = 0;
    if ( failedAtMatchElement ) *failedAtMatchElement = NULL;
    
    // Do we have a key path cache?
    if ( aConfigFile->keyPathCache ) {
        if ( yamlKeyPathCacheLookup(aConfigFile->keyPathCache, theKeyPath, (const void**)&outNode) ) {
            DEBUG_PRINTF("cached yaml_node_t@%p found for yamlKeyPath@%p", outNode, theKeyPath);
            return outNode;
        }
    }
    // Use the key path to walk to the node:
    outNode = yamlKeyPathApplyToDocument(
                    theKeyPath,
                    YAMLCONFIGFILEOPTIONS_KEYPATHOPTIONS(aConfigFile->options),
                    &aConfigFile->document,
                    relativeToNode,
                    outError,
                    failedAtMatchElement
                );
    if ( outNode && aConfigFile->keyPathCache ) yamlKeyPathCacheAdd(aConfigFile->keyPathCache, theKeyPath, outNode);
    return outNode;
}

//

bool
yamlConfigFileGetNodeAtPathIsType(
    yamlConfigFileRef           aConfigFile,
    yamlKeyPathRef              theKeyPath,
    yaml_node_t                 *relativeToNode,
    yamlErrorCode               *outError,
    yamlKeyPathNodeMatchType*   *failedAtMatchElement,
    yaml_node_type_t            yamlNodeType
)
{
    bool                        success = false;
    yaml_node_t                 *targetNode = yamlConfigFileGetNodeAtPath(aConfigFile, theKeyPath, relativeToNode, outError, failedAtMatchElement);
    
    if ( targetNode ) {
        if ( targetNode->type == yamlNodeType ) {
            success = true;
        } else {
            if ( outError ) *outError = yamlConfigFileError_invalidType;
        }
    }
    return success;
}

//

bool
yamlConfigFileGetNodeAtPathContentString(
    yamlConfigFileRef           aConfigFile,
    yamlKeyPathRef              theKeyPath,
    yaml_node_t                 *relativeToNode,
    yamlErrorCode               *outError,
    yamlKeyPathNodeMatchType*   *failedAtMatchElement,
    const char*                 *contentString,
    size_t                      *contentStringLength
)
{
    bool                        success = false;
    yaml_node_t                 *targetNode = yamlConfigFileGetNodeAtPath(aConfigFile, theKeyPath, relativeToNode, outError, failedAtMatchElement);
    
    if ( targetNode ) {
        if ( targetNode->type == YAML_SCALAR_NODE ) {
            *contentString = targetNode->data.scalar.value;
            *contentStringLength = targetNode->data.scalar.length;
            success = true;
        } else {
            if ( outError ) *outError = yamlConfigFileError_invalidType;
        }
    }
    return success;
}

//

bool
yamlConfigFileGetNodeAtPathSequenceLength(
    yamlConfigFileRef           aConfigFile,
    yamlKeyPathRef              theKeyPath,
    yaml_node_t                 *relativeToNode,
    yamlErrorCode               *outError,
    yamlKeyPathNodeMatchType*   *failedAtMatchElement,
    unsigned int                *sequenceLength
)
{
    bool                        success = false;
    yaml_node_t                 *targetNode = yamlConfigFileGetNodeAtPath(aConfigFile, theKeyPath, relativeToNode, outError, failedAtMatchElement);
    
    if ( targetNode ) {
        if ( targetNode->type == YAML_SEQUENCE_NODE ) {
            *sequenceLength = targetNode->data.sequence.items.top - targetNode->data.sequence.items.start;
            success = true;
        } else {
            if ( outError ) *outError = yamlConfigFileError_invalidType;
        }
    }
    return success;
}

//

bool
yamlConfigFileGetNodeAtPathMappingKeyCount(
    yamlConfigFileRef           aConfigFile,
    yamlKeyPathRef              theKeyPath,
    yaml_node_t                 *relativeToNode,
    yamlErrorCode               *outError,
    yamlKeyPathNodeMatchType*   *failedAtMatchElement,
    unsigned int                *mappingKeyCount
)
{
    bool                        success = false;
    yaml_node_t                 *targetNode = yamlConfigFileGetNodeAtPath(aConfigFile, theKeyPath, relativeToNode, outError, failedAtMatchElement);
    
    if ( targetNode ) {
        if ( targetNode->type == YAML_MAPPING_NODE ) {
            *mappingKeyCount = targetNode->data.mapping.pairs.top - targetNode->data.mapping.pairs.start;
            success = true;
        } else {
            if ( outError ) *outError = yamlConfigFileError_invalidType;
        }
    }
    return success;
}

//

bool
yamlConfigFileGetNodeAtPathMappingOrSequenceLength(
    yamlConfigFileRef           aConfigFile,
    yamlKeyPathRef              theKeyPath,
    yaml_node_t                 *relativeToNode,
    yamlErrorCode               *outError,
    yamlKeyPathNodeMatchType*   *failedAtMatchElement,
    unsigned int                *sequenceOrMappingLength
)
{
    bool                        success = false;
    yaml_node_t                 *targetNode = yamlConfigFileGetNodeAtPath(aConfigFile, theKeyPath, relativeToNode, outError, failedAtMatchElement);
    
    if ( targetNode ) {
        switch ( targetNode->type ) {
            case YAML_SEQUENCE_NODE: {
                *sequenceOrMappingLength = targetNode->data.sequence.items.top - targetNode->data.sequence.items.start;
                success = true;
                break;
            }
            case  YAML_MAPPING_NODE: {
                *sequenceOrMappingLength = targetNode->data.mapping.pairs.top - targetNode->data.mapping.pairs.start;
                success = true;
                break;
            }
            default:
                if ( outError ) *outError = yamlConfigFileError_invalidType;
                break;
        }
    }
    return success;
}

//

bool
__yamlConfigFileCoerceScalar(
    yamlConfigFileRef           aConfigFile,
    yamlKeyPathRef              theKeyPath,
    yaml_node_t                 *relativeToNode,
    yamlErrorCode               *outError,
    yamlKeyPathNodeMatchType*   *failedAtMatchElement,
    yamlConfigFileCoerceToType  coerceToType,
    va_list                     vargs
)
{
    yaml_node_t                 *targetNode = yamlConfigFileGetNodeAtPath(aConfigFile, theKeyPath, relativeToNode, outError, failedAtMatchElement);
    
    if ( targetNode ) {
        if ( targetNode->type == YAML_SCALAR_NODE ) {
            switch ( coerceToType ) {
                
                case yamlConfigFileCoerceToType_cstring: {
                    char            *buffer = va_arg(vargs, char*);
                    size_t          *bufferLen = va_arg(vargs, size_t*);
                    
                    if ( bufferLen ) {
                        size_t      copyLen = *bufferLen;
                        
                        if ( buffer && copyLen ) {
                            if ( targetNode->data.scalar.length < copyLen ) {
                                memcpy(buffer, targetNode->data.scalar.value, targetNode->data.scalar.length);
                                buffer[targetNode->data.scalar.length] = '\0';
                            } else {
                                strncpy(buffer, targetNode->data.scalar.value, copyLen);
                            }
                        }
                        *bufferLen = targetNode->data.scalar.length;
                    }
                    return true;
                }
                
                case yamlConfigFileCoerceToType_charArray: {
                    char            *buffer = va_arg(vargs, char*);
                    size_t          *bufferLen = va_arg(vargs, size_t*);
                    
                    if ( bufferLen ) {
                        size_t      copyLen = *bufferLen;
                        
                        if ( buffer && copyLen ) {
                            if ( targetNode->data.scalar.length <= copyLen ) copyLen = targetNode->data.scalar.length;
                            memcpy(buffer, targetNode->data.scalar.value, copyLen);
                        }
                        *bufferLen = targetNode->data.scalar.length;
                    }
                    return true;
                }
                
                case yamlConfigFileCoerceToType_bool: {
                    bool            tmpValue;
                    
                    if ( __yamlScalarNodeToBool(targetNode, &tmpValue) ) {
                        bool        *outValue = va_arg(vargs, bool*);
                    
                        if ( outValue ) *outValue = tmpValue;
                        return true;
                    } else {
                        if ( outError ) *outError = yamlConfigFileError_failedToCoerce;
                    }
                    break;
                }
                
                case yamlConfigFileCoerceToType_int:
                case yamlConfigFileCoerceToType_longInt: 
                case yamlConfigFileCoerceToType_longLongInt:
                case yamlConfigFileCoerceToType_int8:
                case yamlConfigFileCoerceToType_int16:
                case yamlConfigFileCoerceToType_int32:
                case yamlConfigFileCoerceToType_int64: {
                    long long int   tmpValue;
                    
                    if ( __yamlScalarNodeToSignedInt(targetNode, &tmpValue) ) {
                        bool        okay = false;
                        
                        switch ( coerceToType ) {
                        
                            case yamlConfigFileCoerceToType_int: {
                                int     *outValue = va_arg(vargs, int*);
                                
                                if ( (tmpValue >= INT_MIN) && (tmpValue <= INT_MAX) ) {
                                    if ( outValue ) *outValue = (int)tmpValue;
                                    okay = true;
                                } else {
                                    if ( outError ) *outError = yamlConfigFileError_numberOutOfRange;
                                }
                                break;
                            }
                            
                            case yamlConfigFileCoerceToType_longInt: {
                                long int    *outValue = va_arg(vargs, long int*);
                                
                                if ( (tmpValue >= LONG_MIN) && (tmpValue <= LONG_MAX) ) {
                                    if ( outValue ) *outValue = (long int)tmpValue;
                                    okay = true;
                                } else {
                                    if ( outError ) *outError = yamlConfigFileError_numberOutOfRange;
                                }
                                break;
                            }
                            
                            case yamlConfigFileCoerceToType_longLongInt: {
                                long long int   *outValue = va_arg(vargs, long long int*);
                                
                                if ( outValue ) *outValue = tmpValue;
                                okay = true;
                                break;
                            }
                        
                            case yamlConfigFileCoerceToType_int8: {
                                int8_t  *outValue = va_arg(vargs, int8_t*);
                                
                                if ( (tmpValue >= INT8_MIN) && (tmpValue <= INT8_MAX) ) {
                                    if ( outValue ) *outValue = (int8_t)tmpValue;
                                    okay = true;
                                } else {
                                    if ( outError ) *outError = yamlConfigFileError_numberOutOfRange;
                                }
                                break;
                            }
                        
                            case yamlConfigFileCoerceToType_int16: {
                                int16_t *outValue = va_arg(vargs, int16_t*);
                                
                                if ( (tmpValue >= INT16_MIN) && (tmpValue <= INT16_MAX) ) {
                                    if ( outValue ) *outValue = (int16_t)tmpValue;
                                    okay = true;
                                } else {
                                    if ( outError ) *outError = yamlConfigFileError_numberOutOfRange;
                                }
                                break;
                            }
                        
                            case yamlConfigFileCoerceToType_int32: {
                                int32_t *outValue = va_arg(vargs, int32_t*);
                                
                                if ( (tmpValue >= INT32_MIN) && (tmpValue <= INT32_MAX) ) {
                                    if ( outValue ) *outValue = (int32_t)tmpValue;
                                    okay = true;
                                } else {
                                    if ( outError ) *outError = yamlConfigFileError_numberOutOfRange;
                                }
                                break;
                            }
                        
                            case yamlConfigFileCoerceToType_int64: {
                                int64_t *outValue = va_arg(vargs, int64_t*);
                                
                                if ( (tmpValue >= INT64_MIN) && (tmpValue <= INT64_MAX) ) {
                                    if ( outValue ) *outValue = (int64_t)tmpValue;
                                    okay = true;
                                } else {
                                    if ( outError ) *outError = yamlConfigFileError_numberOutOfRange;
                                }
                                break;
                            }
                        }
                        return okay;
                    } else {
                        if ( outError ) *outError = yamlConfigFileError_failedToCoerce;
                    }
                    break;
                }
                
                case yamlConfigFileCoerceToType_unsignedInt:
                case yamlConfigFileCoerceToType_unsignedLongInt:
                case yamlConfigFileCoerceToType_unsignedLongLongInt:
                case yamlConfigFileCoerceToType_uint8:
                case yamlConfigFileCoerceToType_uint16:
                case yamlConfigFileCoerceToType_uint32:
                case yamlConfigFileCoerceToType_uint64: {
                    unsigned long long int  tmpValue;
                    
                    if ( __yamlScalarNodeToUnsignedInt(targetNode, &tmpValue) ) {
                        bool        okay = false;
                        
                        switch ( coerceToType ) {
                        
                            case yamlConfigFileCoerceToType_unsignedInt: {
                                unsigned int    *outValue = va_arg(vargs, unsigned int*);
                                
                                if ( tmpValue <= UINT_MAX ) {
                                    if ( outValue ) *outValue = (unsigned int)tmpValue;
                                    okay = true;
                                } else {
                                    if ( outError ) *outError = yamlConfigFileError_numberOutOfRange;
                                }
                                break;
                            }
                            
                            case yamlConfigFileCoerceToType_unsignedLongInt: {
                                unsigned long int   *outValue = va_arg(vargs, unsigned long int*);
                                
                                if ( tmpValue <= ULONG_MAX ) {
                                    if ( outValue )  *outValue = (unsigned long int)tmpValue;
                                    okay = true;
                                } else {
                                    if ( outError ) *outError = yamlConfigFileError_numberOutOfRange;
                                }
                                break;
                            }
                            
                            case yamlConfigFileCoerceToType_unsignedLongLongInt: {
                                unsigned long long int  *outValue = va_arg(vargs, unsigned long long int*);
                                
                                if ( outValue ) *outValue = tmpValue;
                                okay = true;
                                break;
                            }
                        
                            case yamlConfigFileCoerceToType_uint8: {
                                uint8_t     *outValue = va_arg(vargs, uint8_t*);
                                
                                if ( tmpValue <= UINT8_MAX ) {
                                    if ( outValue ) *outValue = (uint8_t)tmpValue;
                                    okay = true;
                                } else {
                                    if ( outError ) *outError = yamlConfigFileError_numberOutOfRange;
                                }
                                break;
                            }
                        
                            case yamlConfigFileCoerceToType_uint16: {
                                uint16_t    *outValue = va_arg(vargs, uint16_t*);
                                
                                if ( tmpValue <= UINT16_MAX ) {
                                    if ( outValue ) *outValue = (uint16_t)tmpValue;
                                    okay = true;
                                } else {
                                    if ( outError ) *outError = yamlConfigFileError_numberOutOfRange;
                                }
                                break;
                            }
                        
                            case yamlConfigFileCoerceToType_uint32: {
                                uint32_t    *outValue = va_arg(vargs, uint32_t*);
                                
                                if ( tmpValue <= UINT32_MAX ) {
                                    if ( outValue ) *outValue = (uint32_t)tmpValue;
                                    okay = true;
                                } else {
                                    if ( outError ) *outError = yamlConfigFileError_numberOutOfRange;
                                }
                                break;
                            }
                        
                            case yamlConfigFileCoerceToType_uint64: {
                                uint64_t    *outValue = va_arg(vargs, uint64_t*);
                                
                                if ( tmpValue <= UINT64_MAX ) {
                                    if ( outValue ) *outValue = (uint64_t)tmpValue;
                                    okay = true;
                                } else {
                                    if ( outError ) *outError = yamlConfigFileError_numberOutOfRange;
                                }
                                break;
                            }
                        }
                        return okay;
                    } else {
                        if ( outError ) *outError = yamlConfigFileError_failedToCoerce;
                    }
                    break;
                }
                
                case yamlConfigFileCoerceToType_float: {
                    float       tmpValue;
                    
                    if ( __yamlScalarNodeToFloat(targetNode, &tmpValue) ) {
                        float   *outValue = va_arg(vargs, float*);
                        
                        if ( outValue ) *outValue = tmpValue;
                        return true;
                    } else {
                        if ( outError ) *outError = yamlConfigFileError_failedToCoerce;
                    }
                    break;
                }
                
                case yamlConfigFileCoerceToType_double: {
                    double      tmpValue;
                    
                    if ( __yamlScalarNodeToDouble(targetNode, &tmpValue) ) {
                        double  *outValue = va_arg(vargs, double*);
                        
                        if ( outValue ) *outValue = tmpValue;
                        return true;
                    } else {
                        if ( outError ) *outError = yamlConfigFileError_failedToCoerce;
                    }
                    break;
                }
            
                default:
                    if ( outError ) *outError = yamlConfigFileError_invalidType;
                    break;
            }
        } else {
            if ( outError ) *outError = yamlConfigFileError_invalidType;
        }
    }
    return false;
}

//

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
)
{
    bool                        okay = false;
    yamlKeyPathRef              keyPath = yamlKeyPathCreateWithString(
                                                pathString, pathStringLen,
                                                YAMLCONFIGFILEOPTIONS_KEYPATHOPTIONS(aConfigFile->options),
                                                outError, outErrorAtChar
                                            );
    if ( keyPath ) {
        va_list                 vargs;
        
        va_start(vargs, coerceToType);
        okay = __yamlConfigFileCoerceScalar(aConfigFile, keyPath, relativeToNode, outError, failedAtMatchElement, coerceToType, vargs);
        va_end(vargs);
        yamlKeyPathRelease(keyPath);
    }
    return okay;
}

//

bool
yamlConfigFileCoerceScalarAtPath(
    yamlConfigFileRef           aConfigFile,
    yamlKeyPathRef              theKeyPath,
    yaml_node_t                 *relativeToNode,
    yamlErrorCode               *outError,
    yamlKeyPathNodeMatchType*   *failedAtMatchElement,
    yamlConfigFileCoerceToType  coerceToType,
    ...
)
{
    bool                        okay = false;
    va_list                     vargs;
        
    va_start(vargs, coerceToType);
    okay = __yamlConfigFileCoerceScalar(aConfigFile, theKeyPath, relativeToNode, outError, failedAtMatchElement, coerceToType, vargs);
    va_end(vargs);
    return okay;
}

//

bool
__yamlConfigFileCoerceSequence(
    yamlConfigFileRef           aConfigFile,
    yamlKeyPathRef              theKeyPath,
    yaml_node_t                 *relativeToNode,
    yamlErrorCode               *outError,
    yamlKeyPathNodeMatchType*   *failedAtMatchElement,
    unsigned int                startSequenceIndex,
    unsigned int                endSequenceIndex,
    yamlConfigFileCoerceToType  coerceToType,
    va_list                     vargs
)
{
    if ( startSequenceIndex <= endSequenceIndex ) {
        yaml_node_t                 *targetNode = yamlConfigFileGetNodeAtPath(aConfigFile, theKeyPath, relativeToNode, outError, failedAtMatchElement);
        
        if ( targetNode ) {
            if ( targetNode->type == YAML_SEQUENCE_NODE ) {
                unsigned int        sequenceLength = targetNode->data.sequence.items.top - targetNode->data.sequence.items.start;
                
                // Validate the index range:
                if ( (startSequenceIndex < sequenceLength) && (endSequenceIndex < sequenceLength) ) {
                    yaml_node_item_t	*s = targetNode->data.sequence.items.start + startSequenceIndex;
                    yaml_node_item_t	*e = targetNode->data.sequence.items.start + endSequenceIndex + 1;
                    
                    switch ( coerceToType ) {
                        
                        case yamlConfigFileCoerceToType_bool: {
                            bool            *outValue = va_arg(vargs, bool*);
                            
                            if ( outValue ) {
                                while ( s < e ) {
                                    yaml_node_t     *node = yaml_document_get_node(&aConfigFile->document, *s++);
                                    
                                    if ( node ) {
                                        if ( ! __yamlScalarNodeToBool(node, outValue) ) {
                                            if ( outError ) *outError = yamlConfigFileError_failedToCoerce;
                                            return false;
                                        }
                                    } else {
                                        if ( outError ) *outError = yamlConfigFileError_internalError;
                                        return false;
                                    }
                                    outValue++;
                                    s++;
                                }
                            }
                            return true;
                        }
                        
                        case yamlConfigFileCoerceToType_int: {
                            int                     *outValue = va_arg(vargs, int*);
                            
                            if ( outValue ) {
                                while ( s < e ) {
                                    yaml_node_t     *node = yaml_document_get_node(&aConfigFile->document, *s++);
                                    
                                    if ( node ) {
                                        long long int   tmpValue;
                                        
                                        if ( ! __yamlScalarNodeToSignedInt(node, &tmpValue) ) {
                                            if ( outError ) *outError = yamlConfigFileError_failedToCoerce;
                                            return false;
                                        }
                                        if ( (tmpValue >= INT_MIN) && (tmpValue <= INT_MAX) ) {
                                            *outValue++ = tmpValue;
                                        } else {
                                            if ( outError ) *outError = yamlConfigFileError_numberOutOfRange;
                                            return false;
                                        }
                                    } else {
                                        if ( outError ) *outError = yamlConfigFileError_internalError;
                                        return false;
                                    }
                                }
                            }
                            return true;
                        }
                        case yamlConfigFileCoerceToType_longInt: {
                            long int                    *outValue = va_arg(vargs, long int*);
                            
                            if ( outValue ) {
                                while ( s < e ) {
                                    yaml_node_t     *node = yaml_document_get_node(&aConfigFile->document, *s++);
                                    
                                    if ( node ) {
                                        long long int   tmpValue;
                                        
                                        if ( ! __yamlScalarNodeToSignedInt(node, &tmpValue) ) {
                                            if ( outError ) *outError = yamlConfigFileError_failedToCoerce;
                                            return false;
                                        }
                                        if ( (tmpValue >= LONG_MIN) && (tmpValue <= LONG_MAX) ) {
                                            *outValue++ = tmpValue;
                                        } else {
                                            if ( outError ) *outError = yamlConfigFileError_numberOutOfRange;
                                            return false;
                                        }
                                    } else {
                                        if ( outError ) *outError = yamlConfigFileError_internalError;
                                        return false;
                                    }
                                }
                            }
                            return true;
                        }
                        case yamlConfigFileCoerceToType_longLongInt: {
                            long long int           *outValue = va_arg(vargs, long long int*);
                            
                            if ( outValue ) {
                                while ( s < e ) {
                                    yaml_node_t     *node = yaml_document_get_node(&aConfigFile->document, *s++);
                                    
                                    if ( node ) {
                                        long long int   tmpValue;
                                        
                                        if ( ! __yamlScalarNodeToSignedInt(node, &tmpValue) ) {
                                            if ( outError ) *outError = yamlConfigFileError_failedToCoerce;
                                            return false;
                                        }
                                        *outValue++ = tmpValue;
                                    } else {
                                        if ( outError ) *outError = yamlConfigFileError_internalError;
                                        return false;
                                    }
                                }
                            }
                            return true;
                        }
                        case yamlConfigFileCoerceToType_int8: {
                            int8_t                  *outValue = va_arg(vargs, int8_t*);
                            
                            if ( outValue ) {
                                while ( s < e ) {
                                    yaml_node_t     *node = yaml_document_get_node(&aConfigFile->document, *s++);
                                    
                                    if ( node ) {
                                        long long int   tmpValue;
                                        
                                        if ( ! __yamlScalarNodeToSignedInt(node, &tmpValue) ) {
                                            if ( outError ) *outError = yamlConfigFileError_failedToCoerce;
                                            return false;
                                        }
                                        if ( (tmpValue >= INT8_MIN) && (tmpValue <= INT8_MAX) ) {
                                            *outValue++ = (int8_t)tmpValue;
                                        } else {
                                            if ( outError ) *outError = yamlConfigFileError_numberOutOfRange;
                                            return false;
                                        }
                                    } else {
                                        if ( outError ) *outError = yamlConfigFileError_internalError;
                                        return false;
                                    }
                                }
                            }
                            return true;
                        }
                        case yamlConfigFileCoerceToType_int16: {
                            int16_t                 *outValue = va_arg(vargs, int16_t*);
                            
                            if ( outValue ) {
                                while ( s < e ) {
                                    yaml_node_t     *node = yaml_document_get_node(&aConfigFile->document, *s++);
                                    
                                    if ( node ) {
                                        long long int   tmpValue;
                                        
                                        if ( ! __yamlScalarNodeToSignedInt(node, &tmpValue) ) {
                                            if ( outError ) *outError = yamlConfigFileError_failedToCoerce;
                                            return false;
                                        }
                                        if ( (tmpValue >= INT16_MIN) && (tmpValue <= INT16_MAX) ) {
                                            *outValue++ = (int16_t)tmpValue;
                                        } else {
                                            if ( outError ) *outError = yamlConfigFileError_numberOutOfRange;
                                            return false;
                                        }
                                    } else {
                                        if ( outError ) *outError = yamlConfigFileError_internalError;
                                        return false;
                                    }
                                }
                            }
                            return true;
                        }
                        case yamlConfigFileCoerceToType_int32: {
                            int32_t                 *outValue = va_arg(vargs, int32_t*);
                            
                            if ( outValue ) {
                                while ( s < e ) {
                                    yaml_node_t     *node = yaml_document_get_node(&aConfigFile->document, *s++);
                                    
                                    if ( node ) {
                                        long long int   tmpValue;
                                        
                                        if ( ! __yamlScalarNodeToSignedInt(node, &tmpValue) ) {
                                            if ( outError ) *outError = yamlConfigFileError_failedToCoerce;
                                            return false;
                                        }
                                        if ( (tmpValue >= INT32_MIN) && (tmpValue <= INT32_MAX) ) {
                                            *outValue++ = (int32_t)tmpValue;
                                        } else {
                                            if ( outError ) *outError = yamlConfigFileError_numberOutOfRange;
                                            return false;
                                        }
                                    } else {
                                        if ( outError ) *outError = yamlConfigFileError_internalError;
                                        return false;
                                    }
                                }
                            }
                            return true;
                        }
                        case yamlConfigFileCoerceToType_int64: {
                            int64_t                 *outValue = va_arg(vargs, int64_t*);
                            
                            if ( outValue ) {
                                while ( s < e ) {
                                    yaml_node_t     *node = yaml_document_get_node(&aConfigFile->document, *s++);
                                    
                                    if ( node ) {
                                        long long int   tmpValue;
                                        
                                        if ( ! __yamlScalarNodeToSignedInt(node, &tmpValue) ) {
                                            if ( outError ) *outError = yamlConfigFileError_failedToCoerce;
                                            return false;
                                        }
                                        if ( (tmpValue >= INT64_MIN) && (tmpValue <= INT64_MAX) ) {
                                            *outValue++ = (int64_t)tmpValue;
                                        } else {
                                            if ( outError ) *outError = yamlConfigFileError_numberOutOfRange;
                                            return false;
                                        }
                                    } else {
                                        if ( outError ) *outError = yamlConfigFileError_internalError;
                                        return false;
                                    }
                                }
                            }
                            return true;
                        }
                        
                        case yamlConfigFileCoerceToType_unsignedInt: {
                            unsigned int            *outValue = va_arg(vargs, unsigned int*);
                            
                            if ( outValue ) {
                                while ( s < e ) {
                                    yaml_node_t     *node = yaml_document_get_node(&aConfigFile->document, *s++);
                                    
                                    if ( node ) {
                                        unsigned long long int  tmpValue;
                                        
                                        if ( ! __yamlScalarNodeToUnsignedInt(node, &tmpValue) ) {
                                            if ( outError ) *outError = yamlConfigFileError_failedToCoerce;
                                            return false;
                                        }
                                        if ( tmpValue <= UINT_MAX ) {
                                            *outValue++ = tmpValue;
                                        } else {
                                            if ( outError ) *outError = yamlConfigFileError_numberOutOfRange;
                                            return false;
                                        }
                                    } else {
                                        if ( outError ) *outError = yamlConfigFileError_internalError;
                                        return false;
                                    }
                                }
                            }
                            return true;
                        }
                        case yamlConfigFileCoerceToType_unsignedLongInt: {
                            unsigned long int       *outValue = va_arg(vargs, unsigned long int*);
                            
                            if ( outValue ) {
                                while ( s < e ) {
                                    yaml_node_t     *node = yaml_document_get_node(&aConfigFile->document, *s++);
                                    
                                    if ( node ) {
                                        unsigned long long int  tmpValue;
                                        
                                        if ( ! __yamlScalarNodeToUnsignedInt(node, &tmpValue) ) {
                                            if ( outError ) *outError = yamlConfigFileError_failedToCoerce;
                                            return false;
                                        }
                                        if ( tmpValue <= UINT_MAX ) {
                                            *outValue++ = tmpValue;
                                        } else {
                                            if ( outError ) *outError = yamlConfigFileError_numberOutOfRange;
                                            return false;
                                        }
                                    } else {
                                        if ( outError ) *outError = yamlConfigFileError_internalError;
                                        return false;
                                    }
                                }
                            }
                            return true;
                        }
                        case yamlConfigFileCoerceToType_unsignedLongLongInt: {
                            unsigned long long int  *outValue = va_arg(vargs, unsigned long long int*);
                            
                            if ( outValue ) {
                                while ( s < e ) {
                                    yaml_node_t     *node = yaml_document_get_node(&aConfigFile->document, *s++);
                                    
                                    if ( node ) {
                                        unsigned long long int  tmpValue;
                                        
                                        if ( ! __yamlScalarNodeToUnsignedInt(node, &tmpValue) ) {
                                            if ( outError ) *outError = yamlConfigFileError_failedToCoerce;
                                            return false;
                                        }
                                        *outValue++ = tmpValue;
                                    } else {
                                        if ( outError ) *outError = yamlConfigFileError_internalError;
                                        return false;
                                    }
                                }
                            }
                            return true;
                        }
                        case yamlConfigFileCoerceToType_uint8: {
                            uint8_t                 *outValue = va_arg(vargs, uint8_t*);
                            
                            if ( outValue ) {
                                while ( s < e ) {
                                    yaml_node_t     *node = yaml_document_get_node(&aConfigFile->document, *s++);
                                    
                                    if ( node ) {
                                        unsigned long long int  tmpValue;
                                        
                                        if ( ! __yamlScalarNodeToUnsignedInt(node, &tmpValue) ) {
                                            if ( outError ) *outError = yamlConfigFileError_failedToCoerce;
                                            return false;
                                        }
                                        if ( tmpValue <= UINT8_MAX ) {
                                            *outValue++ = (uint8_t)tmpValue;
                                        } else {
                                            if ( outError ) *outError = yamlConfigFileError_numberOutOfRange;
                                            return false;
                                        }
                                    } else {
                                        if ( outError ) *outError = yamlConfigFileError_internalError;
                                        return false;
                                    }
                                }
                            }
                            return true;
                        }
                        case yamlConfigFileCoerceToType_uint16: {
                            uint16_t                *outValue = va_arg(vargs, uint16_t*);
                            
                            if ( outValue ) {
                                while ( s < e ) {
                                    yaml_node_t     *node = yaml_document_get_node(&aConfigFile->document, *s++);
                                    
                                    if ( node ) {
                                        unsigned long long int  tmpValue;
                                        
                                        if ( ! __yamlScalarNodeToUnsignedInt(node, &tmpValue) ) {
                                            if ( outError ) *outError = yamlConfigFileError_failedToCoerce;
                                            return false;
                                        }
                                        if ( tmpValue <= UINT16_MAX ) {
                                            *outValue++ = (uint16_t)tmpValue;
                                        } else {
                                            if ( outError ) *outError = yamlConfigFileError_numberOutOfRange;
                                            return false;
                                        }
                                    } else {
                                        if ( outError ) *outError = yamlConfigFileError_internalError;
                                        return false;
                                    }
                                }
                            }
                            return true;
                        }
                        case yamlConfigFileCoerceToType_uint32: {
                            uint32_t                *outValue = va_arg(vargs, uint32_t*);
                            
                            if ( outValue ) {
                                while ( s < e ) {
                                    yaml_node_t     *node = yaml_document_get_node(&aConfigFile->document, *s++);
                                    
                                    if ( node ) {
                                        unsigned long long int  tmpValue;
                                        
                                        if ( ! __yamlScalarNodeToUnsignedInt(node, &tmpValue) ) {
                                            if ( outError ) *outError = yamlConfigFileError_failedToCoerce;
                                            return false;
                                        }
                                        if ( tmpValue <= UINT32_MAX ) {
                                            *outValue++ = (uint32_t)tmpValue;
                                        } else {
                                            if ( outError ) *outError = yamlConfigFileError_numberOutOfRange;
                                            return false;
                                        }
                                    } else {
                                        if ( outError ) *outError = yamlConfigFileError_internalError;
                                        return false;
                                    }
                                }
                            }
                            return true;
                        }
                        case yamlConfigFileCoerceToType_uint64: {
                            uint64_t                *outValue = va_arg(vargs, uint64_t*);
                            
                            if ( outValue ) {
                                while ( s < e ) {
                                    yaml_node_t     *node = yaml_document_get_node(&aConfigFile->document, *s++);
                                    
                                    if ( node ) {
                                        unsigned long long int  tmpValue;
                                        
                                        if ( ! __yamlScalarNodeToUnsignedInt(node, &tmpValue) ) {
                                            if ( outError ) *outError = yamlConfigFileError_failedToCoerce;
                                            return false;
                                        }
                                        if ( tmpValue <= UINT64_MAX ) {
                                            *outValue++ = (uint64_t)tmpValue;
                                        } else {
                                            if ( outError ) *outError = yamlConfigFileError_numberOutOfRange;
                                            return false;
                                        }
                                    } else {
                                        if ( outError ) *outError = yamlConfigFileError_internalError;
                                        return false;
                                    }
                                }
                            }
                            return true;
                        }
                                                
                        case yamlConfigFileCoerceToType_float: {
                            float               *outValue = va_arg(vargs, float*);
                            
                            if ( outValue ) {
                                while ( s < e ) {
                                    yaml_node_t *node = yaml_document_get_node(&aConfigFile->document, *s++);
                                    
                                    if ( node ) {
                                        float   tmpValue;
                                        
                                        if ( ! __yamlScalarNodeToFloat(node, &tmpValue) ) {
                                            if ( outError ) *outError = yamlConfigFileError_failedToCoerce;
                                            return false;
                                        }
                                        *outValue++ = tmpValue;
                                    } else {
                                        if ( outError ) *outError = yamlConfigFileError_internalError;
                                        return false;
                                    }
                                }
                            }
                            return true;
                        }
                        
                        case yamlConfigFileCoerceToType_double: {
                            double              *outValue = va_arg(vargs, double*);
                            
                            if ( outValue ) {
                                while ( s < e ) {
                                    yaml_node_t *node = yaml_document_get_node(&aConfigFile->document, *s++);
                                    
                                    if ( node ) {
                                        double  tmpValue;
                                        
                                        if ( ! __yamlScalarNodeToDouble(node, &tmpValue) ) {
                                            if ( outError ) *outError = yamlConfigFileError_failedToCoerce;
                                            return false;
                                        }
                                        *outValue++ = tmpValue;
                                    } else {
                                        if ( outError ) *outError = yamlConfigFileError_internalError;
                                        return false;
                                    }
                                }
                            }
                            return true;
                        }
                        
                        default:
                            if ( outError ) *outError = yamlConfigFileError_invalidType;
                            break;
                    }
                } else {
                    if ( outError ) *outError = yamlConfigFileError_invalidSequenceIndex;
                }
            } else {
                if ( outError ) *outError = yamlConfigFileError_invalidType;
            }
        }
    }
    return false;
}

//

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
)
{
    bool                        okay = false;
    yamlKeyPathRef              keyPath = yamlKeyPathCreateWithString(
                                                pathString, pathStringLen,
                                                YAMLCONFIGFILEOPTIONS_KEYPATHOPTIONS(aConfigFile->options),
                                                outError, outErrorAtChar
                                            );
    if ( keyPath ) {
        va_list                 vargs;
        
        va_start(vargs, coerceToType);
        okay = __yamlConfigFileCoerceSequence(aConfigFile, keyPath, relativeToNode, outError, failedAtMatchElement, coerceToType, startSequenceIndex, endSequenceIndex, vargs);
        va_end(vargs);
        yamlKeyPathRelease(keyPath);
    }
    return okay;
}

//

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
)
{
    bool                        okay = false;
    va_list                     vargs;
        
    va_start(vargs, coerceToType);
    okay = __yamlConfigFileCoerceSequence(aConfigFile, theKeyPath, relativeToNode, outError, failedAtMatchElement, coerceToType, startSequenceIndex, endSequenceIndex, vargs);
    va_end(vargs);
    return okay;
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
    yamlErrorCode               lastError;
    yamlKeyPathNodeMatchType    *failedMatch;
    yamlConfigFileRef           theConfigFile = yamlConfigFileCreateWithFileAtPath(
                                                    "test.yaml",
                                                    yamlConfigFileOptions_oneBasedIndices | yamlConfigFileOptions_caseFoldKeys
                                                );
    
    yamlConfigFileCreateWithFileAtPath(
                "/opt/shared/valet/etc/intel.vpkg_yaml",
                0
            );
    yamlconfigfile_summary_();
    
    if ( theConfigFile ) {
        yamlKeyPathRef          keyPath;
        unsigned int            nruns;
        uint64_t                irxtr;
        double                  tin, abyv, solver_abstol, bandwidth_limits[2];
        const char              str[256];
        size_t                  strLen;
        bool                    reactor_ltra;
        bool                    MultiInput;
        
        keyPath = yamlKeyPathCreateWithString("nruns", 0, yamlKeyPathCompileOptions_caseFoldKeys | yamlKeyPathCompileOptions_oneBasedIndices, NULL, NULL);
        if ( keyPath ) {
            if ( yamlConfigFileCoerceScalarAtPath(theConfigFile, keyPath, NULL, &lastError, &failedMatch, yamlConfigFileCoerceToType_unsignedInt, &nruns) ) {
                printf("nruns = %u\n", nruns);
            }
            yamlKeyPathRelease(keyPath);
        }
        
        keyPath = yamlKeyPathCreateWithString("irxtr", 0, yamlKeyPathCompileOptions_caseFoldKeys | yamlKeyPathCompileOptions_oneBasedIndices, NULL, NULL);
        if ( keyPath ) {
            if ( yamlConfigFileCoerceScalarAtPath(theConfigFile, keyPath, NULL, NULL, NULL, yamlConfigFileCoerceToType_uint64, &irxtr) ) {
                printf("irxtr = %llu\n", (unsigned long long int)irxtr);
            }
            yamlKeyPathRelease(keyPath);
        }
        
        keyPath = yamlKeyPathCreateWithString("tin", 0, yamlKeyPathCompileOptions_caseFoldKeys | yamlKeyPathCompileOptions_oneBasedIndices, NULL, NULL);
        if ( keyPath ) {
            if ( yamlConfigFileCoerceScalarAtPath(theConfigFile, keyPath, NULL, NULL, NULL, yamlConfigFileCoerceToType_double, &tin) ) {
                printf("tin = %lg\n", tin);
            }
            yamlKeyPathRelease(keyPath);
        }
        
        keyPath = yamlKeyPathCreateWithString("tin", 0, yamlKeyPathCompileOptions_caseFoldKeys | yamlKeyPathCompileOptions_oneBasedIndices, NULL, NULL);
        if ( keyPath ) {
            if ( yamlConfigFileCoerceScalarAtPath(theConfigFile, keyPath, NULL, NULL, NULL, yamlConfigFileCoerceToType_double, &tin) ) {
                printf("tin = %lg\n", tin);
            }
            yamlKeyPathRelease(keyPath);
        }
        
        keyPath = yamlKeyPathCreateWithString("abyv", 0, yamlKeyPathCompileOptions_caseFoldKeys | yamlKeyPathCompileOptions_oneBasedIndices, NULL, NULL);
        if ( keyPath ) {
            if ( yamlConfigFileCoerceScalarAtPath(theConfigFile, keyPath, NULL, NULL, NULL, yamlConfigFileCoerceToType_double, &abyv) ) {
                printf("abyv = %lg\n", abyv);
            }
            yamlKeyPathRelease(keyPath);
        }
        
        keyPath = yamlKeyPathCreateWithString("solver.abstol", 0, yamlKeyPathCompileOptions_caseFoldKeys | yamlKeyPathCompileOptions_oneBasedIndices, NULL, NULL);
        if ( keyPath ) {
            if ( yamlConfigFileCoerceScalarAtPath(theConfigFile, keyPath, NULL, NULL, NULL, yamlConfigFileCoerceToType_double, &solver_abstol) ) {
                printf("solver.abstol = %lg\n", solver_abstol);
            }
            yamlKeyPathRelease(keyPath);
        }
        
        keyPath = yamlKeyPathCreateWithString("Species-Output.Species", 0, yamlKeyPathCompileOptions_caseFoldKeys | yamlKeyPathCompileOptions_oneBasedIndices, NULL, NULL);
        if ( keyPath ) {
            strLen = sizeof(str);
            if ( yamlConfigFileCoerceScalarAtPath(theConfigFile, keyPath, NULL, NULL, NULL, yamlConfigFileCoerceToType_string, &str, &strLen) ) {
                printf("species-output.species = %s\n", str);
            }
            yamlKeyPathRelease(keyPath);
        }
        
        keyPath = yamlKeyPathCreateWithString("Reactor.LTRA", 0, yamlKeyPathCompileOptions_caseFoldKeys | yamlKeyPathCompileOptions_oneBasedIndices, NULL, NULL);
        if ( keyPath ) {
            if ( yamlConfigFileCoerceScalarAtPath(theConfigFile, keyPath, NULL, NULL, NULL, yamlConfigFileCoerceToType_bool, &reactor_ltra) ) {
                printf("Reactor.LTRA = %s\n", reactor_ltra ? "on" : "off" );
            }
            yamlKeyPathRelease(keyPath);
        }
        
        keyPath = yamlKeyPathCreateWithString("solver.bandwidth-limits", 0, yamlKeyPathCompileOptions_caseFoldKeys | yamlKeyPathCompileOptions_oneBasedIndices, NULL, NULL);
        if ( keyPath ) {
            if ( yamlConfigFileCoerceSequenceAtPath(theConfigFile, keyPath, NULL, &lastError, NULL, 0, 1, yamlConfigFileCoerceToType_double, bandwidth_limits) ) {
                printf("solver.bandwidth-limits = [%lf, %lf]\n", bandwidth_limits[0], bandwidth_limits[1]);
            }
            yamlKeyPathRelease(keyPath);
        }
        
        {
            char                species[256];
            int                 ierr, unit = 1;
            
            if ( yamlconfigfile_getstring_(&unit, "species-output.species2", species, &ierr, strlen("species-output.species2"), sizeof(species)) ) {
                printf("Fortran species-output.species = %s\n", species);
            }
        }
        
        {
            int                 ierr, unit = 1, irxtr;
            
            if ( yamlconfigfile_getinteger_(&unit, "irxtr", &irxtr, &ierr, strlen("irxtr")) ) {
                printf("Fortran irxtr = %d\n", irxtr);
            }
        }
        
        yamlConfigFileRelease(theConfigFile);
    }
    
    theConfigFile = yamlConfigFileCreateWithFileAtPath(
                            "test.yaml",
                            0
                        );
    yamlConfigFileCacheEvict(theConfigFile);
    yamlConfigFileRelease(theConfigFile);
    
    theConfigFile = yamlConfigFileCreateWithFileAtPath(
                            "test.yaml",
                            yamlConfigFileOptions_doNotCache | yamlConfigFileOptions_noKeyPathCache
                        );
    if ( theConfigFile ) {
        yamlKeyPathRef          keyPath;
        unsigned int            nruns, irxtr;
        double                  tin, abyv, solver_abstol, bandwidth_limits[2];
        const char              str[256];
        size_t                  strLen;
        bool                    reactor_ltra;
        bool                    MultiInput;
        
        keyPath = yamlKeyPathCreateWithString("nruns", 0, 0, NULL, NULL);
        if ( keyPath ) {
            if ( yamlConfigFileCoerceScalarAtPath(theConfigFile, keyPath, NULL, NULL, NULL, yamlConfigFileCoerceToType_unsignedInt, &nruns) ) {
                printf("nruns = %u\n", nruns);
            }
            yamlKeyPathRelease(keyPath);
        }
        
        keyPath = yamlKeyPathCreateWithString("irxtr", 0, 0, NULL, NULL);
        if ( keyPath ) {
            if ( yamlConfigFileCoerceScalarAtPath(theConfigFile, keyPath, NULL, NULL, NULL, yamlConfigFileCoerceToType_unsignedInt, &irxtr) ) {
                printf("irxtr = %u\n", irxtr);
            }
            yamlKeyPathRelease(keyPath);
        }
        
        keyPath = yamlKeyPathCreateWithString("tin", 0, 0, NULL, NULL);
        if ( keyPath ) {
            if ( yamlConfigFileCoerceScalarAtPath(theConfigFile, keyPath, NULL, NULL, NULL, yamlConfigFileCoerceToType_double, &tin) ) {
                printf("tin = %lg\n", tin);
            }
            yamlKeyPathRelease(keyPath);
        }
        
        keyPath = yamlKeyPathCreateWithString("tin", 0, 0, NULL, NULL);
        if ( keyPath ) {
            if ( yamlConfigFileCoerceScalarAtPath(theConfigFile, keyPath, NULL, NULL, NULL, yamlConfigFileCoerceToType_double, &tin) ) {
                printf("tin = %lg\n", tin);
            }
            yamlKeyPathRelease(keyPath);
        }
        
        yamlConfigFileRelease(theConfigFile);
    }
    
    yamlConfigFileCacheFlush();
    
    return 0;
}

#endif
