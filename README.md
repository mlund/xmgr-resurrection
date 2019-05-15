Xmgr 4.1.2 - Resurrection
=========================

This is a patched version of the abandoned
[xmgr 4.1.2](http://plasma-gate.weizmann.ac.il/Xmgr)
plotting program that fixes 64-bit related bugs.
We have also changed the build system to
[CMake](http://www.cmake.org), allowing compilation on
32-bit and 64-bit unix systems such as linux and macos x.

Related information:

- <http://randombio.com/linuxsetup131.html>
- <http://disbauxes.upc.es/?p=3699>

Building and installation
-------------------------

    cmake . -DCMAKE_INSTALL_PREFIX=/usr/local
    make
    make install

Typically the following packages are required:

    libice-dev libx11-dev libmotif-dev libxmu-dev libxpm-dev

(tested with Ubuntu 16.04)

Enable NetCDF support (on by default):

    cmake . -DENABLE_NETCDF=on

Why?
----

Because some cannot come to terms with the updated
[Grace](http://plasma-gate.weizmann.ac.il/Grace) user
interface and would rather stick to xmgr (development
frozen in 1998).

/ M. Ullner and M. Lund, 2012-2016

Copyright notice
----------------

~~~~
Copyright (c) 1991-95 Paul J Turner, Portland, OR
Copyright (c) 1996-98 ACE/gr Development Team

Currently maintained by Evgeny Stambulchik, Rehovot, Israel

                             All Rights Reserved

Permission  to  use, copy, modify, and  distribute  this software  and  its
documentation  for any purpose and  without fee is hereby granted, provided
that  the above copyright notice  appear in  all copies and  that both that
copyright  notice  and   this  permission  notice   appear  in   supporting
documentation.

PAUL J TURNER AND OTHER CONTRIBUTORS DISCLAIM ALL WARRANTIES WITH REGARD TO
THIS SOFTWARE, INCLUDING,  BUT  NOT LIMITED  TO, ALL  IMPLIED WARRANTIES OF
MERCHANTABILITY  AND  FITNESS. IN NO EVENT SHALL PAUL J TURNER  OR  CURRENT
MAINTAINER  BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER
IN AN ACTION OF CONTRACT, NEGLIGENCE OR  OTHER TORTUOUS ACTION, ARISING OUT
OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
~~~~

