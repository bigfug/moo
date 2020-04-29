# moo
A telnet Multi-user Object Orientated (MOO) server loosely based on LambdaMOO using Lua as its core language

![C/C++ CI](https://github.com/bigfug/moo/workflows/C/C++%20CI/badge.svg)

"MOOs are network accessible, multi-user, programmable, interactive systems well-suited to the construction of text-based adventure games, conferencing systems, and other collaborative software. Their most common use, however, is as multi-participant, low-bandwidth virtual realities. They have been used in academic environments for distance education, collaboration (such as Diversity University), group decision systems, and teaching object-oriented concepts; but others are primarily social in nature, or used for role-playing video games, or simply to take advantage of the programming possibilities. They have also been used in scientific studies of virtual presence." - Wikipedia - https://en.wikipedia.org/wiki/MOO

Build with Qt5 and Lua 5.3 

Run the Server and connect using telnet on port 1123

The system roughly follows the LambdaMOO programming manual but uses internal Lua functionality whenever possible.

Refer to http://www.hayseed.net/MOO/manuals/ProgrammersManual.html and http://www.lua.org/manual/5.3/

For example, where the MOO manual talks about processing strings, we use Lua functions instead:

http://www.lua.org/manual/5.3/manual.html#6.4

Using Qt5 means it compiles on Windows/OSX/Linux/Raspberry Pi/etc

Lua was chosen for its speed and ease of embedding in a sandbox.

Please note that the server contains a complete implementation of the LambdaMOO security system, and Lua is sandboxed as far I know, but this software is not guaranteed to be secure and may somehow allow a remote user access to your filesystem.

There is no 'core' supplied, so apart from the functions provided by Lua and the LambdaMOO system, you'll have to create your worlds from scratch!

