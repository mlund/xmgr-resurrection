include_directories(${CMAKE_SOURCE_DIR}/)
include_directories(${CMAKE_SOURCE_DIR}/src)

set (PARS_C pars)

set (GRSRCS
  main misc plotone files utils drawticks
  nonlfit lmdif as274c fit fourier
  graphu1 graphu2 setutils
  regionutils objutils computils defaults checkon
  params compute draw ps mif leaf hp chersh
  symfunc dlmodule alloca )

set (GUISRCS
  compwin comwin drawwin eblockwin editpwin 
  events featext fileswin framewin graphwin helpwin hotwin 
  image_f_io labelwin locatewin miscwin monwin 
  motifutils nonlwin pagewin printwin ptswin regionwin 
  rotated setwin statuswin strwin symwin tickwin worldwin 
  xmgr xvlib )

include_directories(${X11_INCLUDE_DIR})
include_directories(${MOTIF_INCLUDE_DIR})
include_directories(${X11_ICE_INCLUDE_DIR})
include_directories(${X11_Xmu_INCLUDE_DIR})
include_directories(${X11_Xpm_INCLUDE_DIR})
include_directories(${X11_Xt_INCLUDE_DIR})

set(LINKLIBS ${LINKLIBS} ${MOTIF_LIBRARIES})
set(LINKLIBS ${LINKLIBS} ${X11_Xpm_LIB})
set(LINKLIBS ${LINKLIBS} ${X11_Xmu_LIB})
set(LINKLIBS ${LINKLIBS} ${X11_Xt_LIB})
set(LINKLIBS ${LINKLIBS} ${X11_LIBRARIES})

add_executable(xmgr ${PARS_C} ${GRSRCS} ${GUISRCS})
if (UNIX)
  target_link_libraries(xmgr m)
endif()
target_link_libraries(xmgr ${LINKLIBS} cephes)
install_targets(/xmgr/bin xmgr)
