include_directories(${CMAKE_SOURCE_DIR}/)
include_directories(${CMAKE_SOURCE_DIR}/grconvert)

add_executable(grconvert defaults.c grconvert.c readbin.c util.c writeasc.c)
INSTALL_TARGETS(/xmgr/bin grconvert)

