include_directories(${X11_INCLUDE_DIR})
include_directories(${MOTIF_INCLUDE_DIR})
include_directories(${X11_ICE_INCLUDE_DIR})
include_directories(${X11_Xmu_INCLUDE_DIR})
include_directories(${X11_Xpm_INCLUDE_DIR})
include_directories(${X11_Xt_INCLUDE_DIR})

find_path(NETCDF_INCLUDE_DIR netcdf.h
          PATHS ${NETCDF_INCLUDE} /ops/tools/include /usr/local/include )
find_library(NETCDF_LIBRARIES NAMES netcdf libnetcdf
             PATHS ${NETCDF_LIB} /ops/tools/lib /usr/local/lib )
include_directories(${NETCDF_INCLUDE_DIR})

set(LINKLIBS ${LINKLIBS} ${MOTIF_LIBRARIES})
set(LINKLIBS ${LINKLIBS} ${X11_Xpm_LIB})
set(LINKLIBS ${LINKLIBS} ${X11_Xmu_LIB})
set(LINKLIBS ${LINKLIBS} ${X11_Xt_LIB})
set(LINKLIBS ${LINKLIBS} ${X11_LIBRARIES})
set(LINKLIBS ${LINKLIBS} ${NETCDF_LIBRARIES})

file(GLOB OBJS "${CMAKE_CURRENT_SOURCE_DIR}/*.c")
add_executable(xmgr ${OBJS})
if (UNIX)
  target_link_libraries(xmgr m)
endif()
target_link_libraries(xmgr ${LINKLIBS} cephes)
install_targets(/xmgr/bin xmgr)
