# pngEdit
png Editor was initially a quick home made tool to work on small 16*16 icons defined in png files.<br>
It growed becoming a tool to check eventual steganography embedded in .png files.<br>
Copyright drehuwann@gmail.com<br>
Published under the terms of the General Public License.<br>
(See https://gnu.org/licenses/gpl.html)<br>
# dependencies
On linux : [wxWidgets](https://github.com/wxWidgets/wxWidgets)<br>
# how to build
You should manually add by yourself a file named <code>tstString.txt</code>, containing the following line :<br>
<br><code>const char \*tstStr = "**\.\***"; </code><br>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&ensp;
<code>**.\***</code> &nbsp;=>&nbsp; insert there whatever data you want to feed ztools autotests<br>
<br>then, to build with cmake :<br><br><code>cd build; cmake ..</code><br><br>
Alternatively, <code>.vscode</code> directory contains config files to build in vscode editor.<br>
Actually, it compiles under ^Ms windows or linux. 

# ToDo
Parsing of many chunk types (as <code>ChunkError Read....(void *, Chunk *)</code> functions) still need to be implemented.<br>
We don't use libpng, so the reference document should be [png specifications](https://www.w3.org/TR/png-3)<br>
