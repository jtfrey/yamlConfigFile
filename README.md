# yamlConfigFile

The **yamlConfigFile** library is an API constructed on top of [libyaml](https://github.com/yaml/libyaml).  Its primary goals are:

- Key-path access to document nodes, e.g. `key.list[2][0].key`

- Coercion of scalar nodes to standard C and Fortran data types

- A simple Fortran API
