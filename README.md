# GSCP
GSCP stands for Gtk SCP. It is a simple gui that transfers files via the ssh protocol. It does not call scp, rather, it has
its own code for tranferring files. The GUI is simple and doesn't feature a file manager like interface, although it may someday.
This was done for a computer science project in very little time so it is not well coded and doesn'v have features.

# Installation
## Unix/Linux
Installation is very simple on a Unix-like system. You'll need git, cmake, a c compiler, and gtk3. Simply clone the repository via git and build normally with CMake.
To do this, navigate into the directory. Run `cmake .` and `make .`.
