# pngEdit
png Editor was initially a quick home made tool to work on small 16*16 icons defined in png files.<br>
It growed becoming a tool to check eventual steganography embedded in .png files.<br>
Copyright drehuwann@gmail.com<br>
Published under the terms of the General Public License.<br>
(See https://gnu.org/licenses/gpl.html)<br>
# dependencies
On linux : [wxWidgets](https://github.com/wxWidgets/wxWidgets)<br>

# how to build
With cmake :<br><br><code>cd build; cmake ..</code><br><br>
Alternatively, <code>.vscode</code> directory contains config files to build in vscode editor.<br>
Actually, it compiles under ^Ms windows or linux.<br>

# build with zlib integration 
after refreshing the git submodule, you should set <code>option(ZLIB_BUILD_EXAMPLES "Enable Zlib Examples" OFF)</code>
in zlib CMakelists.txt<br>

# ToDo
Parsing of many chunk types (as <code>ChunkError Read....(void *, Chunk *)</code> functions) still need to be implemented.<br>
We don't use libpng, so the reference document should be [png specifications](https://www.w3.org/TR/png-3)<br>
<br>
