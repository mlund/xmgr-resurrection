Xmgr 4.1.2 - Resurrection
=========================

This is a patched version of the ancient xmgr 4.1.2
plotting program that fixes a bug that would cause a
crash on newer 64-bit machines. It also contains updated
autotools for correct recognition of MacOS.

Why?
----

Because some people cannot come to terms with the updated
[Grace](http://plasma-gate.weizmann.ac.il/Grace) user
interface and would rather stick to the old xmgr (development
frozen in 1998).

Building
--------

In addition to the `configure.sh` script already supplied
with the original version of xmgr, we have added support
for [CMake](http://www.cmake.org):

    $ cmake .
    $ make

/ m.ullner & m.lund, 2012

Recommended reading (Original README)
-------------------------------------

- COPYRIGHT               - legal stuff
- CHANGES                 - chronological list of changes
- INSTALL                 - directions on installation
- doc/FAQ.html            - frequently asked questions (in HTML format)
- arch/os_name/README   - system-specific (non-unix) important notes

