include_directories(${CMAKE_SOURCE_DIR}/src)
include_directories(${CMAKE_SOURCE_DIR}/)

set (PARS_C pars)

set (GRSRCS
  main misc plotone files utils drawticks
  nonlfit lmdif as274c fit fourier
  graphu1 graphu2 setutils
  regionutils objutils computils defaults checkon
  params compute draw ps mif leaf hp chersh
  symfunc dlmodule alloca )
set_source_files_properties(GRSRCS PROPERTIES LANGUAGE C)

set (GUISRCS
  compwin comwin drawwin eblockwin editpwin 
  events featext fileswin framewin graphwin helpwin hotwin 
  image_f_io labelwin locatewin miscwin monwin 
  motifutils nonlwin pagewin printwin ptswin regionwin 
  rotated setwin statuswin strwin symwin tickwin worldwin 
  xmgr xvlib )
set_source_files_properties(GUISRCS PROPERTIES LANGUAGE C)

#  MOTIF_FOUND        - system has MOTIF
#  MOTIF_INCLUDE_DIR  - include paths to use Motif
#  MOTIF_LIBRARIES    - Link these to use Motif

#  X11_FOUND        - True if X11 is available
#  X11_INCLUDE_DIR  - include directories to use X11
#  X11_LIBRARIES    - link against these to use X11

if (X11_FOUND)
  if (MOTIF_FOUND)
    if (X11_ICE_FOUND)
      if (X11_Xmu_FOUND)
        if (X11_Xpm_FOUND)
          if (X11_Xt_FOUND)
            include_directories(${X11_INCLUDE_DIR})
            include_directories(${MOTIF_INCLUDE_DIR})
            include_directories(${X11_ICE_INCLUDE_DIR})
            include_directories(${X11_Xmu_INCLUDE_DIR})
            include_directories(${X11_Xpm_INCLUDE_DIR})
            include_directories(${X11_Xt_INCLUDE_DIR})
            set(LINKLIBS ${LINKLIBS} ${X11_LIBRARIES})
            set(LINKLIBS ${LINKLIBS} ${MOTIF_LIBRARIES})
            set(LINKLIBS ${LINKLIBS} ${X11_LIBRARIES})
            set(LINKLIBS ${LINKLIBS} ${X11_Xmu_LIB})
            set(LINKLIBS ${LINKLIBS} ${X11_Xpm_LIB})
            set(LINKLIBS ${LINKLIBS} ${X11_Xt_LIB})
            add_executable( xmgr ${PARS_C} ${GRSRCS} ${GUISRCS})
            add_definitions(-DGR_HOME=\"/usr/local/xmgr\")
            add_definitions(-DPRINT_CMD=\"lpr\")
            target_link_libraries(xmgr cephes ${LINKLIBS})
          endif()
        endif()
      endif()
    endif()
  endif()
endif()

