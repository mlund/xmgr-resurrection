cmake_minimum_required(VERSION 2.6)
project("xmgr")

include(FindMotif)
include(FindX11)

add_subdirectory(cephes)
add_subdirectory(grconvert)
add_subdirectory(src)
