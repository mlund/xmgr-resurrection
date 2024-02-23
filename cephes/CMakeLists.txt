set (CEPHESSRCS
  airy beta chbevl chdtr const dawsn ellie ellik 
  ellpe ellpk expn fac fdtr fresnl gamma gdtr hyp2f1 
  hyperg i0 i1 igam igami incbet incbi iv j0 j1 
  jn jv k0 k1 kn log2 mtherr ndtri pdtr polevl 
  polyn psi revers rgamma round shichi sici 
  spence stdtr struve unity yn zeta zetac 
  acosh asinh atanh ndtr isfinite cbrt )
add_library(cephes STATIC ${CEPHESSRCS})
target_link_libraries(cephes project_warnings)


