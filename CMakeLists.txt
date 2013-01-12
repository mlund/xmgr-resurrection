cmake_minimum_required(VERSION 2.6)
project("xmgr")

include(FindMotif)
include(FindX11)

add_definitions(-DGR_HOME=\"${CMAKE_INSTALL_PREFIX}/xmgr\")
add_definitions(-DPRINT_CMD=\"lpr\")

add_subdirectory(cephes)
add_subdirectory(grconvert)
add_subdirectory(src)

file(GLOB files "${CMAKE_SOURCE_DIR}/doc/*.html")
install(FILES ${files} DESTINATION xmgr/doc)

