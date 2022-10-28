# SAMUEL v2.1.2

SAMUEL is a tool for extracting assets from DOOM Eternal. 

It is cross-platform, open-source, and features an easy-to-use graphical interface.

## What this tool is for:

1. Export .decl files and .entities files.
2. Export full-size images for use in texture modding (in .png format).
3. Export .lwo and .md6mesh files to the .obj file format.

Files will be extracted to the "exports" folder. Models will be extracted to the "modelExports" folder. When exporting a model, all of the model's textures and material2 .decl files will also be exported.

## SAMUEL vs. VEGA

This will obviously get compared to VEGA (the other asset extraction tool), so its probably necessary to explain the differences here:

### Differences from VEGA:

1. SAMUEL is cross-platform and open-source. It works on Windows and Linux.
2. Supports extracting of .decl files and .entities files.
3. When exporting a model, all of it's material2 .decl files are also exported.
4. Successfully extracts a few types of textures that VEGA doesn't support.
5. Supports extracting non-streamed or modded textures - texture modders can now recover their source images.
6. Has improved search functionality - can search with multiple keywords separated by spaces.
7. Image files are exported using their full name with "$" modifiers (important for custom assets).
8. Files are extracted to their full path used in the game (e.g. "gameresources_patch1/art/tile/etc" instead of just "images").

## Credits:

* SamPT ([@brongo](https://github.com/brongo)) - Primary author/developer of the SAMUEL tool. Reverse-engineered the Doom Eternal .streamdb file format, .resources format, image file formats, .lwo format, .md6mesh headers, and discovered how to export files from .streamdb.
* [@Powerball253](https://github.com/PowerBall253) - Many HUGE contributions in the form of code & testing. Responsible for Linux/MacOS compatibility, packageMapSpec parser, and figuring out Qt multithreading. Also assisted with reverse-engineering of Doom Eternal BIM (bimage) header formats.
* [DOOM 2016+ Modding Discord](https://discord.gg/ymRvQaU) for support and testing, and encouraging future developments.


## License:

This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with the SAMUEL program.  If not, see <https://www.gnu.org/licenses/>.


## Building from Source:

There is no need to build/compile this program from source code. Simply download and use the included binaries for Windows or Linux (under "Releases").

If you *want* to build/compile from source, you will need a copy of the [Qt development library](https://www.qt.io/). The SAMUEL program uses Qt for its cross-platform GUI features. Please note that usage of Qt is subject to a separate licensing agreement. SAMUEL uses Qt under the [Qt for Open-Source Development](https://www.qt.io/download-open-source). The Qt source code can be acquired here: https://www.qt.io/offline-installers.

SAMUEL for Windows is tested and compiled using a static build of Qt version 6.1.2.

## Contributing:

Contributions are welcomed. There is lots of room for code cleanup/improvement. All issues and pull requests will be considered. Please note I have limited time, so my response may not be immediate.
