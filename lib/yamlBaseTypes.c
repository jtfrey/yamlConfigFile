/*
 * yamlBaseTypes
 * Simplified YAML interface for C/Fortran
 *
 * Basic types used by all component APIs.
 *
 */

#include "yamlBaseTypes.h"

#ifndef HAVE_STRNCPY
char*
strncpy(
    char        *dest,
    const char  *src,
    size_t      n
)
{
    char        *outDest = dest;
    
    while ( *src && n-- ) *dest++ = *src++;
    if ( n ) while ( n-- ) *dest++ = '\0';
    return outDest;
}
#endif

#ifndef HAVE_STRNCASECMP
int
strncasecmp(
    const char  *s1,
    const char  *s2,
    size_t      n
)
{
    int         cmp = 0;
    
    if ( n != 0 ) {
        while ( ((cmp = ((unsigned char)tolower(*s1) - (unsigned char)tolower(*s2))) == 0) && *s1++ ) {
            if ( --n == 0 ) break;
            s2++;
        }
    }
    return cmp;
}
#endif
