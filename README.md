# GSCP

GSCP stands for Gtk SCP. It is a simple gui that transfers files via the ssh
protocol. It does not call scp, rather, it has its own code for transferring
files. The GUI is simple and doesn't feature a file manager like interface,
although it may someday.  This was done for a computer science project in very
little time so there are huge issues and is mainly here as an archive.

# Installation

## Unix/Linux

Installation is very simple on a Unix-like system. You'll need git, CMake, a c
compiler, and gtk3. Simply clone the repository via git and build normally with
CMake.  To do this, navigate into the directory.  Run `cmake .` and `make .`

I've tried building on freebsd but there seems to be an error on that OS when
compiling libssh2 where it doesn't recognize a struct.
