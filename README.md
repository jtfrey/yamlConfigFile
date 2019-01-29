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
    double            yRange[2];
    bool              okay = yamlConfigFileCoerceSequenceAtPathString(
                                      myConfFile,
                                      "axes.range.y",
                                      0,
                                      NULL, NULL, NULL, NULL,
                                      0, 1,
                                      yamlConfigFileCoerceToType_double,
                                      yRange
                                 );
    if ( okay ) {
       printf("The y-axis range is from %lg to %lg.\n", yRange[0], yRange[1]);
    }
    yamlConfigFileRelease(myConfFile);
}
```

The same code looks like this in the Fortran API:

```Fortran
Integer             :: myConfFile, yamlErrCode
Double Precision    :: yRange(2)

Call YAMLConfigFile_Open('stm-config.yaml', myConfFile)
If ( myConfFile > 0 ) then
    If ( YAMLConfigFile_GetDoublePrecisionArray(myConfFile, 'axes.range.y', 1, 2, yRange, yamlErrCode) ) then
        Write(*,*) 'The y-axis range is from ', yRange(1), ' to ', yRange(2), '.'
    EndIf
    YAMLConfigFile_Close(myConfFile);
EndIf
```

## Building the library

CMake (version 3.0 or newer) is used to configure the source code for build.  A C compiler is required.  To use a specific C compiler, set the `CC` environment variable before configuring:

```bash
$ mkdir build
$ cd build
$ CC=pgcc ccmake ..
   :
```

There are several options that control aspects of the code that is generated:

| Flag | Default value | Discussion |
| ---- | ------------- | ---------- |
| `FORTRAN_INTEGER8` | FALSE | The Fortran API assumes the INTEGER type is 8- rather than 4-bytes |
| `FORTRAN_LOGICAL8` | FALSE | The Fortran API assumes the LOGICAL type is 8- rather than 4-bytes |
| `FORTRAN_REAL8` | FALSE | The Fortran API assumes the REAL type is 8- rather than 4-bytes |
| `FORTRAN_NO_UNDERSCORING` | FALSE | Do NOT append a trailing underscore on C functions that should be callable from Fortran |
| `ENABLE_DEMO_PROGRAMS` | FALSE | Also build all demonstration programs that make use of the library |

The default Fortran API thus uses 4-byte (32-bit) INTEGER, LOGICAL, and REAL types and appends an underscore to C functions callable from Fortran (e.g. "yamlconfigfile_open_" and not "yamlconfigfile_open"). 

