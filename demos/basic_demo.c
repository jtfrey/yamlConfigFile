
#include "yamlConfigFile.h"

const char                  *yamlDocument = 
                                "axes:\n"
                                "    range:\n"
                                "        x: [ -6.28319, +6.28319]\n"
                                "        y: [ -3.14159, +3.14159]\n"
                                "\n";

int
main(
    int                     argc,
    const char*             argv[]
)
{
    yamlConfigFileRef       myConfFile = yamlConfigFileCreateWithInputString(
                                                yamlDocument, yamlCStringFullLength,
                                                yamlOptions_none
                                            );
    int                     rc = 0;
    
    printf("The compiled-in YAML document looks like this:\n---- SNIP ----\n%s----SNIP----\n", yamlDocument);
    
    if ( myConfFile ) {
        double                      range[2];
        yamlErrorCode               errorCode = 0;
        const char                  *errorAtChar = NULL;
        yamlKeyPathNodeMatchType    *errorAtMatchElem = NULL;
        bool                        okay;
        
        printf("The YAML document parsed properly.\n");
        
        okay = yamlConfigFileCoerceSequenceAtPathString(
                          myConfFile,
                          "axes.range.y", yamlCStringFullLength,
                          NULL,
                          &errorCode, &errorAtChar, &errorAtMatchElem,
                          0, 1,
                          yamlConfigFileCoerceToType_double,
                          range
                     );
        if ( okay ) {
            printf("The y-axis range is from %lg to %lg\n", range[0], range[1]);
            
            okay = yamlConfigFileCoerceSequenceAtPathString(
                              myConfFile,
                              "axes.range.x", yamlCStringFullLength,
                              NULL,
                              &errorCode, &errorAtChar, &errorAtMatchElem,
                              0, 1,
                              yamlConfigFileCoerceToType_double,
                              range
                         );
            if ( okay ) {
               printf("The x-axis range is from %lg to %lg\n", range[0], range[1]);
            } else if ( errorAtChar ) {
                fprintf(stderr, "ERROR:  unable to compile key path (%d: %s)\n", errorCode, errorAtChar);
            } else if ( errorAtMatchElem ) {
                fprintf(stderr, "ERROR:  unable to find key path \"%s\" (%d at ", "axes.range.y", errorCode);
                YAMLKEYPATH_NODEMATCH_FPRINTF(stderr, errorAtMatchElem);
                fprintf(stderr, ")\n");
            } else {
                fprintf(stderr, "ERROR:  generic error (%p:%d)\n", myConfFile,errorCode);
            }
           
        } else if ( errorAtChar ) {
            fprintf(stderr, "ERROR:  unable to compile key path (%d: %s)\n", errorCode, errorAtChar);
        } else if ( errorAtMatchElem ) {
            fprintf(stderr, "ERROR:  unable to find key path \"%s\" (%d at ", "axes.range.y", errorCode);
            YAMLKEYPATH_NODEMATCH_FPRINTF(stderr, errorAtMatchElem);
            fprintf(stderr, ")\n");
        } else {
            fprintf(stderr, "ERROR:  generic error (%p:%d)\n", myConfFile,errorCode);
        }
        
        rc = errorCode;
        yamlConfigFileRelease(myConfFile);
    } else {
        fprintf(stderr, "ERROR:  unable to parse yamlDocument\n");
        rc = EINVAL;
    }
    return rc;
}
