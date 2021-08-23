# Changelog
All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

## [0.1.1] - 2021-08-23
### Changed
- `yamlKeyPathCacheValueCStringCallbacks` callback array should be `extern const` in header, not `const`; led to compilers generating an empty constant variable in each object file that included the [yamlKeyPath.h](include/yamlKeyPath.h) header and symbol collision at link
- `yamlConfigFileError` enumeration started life as a typedef but then became just an enum — but the `yamlConfigFileError` name was left after the `enum` declaration, leading to the generation of a `yamlConfigFileError` variable in each object file that included the [yamlConfigFile.h](include/yamlConfigFile.h) header and symbol collision at link

## [0.1.0] - 2019-01-19
Initial release
