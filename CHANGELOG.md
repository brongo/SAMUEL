# Changelog
All notable changes to this project will be documented in this file.

## [2.0.9] - 2022-05-01

- Massive code cleanup / refactor.
- Source code published (first source code release since v1.1.0).

## [2.0.8] - 2022-04-12

- First release of SAMUEL v2.0+ for Linux
- Adds support for exporting custom models created with Doom Eternal Model Importer.
- Fixed a crash that could occur when exporting RGBA8 format images.
- Fixed a problem with exporting some images containing "$minmip=" in the filename.

## [2.0.7] - 2022-01-10

- Changed models to be exported as Y-up instead of Z-up. To match default model orientation in Substance Painter.

## [2.0.6] - 2022-01-08

- First public release of SAMUEL v2.0+ (at this time, 2.0+ is Windows only)
- Removes "Export All" button - use shift+click for similar functionality.
- Adds support for .lwo model extraction (to .obj format)
- Adds "filter" options in place of the old "Export All" button.
- Now exports textures to .png format instead of .dds

## [1.1.0] - 2021-09-04

- Adds support for .entities file export
- Fixed Linoodle runtime error (Linux version)
- Now keeps .lwo/.md6 headers and adds them to exported file (previously they were discarded).

## [1.0.0] - 2021-08-24

- Initial Release
