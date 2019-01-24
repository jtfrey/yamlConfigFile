# yamlConfigFile

The **yamlConfigFile** library is an API constructed on top of [libyaml](https://github.com/yaml/libyaml).  Its primary goals are:

- Key-path access to document nodes

- Coercion of scalar nodes to standard C and Fortran data types

- A simple Fortran API

For example, consider the following YAML document:

```yaml
general:
    tolerance: 1.54e-2
    uses-bfgs: on
    
axes:
    order: [x, y]
    range:
      x: [ -3.14159, +3.14159 ]
      y: [ -6.28318, +6.28318 ]
    
visualization:
    include-elements:
      - C
      - H
      - O
    color-scheme: grayscale
```

The negative extent on the y axis is keyed by `axes.range.y[0]`.

## Coercion

Coercion (or coversion to a data type) can be accomplished in the C API as follows:

```C

yamlConfigFileRef     myConfFile = yamlConfigFileCreateWithFileAtPath("stm-config.yaml", yamlConfigFileOptions_doNotCache);

if ( myConfFile ) {
    yamlKeyPathRef    yRangeKey = yamlKeyPathCreateWithString("axes.range.y", 0, 0, NULL, NULL);
    
    if ( yRangeKey ) {
        double        yRange[2];
        bool          okay = yamlConfigFileCoerceSequenceAtPath(
                                      myConfFile,
                                      yRangeKey,
                                      NULL, NULL, NULL,
                                      0, 1,
                                      yamlConfigFileCoerceToType_double,
                                      yRange
                                 );
        if ( okay ) {
           printf("The y-axis range is from %lg to %lg.\n", yRange[0], yRange[1]);
        }
        yamlKeyPathRelease(yRangeKey);
    }
    yamlConfigFileRelease(myConfFile);
}
```

The same code looks like this in the Fortran API:

```Fortran

Integer           :: myConfFile, yamlErrCode
Double Precision  :: yRange(2)

Call YAMLConfigFile_Open('stm-config.yaml', myConfFile)
If ( myConfFile > 0 ) then
    If ( YAMLConfigFile_GetDoublePrecisionArray(myConfFile, 'axes.range.y', 1, 2, yRange, yamlErrCode) ) then
        Write(*,*) 'The y-axis range is from ', yRange(1), ' to ', yRange(2), '.'
    EndIf
    YAMLConfigFile_Close(myConfFile);
EndIf
```
